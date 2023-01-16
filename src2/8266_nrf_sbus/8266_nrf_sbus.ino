#include "nrf.h"
#include <SoftwareSerial.h>


#define channelX 1
#define channelY 0
#define channelZ 3
#define channelT 2
#define channelEnable 4
#define channelExtra 5
#define sbusMinVal 173
#define sbusMidVal 992
#define sbusMaxVal 1811
#define SBUS_CHANNEL_NUMBER 16
#define SBUS_PACKET_LENGTH 25
#define SBUS_FRAME_HEADER 0x0f
#define SBUS_FRAME_FOOTER 0x00
#define SBUS_FRAME_FOOTER_V2 0x04
#define SBUS_STATE_FAILSAFE 0x08
#define SBUS_STATE_SIGNALLOSS 0x04


SoftwareSerial myPort;
void sbusPreparePacket(uint8_t, int);


void setup(){
    // Serial.begin(115200);
  myPort.begin(100000, SWSERIAL_8E2, 3, 5, true);
  Serial.begin(115200);
  nrf.begin();
}


void loop(){
    static unsigned long radio_timer = millis();
    if (nrf.available()) {
      nrf.read(); // read
      nrf.writeAckPayload(); //ack
      radio_timer = millis();
    } else if (abs(millis() - radio_timer > radioTimeOut)) nrf.reset_pwm();

    float button1;
    float button2;
    if(nrf.getRadioo_en()) button1 = 1;
    else button1 = 0;
    if(nrf.getRadioo_extra()) button2 = 1;
    else button2 = 0;
    nrf.setAck_agl(button1, button2, -1);
    nrf.setAck_vgt((float)analogRead(A0)/1024.0f * 2.0f, 0, 0);
    nrf.setAck_pwm( map(nrf.getRadioo_joyStick('x'), 0, 255, 1000, 2000), map(nrf.getRadioo_joyStick('y'), 0, 255, 1000, 2000),
                    map(nrf.getRadioo_joyStick('z'), 0, 255, 1000, 2000), map(nrf.getRadioo_joyStick('t'), 0, 255, 1000, 2000));

    uint8_t sbusPacket[SBUS_PACKET_LENGTH];
    int sbus_data[16];
    for(int i=0; i<16; i++) sbus_data[i] = sbusMinVal;
    sbus_data[channelX] = map(nrf.getRadioo_joyStick('x'), 0, 255, 172, 1811);
    sbus_data[channelY] = map(nrf.getRadioo_joyStick('y'), 0, 255, 172, 1811);
    sbus_data[channelZ] = map(nrf.getRadioo_joyStick('z'), 0, 255, 172, 1811);
    sbus_data[channelT] = map(nrf.getRadioo_joyStick('t'), 0, 255, 172, 1811);
    if(nrf.getRadioo_en()) sbus_data[channelEnable] = 1811;
    else sbus_data[channelEnable] = 172;
    if(nrf.getRadioo_extra()) sbus_data[channelExtra] = 1811;
    else sbus_data[channelExtra] = 172;

    sbusPreparePacket(sbusPacket, sbus_data);

    myPort.write(sbusPacket, SBUS_PACKET_LENGTH);
    Serial.println(nrf.getRadioo_joyStick('t'));
    delay(10);
}



void sbusPreparePacket(uint8_t packet[], int channels[]){

    int output[SBUS_CHANNEL_NUMBER] = {0};
    for (int i=0; i<16; i++) output[i] = channels[i];
    
    uint8_t stateByte = 0x00;
    if (/*isSignalLoss*/ false) stateByte |= SBUS_STATE_SIGNALLOSS;
    if (/*isFailsafe*/ false) stateByte |= SBUS_STATE_FAILSAFE;
    
    packet[0] = SBUS_FRAME_HEADER; //Header
    packet[1] = (uint8_t) ((output[0] & 0x07FF));
    packet[2] = (uint8_t) ((output[0] & 0x07FF)>>8 | (output[1] & 0x07FF)<<3);
    packet[3] = (uint8_t) ((output[1] & 0x07FF)>>5 | (output[2] & 0x07FF)<<6);
    packet[4] = (uint8_t) ((output[2] & 0x07FF)>>2);
    packet[5] = (uint8_t) ((output[2] & 0x07FF)>>10 | (output[3] & 0x07FF)<<1);
    packet[6] = (uint8_t) ((output[3] & 0x07FF)>>7 | (output[4] & 0x07FF)<<4);
    packet[7] = (uint8_t) ((output[4] & 0x07FF)>>4 | (output[5] & 0x07FF)<<7);
    packet[8] = (uint8_t) ((output[5] & 0x07FF)>>1);
    packet[9] = (uint8_t) ((output[5] & 0x07FF)>>9 | (output[6] & 0x07FF)<<2);
    packet[10] = (uint8_t) ((output[6] & 0x07FF)>>6 | (output[7] & 0x07FF)<<5);
    packet[11] = (uint8_t) ((output[7] & 0x07FF)>>3);
    packet[12] = (uint8_t) ((output[8] & 0x07FF));
    packet[13] = (uint8_t) ((output[8] & 0x07FF)>>8 | (output[9] & 0x07FF)<<3);
    packet[14] = (uint8_t) ((output[9] & 0x07FF)>>5 | (output[10] & 0x07FF)<<6);  
    packet[15] = (uint8_t) ((output[10] & 0x07FF)>>2);
    packet[16] = (uint8_t) ((output[10] & 0x07FF)>>10 | (output[11] & 0x07FF)<<1);
    packet[17] = (uint8_t) ((output[11] & 0x07FF)>>7 | (output[12] & 0x07FF)<<4);
    packet[18] = (uint8_t) ((output[12] & 0x07FF)>>4 | (output[13] & 0x07FF)<<7);
    packet[19] = (uint8_t) ((output[13] & 0x07FF)>>1);
    packet[20] = (uint8_t) ((output[13] & 0x07FF)>>9 | (output[14] & 0x07FF)<<2);
    packet[21] = (uint8_t) ((output[14] & 0x07FF)>>6 | (output[15] & 0x07FF)<<5);
    packet[22] = (uint8_t) ((output[15] & 0x07FF)>>3);
    packet[23] = stateByte; //Flags byte
    packet[24] = SBUS_FRAME_FOOTER; //Footer
}