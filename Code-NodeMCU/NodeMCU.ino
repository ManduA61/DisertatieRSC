#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <MQ2.h>
#include <Wire.h> 
#include "DHT.h"

#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);
ESP8266WiFiMulti WiFiMulti;

int Analog_Input = A0;
int Gaz;
int ResultGaz;
int Other;
MQ2 mq2(Analog_Input);

void setup() {

  Serial.begin(115200);
  mq2.begin();
  dht.begin();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("MAM_home", "5--Fresh10");

}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;
    delay(2000);

//Temp sensor
    float temp = dht.readTemperature();

    Serial.print(F("Temp: "));
    Serial.print(temp);
    Serial.print(F("°C "));

    
//Gaz sensor
    float* values= mq2.read(true); 
    Gaz = mq2.readSmoke();
    ResultGaz = Gaz/1000000;
    Serial.print("GAZ:");
    Serial.println(ResultGaz);
//END Gaz sensor

//Other sensor
    Other = (ResultGaz*4.5)/100;
    Serial.print("Other = ");
    Serial.println(Other);
  
   

    Serial.print("[HTTP] begin...\n");
//    if (http.begin(client, "http://localhost:3000/sensors/update/5e6e8725ff76ae223c2f42b5?sensor_param_1="+String(temp)+"&sensor_param_2=69")) {
    if (http.begin(client, "http://disertatie-rsc.herokuapp.com/sensors/update/5e6e8c0dd0015c33e8a529d7?sensor_param_1=" + String(temp) + "&sensor_param_2=" + String(ResultGaz) + "&sensor_param_3=" + String(Other)  )) {
//+ "&sensor_param_2=" + String(ResultGaz)+ "&sensor_param_3=" + String(Other)
//"&sensor_param_2=8888" + "&sensor_param_3=269"
      
      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }

  delay(10000);
}
