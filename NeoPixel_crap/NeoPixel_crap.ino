#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>




#define MQTT_KEEPALIVE 60

const char* ssid = "CNN";
const char* password = "Co!!eenandNei!";
const char* mqttServer = "192.168.1.13";
const int mqttPort = 1883;
const char* clientName = "Office_Lights";
const char* topic_sub = "Lights/office/commands";  //listen to this topic
const char* topic_pub = "lights/office/status";
unsigned long last_change = 0;
unsigned long now = 0;

WiFiClient espClient;         //wifi client
PubSubClient client(espClient); //MQTT client requires wifi client






/****************setup wifi************************************/
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  long rssi = WiFi.RSSI();
  Serial.print("RSSI:");
  Serial.println(rssi);
}
/*****************Connect to MQTT Broker**********************************/
void ConnectBroker(PubSubClient client, const char* clientName)
{
    while (!client.connected())
    {
        Serial.print("Connecting to MQTT: ");
        Serial.println(clientName);
        if(client.connect(clientName))      //command to connect to MQTT broker with the unique client name
        {
          Serial.println("Connected");
        }
        else
        {
          Serial.print("Failed with state ");
          Serial.println(client.state());
          delay(200);
        }
    }
} 
/*****************reconnect to MQTT Broker if it goes down**********************************/
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect, just a name to identify the client
    if (client.connect(clientName)) {
      Serial.println("connected");
      
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/*****************MQTT Listener******************************************************/
void callback(char* topic, byte* payload, unsigned int length){
      Serial.print("Message arrived in topic: ");
      Serial.println(topic);
      Serial.print("Message: ");
      
      for(int i = 0; i<length;i++){
      Serial.print((char)payload[i]);
      }
      Serial.println ("");
    
       payload[length] = 0;
      
}


void mqtt_test(){
        StaticJsonBuffer<300> JSONbuffer; 
        Serial.println("Neil Message");
        JsonObject& JSONencoder = JSONbuffer.createObject();
        JSONencoder["ID"] = clientName;
        JSONencoder["WiFiSig"] = WiFi.RSSI();
        char JSONmessageBuffer[300];
        JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
        client.publish(topic_pub, JSONmessageBuffer);
        Serial.println(JSONmessageBuffer);

      

}
/************************SETUP***********************************/
void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer,mqttPort);
  ConnectBroker(client, clientName);    //connect to MQTT borker
  client.setCallback(callback);
  client.subscribe(topic_sub);
  mqtt_test(); 
 

  

}

/************************LOOP***********************************/
void loop() {
//  now = millis();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

/*
  ws2812fx.service();

  if(now - last_change > TIMER_MS) {
    //ws2812fx.setMode((ws2812fx.getMode() + 1) % ws2812fx.getModeCount());
    ws2812fx.setMode(3);
    last_change = now;
    Serial.print(ws2812fx.getMode());
    Serial.print("->");
    Serial.println(ws2812fx.getModeCount());
    send_status();
  }
 */ 

}
