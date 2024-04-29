# ESP32 tank robot

This is a simple tank robot based on ESP32 and L298N motor driver. The robot is controlled by UDP packets sent over network. The robot can be controlled by any device that can send UDP packets. The robot accepts a simple protocol that allows to control the robot movement and speed similar to G code. I.e. `L255 R255` will make the robot move forward with maximum speed. `L-255 R-255` will make the robot move backwards with maximum speed. `L255 R-255` will make the robot turn left. `L-255 R255` will make the robot turn right. The speed can be adjusted from -255 to 255. The robot will move with the speed of the difference between the left and right speed. The robot will stop if the speed is 0. The minimum PWM value is dependent on the motor driver and the motors used, and also the battery charge. The robot will not move if the speed is below the minimum PWM value. The robot will also stop if the robot receives a stop command.

## WIP warning

This is a work in progress. Heed with caution. This page is used as a log of the progress of the project. Please star the project if you are interested. It will motivate me to work on the project or maybe even finish it.

Todo list:

- The robot should stop if the robot does not receive any UDP packets for a certain amount of time(watchdog/heartbeat)
- Read input from IR receiver to detect hits from IR transmitter
- Use turrent IR LED to "shoot" IR signal
- Send UDP packets with the status of the robot from the ESP. Broadcast the status of the robot to the network?
- Hook up built-in LED bar to show the status of the robot
- Hook up a buzzer to make sounds
- Add a camera to the robot(ESP32-CAM?)

## Hardware

- ESP-32 Lite REV1 CH340G with USB-C and battery charging circuit built-in
- L298N Lite motor driver with no heat sink and enable pin
- 1x T-34 tank toy
- 2x original DC toy motors from the tank toy
- 1x 14500 battery with connector
- 1x 4.8V 2000mah Rechargeable Ni-CD Battery with connector

## Software

- PlatformIO
- Arduino framework
- OTA
- UDP
- Python REPL client for testing

## Plans

### Adding a camera

This will probably be the ESP32-CAM module streaming video over WiFi. I have to reconsider power draw if I decide to add in a camera. I will probably have to add a bigger or drone battery to the robot. While having video, especially multiple cameras, does sound super cool, I have to consider the added power draw of one or two more modules. It also increases complexity in general and makes wiring the turret really difficult.

### Steam Deck integration

I think the Steam Deck is perfect for this project as a controller. It has a screen and plenty of physical controls to make manouvering the robot easy. It's powerful enough to handle HTML5 API's and JavaScript, and display video(s) at the same time.
