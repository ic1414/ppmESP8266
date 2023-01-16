#ifndef _nrf_h_
#define _nrf_h_

#include "Arduino.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


#define ce 2
#define csn 4
RF24 radio(ce, csn); // CE, CSN

#define radioTimeOut 600


//接收
struct nrf {
    bool en; //
    bool extra;
    byte joyStick[4]; // x, y, z, t  
};
//send
struct drone_data {
    byte motorOut[4]; // a, b, c, d
    float voltage[3]; // 当前电压 1s, 2s, 3s
    float cur_angle[3]; // 当前角度 x, y, z
};


class NRF{
    private:
        const byte defautT = 0;   
        const byte defautX = 127;
        const byte defautY = 127;
        const byte defautZ = 127;
        const byte addresses[2][6] = {"00001", "00002"}; 
        struct nrf radioo;
        struct drone_data ack;    
        
    public:
        void begin();
        void reset_pwm();
        void setAck_pwm(int, int, int, int);
        void setAck_vgt(float, float, float);
        void setAck_agl(float, float, float);
        void read();
        bool available();
        bool writeAckPayload();
        int getRadioo_joyStick(char);
        bool getRadioo_en();
        bool getRadioo_extra();
};


void NRF::begin(){
    //initiate radio
    radio.begin();
    radio.enableDynamicPayloads();
    radio.enableAckPayload();
    radio.openReadingPipe(1, addresses[1]); // address global
    radio.openWritingPipe(addresses[0]); // 00001
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();
    radio.writeAckPayload(1, &ack, sizeof(ack));   
}


bool NRF::available(){
    return radio.available();
}


void NRF::read(){
    radio.read(&radioo, sizeof(radioo));    
}


bool NRF::writeAckPayload(){
    return radio.writeAckPayload(1, &ack, sizeof(ack));
}


void NRF::reset_pwm(){
    radioo.joyStick[0] = defautX;
    radioo.joyStick[1] = defautY;
    radioo.joyStick[2] = defautZ;
    radioo.joyStick[3] = defautT;
    radioo.en = 0;
}


void NRF::setAck_pwm(int p1, int p2, int p3, int p4){
    // update ack 
    ack.motorOut[0] = map(p1, 1000, 2000, 0, 100);
    ack.motorOut[1] = map(p2, 1000, 2000, 0, 100);
    ack.motorOut[2] = map(p3, 1000, 2000, 0, 100);
    ack.motorOut[3] = map(p4, 1000, 2000, 0, 100);
}


void NRF::setAck_agl(float x, float y, float z){
    ack.cur_angle[0] = x;
    ack.cur_angle[1] = y;
    ack.cur_angle[2] = z;
}


void NRF::setAck_vgt(float v1s, float v2s, float v3s){
    ack.voltage[0] = v1s;
    ack.voltage[1] = v2s;
    ack.voltage[2] = v3s;
}


int NRF::getRadioo_joyStick(char which){
  switch(which){
    case 'x':
      return radioo.joyStick[0];
      break;
    
    case 'y':
      return radioo.joyStick[1];
      break;

    case 'z':
      return radioo.joyStick[2];
      break; 

    case 't':
      return radioo.joyStick[3];
      break; 
  }
}


bool NRF::getRadioo_en(){
    return radioo.en;
}

bool NRF::getRadioo_extra(){
    return radioo.extra;
}


NRF nrf;

#endif