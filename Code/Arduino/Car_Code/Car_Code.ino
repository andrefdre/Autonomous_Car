/*
  Autonomous Car

  The code first checks if it is still receiving information, if it is then keep receiving and then send use that information to control the servo and the motor. In the end
  send the information to the serial for debbguing purposes.

  Components:
  - Arduino Mega 2560
  - RF24 module
  - pca9685s
  - DS3235SG
  - 9T 4370KV Metal Brushless Motor
  - 60A ESC
  - Lipo battery 11,1V 3S 45C 4500mAh
  - Lipo battery 11,1V 3S 40C 1500mAh

  PinOut:
  *RF24
   -

  https://github.com/andrefdre/Carro
*/

// Libraries
#include <LibPrintf.h>               //Includes the Libprintf library to use the printf function to write to serial
#include <Wire.h>                    //Includes library for I2C communication
#include <Adafruit_PWMServoDriver.h> //Includes library to control the servo driver
#include <SPI.h>                     //Required library for RF24 communication
#include <nRF24L01.h>                //Includes the library for the communication with radio waves
#include <RF24.h>                    //Includes the library for the communication with radio waves
#include <TinyGPS.h>                 //Includes the library for the GPS data aquisition

// PCA9685
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(); // Creates the servo driver object

// GPS
TinyGPS gps; // Create gps object

// RF24
const byte Address[][6] = {"00001", "00002"}; // Sets the address for communitating with the controller(must be the same in the controller)
RF24 radio(7, 8);                             // CE Pin, CSN Pin

// Structure with the received information
struct RECEIVE_DATA_STRUCTURE
{
  int velocity = 90;
  int steer = 90;
  int light;
  int slow;
  int alone;
  int vmais;
};
RECEIVE_DATA_STRUCTURE receive_data; // Atributes a easier name to be used in the structure

// Structure with the information to be sent back
struct SEND_DATA_STRUCTURE
{
  int lati = 1000;
  int longi = 1000;
};
SEND_DATA_STRUCTURE send_data; // Atributes a easier name to be used in the structure

// General variables
#define Baud 115200   // Sets the communication baudRate with the computer
#define center 90     // Center of the wheels
#define steervalue 22 // Defines the amount the wheels will turn
#define lightpin 22   // Defines the light pin
#define motorPin 0    // Define the motor pin
#define servoPin 1    // Define the servo pin
#define SERVOMIN 500  // Defines the min pulse for servo
#define SERVOMAX 2500 // Defines the max pulse for servo
#define MOTORMiN 1000 // Defines the min pulse for motor
#define MOTORMAX 2000 // Defines the max pulse for motor

// Variables
bool rslt;              // Stores the result of the communication
int light1 = 0;         // Lights value(0-off, 1-on)
long previoustime = 0;  // Time the lights value was changed
long time3 = 0;         // Time for the commutation of the light pin
long previoustime1 = 0; // Last time it was received communication
long time1 = 0;         // Time to check if the arduino is still communicating with controller
long previoustime2 = 0; // Time the modo was changed
long time2 = 0;         // Time for the commutation of the modo
long time4 = 0;         // Timer for counting how long the car lost communication
int mode = 0;           // Defines if its in automatic mode or manual mode (0-manual, 1-automatic)
int motor_pulselength;  // Declares the variable that be used to map the value from degrees to microseconds pwm wave for motor
int servo_pulselength;  // Declares the variable that be used to map the value from degrees to microseconds pwm wave for servo
float lat, lon;         // Variables for current position of the car
int chanell;            // Variable to store the communication channel to set it again when the car loses communication

// Initializes everything, only runs once
void setup()
{
  // Serial communication
  Serial.begin(Baud); // Initializes Serial communication with the computer for debugging

  // Adafruit PWMServoDriver
  pwm.begin();                                                 // Initializes the PWM driver
  pwm.setOscillatorFrequency(27000000);                        // Value that needs to be adjusted to have a perfect 50hz (need an oscilloscope to check this)
  pwm.setPWMFreq(50);                                          // Sets the frequency of the servos which tipically is 50
  motor_pulselength = map(90, 0, 180, MOTORMiN, MOTORMAX);     // Maps the the angle value to PWM microseconds
  pwm.writeMicroseconds(motorPin, motor_pulselength);          // Writes to the motor to neutral position
  servo_pulselength = map(center, 0, 270, SERVOMIN, SERVOMAX); // Maps the the angle value to PWM microseconds
  pwm.writeMicroseconds(servoPin, servo_pulselength);          // Writes for the servo to center the wheels

  // Lights
  pinMode(lightpin, OUTPUT);   // Initializes light Pin
  digitalWrite(lightpin, LOW); // Starts the lights as turnoff

  // SPI
  SPI.setClockDivider(100000);
  SPI.setDataMode(SPI_MODE0); // try mode 0 1 2 3

  // RF24
  // Checks if the NRF24 module is connected if not hold in a loop to not initialize the car
  if (!radio.begin())
  {
    printf("radio hardware is not responding!!");
    while (1)
    {
    } // hold in infinite loop
  }
  radio.openReadingPipe(0, Address[0]); // Opens a pipe to communicate(must be the same as the controller) with the controller
  radio.openWritingPipe(Address[1]);    // Opens a pipe to communicate(must be the same as the controller) with the controller
  radio.startListening();               // Sets the RF24 driver to listening for new information
  chanell = radio.getChannel();         // Stores the channel to attribute it later

  // GPS
  Serial1.begin(9600); // Initializes Serial communication with the GPS module

  // Arduino setup Check
  printf("Arduino Initialized"); // Checks if the arduino setups correctly
  delay(10);
}

