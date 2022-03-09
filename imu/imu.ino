#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <SparkFunLSM9DS1.h>

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
LSM9DS1 imu;
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
const char *server = "10.0.0.254";
IPAddress ip_atmega(10,0,0,2);
const int port = 1883;
char packet[90];

void setup() {
  Ethernet.init(10); // SCSn pin
  Serial.begin(9600);

  Wire.begin();
  if (imu.begin((uint8_t)0x6a, (uint8_t)0x1c, Wire) == false){
    Serial.println("Failed to communicate with LSM9DS1.");
    while (1);
  }
  
  //Serial.print("Initialize Ethernet with DHCP:\n");
  Ethernet.begin(mac, ip_atmega);
    /*Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware){
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    }
    else if (Ethernet.linkStatus() == LinkOFF){
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true);
  }*/

  // print your local IP address:
  //Serial.println(Ethernet.localIP());

  mqttClient.setServer(server, port);
  if (mqttClient.connect("atmega_imu")){
    Serial.println("Connection has been established, well done");
  }
  else{
    Serial.println("Looks like the server connection failed...");
    // no point in carrying on, so do nothing forevermore:
    //while (true);
  }
}

void loop() {
  if (mqttClient.connected()==false){
    //Serial.println("MQTT Broker connection is down");
    if (mqttClient.connect("atmega_imu")) {
       //Serial.println("MQTT Broker Connection Restarted");
    }
  }
  switch (Ethernet.maintain()){
  case 1:
    //renewed fail
    //Serial.println("Error: renewed fail");
    break;

  case 2:
    //renewed success
    //Serial.println("Renewed success");
    //print your local IP address:
    //Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
    break;

  case 3:
    //rebind fail
    //Serial.println("Error: rebind fail");
    break;

  case 4:
    //rebind success
    //Serial.println("Rebind success");
    //print your local IP address:
    //Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
    break;

  default:
    //nothing happened
    break;
  }

  if(imu.accelAvailable()){
    imu.readAccel();
  }
  if(imu.gyroAvailable()){
    imu.readGyro();
  }
  if (imu.magAvailable()){
    imu.readMag();
  }
  delay(500);
  
  sprintf(packet, 
    "{\"ax\":%s,\"ay\":%s,\"az\":%s,\"gx\":%s,\"gy\":%s,\"gz\":%s,\"mx\":%s\"my\":%s,\"mz\":%s}",
    String(imu.calcAccel(imu.ax)).c_str(),
    String(imu.calcAccel(imu.ay)).c_str(),
    String(imu.calcAccel(imu.az)).c_str(),
    String(imu.calcGyro(imu.gx)).c_str(),
    String(imu.calcGyro(imu.gy)).c_str(),
    String(imu.calcGyro(imu.gz)).c_str(),
    String(imu.calcMag(imu.mx)).c_str(),
    String(imu.calcMag(imu.my)).c_str(),
    String(imu.calcMag(imu.mz)).c_str());
  Serial.println(packet);

  if(mqttClient.publish("imu", packet)){
    Serial.println("Publish succeded!");
  }
  else{
    Serial.println("Publish failed!");
  }
}
