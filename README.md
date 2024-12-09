# wemos-d1-mini-dht11-bmp280-ds18b20
Wemos D1 Mini Multi Sensor device for SensorLogger

### Features
* Sensors
  * BMP280 (temperature, air pressure)
  * DHT11 (temperature, humidity)
  * DS18B20 (temperature)
* WiFi connection
* Register multi sensor device on the fly and store device ids to eeprom
* ESP deep sleep

### Dependencies
* Board: LOLIN(WEMOS) D1 mini (ESP8266 3.1.2)
  * Wire
  * ESP8266WiFi
  * WiFiUdp
  * WiFiClient
  * ESP8266HTTPClient
  * EEPROM
* OneWire 2.3.8
* DHT sensor library 1.4.6 by Adafruit
* DallasTemperature 3.9.0 by Miles Burton
* Adafruit_BMP280 Adafruit BMP2800 Library 2.6.8 by Adafruit
* ArduinoJson 5.13.5 by Benoit Blanchon
* rBase64 https://github.com/tschaban/rBase64
* NTPClient 3.2.1 by Fabrice Weinberg

