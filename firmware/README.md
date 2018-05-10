# Firmware Binary
If you do not wish to compile this firmware by your own, you can use the
precompiled firmware in this directory.

---

## USB_UPLOAD script
A sample script to upload the firmware via USB connection.

**Change the USB device in the script
to your device path.**

### esptool
The [esptool](https://github.com/espressif/esptool) is needed to upload the firmware over USB.
I used the `esptool` comes with [PlatformIO](https://platformio.org/).

### Warning
Some esp8266 have problems with OTA upload after an USB upload. Press the reset button or do a power cycle after an USB upload if you like to do OTA upload.

---

## OTA_UPLOAD script
A sample script to upload the firmware **o**ver **t**he **a**ir via WiFi connection.

**Change hostname and auth password to your PowerMeter settings**

### espota.py
The [espota.py](https://github.com/esp8266/Arduino/blob/master/tools/espota.py) Python script is needed to upload the firmware over the air.
Again i used the `espota.py` comes with [PlatformIO](https://platformio.org/).

## References
- [PlatformIO](https://platformio.org/)
- [esptool](https://github.com/espressif/esptool)
- [espota.py](https://github.com/esp8266/Arduino/blob/master/tools/espota.py)
