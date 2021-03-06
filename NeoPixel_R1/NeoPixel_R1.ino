#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WS2812FX.h>

#define LED_COUNT 12
#define LED_PIN D2    //control pin from ESP

#define TIMER_MS 5000


#define MQTT_KEEPALIVE 60

const char* ssid = "CNN";
const char* password = "Co!!eenandNei!";
const char* mqttServer = "192.168.1.13";
const int mqttPort = 1883;
const char* clientName = "Office_Lights";
const char* topic_sub_color = "Lights/office/commands/color";  //listen to this topic
const char* topic_sub_mode = "Lights/office/commands/mode";  //listen to this topic
const char* topic_pub = "lights/office/status";
unsigned long last_change = 0;
unsigned long now = 0;

WiFiClient espClient;         //wifi client
PubSubClient client(espClient); //MQTT client requires wifi client



// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
//current string uses GRBW
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);




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
void callback(char* topic, byte* payload, unsigned int length2){
      Serial.print("Message arrived in topic: ");
      Serial.println(topic);
      Serial.print("Message: ");
      
      for(int i = 0; i<length2;i++){
      Serial.print((char)payload[i]);
      }
      Serial.println ("");
    
       payload[length2] = 0;
  
  if (strcmp(topic,"Lights/office/commands/color")==0)
  {   
        StaticJsonBuffer<300> JSONbuffer; 
        String inData = String((char*)payload);
        JsonObject& root = JSONbuffer.parseObject(inData);

        if (!root.success()){
          Serial.println("ERROR");
          return;
        }
      
      const char* color = root["color"];
      ws2812fx.setColor(strtoul(color, NULL, 16));
  }


  if (strcmp(topic,"Lights/office/commands/mode")==0)
  {   
        StaticJsonBuffer<300> JSONbuffer; 
        String inData = String((char*)payload);
        JsonObject& root = JSONbuffer.parseObject(inData);

        if (!root.success()){
          Serial.println("ERROR");
          return;
        }
      
      int light_mode = root["mode"];
      ws2812fx.setMode(light_mode);
      send_status();
  }



         
}

/************************setup lights***********************************/
void setup_lights(){
  ws2812fx.init();
  ws2812fx.setBrightness(50);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(PURPLE);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();
  ws2812fx.setMode(1);
}

/************************send status***********************************/

void send_status(){
  
    StaticJsonBuffer<300> JSONbuffer;
    JsonObject& JSONencoder = JSONbuffer.createObject();
    JSONencoder["ID"] = clientName;
    JSONencoder["MODE"] = ws2812fx.getMode();
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
  client.subscribe(topic_sub_color); 
  client.subscribe(topic_sub_mode); 
  setup_lights();  

  

}

/************************LOOP***********************************/
void loop() {
//now = millis();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  ws2812fx.service();
}
