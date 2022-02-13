/*
	Autonumous Car

	This is the code for the controller. It is what establishes the interactions with the user, using joysticks and button it reads the inputs the user gives 
  and then sends it to the car through the RF24 module, wich uses radio waves to communicate.  
  
  Componnents:
  - Arduino Uno
  - 2 Joystick modules
  - 4 PushButtons
  - RF24 

	PinOut:
	*RF24
   - 

	https://github.com/andrefdre/Carro
*/

// libraries
#include <LibPrintf.h> //Includes the Libprintf library to use the printf function to write to serial
#include <SPI.h>       //Required library for RF24 communication
#include <nRF24L01.h>  //Includes the library for the communication with radio waves
#include <RF24.h>      //Includes the library for the communication with radio waves

// Structure with the information to be sent
struct RECEIVE_DATA_STRUCTURE
{
  int velocity;
  int steer;
  int light;
  int slow;
  int alone;
  int vmais=90;
};
RECEIVE_DATA_STRUCTURE mydata; // Atributtes a easier name for the structure

// General variables
#define BaudRate 115200 // Defines the baudRate for the communition with serial
#define CE_PIN 7        // Defines the CE Pin
#define CSN_PIN 8       // Defines the CSN Pin
#define center 90      // Defines the center of the neutral in the motor
#define centerw 90     // Defines the center of the wheels
#define steervalue 22   // Defines the amount the wheels will turn
#define velocity_pin A0 // Defines velocity pin
#define steer_pin A2    // Defines steer pin
#define mode_pin 2      // Defines mode pin
#define light_pin 3     // Defines light pin
#define vdown_pin 4     // Defines vdown pin
#define vup_pin 5       // Defines vup pin
#define slow_pin 6      // Defines break pin

// Variables
int vup = 0;       // Checks if the button to increase max velocity was pressed
int vdown = 0;     // Checks if the button to dreaces max velocity was pressed
int countup = 0;   // Timing to only register if the button was pressed after a while
int countdown = 0; // Timing to only register if the button was pressed after a while
bool rslt;         // Stores the result of the communication, if it was sucessful or not

// RF24
RF24 radio(CE_PIN, CSN_PIN);          // Create a Radio
const byte slaveAddress[6] = "00001"; // Defines the adress for the radio communication

void setup()
{
  // Serial
  Serial.begin(115200); // Initializes the serial communication
  // Pin initialization
  pinMode(velocity_pin, INPUT); // Velocity
  pinMode(steer_pin, INPUT);    // Steer
  pinMode(mode_pin, INPUT);     // Mode
  pinMode(light_pin, INPUT);    // Light
  pinMode(vdown_pin, INPUT);    // vdown
  pinMode(vup_pin, INPUT);      // vup
  pinMode(slow_pin, INPUT);     // Break

    //SPI
  SPI.setClockDivider( 100000 );
  SPI.setDataMode( SPI_MODE0 ); //tentar ate ao modulo 0 1 2 3
  
  // RF24
   if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {} // hold in infinite loop
  }
  radio.openWritingPipe(slaveAddress); // Opens a pipe to communicate(must be the same as the car) with the car


  // Arduino setup Check
  printf("Arduino Initialized"); // Checks if the arduino setups correctly
}

void loop()
{
  countup++;   // Increases the counter so it can after actuate in the respectivel variable or not
  countdown++; // Increases the counter so it can after actuate in the respectivel variable or not

  // Acquire information from the pins
  mydata.slow = digitalRead(slow_pin);        // Break
  mydata.light = digitalRead(light_pin);      // Light
  mydata.alone = !digitalRead(mode_pin);       // Mode
  vup = digitalRead(vup_pin);                 // Vup
  vdown = digitalRead(vdown_pin);             // Vdown
  mydata.velocity = analogRead(velocity_pin); // Velocity
  mydata.steer = analogRead(steer_pin);       // Steer

  // Maps the variavles so it takes the joystick value and makes it compatible to use in the servo driver
  mydata.velocity = map(mydata.velocity, 0, 1023, center + mydata.vmais, center - mydata.vmais);
  mydata.steer = map(mydata.steer, 0, 1023, centerw + steervalue, centerw - steervalue);

  // RF24
  rslt = radio.write(&mydata, sizeof(mydata)); // Writes the information to the car and puts the result in 'reslt' variable

  // Checks if the buttons where pressed if they were they increase or decrease the max velocity accordingly
  if (vup == 1 && countup > 5)
  {
    countup = 0;
    mydata.vmais++;
  }
  if (vdown == 1 && countdown > 5)
  {
    countdown = 0;
    mydata.vmais--;
  }

  // Prints the information to the serial for debuging
  Serial.print("\nSteer:");
  Serial.print(mydata.steer);
  Serial.print("  Velocity:");
  Serial.print(mydata.velocity);
  Serial.print("  Light:");
  Serial.print(mydata.light);
  Serial.print("  Break:");
  Serial.print(mydata.slow);
  Serial.print("  Mode:");
  Serial.print(mydata.alone);
  Serial.print("  Acknowledge:");
  Serial.print(rslt);
  Serial.print("  Chanell: ");
  Serial.print(radio.getChannel());
  
  

  delay(10);
}
