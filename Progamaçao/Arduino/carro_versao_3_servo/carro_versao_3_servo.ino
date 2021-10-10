//Librarys
#include <LibPrintf.h>                                 //Includes the Libprintf library to use the printf function to write to serial 
#include "HCPCA9685.h"                                 //Includes the library to use the driver for the servos
#include <SPI.h>                                       //Required library for RF24 communication 
#include <nRF24L01.h>                                  //Includes the library for the communication with radio waves
#include <RF24.h>                                      //Includes the library for the communication with radio waves



//Defines the adress of the servo driver
#define  I2CAdd 0x40
HCPCA9685 HCPCA9685(I2CAdd);


//RF24
const byte slaveAddress[6] = "00001";                         //Sets the adress for communitating with the controller(must be the same in the controller)                
RF24 radio(7, 53);                                            // CE Pin, CSN Pin


//Structure with the received information
struct RECEIVE_DATA_STRUCTURE {
  int velocity;
  int steer;
  int light;
  int freio;
  int alone;
} ;
RECEIVE_DATA_STRUCTURE mydata;                                 //Atributes a easier name to be used in the structure

//General variables
#define Baud 250000                                            //Sets the communication baudRate with the computer
#define steerc 150                                             //Center of the wheels
#define lightpin 46                                            //Defines the light pin
#define motorPin 0                                              //Define the motor pin
#define servoPin 1                                             //Define the servo pin

//Variables
bool rslt;                                                     //Stores the result of the communication
int light1 = 0;                                                //Lights value(0-off, 1-on)
long tempoanterior = 0;                                        //Time the lights value was changed
long tempo = 0;                                                //Time for the commutation of the light pin
long tempoanterior1 = 0;                                       //Last time it was received communication
long tempo1 = 0;                                               //Time to check if the arduino is still communicating with controller
long tempoanterior2 = 0;                                       //Time the modo was changed
long tempo2 = 0;                                               //Time for the commutation of the modo
int modo = 0;                                                  //Defines if its in automatic mode or manual mode (0-manual, 1-automatic)



void setup() {
  //Serial communication
  Serial.begin(Baud);                                           //Inicializes Serial comunnication with the computer for debugging

  //HCPCA9685
  HCPCA9685.Init(SERVO_MODE);                                   //Initialise the library and set it to 'servo mode
  HCPCA9685.Sleep(false);                                       //Wakes the driver
  HCPCA9685.Servo(servoPin, steerc);                            //Center the wheels
  HCPCA9685.Servo(motorPin, 150);                                //Defines the neutral in motor
  pinMode(lightpin, OUTPUT);                                    //Initializes light Pin
  digitalWrite(lightpin, LOW);                                  //Starts the lights as turnoff

  //RF24
  radio.begin();                                                //Starts the radio driver
  radio.openReadingPipe(0, slaveAddress);                       //Opens a pipe to communicate(must be the same as the controller) with the controller
  radio.startListening();                                       //Sets the RF24 driver to listening for new information
}

void loop() {
  //Checks if the arduino is still receiving information through radio waves, if not centers the wheels and puts it in neutral
  if ((tempo1 - tempoanterior1) >= 20) {
    HCPCA9685.Servo(motorPin, 150);                             //Velocity
    HCPCA9685.Servo(servoPin, 150);                             //Steer
    mydata.steer = 150;
    mydata.velocity = 150;
  }

  else {
    //Nrf24
    if (radio.available())  {                                   //Looking for the data.
      radio.read(&mydata, sizeof(mydata));                      //Reading the data
      tempoanterior1 = tempo1;                                  //Resert the time variable
    }
  }

  //Checks the received information is within accepted values and then sends it to the servo driver
if (mydata.steer < 124 || mydata.steer > 175 || mydata.velocity > 183 || mydata.velocity < 117 ) {
      HCPCA9685.Servo(motorPin, 150);                                     //Velocity
      HCPCA9685.Servo(servoPin, 150);                                     //Steer
    }
    else {
      HCPCA9685.Servo(motorPin, mydata.velocity);                         //Velocity
      HCPCA9685.Servo(servoPin, mydata.steer);                            //Steer
    }

  //Checks if the button was pressed recently, if not it will change its output
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

  //Changes the modo variable
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

  //Increments the time variables to check in the future if the respective variables can be changed
  tempo++;
  tempo1++;
  tempo2++;

  //Writes to Serial for debbuging
  printf("\nVelocity: " , mydata.velocity , "  Steer: " , mydata.steer , "  Diferença: " , tempo1-tempoanterior1 , "  Modo: " , modo);

  delay(20);                                     
}
