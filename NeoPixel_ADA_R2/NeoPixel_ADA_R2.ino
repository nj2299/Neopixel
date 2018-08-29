/*Top Comment Lines
 * 
 * 
 * blah
 * 
 * 
 * 
 */


#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <WiFiConnect.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LED_COUNT 12
#define LED_PIN D2    //control pin from ESP
#define TIMER_MS 5000
#define MQTT_KEEPALIVE 120



const int mqttPort = 1883;
const char* clientName = "Office_Lights";
const char* topic_sub_color = "OLAF/commands/color";  //listen to this topic
const char* topic_sub_mode = "OLAF/commands/mode";  //listen to this topic
const char* topic_pub = "OLAF/status";
const char* topic_sub_firmware = "OLAF/commands/firmware";  //listen for firmware update
unsigned long last_change = 0;
unsigned long now = 0;
int Red=0;
int Blue = 0;
int Green = 0;
int effect = 0;
int loops = 0;
String colour = "";
//int updatenow = 0 ;

WiFiClient espClient;         //wifi client
PubSubClient client(espClient); //MQTT client requires wifi client
ESP8266WiFiMulti WiFiMulti; //for httpUpdate (OTA)


// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);




/****************setup wifi************************************/
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
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

/*****************Firmware UPdate**********************************/
void firmware_update(){
 // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {

        t_httpUpdate_return ret = ESPhttpUpdate.update("http://nj2299.duckdns.org/updateArduinoOLAF");
        //t_httpUpdate_return  ret = ESPhttpUpdate.update("https://server/file.bin");

        switch(ret) {
            case HTTP_UPDATE_FAILED:
                Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                break;

            case HTTP_UPDATE_NO_UPDATES:
                Serial.println("HTTP_UPDATE_NO_UPDATES");
                break;

            case HTTP_UPDATE_OK:
                Serial.println("HTTP_UPDATE_OK");
                break; 
        }
    }
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
      //send_status();
      Serial.print("Message arrived in topic: ");
      Serial.println(topic);
      Serial.print("Message: ");
      
      for(int i = 0; i<length2;i++){
      Serial.print((char)payload[i]);
      }
      Serial.println ("");
    
       payload[length2] = 0;
  
  if (strcmp(topic,"OLAF/commands/color")==0)
  {   
        StaticJsonBuffer<300> JSONbuffer; 
        String inData = String((char*)payload);
        JsonObject& root = JSONbuffer.parseObject(inData);

        if (!root.success()){
          Serial.println("ERROR");
          return;
        }
      
      Red = root["red"];
      Green = root["green"];
      Blue = root["blue"];
      
      JsonObject& JSONencoder = JSONbuffer.createObject();
      JSONencoder["ID"] = clientName;
      JSONencoder["Red"] = Red;
      JSONencoder["Green"] = Green;
      JSONencoder["Blue"] = Blue;
      JSONencoder["Effect"] = effect;
      char JSONmessageBuffer[300];
      JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
      client.publish(topic_pub, JSONmessageBuffer);
      Serial.println(JSONmessageBuffer);
 
  }


  if (strcmp(topic,"OLAF/commands/mode")==0)
  {   
        Serial.println("Message arrived in topic mode: ");
        StaticJsonBuffer<300> JSONbuffer; 
        String inData = String((char*)payload);
        JsonObject& root = JSONbuffer.parseObject(inData);

        if (!root.success()){
          Serial.println("ERROR");
          return;
        }
      
      effect = root["effect"];
      loops = root["loops"];
      //ws2812fx.setMode(light_mode);
     // send_status();
      Serial.println(effect);
      Serial.println(loops);
     
  }



  if (strcmp(topic,"OLAF/commands/firmware")==0)
  {   
        StaticJsonBuffer<300> JSONbuffer; 
        String inData = String((char*)payload);
        JsonObject& root = JSONbuffer.parseObject(inData);

        if (!root.success()){
          Serial.println("ERROR");
          return;
        }
      
      int firmwareupdate = root["UpdateNow"];
      //ws2812fx.setMode(light_mode);
      //send_status();

      if (firmwareupdate == 1){
        firmware_update();
      }
  }
         
}

