//librarias
#include <LibPrintf.h>
#include "HCPCA9685.h"
#include <SPI.h>
#include <nRF24L01.h>                                  //Inclui a libraria para comunicaçao por ondas de radio
#include <RF24.h>                                      //Inclui a libraria para comunicaçao por ondas de radio



//Define o objeto servo e motor
#define  I2CAdd 0x40
HCPCA9685 HCPCA9685(I2CAdd);


//Comunicacao Radio
const byte slaveAddress[6] = "00001";

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


//Variaveis gerais
#define Baud 250000                                            //Atribui a baudRate para comunicar entre o arduino e o computador
#define steerc 150                                             //Centro das rodas
#define lightpin 46                                            //Define o pino das luzes

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
  /* Initialise the library and set it to 'servo mode' */
  HCPCA9685.Init(SERVO_MODE);

  /* Wake the device up */
  HCPCA9685.Sleep(false);                                       //Acorda o driver
  HCPCA9685.Servo(1, steerc);                                   //Centra as Rodas
  HCPCA9685.Servo(0, 150);                                      //Define o ponto morto do motor
  pinMode(lightpin, OUTPUT);                                    //Inicia o pino das luzes
  digitalWrite(lightpin, LOW);                                  //Inicializa as luzes no modo desligado
  Serial.begin(Baud);                                           //Inicializa a comunicaçao Serial com o computador

  //Inicia o Radio
  radio.begin();
  radio.openReadingPipe(0, slaveAddress);
  radio.startListening();
}

void loop() {
  if ((tempo1 - tempoanterior1) >= 20) {
    HCPCA9685.Servo(0, 150);
    HCPCA9685.Servo(1, 150);
    mydata.steer = 150;
    mydata.velocity = 150;
  }

  else {
    //Nrf24
    if (radio.available())  {            //Looking for the data.
      radio.read(&mydata, sizeof(mydata));    //Reading the data
      tempoanterior1 = tempo1;
    }

    if (mydata.steer < 124 || mydata.steer > 175 || mydata.velocity > 183 || mydata.velocity < 117 ) {
      HCPCA9685.Servo(0, 150);
      HCPCA9685.Servo(1, 150);
    }
    else {
      //Velocity
      HCPCA9685.Servo(0, mydata.velocity);

      //Steer
      HCPCA9685.Servo(1, mydata.steer);
    }
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
  printf("Velocity: " + mydata.velocity + "  Steer: " + mydata.steer + "  Diferença: " + tempo1-tempoanterior1 + "  Modo: " + modo);

  delay(20);
}
