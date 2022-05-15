#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#include <Linduino.h>

#include <LT_I2CBus.h>
#include <LT_SMBus.h>
#include <LT_SMBusARA.h>
#include <LT_SMBusBase.h>
#include <LT_SMBusGroup.h>
#include <LT_SMBusNoPec.h>
#include <LT_SMBusPec.h>
#include <LT_Wire.h>

#include <LT_FaultLog.h>
#include <LT_PMBus.h>
#include <LT_PMBusDetect.h>
#include <LT_PMBusDevice.h>
#include <LT_PMBusMath.h>
#include <LT_PMBusRail.h>
#include <LT_PMBusSpeedTest.h>

 uint8_t address1 = 32 ;
 uint8_t address2 = 36 ;
 float temperature1;
 float Iout1;
 float Vin1;
 float Vout1;
 float Iin1;

 float temperature2;
 float Iout2;
 float Vin2;
 float Vout2;
 float Iin2;
 
static LT_SMBus *smbus = new LT_SMBusNoPec();
static LT_PMBus *pmbus = new LT_PMBus(smbus);

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
  digitalWrite(_12_V_PIN, HIGH);

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

char current_packet[40];
char voltage_packet[40];

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

  Iout1 = pmbus->readIout(address1, false);
  temperature1 = pmbus->readOtemp(address1);
  Vout1 = pmbus->readVout(address1, false);
  Iout2 = pmbus->readIout(address2, false);
  temperature2 = pmbus->readOtemp(address2);
  Vout2 = pmbus->readVout(address2, false);

  sprintf(current_packet, "{\"current\": %s}", String(Iout1 + Iout2).c_str());

  mqttClient.publish("sensors/", current_packet);

  sprintf(voltage_packet, "{\"voltage\": %s}", String((Vout1 + Vout2)/2.0).c_str());

  mqttClient.publish("sensors/", voltage_packet);
  
  mqttClient.loop();
  
  delay(500);
}
