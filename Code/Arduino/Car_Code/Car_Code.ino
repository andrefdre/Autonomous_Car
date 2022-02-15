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

// Library
#include <LibPrintf.h> //Includes the Libprintf library to use the printf function to write to serial
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <SPI.h>      //Required library for RF24 communication
#include <nRF24L01.h> //Includes the library for the communication with radio waves
#include <RF24.h>     //Includes the library for the communication with radio waves

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// RF24
const byte slaveAddress[6] = "00001"; // Sets the address for communitating with the controller(must be the same in the controller)
RF24 radio(7, 8);                     // CE Pin, CSN Pin

// Structure with the received information
struct RECEIVE_DATA_STRUCTURE
{
  int velocity;
  int steer;
  int light;
  int slow;
  int alone;
  int vmais;
};
RECEIVE_DATA_STRUCTURE mydata; // Atributes a easier name to be used in the structure

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
int chanell;

void setup()
{
  // Serial communication
  Serial.begin(Baud); // Initializes Serial communication with the computer for debugging

  // HCPCA9685
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
  if (!radio.begin())
  {
    Serial.println(F("radio hardware is not responding!!"));
    while (1)
    {
    } // hold in infinite loop
  }
  radio.openReadingPipe(0, slaveAddress); // Opens a pipe to communicate(must be the same as the controller) with the controller
  radio.startListening();                 // Sets the RF24 driver to listening for new information
  chanell = radio.getChannel();

  // Arduino setup Check
  printf("Arduino Initialized"); // Checks if the arduino setups correctly
}

void loop()
{
  // Checks if the arduino is still receiving information through radio waves, if not centers the wheels and puts it in neutral
  if ((time1 - previoustime1) >= 20)
  {
    motor_pulselength = map(90, 0, 180, MOTORMiN, MOTORMAX);     // Maps the the angle value to PWM microseconds
    pwm.writeMicroseconds(motorPin, motor_pulselength);          // Writes to the motor to neutral position
    servo_pulselength = map(center, 0, 270, SERVOMIN, SERVOMAX); // Maps the the angle value to PWM microseconds
    pwm.writeMicroseconds(servoPin, servo_pulselength);          // Writes for the servo to center the wheels
    mydata.steer = center;
    mydata.velocity = 90;
    time4++;
    radio.powerDown();
    radio.setChannel(chanell);
    delay(20);
  }

if (time4>=250)
{
  previoustime1 = time1;
  radio.powerUp();
  radio.openReadingPipe(0, slaveAddress); // Opens a pipe to communicate(must be the same as the controller) with the controller
  radio.startListening();  

  

}

  else
  {
    // Nrf24
    if (radio.available())
    {                                      // Looking for the data.
      radio.read(&mydata, sizeof(mydata)); // Reading the data
      previoustime1 = time1;               // Reset the time variable
    }
  }
  // Checks the received information is within accepted values and then sends it to the servo driver
  if (mydata.steer < center - steervalue || mydata.steer > center + steervalue || mydata.velocity > 90 + mydata.vmais || mydata.velocity < 90 - mydata.vmais)
  {
    motor_pulselength = map(90, 0, 180, MOTORMiN, MOTORMAX);     // Maps the the angle value to PWM microseconds
    pwm.writeMicroseconds(motorPin, motor_pulselength);          // Writes to the motor to neutral position
    servo_pulselength = map(center, 0, 270, SERVOMIN, SERVOMAX); // Maps the the angle value to PWM microseconds
    pwm.writeMicroseconds(servoPin, servo_pulselength);          // Writes for the servo to center the wheels
  }
  else
  {
    motor_pulselength = map(mydata.velocity, 0, 180, MOTORMiN, MOTORMAX); // Maps the the angle value to PWM microseconds
    pwm.writeMicroseconds(motorPin, motor_pulselength);                   // Writes the mapped value to change the motor speed
    servo_pulselength = map(mydata.steer, 0, 270, SERVOMIN, SERVOMAX);    // Maps the the angle value to PWM microseconds
    pwm.writeMicroseconds(servoPin, servo_pulselength);                   // Writes the mapped value to steer the wheels
  }

  // Checks if the button was pressed recently, if not it will change its output
  // Changes the light output
  if (time3 - previoustime >= 10)
  {
    if (mydata.light == 1)
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
    if (mydata.alone == 1)
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

  // Increments the time variables to check in the future if the respective variables can be changed

  time1++;
  time2++;
  time3++;

  // Writes to Serial for debbuging
  Serial.print("\nSteer:  ");
  Serial.print(mydata.steer);
  Serial.print("  Steer pulse: ");
  Serial.print(servo_pulselength);
  Serial.print("  Velocity:  ");
  Serial.print(mydata.velocity);
  Serial.print("  Motor pulse:  ");
  Serial.print(motor_pulselength);
  Serial.print("  Diferença: ");
  Serial.print(time1 - previoustime1);
  Serial.print("  Modo: ");
  Serial.print(mode);
  Serial.print("  Chanell: ");
  Serial.print(radio.getChannel());

  delay(10);
}
