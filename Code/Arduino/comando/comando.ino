#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


#define CE_PIN   7
#define CSN_PIN 8
#define center 90
#define steere 17
#define steerd 17

int vmais = 20;
int vup = 0;
int vdown = 0;
int countup = 0;
int countdown = 0;

const byte slaveAddress[6] = "00001";
const byte masterAddress[6] = "00002";

bool rslt;

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

struct RECEIVE_DATA_STRUCTURE {
  int velocity;
  int steer;
  int light;
  int freio;
  int alone;
} ;


RECEIVE_DATA_STRUCTURE mydata;                                   //Atribui um nome mais facil de utilizar

struct SEND_DATA_STRUCTURE {
  float longitude0;                                            //4 bytes
  float latitude0;                                             //4 bytes                                     
  int velocity;                                            //4 bytes
  int steer;                                                //4 bytes 
} ;


SEND_DATA_STRUCTURE datasend;                                   //Atribui um nome mais facil de utilizar

void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT); //velocidade
  pinMode(A2, INPUT); //steer
  pinMode(2, INPUT); //Modo
  pinMode(3, INPUT); //luz
  pinMode(4, INPUT); //vdown
  pinMode(5, INPUT); //vup
  pinMode(6, INPUT); //travao
  radio.begin();
  //radio.enableAckPayload(); 
  radio.openWritingPipe(slaveAddress);
}

void loop() {
  countup++;
  countdown++;
  mydata.freio = digitalRead(6);
  mydata.light = digitalRead(3);
  mydata.alone = digitalRead(2);
  vup = digitalRead(5);
  vdown = digitalRead(4);
  mydata.velocity = analogRead(A0);
  mydata.steer = analogRead(A2);
  
  mydata.velocity = map(mydata.velocity, 0, 1023, 90 + vmais, 90 - vmais);
  mydata.steer = map(mydata.steer, 0, 1023, 105, 74);



  rslt = radio.write( &mydata, sizeof(mydata) );


  if (vup == 1 && countup > 5) {
    countup = 0;
    vmais++;
  }
  if (vdown == 1 && countdown > 5) {
    countdown = 0;
    vmais--;
  }
  
  Serial.print("  Steer: "); 
  Serial.print(mydata.steer); 
  Serial.print("  Velocity: ");  
  Serial.print(mydata.velocity); 
  Serial.print("  Steer1: "); 
  Serial.print(datasend.steer); 
  Serial.print("  Velocity1: ");  
  Serial.print(datasend.velocity);  
  Serial.print("  Light ");
  Serial.print(mydata.light);
  Serial.print("  Travao: ");
  Serial.print(mydata.freio);
  Serial.print("  Vdown: ");
  Serial.print(vdown);
  Serial.print("  Vup: ");
  Serial.print(vup);
  Serial.print("  Modo: ");
  Serial.print(mydata.alone);
  Serial.print("  Latitude= ");
  Serial.print(datasend.latitude0,6);
  Serial.print(" Longitude= ");
  Serial.print(datasend.longitude0,6);

    if (rslt) {
            Serial.println("  Acknowledge but no data ");
    }
    else {
        Serial.println("  Tx failed");
    }
  
   delay(10);
}