/************************setup lights***********************************/
void setup_lights(){
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

/************************send status***********************************/

void send_status(){

  
    StaticJsonBuffer<300> JSONbuffer;
    JsonObject& JSONencoder = JSONbuffer.createObject();
    JSONencoder["ID"] = clientName;
    JSONencoder["Red"] = Red;
    JSONencoder["Green"] = Green;
    JSONencoder["Blue"] = Blue;
    JSONencoder["Effect"] = effect;
    
//    JSONencoder["MODE"] = ws2812fx.getMode();
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
  client.subscribe(topic_sub_firmware);
  setup_lights();  

  

}

/************************LOOP***********************************/
void loop() {
//now = millis();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  effect_control(effect, loops);

}

  void effect_control (uint16_t eff, uint16_t lps){
    for (uint16_t i=0; i<=lps; i++){
     switch (eff)
    {   case 0:
          client.loop();
         break;
      
        case 1:
          nj_rainbow();
          client.loop();
          Serial.println(i);
          break;
       
        case 2:
          colorWipe(strip.Color(Red, Green, Blue), 50);
          client.loop();
          Serial.println(i);
          break;
    
        case 3:
         colorWipeReverse (strip.Color(Red, Green, Blue), 50); 
         client.loop();
          break;

        case 4:
          clear_strip();
          break;

        case 5:
          theaterChaseRainbow(50);
          break;

        case 6:
          rainbow(20);
          break;

        case 7:
          full_on (strip.Color(Red, Green, Blue));
          client.loop();
          break;
       }
    }
 //   Serial.println(i);
    if(effect != 0 || effect !=7){
        //clear_strip();
        effect = 0;
    }

  }

//    nj_rainbow();
//  candle();
//    bloodDrip (strip.Color(80,0,0),strip.Color(0,0,0), 50); // Red
//    colorWipe(strip.Color(Red, Green, Blue), 50); 
//    colorWipeReverse (strip.Color(Red, Green, Blue), 50); 
    
//  colorWipe(strip.Color(0, 255, 0), 50); // Green
//  colorWipe(strip.Color(0, 0, 255), 50); // Blue
//colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW
  // Send a theater pixel chase in...
//  theaterChase(strip.Color(127, 127, 127), 50); // White
//  theaterChase(strip.Color(127, 0, 0), 50); // Red
//  theaterChase(strip.Color(0, 0, 127), 50); // Blue

//  rainbow(20);
//  rainbowCycle(20);
//  theaterChaseRainbow(50);

//}



/************************LIGHT EFFECTS***********************************/

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
    }
  
   stall(40); 
   clear_strip();
}

void colorWipeReverse (uint32_t c, uint8_t wait) {
  for(uint16_t i= 0 ; i<LED_COUNT+1; i++) {
    strip.setPixelColor(strip.numPixels()-i,c);
    strip.show();
    delay(wait);
    }
   stall(40); 
   clear_strip();
}

void stall(uint16_t s){
  for (uint16_t i = 0; i<s; i++){
  client.loop();
  delay(50);
  }
}


void clear_strip(){
  for (uint16_t i=0; i<strip.numPixels(); i++){
    strip.setPixelColor(i,strip.Color(0,0,0));
    strip.show();
  }
}


void full_on(uint32_t c){
  for (uint16_t i=0; i<strip.numPixels(); i++){
    strip.setPixelColor(i,c);
    strip.show();
  }
}

void bloodDrip (uint32_t c, uint32_t c2, uint8_t wait){
  uint16_t i=0;
  strip.setPixelColor (i,c);
  strip.show();
  delay (random(1000,5000));
  i++;
  strip.setPixelColor (i,c);
  strip.show();
  delay (random(3000));
  
  for(i=1;i<strip.numPixels()+2 ;i++){
    strip.setPixelColor(i,c);
    strip.setPixelColor(i-2, c2);
    strip.show();
    delay (wait);
    }
    
  for (i =0; i<6; i++){
    strip.setPixelColor (strip.numPixels()-i,c);
    strip.show();
    delay(wait);
   }
 
  for (i =6; i>0; i--){
    strip.setPixelColor (strip.numPixels()-i,c2);
    strip.show();
    delay(wait);
   }

//strip.setPixelColor (strip.numPixels()-1,c);
//  strip.show();
//  delay (3000);
   
for(i=0;i<strip.numPixels();i++){
    strip.setPixelColor(i, c2);
    }
strip.show();
delay(random (5000));
   
}

void candle() {
   uint8_t green; // brightness of the green 
   uint8_t red;  // add a bit for red
   for(uint8_t i=0; i<100; i++) {
     green = 50 + random(155);
     red = green + random(50);
     strip.setPixelColor(random(strip.numPixels()), red, green, 0);
     strip.show();
     delay(50);
  }
}


void nj_rainbow(){
  for(uint8_t i=0; i<strip.numPixels(); i++) {
      uint32_t c = strip.Color(random(255),random(255),random(255));
      //strip.setPixelColor(random(strip.numPixels()), c);
      strip.setPixelColor(random(strip.numPixels()), Wheel(random(255)));
      strip.show();
      delay(500);
    }
   
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
    //client.loop();
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      client.loop();
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
