# WheresDaddy
My family sometimes want to know whether I left work yet. So I set up geofencing for my phone around my house and work using IFTTT, which updates a web service I wrote with AppEngine and Python. When I leave or arrive at work or home, the service gets a message. The Arduino makes periodic requests to the service, parses it, and turns on the assigned LED. And it works! Yay!
