GarageAlarm
===========

GarageAlarm can detect whether the garage door is open or closed.

Hardware requirements
---------------------

 - 2x Raspberry Pico
 - 2x RFM95W LoRa module
 - 1x Infra Sensor


doorwatcher
-----------

TODO


watchdog
--------

TODO


Building
--------

Developing workflow with a Raspberry Pi 3B

```
ssh pi@RASPBERRY_IP
mkdir -p /home/pi/pico/garagealarm
sshfs youruser@HOST_IP:/home/tomi/qt_workspace/pico/garagealarm /home/pi/pico/garagealarm
cd ~/pico/garagealarm

mkdir build
cd build/
cmake -DCMAKE_BUILD_TYPE=Debug ..
make doorwatcher
openocd -f interface/raspberrypi-swd.cfg -f target/rp2040.cfg -c "program doorwatcher/doorwatcher.elf verify reset exit"
minicom -b 115200 -o -D /dev/serial0
```
