//#include <Servo.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
//#include <AccelStepper.h>
#define STEPS 400
#define pin_step 4
#define pin_dir 5
//Stepper stepper(STEPS, pin_dir, pin_step);
#define motorInterfaceType 1

// Create a new instance of the AccelStepper class:
//AccelStepper stepper = AccelStepper(motorInterfaceType, pin_step, pin_dir);
byte mac[] = {
    0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x06};
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
const char *server = "10.0.0.254";
IPAddress ip_arm(10,0,0,6);
const int port = 1883;

void subscribeArm(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Topic: ");
  Serial.println(topic);
  char *cmd = new char[length + 1];

  //Serial.print("Message: ");
  memcpy((void *)cmd, payload, length);

  cmd[length] = '\0';

  /*if(String(topic) == "wrist"){
    stepper.setCurrentPosition(0);
    // Run the motor forward at 200 steps/second until the motor reaches 400 steps (2 revolutions):
    while(stepper.currentPosition() != 400)
    {
      stepper.setSpeed(400);
      stepper.runSpeed();
    }
  }*/

  if(String(topic) == "commands/"){
    if(String(cmd) == "OPEN NIPPER"){
      Serial.println(String(cmd));
      digitalWrite(7, HIGH);
      analogWrite(6, 127);
    }
    else if(String(cmd) == "STOP NIPPER"){
      analogWrite(6, 0);
    }
    else if(String(cmd) == "CLOSE NIPPER"){
      digitalWrite(7, LOW);   //set direction for actuator
      analogWrite(6, 127);    //move actuator
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  //servo.attach(6);
  Ethernet.init(10); // SCSn pin
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);
  // Set the maximum speed in steps per second:
  //stepper.setMaxSpeed(1000);
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);  //disable stepper

  // start the Ethernet connection:
  Ethernet.begin(mac, ip_arm);

  mqttClient.setServer(server, port);
  if (mqttClient.connect("atmega_arm"))
  {
    Serial.println("Connection has been established, well done");

    mqttClient.setCallback(subscribeArm);

    //subscribe to a specific topic in order to receive those messages
    mqttClient.subscribe("commands/");
  }
  else
  {
    Serial.println("Looks like the server connection failed...");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (mqttClient.connected()==false){
    //Serial.println("MQTT Broker connection is down");
    if (mqttClient.connect("atmega_arm")) {
       //Serial.println("MQTT Broker Connection Restarted");
       mqttClient.setCallback(subscribeArm);
       mqttClient.subscribe("commands/");
    }
  }
  mqttClient.loop();

  delay(15);
}
