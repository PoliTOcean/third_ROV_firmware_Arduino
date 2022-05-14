#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#define _12_V_PIN 4

unsigned char power_on = 0;

// Enter a MAC address for your controller below.
byte mac[] = {
    0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x05};

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
const char *server = "10.0.0.254";
IPAddress ip_power(10,0,0,5);
const int port = 1883;

void subscribePower(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Topic: ");
  Serial.println(topic);
  char *cmd = new char[length + 1];

  Serial.print("Message: ");
  memcpy((void *)cmd, payload, length);

  /*for (int i = 0; i < length; i++)
  {
    cmd[i] = ((char *)payload)[i];
  }*/
  cmd[length] = '\0';

  if(String(topic) == "commands/"){
    if (/*payload[0] & 0x01*/ String(cmd) == "TOGGLE POWER")
    {
      if(!power_on){
        power_on = 1;
        digitalWrite(_12_V_PIN, HIGH);
        mqttClient.publish("components/", "{\"power\": \"Enabled\"}");
      }
       else{
        power_on = 0;
        digitalWrite(_12_V_PIN, LOW); 
        mqttClient.publish("components/", "{\"power\": \"Disabled\"}");
       }
    }
    else
    {
      //TODO (eventually)
    }
  }

  //newline
  Serial.println("");
}

void setup()
{
  pinMode(_12_V_PIN, OUTPUT); //Led pin

  Ethernet.init(10); // SCSn pin

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet:");
  Ethernet.begin(mac, ip_power);
 

  mqttClient.setServer(server, port);
  if (mqttClient.connect("atmega_power"))
  {
    Serial.println("Connection has been established, well done");

    mqttClient.setCallback( subscribePower );

    //subscribe to a specific topic in order to receive those messages
    mqttClient.subscribe("commands/");
  }
  else
  {
    Serial.println("Looks like the server connection failed...");
  }
}

void loop()
{
  if (mqttClient.connected()==false){
    //Serial.println("MQTT Broker connection is down");
    if (mqttClient.connect("atmega_power")) {
       //Serial.println("MQTT Broker Connection Restarted");
           mqttClient.setCallback( subscribePower );

    //subscribe to a specific topic in order to receive those messages
    mqttClient.subscribe("commands/");
    }
  }
  mqttClient.loop();
  
  delay(4000);
}
