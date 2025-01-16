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
__pypackages__/3.13/bin/black main.py

# running
./main.py
```

Configuring using environment variables or an env file.
```
MY_GARAGE_SERVICE_ACCOUNT_FILE="The location of the generated private key json file for the Firebase service: https://firebase.google.com/docs/cloud-messaging/migrate-v1#python"
MY_GARAGE_FCM_PROJECT_ID="The project ID from the service account file"
MY_GARAGE_DEVICE_TOKEN="The device token id that your device received from the MyGarage android app"
```

Deploying on a Raspberry Pi
```
sudo cp garagealarm.service /lib/systemd/system/.
sudo systemctl start garagealarm.service
sudo systemctl status garagealarm.service
journalctl -u garagealarm.service
sudo systemctl enable garagealarm.service
```
