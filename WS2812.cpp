/*
 * WS2812.cpp
 * LED Strip driver Library
 *
 * Copyright (c) 2014 seeed technology inc.
 * Author        :   lawliet.zou
 * Contribution  :   
 * Create Time   :   Sep 2014
 * Change Log    :   
 *  
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "WS2812.h"
#include <stdlib.h>
#include <Wire.h>

WS2812::WS2812(uint16_t num_leds) 
{
    _leds = num_leds;
    degree = 1;
}

void WS2812::_WriteI2C(uint8_t CMD, uint8_t* args, int8_t argLen)
{
    Wire.requestFrom(ADDR_LED_STRIP, 1);
    while(!Wire.available());
    uint8_t data = Wire.read();
    while(data != STATE_AVAILABLE){
        delay(500);
        Wire.requestFrom(ADDR_LED_STRIP, 1);
        while(!Wire.available());
        data = Wire.read();
    }
    Wire.beginTransmission(ADDR_LED_STRIP);
    Wire.write(CMD);
    if(argLen)
        Wire.write(argLen);
    if(NULL != args){
        for(int i = 0; i < argLen; i++){
            Wire.write(args[i]);
        }
    }
    Wire.endTransmission();
}

#ifndef ARDUINO
void WS2812::init(const volatile uint8_t* port, volatile uint8_t* reg, uint8_t pin) {
    pinMask = (1<<pin);
    ws2812_port = port;
    ws2812_port_reg = reg;
    if(NULL == pixels)
        pixels = (uint8_t*)malloc(_leds*3);
    clear();
}
#else
void WS2812::init(uint8_t pin) {
#if (PIXEL_MODE == MODE_MASTER)
    pinMask = digitalPinToBitMask(pin);
    ws2812_port = portOutputRegister(digitalPinToPort(pin));
    ws2812_port_reg = portModeRegister(digitalPinToPort(pin));
    if(NULL == pixels)
        pixels = (uint8_t*)malloc(_leds*3);
    clear();
#elif(PIXEL_MODE == MODE_SLAVE)
    Wire.begin();
    uint8_t cmd[] = {highByte(_leds), lowByte(_leds), pin};
    _WriteI2C(CMD_INIT, cmd, 3);
#else
    #error "please define the PIXEL_MODE. we supply two mode: MODE_SLAVE & MODE_MASTER.\
MODE_SLAVE usually means that mcu(main board) send command to slave device via IIC,\
while MODE_MASTER is the one that really works."
#endif
}
#endif 

void WS2812::sync() 
{
#if (PIXEL_MODE == MODE_MASTER)
    *ws2812_port_reg |= pinMask; // Enable DDR
    ws2812_sendarray_mask(pixels,3*_leds,pinMask,(uint8_t*) ws2812_port,(uint8_t*) ws2812_port_reg );   
#elif(PIXEL_MODE == MODE_SLAVE)
    _WriteI2C(CMD_SYNC, NULL, 0);
#else

#endif
}

void WS2812::setRGB(uint16_t index, uint32_t px_value) 
{
#if (PIXEL_MODE == MODE_MASTER)
    if(index < _leds) { 
        uint16_t tmp;
        tmp = index * 3;
        pixels[tmp] = degree*((px_value>>8)&0xff);//green
        pixels[tmp+1] = degree*((px_value>>16)&0xff);//red
        pixels[tmp+2] = degree*(px_value&0xff);//blue
    } 
#elif(PIXEL_MODE == MODE_SLAVE)
    if(index < _leds){
        uint8_t cmd[] = {highByte(index), lowByte(index), (px_value>>16)&0xff, (px_value>>8)&0xff, px_value&0xff};
        _WriteI2C(CMD_SET_RGB, cmd, 5);
    }
#else

#endif
}

void WS2812::clear()
{
#if (PIXEL_MODE == MODE_MASTER)
    memset(pixels, 0, _leds*3);
    sync();
#elif (PIXEL_MODE == MODE_SLAVE)
    _WriteI2C(CMD_CLEAR, NULL, 0);
#else

#endif
}

void WS2812::setBrightness(uint8_t brightness)
{
#if (PIXEL_MODE == MODE_MASTER)
    if((brightness > 255) || (brightness < 0))
        return;
    degree = brightness/255.0;  
#elif (PIXEL_MODE == MODE_SLAVE)
    uint8_t cmd[] = {brightness};
    _WriteI2C(CMD_SET_BRI, cmd, 1);
#else

#endif
}

void WS2812::scrolling(uint32_t px_value, uint8_t time)
{
#if (PIXEL_MODE == MODE_MASTER)
    for(int i = 0; i < _leds; i++){
        clear();
        setRGB(i, px_value);
        sync();
        delay(time);
    }
#elif (PIXEL_MODE == MODE_SLAVE)
    uint8_t cmd[] = {(px_value>>16)&0xff, (px_value>>8)&0xff, px_value&0xff, time};
    _WriteI2C(CMD_SINGLE_SCROLL, cmd, 4);
#else

#endif
}

void WS2812::scrolling(uint8_t size, uint32_t px_value, uint8_t time)
{
#if (PIXEL_MODE == MODE_MASTER)
    if(size > _leds){
        return;
    }
    for(int i = 1; i < _leds + size; i++){
        if(i < size){
            for(int j = 0; j < i; j++){
                setRGB(j, px_value);
            }
        }else if(i >= _leds){
            for(int k = i - size; k < _leds; k++){
                setRGB(k, px_value);
            }
        }else{
            for(int m = i-size; m < i; m++){
                setRGB(m, px_value);
            }
        }
        sync();
        delay(time);
        clear();
    }
#elif (PIXEL_MODE == MODE_SLAVE)
    uint8_t cmd[] = {size, (px_value>>16)&0xff, (px_value>>8)&0xff, px_value&0xff, time};
    _WriteI2C(CMD_MULTI_SCROLL, cmd, 5);
#else

#endif
}


void WS2812::passingBy(uint8_t size, uint32_t px_value1, uint32_t px_value2, uint8_t time)
{
#if (PIXEL_MODE == MODE_MASTER)
    if(size > _leds){
        return;
    }
    for(int i = 1; i < _leds + size; i++){
        if(i < size){
            for(int j = 0; j < i; j++){
                setRGB(j, px_value1);
                setRGB(_leds -1 - j, px_value2);
            }
        }else if(i >= _leds){
            for(int k = i - size; k < _leds; k++){
                setRGB(k, px_value1);
                setRGB(_leds -1 - k, px_value2);
            }
        }else{
            for(int m = i-size; m < i; m++){
                setRGB(m, px_value1);
                setRGB(_leds -1 - m, px_value2);
            }
        }
        sync();
        delay(time);
        clear();
    }   
#elif (PIXEL_MODE == MODE_SLAVE)
    uint8_t cmd[] = {size, (px_value1>>16)&0xff, (px_value1>>8)&0xff, px_value1&0xff, 
        (px_value2>>16)&0xff, (px_value2>>8)&0xff, px_value2&0xff, time};
    _WriteI2C(CMD_PASSINGBY, cmd, 8);
#else

#endif
}

void WS2812::blink(uint16_t index, uint32_t px_value, uint8_t time)
{
#if (PIXEL_MODE == MODE_MASTER)
    setRGB(index, px_value);
    sync();
    delay(time);
    setRGB(index, 0);
    sync();
    delay(time);
#elif (PIXEL_MODE == MODE_SLAVE)
    uint8_t cmd[] = {highByte(index), lowByte(index), (px_value>>16)&0xff, (px_value>>8)&0xff, px_value&0xff, time};
    _WriteI2C(CMD_SINGLE_BLINK, cmd, 6);    
#else

#endif
}

void WS2812::blinkAll(uint32_t px_value, uint8_t time)
{
#if (PIXEL_MODE == MODE_MASTER)
    for(int i = 0; i < _leds; i++){
        setRGB(i, px_value);
    }
    sync();
    delay(time);
    clear();
    delay(time);
#elif (PIXEL_MODE == MODE_SLAVE)
    uint8_t cmd[] = {(px_value>>16)&0xff, (px_value>>8)&0xff, px_value&0xff, time};
    _WriteI2C(CMD_ALL_BLINK, cmd, 4);   
#else

#endif
}

void WS2812::colorBrush(uint32_t px_value, uint8_t time) 
{
#if (PIXEL_MODE == MODE_MASTER)
    for(uint16_t index = 0; index < numLeds(); index++) {
        setRGB(index, px_value);
        sync();
        delay(time);
    }
#elif (PIXEL_MODE == MODE_SLAVE)
    uint8_t cmd[] = {(px_value>>16)&0xff, (px_value>>8)&0xff, px_value&0xff, time};
    _WriteI2C(CMD_COLOR_BRUSH, cmd, 4);
#else

#endif
}

void WS2812::rainbow(uint8_t time) 
{
#if (PIXEL_MODE == MODE_MASTER)
    uint16_t i, j;
    for(j = 0; j < 256; j++) {
        for(i = 0; i < numLeds(); i++) {
            setRGB(i, Wheel((i+j) & 255));
        }
        sync();
        delay(time);
    }
#elif (PIXEL_MODE == MODE_SLAVE)
    uint8_t cmd[] = {time};
    _WriteI2C(CMD_RAINBOW, cmd, 1);
#else

#endif
}

void WS2812::rainbowCycle(uint8_t time) 
{
#if (PIXEL_MODE == MODE_MASTER)
    uint16_t i, j;
    for(j = 0; j < 256*5; j++) { // 5 cycles of all colors on wheel
        for(i = 0; i < numLeds(); i++) {
            setRGB(i, Wheel(((i * 256 / numLeds()) + j) & 255));
        }
        sync();
        delay(time);
    }
#elif (PIXEL_MODE == MODE_SLAVE)
    uint8_t cmd[] = {time};
    _WriteI2C(CMD_RAINBOWCYCLE, cmd, 1);
#else

#endif
}

void WS2812::theaterChase(uint32_t px_value, uint8_t time) 
{
#if (PIXEL_MODE == MODE_MASTER)
    for (int j = 0; j < 10; j++) {  //do 10 cycles of chasing
        for (int q = 0; q < 3; q++) {
            for (int i = 0; i < numLeds(); i = i+3) {
                setRGB(i+q, px_value);    //turn every third pixel on
            }
            sync();
            delay(time);
            for (int i = 0; i < numLeds(); i = i+3) {
                setRGB(i+q, 0);        //turn every third pixel off
            }
        }
    }
#elif (PIXEL_MODE == MODE_SLAVE)
    uint8_t cmd[] = {(px_value>>16)&0xff, (px_value>>8)&0xff, px_value&0xff, time};
    _WriteI2C(CMD_THEATERCHASE, cmd, 4);
#else

#endif
}

void WS2812::theaterChaseRainbow(uint8_t time) 
{
#if (PIXEL_MODE == MODE_MASTER)
    for (int j = 0; j < 256; j++) {     // cycle all 256 colors in the wheel
        for (int q = 0; q < 3; q++) {
            for (int i = 0; i < numLeds(); i=i+3) {
                setRGB(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
            }
            sync();
            delay(time);
            for (int i = 0; i < numLeds(); i = i+3) {
                setRGB(i+q, 0);        //turn every third pixel off
            }
        }
    }
#elif (PIXEL_MODE == MODE_SLAVE)
    uint8_t cmd[] = {time};
    _WriteI2C(CMD_THEATERCHASE_RAINBOW, cmd, 1);    
#else

#endif
}

uint32_t WS2812::Wheel(byte WheelPos) 
{
    if(WheelPos < 85) {
        return RGB(WheelPos * 3, 255 - WheelPos * 3, 0);
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        return RGB(255 - WheelPos * 3, 0, WheelPos * 3);
    } else {
        WheelPos -= 170;
        return RGB(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}
