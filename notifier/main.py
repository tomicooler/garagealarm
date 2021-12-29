#!/usr/bin/env python3

from decouple import config
import time
import requests
import json
import serial

SECRET_KEY = config("MY_GARAGE_SERVER_KEY")
DEVICE_TOKEN = config("MY_GARAGE_DEVICE_TOKEN")

SERIAL_INTERFACE = config("MY_GARAGE_SERIAL_INTERFACE", default="/dev/ttyACM0")


def send_notification(action: str):
    timestamp = int(time.time() * 1000)

    headers = {
        "Content-Type": "application/json",
        "Authorization": "key={}".format(SECRET_KEY),
    }

    body = {
        "to": DEVICE_TOKEN,
        "priority": "high",
        "data": {
            "timestamp": timestamp,
            "action": action,
        },
    }

    print("Sending message action='{}' timestamp='{}'".format(action, timestamp))

    response = requests.post(
        "https://fcm.googleapis.com/fcm/send", headers=headers, data=json.dumps(body)
    )

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
