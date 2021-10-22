/*
	Autonumous Car

	The code first checks if it is still receving information, if it is then keep receiving and then send use that information to control the servo and the motor. In the end 
  send the information to the serial for debbguing porposes.  
  
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
};
RECEIVE_DATA_STRUCTURE mydata; // Atributtes a easier name for the structure

// General variables
#define BaudRate 115200 // Defines the baudRate for the communition with serial
#define CE_PIN 7        // Defines the CE Pin
#define CSN_PIN 8       // Defines the CSN Pin
#define center 150      // Defines the certer of the wheels
#define steervalue 25   // Defines the amount the wheels will turn
#define velocity_pin A0 // Defines velocity pin
#define steer_pin A2    // Defines steer pin
#define mode_pin 2      // Defines mode pin
#define light_pin 3     // Defines light pin
#define vdown_pin 4     // Defines vdown pin
#define vup_pin 5       // Defines vup pin
#define slow_pin 6      // Defines break pin

// Variables
int vmais = 33;    // The amount the motor variable can go
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
  // RF24
  radio.begin();                       // Starts the radio driver
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
  mydata.alone = digitalRead(mode_pin);       // Mode
  vup = digitalRead(vup_pin);                 // Vup
  vdown = digitalRead(vdown_pin);             // Vdown
  mydata.velocity = analogRead(velocity_pin); // Velocity
  mydata.steer = analogRead(steer_pin);       // Steer

  // Maps the variavles so it takes the joystick value and makes it compatible to use in the servo driver
  mydata.velocity = map(mydata.velocity, 0, 1023, 150 + vmais, 150 - vmais);
  mydata.steer = map(mydata.steer, 0, 1023, center + steervalue, center - steervalue);

  // RF24
  rslt = radio.write(&mydata, sizeof(mydata)); // Writes the information to the car and puts the result in 'reslt' variable

  // Checks if the buttons where pressed if they were they increase or decrease the max velocity accordingly
  if (vup == 1 && countup > 5)
  {
    countup = 0;
    vmais++;
  }
  if (vdown == 1 && countdown > 5)
  {
    countdown = 0;
    vmais--;
  }

  // Prints the information to the serial for debuging
  printf("\nSteer:", mydata.steer, "  Velocity:", mydata.velocity, "  Light:", mydata.light, "  Break:", mydata.slow, "  Mode:", mydata.alone, "  Acknowledge:", rslt);

  delay(10);
}
