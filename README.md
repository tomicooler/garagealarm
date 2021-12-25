Developing workflow
```
mkdir build
cd build/
cmake -DCMAKE_BUILD_TYPE=Debug ..
make doorwatcher
openocd -f interface/raspberrypi-swd.cfg -f target/rp2040.cfg -c "program doorwatcher/doorwatcher.elf verify reset exit"
minicom -b 115200 -o -D /dev/serial0
```

sshfs
```
HOST_URL=192.168.0.11 sshfs tomi@$HOST_URL:/home/tomi/qt_workspace/pico/garagealarm /home/pi/pico/garagealarm
```
