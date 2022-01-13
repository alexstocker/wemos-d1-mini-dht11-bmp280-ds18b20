#include <Wire.h>
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_BMP280.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <rBase64.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>

#ifndef STASSID
#define STASSID "[SSID]"
#define STAPSK "[KEY]"
#endif

ESP8266WiFiMulti WiFiMulti;

const char* ssid = STASSID;
const char* password = STAPSK;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String formattedDate;
String dayStamp;
String timeStamp;

const char* url = "http://[OWNCLOUDINSTANCE]/index.php/apps/sensorlogger/api/v1/createlog/";
const char* urlRegister = "http://[OWNCLOUDINSTANCE]/index.php/apps/sensorlogger/api/v1/registerdevice/";
const char* basicAuth = "[USERNAME]:[TOKEN]";
const char* deviceUuid = "multi4fb-f0ae-4071-9de1-19d1b57624dc";

#define DHTTYPE DHT11   // DHT 11
#define ONE_WIRE_BUS 2  //D4

// DHT Sensor
uint8_t DHTPin = 14;

// BME280 I2C address is 0x76(108)
#define Addr 0x76

#define EESIZE 1024

int randomValue;

Adafruit_BMP280 bmp;

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

float t0;

uint eeAddress = 0;

int sId_0;
int sId_1;
int sId_2;
int sId_3;
int sId_4;

struct {
  uint sId_0 = 0;
  uint sId_1 = 0;
  uint sId_2 = 0;
  uint sId_3 = 0;
  uint sId_4 = 0;
} SensorIds;