// Main Code
void loop()
{

  while (Serial1.available())
  {                                 // check for gps data
    if (gps.encode(Serial1.read())) // encode gps data
    {
      gps.f_get_position(&lat, &lon); // get latitude and longitude

      // Latitude
      printf(" Latitude: %f", lat);

      // Longitude
      printf(" Longitude: %f", lon);
    }
  }

  // Checks if the arduino is still receiving information through radio waves, if not centers the wheels and puts it in neutral
  if ((time1 - previoustime1) >= 20)
  {
    motor_pulselength = map(90, 0, 180, MOTORMiN, MOTORMAX);     // Maps the the angle value to PWM microseconds
    pwm.writeMicroseconds(motorPin, motor_pulselength);          // Writes to the motor to neutral position
    servo_pulselength = map(center, 0, 270, SERVOMIN, SERVOMAX); // Maps the the angle value to PWM microseconds
    pwm.writeMicroseconds(servoPin, servo_pulselength);          // Writes for the servo to center the wheels
    mydata.steer = center;                                       // Resets the variables to safe values
    mydata.velocity = 90;                                        // Resets the variables to safe values
    time4++;                                                     // Increase timmer that is used to check how long the communication was lost
    radio.powerDown();                                           // Turns the radio module off
    radio.setChannel(chanell);                                   // Sets the channel that was being used previously
    delay(20);
  }
  else
  {
    // Nrf24
    if (radio.available())
    {                                                  // Looking for the data.
      radio.read(&receive_data, sizeof(receive_data)); // Reading the data
      previoustime1 = time1;                           // Reset the time variable
      time4 = 0;                                       // Reset the time variable
    }
  }

  radio.stopListening();

  // Checks how long and if the communication was lost to restore it
  if (time4 >= 100)
  {
    previoustime1 = time1;                // Resets the variable to let it communicate again
    radio.powerUp();                      // Turns the radio module on
    radio.openReadingPipe(0, Address[0]); // Opens a pipe to communicate(must be the same as the controller) with the controller
    radio.startListening();               // Sets the radio to listen

  }

  // Checks the received information is within accepted values and then sends it to the servo driver
  if (receive_data.steer < center - steervalue || receive_data.steer > center + steervalue || receive_data.velocity > 90 + receive_data.vmais || receive_data.velocity < 90 - receive_data.vmais)
  {
    motor_pulselength = map(90, 0, 180, MOTORMiN, MOTORMAX);     // Maps the the angle value to PWM microseconds
    pwm.writeMicroseconds(motorPin, motor_pulselength);          // Writes to the motor to neutral position
    servo_pulselength = map(center, 0, 270, SERVOMIN, SERVOMAX); // Maps the the angle value to PWM microseconds
    pwm.writeMicroseconds(servoPin, servo_pulselength);          // Writes for the servo to center the wheels
  }
  else
  {
    motor_pulselength = map(receive_data.velocity, 0, 180, MOTORMiN, MOTORMAX); // Maps the the angle value to PWM microseconds
    pwm.writeMicroseconds(motorPin, motor_pulselength);                         // Writes the mapped value to change the motor speed
    servo_pulselength = map(receive_data.steer, 0, 270, SERVOMIN, SERVOMAX);    // Maps the the angle value to PWM microseconds
    pwm.writeMicroseconds(servoPin, servo_pulselength);                         // Writes the mapped value to steer the wheels
  }

  // Checks if the button was pressed recently, if it was it will change its output
  // Changes the light output
  if (time3 - previoustime >= 10)
  {
    if (receive_data.light == 1)
    {
      switch (light1)
      {
      case 0:
        digitalWrite(lightpin, HIGH);
        light1 = 1;
        previoustime = time3;
        break;
      case 1:
        digitalWrite(lightpin, LOW);
        light1 = 0;
        previoustime = time3;
        break;
      }
    }
  }

  // Changes the mode variable
  if (time2 - previoustime2 >= 10)
  {
    if (receive_data.alone == 1)
    {
      switch (mode)
      {
      case 0:
        mode = 1;
        previoustime2 = time2;
        break;
      case 1:
        mode = 0;
        previoustime2 = time2;
        break;
      }
    }
  }

  // NRF24 sends the information to the controller
  rslt = radio.write(&send_data, sizeof(send_data)); // Writes the information to the car and puts the result in "rslt" variable
  radio.startListening();                            // Sets the RF24 driver to listening for new information


  // Increments the time variables to check in the future if the respective variables can be changed
  time1++;
  time2++;
  time3++;

  // Writes to Serial for debbuging
  printf("\nSteer: %d", mydata.steer);
  printf("  Steer pulse: %d", servo_pulselength);
  printf("  Velocity:  %d", mydata.velocity);
  printf("  Motor pulse:  %d", motor_pulselength);
  printf("  Diferença: %d", time1 - previoustime1);
  printf("  Modo: %d", mode);
  printf("  Chanell: %d", radio.getChannel());
  printf(" Latitude: %d", send_data.lati);
  printf(" Longitude: %d", send_data.longi);

  delay(10);
}
