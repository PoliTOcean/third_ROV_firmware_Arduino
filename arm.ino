#include <Servo.h>
#include <AccelStepper.h>
#define STEPS 400
#define pin_step 4
#define pin_dir 5
//Stepper stepper(STEPS, pin_dir, pin_step);
#define motorInterfaceType 1

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(motorInterfaceType, pin_step, pin_dir);

void setup() {
  // put your setup code here, to run once:
  //servo.attach(6);
  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);
  // Set the maximum speed in steps per second:
  stepper.setMaxSpeed(1000);
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);  //disable stepper
}

void loop() {
  // put your main code here, to run repeatedly:
    // Set the current position to 0:
  stepper.setCurrentPosition(0);
  // Run the motor forward at 200 steps/second until the motor reaches 400 steps (2 revolutions):
  while(stepper.currentPosition() != 400)
  {
    stepper.setSpeed(400);
    stepper.runSpeed();
  }

  delay(1000);

  // Reset the position to 0:
  stepper.setCurrentPosition(0);

  // Run the motor backwards at 600 steps/second until the motor reaches -200 steps (1 revolution):
  while(stepper.currentPosition() != -200) 
  {
    stepper.setSpeed(-600);
    stepper.runSpeed();
  }

  delay(1000);

  // Reset the position to 0:
  stepper.setCurrentPosition(0);

  // Run the motor forward at 400 steps/second until the motor reaches 600 steps (3 revolutions):
  while(stepper.currentPosition() != 600)
  {
    stepper.setSpeed(400);
    stepper.runSpeed();
  }

  delay(3000);
  
  digitalWrite(7, LOW);   //set direction for actuator
  analogWrite(6, 127);	  //move actuator
  delay(2000);
  digitalWrite(7, HIGH);
  analogWrite(6, 127);
  delay(2000);
}
