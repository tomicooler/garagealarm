#!/usr/bin/env bash
export PATH=/home/pi/.local/bin:$PATH
eval "$(pdm --pep582)"
./main.py
