# MSPWIfiBridge

With MSPWIfiBride, flight controllers with MSP protocol can be easily configured wirelessly via Wifi.

- MSP error checking
- Easy to use web configuration
- Station and access point mode
- TCP and UDP support
- OTA updates 

## Supported Hardware

- NodeMCU V2/3
- ESP-01(M)
- ESP 12E/f
- ESP32 (Not tested!)

## Wiring
Connect the UART of the ESP module to a UART of the flight controller. Attention: The small ESP8266 modules only tolerate 3.3 V supply voltage.
Set the UART of the flight controller to MSP, with the configured baud rate, default is 115200 baud.

## Configuration:
After flashing, connect to the network "MSPWifiBridge".
Wifi key: 123456789
If there is no automatic forwarding to the configuration page, go to http://192.168.4.1 manually.
Standard login data:
Username: admin
Password: 123456789

## Flashing
Use Visual Studio Code and PlatformIO to compile and flash the code yourself or use the precompiled binary files under releases and use "NodeMCU Flasher" https://github.com/nodemcu/nodemcu-flasher/
For the NodeMCU modules you can simply use the integrated USB port, for the small modules without USB you need a USB/serial converter (FTDI232 or similar). 
GPIO0 must be pulled to GND during power up to enter bootloader mode.
