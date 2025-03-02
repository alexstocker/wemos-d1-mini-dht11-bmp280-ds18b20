#include <Wire.h> // esp8266 3.1.2
#include "DHT.h" // DHT sensor library 1.4.6 by Adafruit
#include <OneWire.h> // OneWire 2.3.8
#include <DallasTemperature.h> // DallasTemperature 3.9.0 by Miles Burton
#include <Adafruit_BMP280.h> // Adafruit BMP2800 Library 2.6.8 by Adafruit
#include <ESP8266WiFi.h> // esp8266 3.1.2
#include <ESP8266WiFiMulti.h> // esp8266 3.1.2
#include <ArduinoJson.h>  // ArduinoJson 5.13.5 by Benoit Blanchon
#include <rBase64.h> // https://github.com/tschaban/rBase64
#include <NTPClient.h> // NTPClient 3.2.1 by by Fabrice Weinberg
#include <WiFiUdp.h> // esp8266 3.1.2
#include <WiFiClient.h> // esp8266 3.1.2
#include <ESP8266HTTPClient.h> // esp8266 3.1.2
#include <EEPROM.h> // esp8266 3.1.2

#ifndef STASSID
#define STASSID "[SSID]"
#define STAPSK "[KEY]"
#endif

ESP8266WiFiMulti WiFiMulti;

WiFiClient wifiClient;

const char* ssid = STASSID;
const char* password = STAPSK;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String formattedDate;
String dayStamp;
String timeStamp;

const char* url = "http://[OWNCLOUDINSTANCE]/index.php/apps/sensorlogger/api/v1/createlog/";
const char* urlRegister = "http://[OWNCLOUDINSTANCE]/index.php/apps/sensorlogger/api/v1/registerdevice/";
String basicAuth = String("[USERNAME]:[TOKEN]");

const char* deviceUuid = "101ti4fb-f0ae-4071-9de1-19d1b57624dc";

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

  Serial.println("");
  Serial.println("Connecting... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  timeClient.begin();
  timeClient.setTimeOffset(0);

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println("");

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

  time_t epochTime = timeClient.getEpochTime();

  String formattedTime = timeClient.getFormattedTime();

  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();

  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  int monthDay = ptm->tm_mday;

  int currentMonth = ptm->tm_mon+1;

  int currentYear = ptm->tm_year+1900;

  String currentDateTime = String(currentYear) + "-" + currentMonth + "-" + monthDay + " " + currentHour + ":" + currentMinute + ":" + currentSecond;

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
    Serial.println("");
    Serial.println("--- BMP280 --------------");
    Serial.print(F("Temperature: "));
    Serial.print(bmp280temp);
    Serial.println(" *C");
    Serial.print(F("Pressure: "));
    Serial.print(bmp280hPa);
    Serial.println(" Pa");
    Serial.println(F("------------------------"));

    Serial.println("");
    Serial.println("--- DHT11 --------------");
    Serial.print(F("Temperature: "));
    Serial.print(dth11temp);
    Serial.println(" *C");
    Serial.print(F("Humidity: "));
    Serial.print(dth11hum);
    Serial.println(" %");
    Serial.println(F("-----------------------"));
    
    Serial.println("");
    Serial.println("--- DS18B20 -----------");
    Serial.print(F("Temperature: "));
    Serial.print(t0);
    Serial.println(" *C");
    Serial.println(F("-----------------------"));
    
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

    HTTPClient httpRegister;

    rbase64.encode(basicAuth);

    String authString = String(rbase64.result());

    httpRegister.begin(wifiClient, urlRegister);  //Specify request destination
    httpRegister.addHeader("Content-Type", "application/json");
    httpRegister.addHeader("Authorization", "Basic " + authString);

    int httpRegisterCode = httpRegister.POST(JSONdeviceMessageBuffer);  //Send the request
    String responseRegister = httpRegister.getString();                 //Get the response payload

    jsonDeviceBuffer.clear();

    DynamicJsonBuffer registerResponseBuffer;
    JsonObject& registerResponse = registerResponseBuffer.parseObject(responseRegister);

    bool success = registerResponse["success"];

    if (success) {

      Serial.println("### REGISTER Device ###"); 

      JsonArray& data = registerResponse["data"];

      JsonObject& data_0 = data[0];
      SensorIds.sId_0 = data_0["id"];

      JsonObject& data_1 = data[1];
      SensorIds.sId_1 = data_1["id"];

      JsonObject& data_2 = data[2];
      SensorIds.sId_2 = data_2["id"];

      JsonObject& data_3 = data[3];
      SensorIds.sId_3 = data_3["id"];

      JsonObject& data_4 = data[4];
      SensorIds.sId_4 = data_4["id"];

      EEPROM.put(eeAddress, SensorIds);
      EEPROM.commit();

      registerResponseBuffer.clear();
    } else {

      Serial.println("### SUBMITTING Data ###"); 

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

      HTTPClient httpExtend;

      httpExtend.begin(wifiClient, url);  //Specify request destination
      httpExtend.addHeader("Content-Type", "application/json");
      httpExtend.addHeader("Authorization", "Basic " + authString);  //Specify content-type header

      int httpCodeExtend = httpExtend.POST(JSONmessageBufferExtended);  //Send the request
      String responseExtend = httpExtend.getString();                   //Get the response payload

      Serial.println(responseExtend);
      jsonBufferExtended.clear();
    }
  }

  goToSleep();
}

void goToSleep(){
	Serial.println("Going to sleep...");
	ESP.deepSleep(13 * 1000000); yield();
}
