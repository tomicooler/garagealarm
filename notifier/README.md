garagenotifier
--------------

A python script to read the watchdog's messages from the serial output.
When somem actions are logged (e.g.: door is opened or closed) a push
notification is sent to the configured device using Firebase
Cloud Messaging.


Requirements:
 - <a href="https://pdm.fming.dev/">python-pdm</a>


Building
```
pdm update
eval "$(pdm --pep582)"

# auto format code
__pypackages__/3.10/bin/black main.py

# running
./main.py
```

Configuring using environment variables or an env file.
```
MY_GARAGE_SERVER_KEY="fcm server key that you download from your Firebase console"
MY_GARAGE_DEVICE_TOKEN="device token id that your device received from the MyGarage android app"
```
