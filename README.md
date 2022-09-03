# Raspberry Pi GPIO through MQTT
- this code runs on Raspberry pi and permits to control GPIO at a distance
- the code can also act as a MQTT remapper in C++
![Some concepts](doc/gpio2mqtt.png)

# Set GPIO mode and output 
- MQTT publish   topic : "dst/GPIO/gpio25/mode" message :  "OUTPUT" => sets gpio25 to mode OUTPUT
- MQTT publish   topic : "dst/GPIO/gpio25/value" message :  "1" => sets gpio25 to HIGH

I've only implemented GPIO output, as this is the only thing I need at this time.
I2C and SPI should be easy added

The code should run as root to have access to GPIO wiringPi. 
# run as a service on Raspberry Pi
```
cp gpio2mqtt.service /etc/systemd/system
systemctl daemon-reload
journalctl | tail -f   # check logs
systemctl status gpio2mqtt  # chck status
systemctl enable gpio2mqtt  # run at reboot
```
# Build instruction
```
git clone https://github.com/vortex314/gpio2mqtt
git clone https://github.com/vortex314/limero
cd limero
mkdir build
cd build
cmake ..
cmake --build .
cd ../../gpio2mqtt
mkdir build
cd build
cmake ..
cmake --build .
```

# Connect joystick to GPIO 
```
  mqtt.fromTopic<int>("src/pcdell/js0/axis0") >>  mqtt.toTopic<int>("dst/GPIO/gpio25/value");
```
any value different from zero will set gpio25 value to high. 
See my joystick2mqtt project.
