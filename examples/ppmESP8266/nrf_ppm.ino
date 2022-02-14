#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "ppmESP8266.h"

//ppm
#define OUTPUT_PIN 4


// special
struct nrf {
  byte pwm[4];
  float rpid[3];
  float rpidz[3];
  byte en;
};
//send
struct drone_data {
  float voltages[3];
  byte motorOut[4];
  float cAngles[3];
};
struct nrf radioo;
struct drone_data ack;

// radio
#define radioTimeOut 600
const byte addresses[][6] = {"00001", "00002"};
byte defautX = 127;
byte defautY = 127;
byte defautZ = 127;
byte defautT = 0;
RF24 radio(2, 5); // CE, CSN


void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT);
  ppmEncoder.begin(OUTPUT_PIN);

  radio.begin();
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  radio.openReadingPipe(1, addresses[1]); // address global
  radio.openWritingPipe(addresses[0]); // 00001
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
  radio.writeAckPayload(1, &ack, sizeof(ack));
}


void loop() {
  static unsigned int channels[5];
  static unsigned long radio_timer = millis();

  // recieve radio
  if (radio.available()) {
    radio.read(&radioo, sizeof(radioo));
    radio.writeAckPayload(1, &ack, sizeof(ack));
    radio_timer = millis();
  } else {
    // radio time out
    if (millis() - radio_timer > radioTimeOut) {
      radioo.pwm[0] = defautX;
      radioo.pwm[1] = defautY;
      radioo.pwm[2] = defautZ;
      radioo.pwm[3] = defautT;
    }
  }

  // ack
  ack.voltages[0] = ((float)analogRead(A0))/1023*3.3*2;
  ack.voltages[1] = 3.14; 
  ack.voltages[2] = 2.71;
  if (radioo.en)ack.cAngles[0] = 1;
  else ack.cAngles[0] = 0;
  for(int i=0; i<4; i++){
    ack.motorOut[i] = radioo.pwm[3];
  }
  

  // set channels
  channels[0] = map((int)radioo.pwm[0], 0, 255, 1000, 2000);
  channels[1] = map((int)radioo.pwm[1], 0, 255, 1000, 2000);
  channels[2] = map((int)radioo.pwm[2], 0, 255, 1000, 2000);
  channels[3] = map((int)radioo.pwm[3], 0, 255, 1000, 2000);
  channels[4] = map((int)radioo.en, 0, 1, 1000, 1500);
  ppmEncoder.setChannel(0, channels[3]);
  ppmEncoder.setChannel(1, channels[1]);
  ppmEncoder.setChannel(2, channels[0]);
  ppmEncoder.setChannel(3, channels[2]);
  if (radioo.en)ppmEncoder.setChannel(4, 1500);
  else ppmEncoder.setChannel(4, 1000);

/*
  Serial.print(channels[0]);
  Serial.print("   ");
  Serial.print(channels[1]);
  Serial.print("   ");
  Serial.print(channels[2]);
  Serial.print("   ");
  Serial.print(channels[3]);
  Serial.print("   ");
  Serial.println(channels[4]);
  */
}
