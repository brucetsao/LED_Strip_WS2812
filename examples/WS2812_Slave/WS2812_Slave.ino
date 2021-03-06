/* WS2812_Slave Source file.
 * note: PIXEL_MODE should be choosen as MODE_SLAVE in WS2812.h
 */

#include <Wire.h>
#include <WS2812.h>
#include <avr/io.h> 
#include <avr/interrupt.h> 
#include <util/delay.h> 

#define LED_PIN 5

uint8_t cmd = 0x00;
uint8_t cmdBuf[10];

WS2812 LED(0); //LED number
uint8_t interruptHandle = 0;
uint8_t buttonCtrl = 0;
uint8_t handleState;
void setup() { 
  handleState = STATE_AVAILABLE;
  pinMode(3,OUTPUT);
  digitalWrite(3, LOW);
  Wire.begin(ADDR_LED_STRIP);    // join I2C bus as a slave with address 1
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent);
  Serial.begin(9600);
  PCICR |= _BV(PCIE2);   //Enable PCINT2
  PCMSK2 |= _BV(PCINT20); //Trigger on change of PCINT20 (PD4)
  sei(); 
}

ISR(SIG_PIN_CHANGE2) 
{ 
   interruptHandle++; 
   if(interruptHandle%2 == 0){
     interruptHandle = 0;
     buttonCtrl = !buttonCtrl;
     Serial.println(buttonCtrl);
     digitalWrite(3, buttonCtrl);
   }
}

void loop() {
   if(handleState == STATE_BUSY){
        cmd = Wire.read();
        switch(cmd){
          case (CMD_INIT):
            if(_getCMD(3)){
               uint16_t leds = cmdBuf[0]<<8|cmdBuf[1];
               uint8_t pin = cmdBuf[2];
               LED = WS2812(leds);
               LED.init(pin);
            }  
            break;
          case (CMD_SET_RGB):
            if(_getCMD(5)){
               uint16_t index = (cmdBuf[0]<<8)|cmdBuf[1];
               uint32_t color = LED.RGB(cmdBuf[2], cmdBuf[3], cmdBuf[4]);
               Serial.println(color,HEX);
               LED.setRGB(index, color);
            }     
            break;
          case (CMD_SET_BRI):
            if(_getCMD(1)){
              LED.setBrightness(cmdBuf[0]);
            }    
            break;
          case (CMD_CLEAR):
            LED.clear();
            break;
          case (CMD_SYNC):
            LED.sync();
            break;
          case (CMD_SINGLE_SCROLL):
            if(_getCMD(4)){
              uint32_t color = LED.RGB(cmdBuf[0], cmdBuf[1], cmdBuf[2]);
              uint8_t time = cmdBuf[3];
              LED.scrolling(color, time);
            }
            break;
          case (CMD_MULTI_SCROLL):
            if(_getCMD(5)){
              uint8_t size = cmdBuf[0];
              uint32_t color = LED.RGB(cmdBuf[1], cmdBuf[2], cmdBuf[3]);
              uint8_t time = cmdBuf[4];
              LED.scrolling(size, color, time);
            }
            break;
          case (CMD_PASSINGBY):
            if(_getCMD(8)){
              uint8_t size = cmdBuf[0];
              uint32_t pixel_color1 = LED.RGB(cmdBuf[1], cmdBuf[2], cmdBuf[3]);
              uint32_t pixel_color2 = LED.RGB(cmdBuf[4], cmdBuf[5], cmdBuf[6]);
              uint8_t time = cmdBuf[7];
              LED.passingBy(size, pixel_color1, pixel_color2, time);
            }
            break;
          case (CMD_SINGLE_BLINK):
            if(_getCMD(6)){
              uint16_t index = (cmdBuf[0]<<8)|cmdBuf[1];
              uint32_t pixel_color = LED.RGB(cmdBuf[2], cmdBuf[3], cmdBuf[4]);
              uint8_t time = cmdBuf[5];
              LED.blink(index, pixel_color, time);
            }
            break;
          case (CMD_ALL_BLINK):
            if(_getCMD(4)){
              uint32_t pixel_color = LED.RGB(cmdBuf[0], cmdBuf[1], cmdBuf[2]);
              uint8_t time = cmdBuf[3];
              LED.blinkAll(pixel_color, time);
            }
            break;
          case (CMD_COLOR_BRUSH):
            if(_getCMD(4)){
              uint32_t pixel_color = LED.RGB(cmdBuf[0], cmdBuf[1], cmdBuf[2]); 
              uint8_t time = cmdBuf[3];
              LED.colorBrush(pixel_color, time);
            }
            break;
          case (CMD_RAINBOW):
            if(_getCMD(1)){
              uint8_t time = cmdBuf[0];
              LED.rainbow(time);
            }
            break;
          case (CMD_RAINBOWCYCLE):
            if(_getCMD(1)){
              uint8_t time = cmdBuf[0];
              LED.rainbowCycle(time);
            }
            break;
          case (CMD_THEATERCHASE):
            if(_getCMD(4)){
              uint32_t pixel_color = LED.RGB(cmdBuf[0], cmdBuf[1], cmdBuf[2]); 
              uint8_t time = cmdBuf[3];             
              LED.theaterChase(pixel_color, time);
            }
            break;
          case (CMD_THEATERCHASE_RAINBOW):
            if(_getCMD(1)){
              uint8_t time = cmdBuf[0];
              LED.theaterChaseRainbow(time);
            }
            break;
          default:
            while(Wire.available()){
              Wire.read();
            }
            break;   
         }
         handleState = STATE_AVAILABLE;   
    }
}

void requestEvent()
{
    Wire.write(handleState);
}

void receiveEvent(int)
{
    if(Wire.available()){
        handleState = STATE_BUSY;
    }
}


boolean _getCMD(int dataLen)
{
   uint8_t cmdLen = Wire.read();
   if(cmdLen == dataLen){
     for(int i = 0; i < cmdLen; i++){
       cmdBuf[i] = Wire.read(); 
     }
     return true;
   }
   return false;
}


