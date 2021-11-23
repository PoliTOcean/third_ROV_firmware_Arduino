#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#define LED_PIN 8

void subscribeReceiveLed(char *topic, byte *payload, unsigned int length)
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

  if (/*payload[0] & 0x01*/ String(cmd) == "on")
  {
    Serial.print(cmd);
    digitalWrite(LED_PIN, HIGH); // turn the LED on (HIGH is the voltage level)
  }
  else if (/*payload[0] & 0x01 == 0x00*/ String(cmd) == "off")
  {
    Serial.print(cmd);
    digitalWrite(LED_PIN, LOW); // turn the LED off by making the voltage LOW
  }
  else
  {
    //TODO (eventually)
  }

  //newline
  Serial.println("");
}

// Enter a MAC address for your controller below.
byte mac[] = {
    0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
const char *server = "192.168.0.108";
const int port = 1883;

void setup()
{
  pinMode(LED_PIN, OUTPUT); //Led pin

  Ethernet.init(10); // SCSn pin

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    }
    else if (Ethernet.linkStatus() == LinkOFF)
    {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true)
    {
      delay(1);
    }
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());

  mqttClient.setServer(server, port);
  if (mqttClient.connect("Arduino"))
  {
    Serial.println("Connection has been established, well done");

    mqttClient.setCallback(subscribeReceiveLed);

    //subscribe to a specific topic in order to receive those messages
    mqttClient.subscribe("led");
  }
  else
  {
    Serial.println("Looks like the server connection failed...");
  }
}

void loop()
{
  switch (Ethernet.maintain())
  {
  case 1:
    //renewed fail
    Serial.println("Error: renewed fail");
    break;

  case 2:
    //renewed success
    Serial.println("Renewed success");
    //print your local IP address:
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
    break;

  case 3:
    //rebind fail
    Serial.println("Error: rebind fail");
    break;

  case 4:
    //rebind success
    Serial.println("Rebind success");
    //print your local IP address:
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
    break;

  default:
    //nothing happened
    break;
  }

  mqttClient.loop();
  //int adcVal = analogRead(A0);

  //float voltage = adcVal * 5.0 / 1024;
  //float Rt = 10 * voltage / (5 - voltage);
  //float tempK = 1 / (log(Rt / 10) / 3950 + 1 / (273.15 + 25));
  //float tempC = tempK - 273.15;

  if (mqttClient.publish("Temperature", /*String(tempC, 3).c_str()*/ "{\"value\":\"prova JSON\"}"))
  {
    Serial.println("Publish succeded!");
  }
  else
  {
    Serial.println("Publish failed!");
  }

  delay(4000);
}
