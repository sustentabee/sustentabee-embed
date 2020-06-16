#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <time.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "EmonLib.h"
#include <SPI.h>
#include <ArduinoJson.h>

const char* ssid = "LodiWIFI";
const char* password = "281010lodi";
const char* mqtt_server = "buck.rmq.cloudamqp.com";
const char* mqtt_user = "kdtloxga:kdtloxga";
const char* mqtt_password = "2QKIDpHEaorsmgYusaeUnVO04reAw-y1";
const char* serial_no = "42";

long millisNow = 0;
long lastMillis = 0;
int doorState = 0;
int doorNow = 0;
int rede = 110;

const int reedSwitchPin = 4;
const int temperaturePin = 5;
const int currentPin = A0;

WiFiClient espClient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
PubSubClient client(espClient);
OneWire oneWire(temperaturePin);
DallasTemperature sensors(&oneWire);
EnergyMonitor eMon;

void setup() {
    Serial.begin(9600);
    Serial.setDebugOutput(true);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    timeClient.begin();

    pinMode(reedSwitchPin, INPUT);
    eMon.current(currentPin, 90);
}

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
            Serial.println("MQTT Connected");
        } else {
            Serial.print("MQTT Connection Failed, RC=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

String getCurrentTimestampString() {
    timeClient.update();
    time_t rawtime = timeClient.getEpochTime();
    struct tm * ti;
    ti = localtime (&rawtime);

    uint16_t year = ti->tm_year + 1900;
    String yearStr = String(year);

    uint8_t month = ti->tm_mon + 1;
    String monthStr = month < 10 ? "0" + String(month) : String(month);

    uint8_t day = ti->tm_mday;
    String dayStr = day < 10 ? "0" + String(day) : String(day);

    uint8_t hours = ti->tm_hour;
    String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

    uint8_t minutes = ti->tm_min;
    String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

    uint8_t seconds = ti->tm_sec;
    String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

    return yearStr + "-" + monthStr + "-" + dayStr + " " +
            hoursStr + ":" + minuteStr + ":" + secondStr;
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    // Every 2 seconds
    millisNow = millis();
    if (millisNow - lastMillis > 2000) {
        lastMillis = millisNow;
        sensors.requestTemperatures();
        float temperature = sensors.getTempCByIndex(0);
        double current = eMon.calcIrms(1480);
        StaticJsonDocument<200> time_payload;
        time_payload["serial_no"] = serial_no;
        time_payload["timestamp"] = getCurrentTimestampString();
        time_payload["temperature"] = char(temperature);
        time_payload["current"] = char(current);
        char time_payload_charbuff[200];
        serializeJson(time_payload, time_payload_charbuff);
        client.publish("test", time_payload_charbuff);
    }

    // When the door state changes
    doorNow = digitalRead(reedSwitchPin);
    if (doorNow != doorState) {
        doorState = doorNow;
        StaticJsonDocument<200> door_payload;
        door_payload["serial_no"] = serial_no;
        door_payload["timestamp"] = getCurrentTimestampString();
        door_payload["door_state"] = doorState;
        char door_payload_charbuff[200];
        serializeJson(door_payload, door_payload_charbuff);
        client.publish("test", door_payload_charbuff);
    }
}