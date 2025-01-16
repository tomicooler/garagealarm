#!/usr/bin/env python3

from decouple import config
import time
import requests
import json
import serial
import google.auth.transport.requests

from google.oauth2 import service_account

SERVICE_ACCOUNT_FILE = config("MY_GARAGE_SERVICE_ACCOUNT_FILE")
PROJECT_ID = config("MY_GARAGE_FCM_PROJECT_ID")
DEVICE_TOKEN = config("MY_GARAGE_DEVICE_TOKEN")
SERIAL_INTERFACE = config("MY_GARAGE_SERIAL_INTERFACE", default="/dev/ttyACM0")

BASE_URL = "https://fcm.googleapis.com"
FCM_ENDPOINT = "v1/projects/" + PROJECT_ID + "/messages:send"
FCM_URL = BASE_URL + "/" + FCM_ENDPOINT
SCOPES = ["https://www.googleapis.com/auth/firebase.messaging"]


def _get_access_token():
    credentials = service_account.Credentials.from_service_account_file(
        SERVICE_ACCOUNT_FILE, scopes=SCOPES
    )
    request = google.auth.transport.requests.Request()
    credentials.refresh(request)
    return credentials.token


def send_notification(action: str):
    timestamp = int(time.time() * 1000)

    headers = {
        "Content-Type": "application/json; UTF-8",
        "Authorization": "Bearer " + _get_access_token(),
    }

    body = {
        "message": {
            "token": DEVICE_TOKEN,
            "android": {"priority": "high"},
            "data": {"timestamp": str(timestamp), "action": action},
        }
    }

    print("Sending message action='{}' timestamp='{}'".format(action, timestamp))

    response = requests.post(FCM_URL, headers=headers, data=json.dumps(body))

    print(
        "Response status_code='{}' json='{}'".format(
            response.status_code, response.json()
        )
    )


if __name__ == "__main__":
    print("GarageNotifier is started")

    ser = serial.Serial(SERIAL_INTERFACE)
    ser.baudrate = 115200
    while True:
        line = ser.readline()
        if line.find(b"#ACTION_DOOR_OPEN#") != -1:
            send_notification("open")
        elif line.find(b"#ACTION_DOOR_CLOSE#") != -1:
            send_notification("close")
        elif line.find(b"#IV_TOP_MISMATCH#") != -1:
            send_notification("iv_top_mismatch")
        elif line.find(b"#IV_BOTTOM_OLD#") != -1:
            send_notification("iv_bottom_old")
