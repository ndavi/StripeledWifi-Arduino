
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArtnetWifi.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>




//Wifi settings
const char* ssid = "ssid";
const char* password = "pAsSwOrD";

/**************************** FOR OTA **************************************************/
#define SENSORNAME "nico-stripe" //change this to whatever you want to call your device
#define OTApassword "pass" //the password you will need to enter to upload remotely via the ArduinoIDE
int OTAport = 8266;

#pragma region artnetInit
/****************************** -- ARTNET INIT ***********************************/

const int numLeds = 300; 
const int numberOfChannels = numLeds * 3; 
// Artnet settings
ArtnetWifi artnet;
const int startUniverse = 1; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

StaticJsonBuffer<200> jsonBuffer;
JsonObject& artnetData = jsonBuffer.createObject();
#pragma endregion

#pragma region mqttInit
const char* mqtt_server = "your.MQTT.server.ip";
const char* mqtt_username = "yourMQTTusername";
const char* mqtt_password = "yourMQTTpassword";
const int mqtt_port = 1883;

/************* MQTT TOPICS **************************/
const char* light_state_topic = "stripled/state";
const char* light_set_topic = "stripeled/set";

const char* on_cmd = "ON";
const char* off_cmd = "OFF";
const char* effect = "solid";
String effectString = "solid";
String oldeffectString = "solid";



/****************************************FOR JSON***************************************/
const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);
#define MQTT_MAX_PACKET_SIZE 512

WiFiClient espClient;
PubSubClient client(espClient);

#pragma endregion mqttInit

// connect to wifi – returns true if successful or false if not
boolean ConnectWifi(void)
{
  boolean state = true;
  int i = 0;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");
  
  // Wait for connection
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20){
      state = false;
      break;
    }
    i++;
  }
  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  
  return state;
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(SENSORNAME, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(light_set_topic);
      //setColor(0, 0, 0);
      //sendState();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  artnetData["universe"] = universe;
  artnetData["length"] = length;
  artnetData["sequence"] = sequence;
  artnetData["data"] = data;
  artnetData.printTo(Serial);
}

/********************************** START CALLBACK*****************************************/
void callback(char* topic, byte* payload, unsigned int length) {
  //Serial.print("Message arrived [");
  //Serial.print(topic);
  //Serial.print("] ");

  Serial.print("payload + length");
}

void otaSetup() {
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  //OTA SETUP
  ArduinoOTA.setPort(OTAport);
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(SENSORNAME);

  // No authentication by default
  ArduinoOTA.setPassword((const char *)OTApassword);

  ArduinoOTA.onStart([]() {
    Serial.println("Starting");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}


void sendState() {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();

  // jsonState = Serial.read();


  /*root["state"] = (stateOn) ? on_cmd : off_cmd;
  JsonObject& color = root.createNestedObject("color");
  color["r"] = red;
  color["g"] = green;
  color["b"] = blue;

  root["brightness"] = brightness;
  root["effect"] = effectString.c_str();


  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));

  client.publish(light_state_topic, buffer, true);*/
}

void setup()
{
  Serial.begin(115200);
  ConnectWifi();
  otaSetup();
  artnet.begin();

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
  
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();
  DynamicJsonBuffer jsonBuffer(1024);
  JsonObject& root = jsonBuffer.parseObject(Serial);
  if(root["true"]) {
      sendState();
  }
  //const char* sensor = root["sensor"];
  //Serial.println(sensor);
}