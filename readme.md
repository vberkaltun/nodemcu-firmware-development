### Compile Settings

This article includes requisite info about Arduino IDE compile settings. 
Before begin, please make sure that the IDE have the following installed. 
After that, do not forget configure the compile settings as same as given settings at below from the GUI of Arduino IDE:

+ Arduino IDE and ESP8266 board support as described under [Installing with Boards Manager](https://github.com/esp8266/Arduino#installing-with-boards-manager)  
+ [Python 2.7](https://www.python.org/) (do not install Python 3.5 that is not supported)

The given settings are related with first compile of a NodeMCU device. 
When you make a preliminary first compile, You can make next and next copile with Arduino OTA option without com port.
For this feature, You need to import Arduino OTA library to top of your sketch file and Your device have to be connected on internet with this build of sketch file.
Otherwise, OTA library cannot start internet communication with your device.

#### First Compile with COM Port

**`Board:`** | NodeMCU 1.0 (ESP-12E Module)
:--- | :---  
**`Flash Size:`** | 4M (1M SPIFFS)
**`Debug port:`** | Disabled
**`Debug Level:`** | SSL+TLS_MEM+HTTP_CLIENT+HTTP_SERVER+CORE+WIFI+HTTP
**`IwIP Variant:`** | v2 Lower Memory
**`CPU Frequency:`** | 80 MHz
**`Upload Speed:`** | 921600
**`Erase Flash:`** | Sketch + WiFi Settings
**`Port:`** | COMX

#### Next Compile with OTA

**`Board:`** | NodeMCU 1.0 (ESP-12E Module)
:--- | :---  
**`Flash Size:`** | 4M (1M SPIFFS)
**`Debug port:`** | Disabled
**`Debug Level:`** | SSL+TLS_MEM+HTTP_CLIENT+HTTP_SERVER+CORE+WIFI+HTTP
**`IwIP Variant:`** | v2 Lower Memory
**`CPU Frequency:`** | 80 MHz
**`Upload Speed:`** | 921600
**`Erase Flash:`** | Sketch + WiFi Settings
**`Port:`** | XXXXX at 255.255.255

#### Contribute

For more info, please follow [ESP8266 Arduino Core Documentation](http://esp8266.github.io/Arduino/versions/2.0.0/doc/ota_updates/ota_updates.html).
