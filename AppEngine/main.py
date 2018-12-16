# Copyright 2018 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import datetime

from flask import Flask, render_template, request

# [START gae_python37_datastore_store_and_fetch_times]
from google.cloud import datastore

datastore_client = datastore.Client()

# [END gae_python37_datastore_store_and_fetch_times]
app = Flask(__name__)


# [START gae_python37_datastore_store_and_fetch_times]

def store_where(type, dt, where):
	query = datastore_client.query(kind=type)
	query.order = ['-timestamp']

	times = query.fetch(limit=1)
	
	for time in times:
		datastore_client.delete(time.key)

	entity = datastore.Entity(key=datastore_client.key(type))
	entity.update({
		'timestamp': dt,
		'where': where
	})
	datastore_client.put(entity)

def fetch_where(type):
	query = datastore_client.query(kind=type)
	query.order = ['-timestamp']

	times = query.fetch(limit=1)
	
	for time in times:
		return '%s|%s|%s' % (type, time['where'], time['timestamp'])
	return '%s|%s|%s' % (type, 'Unknown', datetime.datetime.now())
# [END gae_python37_datastore_store_and_fetch_times]

# [START gae_python37_datastore_render_times]
@app.route('/')
def rootindex():
    where = fetch_where(request.args.get('w', 'Daddy'))
    
    strings = where.split('|')
    
    return render_template(
        'index.html',
        name=strings[0], where=strings[1])
# [END gae_python37_datastore_render_times]

# [START gae_python37_datastore_render_times]
@app.route('/set')
def rootset():
    # Store the current access time in Datastore.
    store_where(request.args.get('w'), datetime.datetime.now(), request.args.get('v'))

    return 'ok'
# [END gae_python37_datastore_render_times]

# [START gae_python37_datastore_render_times]
@app.route('/get')
def rootget():
    where = fetch_where(request.args.get('w'))

    return where
# [END gae_python37_datastore_render_times]


if __name__ == '__main__':
    # This is used when running locally only. When deploying to Google App
    # Engine, a webserver process such as Gunicorn will serve the app. This
    # can be configured by adding an `entrypoint` to app.yaml.

    # Flask's development server will automatically serve static files in
    # the "static" directory. See:
    # http://flask.pocoo.org/docs/1.0/quickstart/#static-files. Once deployed,
    # App Engine itself will serve those files as configured in app.yaml.
    app.run(host='127.0.0.1', port=8080, debug=True)
