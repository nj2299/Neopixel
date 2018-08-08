
#include <FastLED.h>

#define NUM_LEDS 5
#define DATA_PIN D2


CRGB leds[NUM_LEDS];


void setup() {
  // put your setup code here, to run once:
FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
//pinMode (D2, OUTPUT);


}

void loop() {
  // put your main code here, to run repeatedly:

int i=0;
while (i<=NUM_LEDS){
    leds[i] = CRGB::Red;
    FastLED.show();
    delay(1000); 
    i++;
}

i=0;
while(i<=NUM_LEDS){
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(1000); 
    i++;
}



/*
digitalWrite (D2,HIGH);
delay (1000);
digitalWrite (D2,LOW);
delay (1000);
*/

}
