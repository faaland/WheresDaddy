/* Copyright (c) 2018 Nikolai Faaland

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#include <SPI.h>
#include <WiFiNINA.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

char ssid[] = "MosEisleyCantina";  // your network SSID (name)
char pass[] = "mosN0|Huxley";      // your network password

const int checkLen = 11;
const char homeCheck[] = "Daddy|Home|";
const char workCheck[] = "Daddy|Work|";

// Initialize the Wifi client library
WiFiClient client;
WiFiUDP ntpUDP;
NTPClient timeClient(
  ntpUDP, 
  "us.pool.ntp.org",  // US time server
  -8 * 3600,          // PST, -8 hours.  Doesn't handle DST, whatever.
  86400000);          // Update every day

// server address:
char server[] = "excellent-ion-224917.appspot.com";

bool isHome = false;
bool isWork = false;
int lastMinute = -1;

void setup() {
  // put your setup code here, to run once:
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(2, INPUT);
  Serial.begin(115200);

  timeClient.begin();

  httpRequest();
}

void loop() 
{
  timeClient.update();

  bool sendRequest = false;
  int hour = timeClient.getHours();
  if (hour >= 6 && hour <= 20) // only check between 6am and 9pm
  {
    int minute = timeClient.getMinutes();
    sendRequest = minute != lastMinute;
    lastMinute = minute;
  }

  if (sendRequest) {
    sendRequest = false;
    
    LightPin(3);
    delay(250);
    LightPin(4);
    delay(250);
    LightPin(5);
    delay(250);
    LightPin(0);
    delay(250);
    
    httpRequest();
  }

  if (digitalRead(2) == LOW)
  {
    digitalWrite(3, isHome ? HIGH : LOW);
    digitalWrite(4, (!isHome && !isWork) ? HIGH : LOW);
    digitalWrite(5, isWork ? HIGH : LOW);
  }
  else
  {
    sendRequest = true;
  }
}

// this method makes a HTTP connection to the server:
void httpRequest() {

  if (WiFi.status() != WL_CONNECTED)
  {
    connectToAP();
  }

  if (client.connect(server, 80)) {
    Serial.print("Contact server at ");
    Serial.println(timeClient.getFormattedTime());
    // Make a HTTP request:
    client.println("GET /get?w=Daddy HTTP/1.1");
    client.println("Host: excellent-ion-224917.appspot.com");
    client.println("Connection: close");
    client.println();
  }
  else {
    Serial.println("connection failed");
  }
  
  unsigned long startTime = millis();
  bool received = false;

  while ((millis() - startTime < 5000) && !received) { //try to listen for 5 seconds
    bool lastReturn = false;
    bool lastNewline = false;
    bool readHeader = false;
    int checkIdx = 0;
    isHome = true;
    isWork = true;
    while (client.available()) {
      received = true;
      char c = client.read();
      if (!readHeader)
      {
        if (!lastReturn)
        {
          if (c == '\r')
          {
            lastReturn = true;
          }
          else
          {
            lastReturn = false;
            lastNewline = false;
          }
        }
        else
        {
          lastReturn = false;
          if (c == '\n')
          {
            if (lastNewline) readHeader = true;
            lastNewline = true;
          }
          else
          {
            lastNewline = false;
          }
        }
      }
      else
      {
        if (checkIdx < checkLen)
        {
          if (homeCheck[checkIdx] != c)
          {
            isHome = false;
          }
          if (workCheck[checkIdx] != c)
          {
            isWork = false;
          }
        }
        checkIdx++;
      }
    }
  }

  if (isHome)
  {
    Serial.print("home");
  }
  
  if (isWork)
  {
    Serial.print("work");
  }
  
  client.stop();
  Serial.println();
}

void connectToAP() {
  int status = WL_IDLE_STATUS;
  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    for(int i = 0; i < 10; i++)
    {
      LightPin(5);
      delay(100);
      LightPin(4);
      delay(100);
      LightPin(3);
      delay(100);
      LightPin(0);
    }
  }
  Serial.println("Connected to wifi");
}

void LightPin(int pin)
{
  digitalWrite(3, pin == 3 ? HIGH : LOW);
  digitalWrite(4, pin == 4 ? HIGH : LOW);
  digitalWrite(5, pin == 5 ? HIGH : LOW);
} 