void setup() {
  Serial.begin(9600);  // The baudrate of Serial monitor is set in 9600
  while (!Serial)
    ;  // Waiting for Serial Monitor

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  Serial.print("Wait for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  timeClient.begin();
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  EEPROM.begin(512);
  // cast bytes into structure called data
  EEPROM.get(eeAddress, SensorIds);

  delay(500);

  Wire.begin();  // Wire communication begin
  pinMode(DHTPin, INPUT);

  dht.begin();
  DS18B20.begin();
}

void loop() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  String currentTime = timeClient.getFormattedTime();
  
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  int currentYear = ptm->tm_year+1900;

  char dayMonth[6];
  sprintf(dayMonth, "%02d-%02d", currentMonth, monthDay);

  String currentDateTime = String(currentYear) + "-" + dayMonth + " " + String(currentTime);
  Serial.println(currentDateTime);

  if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1)
      ;
  }

  float bmp280temp = bmp.readTemperature();
  float bmp280hPa = bmp.readPressure() / 100;
  delay(300);
  float dth11temp = dht.readTemperature();
  float dth11hum = dht.readHumidity();
  delay(300);
  DS18B20.requestTemperatures();
  t0 = DS18B20.getTempCByIndex(0);

  for (int i = 0; i < 1; i++) {
    Serial.println("--- BMP280 ---");
    Serial.print(F("Temperature = "));
    Serial.print(bmp280temp);
    Serial.println(" *C");
    Serial.print(F("Pressure = "));
    Serial.print(bmp280hPa);
    Serial.println(" Pa");
    Serial.println(F("--- END BMP280 ---"));
    Serial.println("--- DHT11 ---");
    Serial.print(F("Temperature = "));
    Serial.print(dth11temp);
    Serial.println(" *C");
    Serial.print(F("Humidity = "));
    Serial.print(dth11hum);
    Serial.println(" %");
    Serial.println(F("--- END DHT11 ---"));
    Serial.println("--- DS18B20 ---");
    Serial.print(F("Temperature = "));
    Serial.print(t0);
    Serial.println(" *C");
    Serial.println(F("--- END DS18B20 ---"));
    delay(300);

    DynamicJsonBuffer jsonDeviceBuffer;

    JsonObject& RegisterDevice = jsonDeviceBuffer.createObject();
    RegisterDevice["deviceId"] = deviceUuid;
    RegisterDevice["deviceName"] = "Wemos D1 MultiSensor";
    RegisterDevice["deviceType"] = "Prototype";
    RegisterDevice["deviceGroup"] = "ESP";
    RegisterDevice["deviceParentGroup"] = "Environment";

    JsonArray& deviceDataTypes = RegisterDevice.createNestedArray("deviceDataTypes");

    JsonObject& deviceDataTypes_0 = deviceDataTypes.createNestedObject();
    deviceDataTypes_0["type"] = "temperature_bmp280";
    deviceDataTypes_0["description"] = "Temperature BMP280";
    deviceDataTypes_0["unit"] = "C";

    JsonObject& deviceDataTypes_1 = deviceDataTypes.createNestedObject();
    deviceDataTypes_1["type"] = "pressure_bmp280";
    deviceDataTypes_1["description"] = "Air Pressure BMP280";
    deviceDataTypes_1["unit"] = "hPa";

    JsonObject& deviceDataTypes_2 = deviceDataTypes.createNestedObject();
    deviceDataTypes_2["type"] = "temperature_dht11";
    deviceDataTypes_2["description"] = "Temperature DHT11";
    deviceDataTypes_2["unit"] = "C";

    JsonObject& deviceDataTypes_3 = deviceDataTypes.createNestedObject();
    deviceDataTypes_3["type"] = "humidity_dht11";
    deviceDataTypes_3["description"] = "Humidity DHT11";
    deviceDataTypes_3["unit"] = "%";

    JsonObject& deviceDataTypes_4 = deviceDataTypes.createNestedObject();
    deviceDataTypes_4["type"] = "temperature_ds18b20";
    deviceDataTypes_4["description"] = "Temperature DS18B20";
    deviceDataTypes_4["unit"] = "C";

    char JSONdeviceMessageBuffer[1024];
    RegisterDevice.prettyPrintTo(JSONdeviceMessageBuffer, sizeof(JSONdeviceMessageBuffer));
    Serial.println(JSONdeviceMessageBuffer);

    HTTPClient httpRegister;

    httpRegister.begin(urlRegister);  //Specify request destination
    httpRegister.addHeader("Content-Type", "application/json");
    httpRegister.addHeader("Authorization", "Basic " + rbase64.encode(basicAuth));  //Specify content-type header

    int httpRegisterCode = httpRegister.POST(JSONdeviceMessageBuffer);  //Send the request
    String responseRegister = httpRegister.getString();                 //Get the response payload

    Serial.println(httpRegisterCode);  //Print HTTP return code
    Serial.println(responseRegister);

    jsonDeviceBuffer.clear();

    DynamicJsonBuffer registerResponseBuffer;
    JsonObject& registerResponse = registerResponseBuffer.parseObject(responseRegister);

    bool success = registerResponse["success"];

    if (success) {
      JsonArray& data = registerResponse["data"];

      JsonObject& data_0 = data[0];
      SensorIds.sId_0 = data_0["id"];
      Serial.println(SensorIds.sId_0);

      JsonObject& data_1 = data[1];
      SensorIds.sId_1 = data_1["id"];
      Serial.println(SensorIds.sId_1);

      JsonObject& data_2 = data[2];
      SensorIds.sId_2 = data_2["id"];
      Serial.println(SensorIds.sId_2);

      JsonObject& data_3 = data[3];
      SensorIds.sId_3 = data_3["id"];
      Serial.println(SensorIds.sId_3);

      JsonObject& data_4 = data[4];
      SensorIds.sId_4 = data_4["id"];
      Serial.println(SensorIds.sId_4);

      EEPROM.put(eeAddress, SensorIds);
      EEPROM.commit();

      registerResponseBuffer.clear();
    } else {
      DynamicJsonBuffer jsonBufferExtended;

      JsonObject& sensorDataExtended = jsonBufferExtended.createObject();
      sensorDataExtended["deviceId"] = deviceUuid;
      sensorDataExtended["date"] = currentDateTime;

      JsonArray& dataExtended = sensorDataExtended.createNestedArray("data");

      JsonObject& data_0 = dataExtended.createNestedObject();
      data_0["dataTypeId"] = SensorIds.sId_0;
      data_0["value"] = bmp280temp;

      JsonObject& data_1 = dataExtended.createNestedObject();
      data_1["dataTypeId"] = SensorIds.sId_1;
      data_1["value"] = bmp280hPa;

      JsonObject& data_2 = dataExtended.createNestedObject();
      data_2["dataTypeId"] = SensorIds.sId_2;
      data_2["value"] = dth11temp;

      JsonObject& data_3 = dataExtended.createNestedObject();
      data_3["dataTypeId"] = SensorIds.sId_3;
      data_3["value"] = dth11hum;

      JsonObject& data_4 = dataExtended.createNestedObject();
      data_4["dataTypeId"] = SensorIds.sId_4;
      data_4["value"] = t0;

      char JSONmessageBufferExtended[1024];
      sensorDataExtended.prettyPrintTo(JSONmessageBufferExtended, sizeof(JSONmessageBufferExtended));
      Serial.println(JSONmessageBufferExtended);

      HTTPClient httpExtend;

      httpExtend.begin(url);  //Specify request destination
      httpExtend.addHeader("Content-Type", "application/json");
      httpExtend.addHeader("Authorization", "Basic " + rbase64.encode(basicAuth));  //Specify content-type header

      int httpCodeExtend = httpExtend.POST(JSONmessageBufferExtended);  //Send the request
      String responseExtend = httpExtend.getString();                   //Get the response payload

      Serial.println(httpCodeExtend);  //Print HTTP return code
      Serial.println(responseExtend);
      jsonBufferExtended.clear();
    }
  }
  Serial.println("Go to sleep.");
  delay(300);
  ESP.deepSleep(1000000L * 10);
}
