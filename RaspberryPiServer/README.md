# Raspberry Pi Network and Data Server for GU SkiMon Project #

These are notes and details about the networking server and data storage system for the Gonzaga University SkiMon project.

Hardware:
- Raspberry Pi Model 3 B+
- Adafruit Ultimate GPS Hat
- Adafruit GPS external antenna
- Anker USB Battery (10k mAh)
- Custom case for protection and carrying in the backpack


Software:
- gpsd
- Python3
- Python3 gps3
- Influxdb
- Mosquitto
  - Python3 paho-mqtt: https://pypi.org/project/paho-mqtt/
  - Main data channel is called: SkiMon

Sources of Notes:
- https://thepi.io/how-to-use-your-raspberry-pi-as-a-wireless-access-point/
- https://www.raspberrypi.org/documentation/configuration/wireless/access-point-routed.md 
- https://learn.adafruit.com/diy-esp8266-home-security-with-lua-and-mqtt/configuring-mqtt-on-the-raspberry-pi
  - sudo apt-get install mosquitto mosquitto-clients
  - sudo python3 -m pip install paho-mqtt
- https://pimylifeup.com/raspberry-pi-influxdb/ 


(Didn't use this in the end)
- https://how2electronics.com/esp8266-to-esp8266-communication-ad-hoc-networking/
  - Decided to go with a RPi for the net instead of a point to point/adhoc approach


Raspberry Pi 3 UART issues
- Mini UART and CPU core frequency section of: https://www.raspberrypi.org/documentation/configuration/uart.md
- You really do need to set the core_freq=250 option because someone botched serial console stuff on the Pi 3 (sigh).
- This stuff's not easy, but damn this is a core part of being able to set up the Pi. It's not like the BBB or OrangePi where the USB port also acts like a serial console so it's really easy to drop right onto ttyS0. /rant

Setting up the wireless as a router
- Plugged into a USB wifi/802.11bn card
- Enabled "predictable" network names in raspi-config
- Set the names explicitly using these discussions: https://www.raspberrypi.org/forums/viewtopic.php?t=198687
- This Howto was invaluable!
  - https://thepi.io/how-to-use-your-raspberry-pi-as-a-wireless-access-point/
- This is starting to get crazy. All kinds of little wacky configs beyond the howto guide like:
  - https://raspberrypi.stackexchange.com/questions/95916/why-is-hostapd-masked-after-installation
  - https://raspberrypi.stackexchange.com/questions/74538/how-do-i-configure-dhcpcd-to-call-wpa-supplicant-for-a-specific-interface





@copyright 2021

