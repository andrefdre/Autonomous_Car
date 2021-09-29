#include <SoftwareSerial.h>

SoftwareSerial BTControl(10, 11); // RX | TX

int velocity;
int steer;
int Data[2];
void setup() {
  pinMode(A0,INPUT);  // Velocity
  pinMode(A1,INPUT); // Steer
  Serial.begin(9600);
  BTControl.begin(9600);
}

void loop() {
  // Read The values
  velocity=analogRead(A0);
  steer=analogRead(A1);
  // Work the values read before
  velocity=map(velocity,0,1023,0,255);
  steer=map(steer,0,1023,0,180);
  Serial.println(velocity);
  Serial.println(steer);
  // Send the values trough the bluetooth to the car
  Data[0]=velocity;
  Data[1]=steer;
  BTControl.write(Data[0]);
  BTControl.write(Data[1]);
}
