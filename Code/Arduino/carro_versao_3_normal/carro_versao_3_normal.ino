//librarias
#include <Servo.h>                                     //Inclui a libraria para o servo  
#include <SPI.h>
#include <nRF24L01.h>                                  //Inclui a libraria para comunicaçao por ondas de radio
#include <RF24.h>                                      //Inclui a libraria para comunicaçao por ondas de radio
#include <TinyGPS++.h>                                 //Inclui a libraria para o GPS
#include <printf.h>


//Define o objeto servo e motor
Servo servo;
Servo motor;

//GPS
static const uint32_t GPSBaud = 9600;                 //Atribui a baudRate para comunicar entre o arduino e o GPS

TinyGPSPlus gps;                                      //Define o objeto gps


//Comunicacao Radio
const byte slaveAddress[6] = "00001";
const byte masterAddress[6] = "00002";

RF24 radio(7, 53); // CE, CSN


//Estrutura com a informacao recebida
struct RECEIVE_DATA_STRUCTURE {
  int velocity;
  int steer;
  int light;
  int freio;
  int alone;
} ;


RECEIVE_DATA_STRUCTURE mydata;                                   //Atribui um nome mais facil de utilizar


//Estrutura com a informacao enviada
struct SEND_DATA_STRUCTURE {
  float longitude0;                                            //4 bytes
  float latitude0;                                             //4 bytes
  int velocity;                                            //4 bytes
  int steer;                                                //4 bytes
} ;


SEND_DATA_STRUCTURE datasend;                                   //Atribui um nome mais facil de utilizar



//Variaveis gerais
#define Baud 115200                                              //Atribui a baudRate para comunicar entre o arduino e o computador
#define steerc 90                                             //Centro das rodas
#define steerd 15                                              //Maximo as rodas podem virar
#define lightpin 46                                              //Define o pino das luzes

//Variaveis
bool rslt;                                                     //Resultado de envio de mensagem
int light1 = 0;                                                //Luzes desligadas
long tempoanterior = 0;                                        //Altura em que foi alterada as luzes
long tempo = 0;                                                //Tempo que nao foi ativado o comando das luzes
long tempoanterior1 = 0;                                       //Tempo da altura em que foi alterada
long tempo1 = 0;
long tempoanterior2 = 0;                                       //Time on the last command
long tempo2 = 0;
int modo = 0;                                                  //Define se está no modo manual ou automatico



void setup() {
  servo.attach(10);                                              //Inicia o Servo
  servo.write(steerc);                                          //Centra as Rodas
  motor.attach(6);                                              //Inicia o motor
  motor.write(90);                                              //Define o ponto morto do motor
  pinMode(lightpin, OUTPUT);                                    //Inicia o pino das luzes
  digitalWrite(lightpin, LOW);                                  //Inicializa as luzes no modo desligado
  Serial.begin(Baud);                                           //Inicializa a comunicaçao Serial com o computador
  printf_begin();

  //Inicia o Radio
  radio.begin();
  radio.openReadingPipe(0, slaveAddress);
 // radio.enableAckPayload();
  radio.startListening();
  //radio.writeAckPayload(0, &datasend, sizeof(datasend)); // pre-load data

  //Inicia a comunicaçao Serial para o GPS
  Serial1.begin(GPSBaud);
}


void loop() {
  //GPS

  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
    if (gps.location.isUpdated()) {
      datasend.latitude0 = gps.location.lat();
      datasend.longitude0 = gps.location.lng();
    }
  }

  if ((tempo1 - tempoanterior1) >= 20) {
    motor.write(90);
    servo.write(90);
    mydata.steer = 90;
    mydata.velocity = 90;
  }

  else {
    //Nrf24
      if (radio.available())  {            //Looking for the data.
        radio.read(&mydata, sizeof(mydata));    //Reading the data
        tempoanterior1 = tempo1;
    }
  }

  if  (modo == 0) {
    if (mydata.steer < 74 || mydata.steer > 105 || mydata.velocity > 110 || mydata.velocity < 70 ) {
    motor.write(90);
    servo.write(90);
  }
  else {
    //Velocity
    motor.write(mydata.velocity);

    //Steer
    servo.write(mydata.steer);
  }
    datasend.steer = mydata.steer;
    datasend.velocity = mydata.velocity;
  }

  //luzes
  if (tempo - tempoanterior >= 10) {
    if (mydata.light == 1) {
      switch (light1) {
        case 0:
          digitalWrite(lightpin, HIGH);
          light1 = 1;
          tempoanterior = tempo;
          break;
        case 1:
          digitalWrite(lightpin, LOW);
          light1 = 0;
          tempoanterior = tempo;
          break;
      }
    }
  }

  //Envia a informaçao do carro para o comando
  //radio.writeAckPayload(0, &datasend, sizeof(datasend)); // load the payload for the next time

  //Comuta a variavel modo
  if (tempo2 - tempoanterior2 >= 10) {
    if (mydata.alone == 1) {
      switch (modo) {
        case 0:
          modo = 1;
          tempoanterior2 = tempo2;
          break;
        case 1:
          modo = 0;
          tempoanterior2 = tempo2;
          break;
      }
    }
  }


  //Incrementa as variaveis de tempo para futuramente trocar as respetivas variaveis
  tempo++;
  tempo1++;
  tempo2++;


  //Escreve no Serial para debug
  Serial.print("  Velocity:");
  Serial.print(mydata.velocity);
  Serial.print("  Steer:");
  Serial.print(mydata.steer);
  //Serial.print("Light carro: ");
  //Serial.println(light1);
  ///Serial.print("Travao:");
  //Serial.println(mydata.freio);
  Serial.print("  Diferença: ");
  Serial.print(tempo1 - tempoanterior1);
  Serial.print("  Modo:");
  Serial.print(modo);
  Serial.print("  Latitude= ");
  Serial.print(datasend.latitude0, 6);
  Serial.print(" Longitude= ");
  Serial.println(datasend.longitude0, 6);

  delay(20);
}
