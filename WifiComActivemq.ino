#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> //5.7.0
#include "DHT.h"

const char* ssid     = "LIVE TIM_41F9_2G";
const char* password = "2pxtrvv3hd";

unsigned long lastTime = 0;

int pinLed = 18;
int pinTerm = 19;

const char* MQTT_SERVER = "iot.plug.farm";
WiFiClient CLIENT;
PubSubClient MQTT(CLIENT);

DHT dht(pinTerm, DHT11);

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(pinLed, OUTPUT);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  dht.begin();
  
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  MQTT.setServer(MQTT_SERVER, 1883);
  MQTT.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
    DynamicJsonBuffer jsonBuffer;
    String json = (char*)payload;
    Serial.println(json);
    JsonObject& rootRead = jsonBuffer.parseObject(json);
    long s = rootRead["number"];
    Serial.println(s);
    payload = 0;
    if(s == 1) {
      digitalWrite(pinLed, HIGH);
      Serial.println("...ligar");
    } else {
      digitalWrite(pinLed, LOW);
      Serial.println("...desligar");
    }
}


void reconnect() {
  while(!MQTT.connected()) {
    if(MQTT.connect("ESP32-TestePorta")) {
      MQTT.subscribe("hello/led");
    } else {
      Serial.print(".");
      delay(3000);
    }
  }
}


void loop() {
  if(!MQTT.connected()) {
    reconnect();
  }
  
  MQTT.loop();

  unsigned long now = millis();
  if((lastTime + 10000) < now) {
    lastTime = now;

    DynamicJsonBuffer jsonBuffer;
    JsonObject & root = jsonBuffer.createObject();

    int humidity = dht.readHumidity();
    int temperature = dht.readTemperature();
    root["temp"] = temperature;
    root["humid"] = humidity;
    root["time"] = 123;
    root["id"] = "Francisco";
    String msg;
    root.printTo(msg);
    Serial.println(msg);
    MQTT.publish("hello/temp", msg.c_str());
  }
}
