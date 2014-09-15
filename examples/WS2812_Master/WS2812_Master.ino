/*
 * LED Strip example
 * 
 * Created: 08.28.2014
 * Author: lawliet.zou@gmail.com
 */ 

#include <WS2812.h>
#include <Wire.h>

#define LED_PIN  5
#define LED_NUM  50

WS2812 LED(LED_NUM); //LED number

void setup() 
{
    Serial.begin(9600);
    delay(1000);
    //pinMode(3,OUTPUT);
    //digitalWrite(3, LOW);
    LED.init(LED_PIN); //Digital Pin 2
    //LED.setBrightness(50);
}


void loop() 
{  
    LED.scrolling(LED.RGB(0,255,0),50);
    LED.scrolling(5, LED.RGB(255,255,0),50);
    LED.blink(3,LED.RGB(255,0,0),100);
    for(int i = 0; i < 5; i++){
      LED.blinkAll(LED.RGB(0,0,255),200);
    }
    LED.passingBy(10, LED.RGB(255,0,0), LED.RGB(0,0,255), 50);
    LED.colorBrush(LED.RGB(255, 0, 0), 50); // Red
    LED.colorBrush(LED.RGB(0, 255, 0), 50); // Green
    LED.colorBrush(LED.RGB(0, 0, 255), 50); // Blue
    LED.theaterChase(LED.RGB(  0, 255,   0), 50); // green
    LED.theaterChase(LED.RGB(255,   0,   0), 50); // Red
    LED.theaterChase(LED.RGB(  0,   0, 255), 50); // Blue
    LED.rainbow(2);
    LED.rainbowCycle(1);
    LED.theaterChaseRainbow(1);
}

