#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Servo.h>

#define LED_PIN 8

// Enter a MAC address for your controller below.
byte mac[] = {
    0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
const char *server = "10.0.0.254";
IPAddress ip_servo(10,0,0,4);
const int port = 1883;
Servo myservo;  // create servo object to control a servo
int potpin = 0;  // analog pin used to connect the potentiometer
int val;    // variable to read the value from the analog pin


void subscribeReceivePosition(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Topic: ");
  Serial.println(topic);
  char *cmd = new char[length + 1];
  int val;

  Serial.print("Message: ");
  
  memcpy((void *)cmd, payload, length);

  cmd[length] = '\0';
  String value(cmd);
  val = value.toInt();
  Serial.print(val);
  myservo.write(val);                  // sets the servo position according to the scaled value
  delay(15);                           // waits for the servo to get there

  //newline
  Serial.println("");
}

void setup()
{
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object

  Ethernet.init(10); // SCSn pin

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  Ethernet.begin(mac, ip_servo);

  mqttClient.setServer(server, port);
  if (mqttClient.connect("Servo"))
  {
    Serial.println("Connection has been established, well done");

    mqttClient.setCallback(subscribeReceivePosition);

    //subscribe to a specific topic in order to receive those messages
    mqttClient.subscribe("servo");
  }
  else
  {
    Serial.println("Looks like the server connection failed...");
  }
}

void loop()
{
  mqttClient.loop();
  
  
  delay(4000);
}
