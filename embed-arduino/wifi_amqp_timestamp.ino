#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <time.h>

const char* ssid = "LodiWIFI";
const char* password = "281010lodi";
const char* mqtt_server = "buck.rmq.cloudamqp.com";
const char* mqtt_user = "kdtloxga:kdtloxga";
const char* mqtt_password = "2QKIDpHEaorsmgYusaeUnVO04reAw-y1";

WiFiClient espClient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  timeClient.begin();
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
   timeClient.update()
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

  long now = millis();
  if (now - lastMsg > 3000) {
    lastMsg = now;
    String current_time = getCurrentTimestampString();
    char charBuf[50];
    current_time.toCharArray(charBuf, 50);
    Serial.print(charBuf);
    client.publish("test", charBuf);
  }
}