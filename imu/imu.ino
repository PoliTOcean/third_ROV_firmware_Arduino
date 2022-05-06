#include <Wire.h>
#include "MS5837.h"
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <SparkFunLSM9DS1.h>
#include <Adafruit_BMP280.h>

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};

LSM9DS1 imu;
MS5837 sensor;    //Pressure sensor
Adafruit_BMP280 bmp; // Temperature sensor

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
const char *server = "10.0.0.254";
IPAddress ip_atmega(10,0,0,2);
const int port = 1883;
char packet[90];
char pressure_packet[50];
char temperature_packet[40];

void setup() {
  Ethernet.init(10); // SCSn pin
  Serial.begin(9600);
  
  Wire.begin();
  if (imu.begin((uint8_t)0x6a, (uint8_t)0x1c, Wire) == false){
    //Serial.println("Failed to communicate with LSM9DS1.");
    while (1);
  }

  unsigned status;
  status = bmp.begin(/*x76, 0x58*/);  //address forced to 0x77 connectin SDO to VCC
  if (!status) {
    /*Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");*/
    Serial.println("Here");
    while (1) delay(10);
  }
  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  
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
  
  // Initialize pressure sensor
  // Returns true if initialization was successful
  // We can't continue with the rest of the program unless we can initialize the sensor
  while (!sensor.init()) {
    Serial.println("Pressure sensor init failed!");
    //Serial.println("Are SDA/SCL connected correctly?");
    //Serial.println("Blue Robotics Bar30: White=SDA, Green=SCL");
    //Serial.println("\n\n\n");
    delay(5000);
  }
  sensor.setFluidDensity(997); // kg/m^3 (freshwater, 1029 for seawater)

  mqttClient.setServer(server, port);

  if (mqttClient.connect("atmega_imu")){
    Serial.println("done");
  }
  else{
    Serial.println("failed");
    // no point in carrying on, so do nothing forevermore:
    while (true);
  }
}

void loop() {
  if (mqttClient.connected()==false){
    //Serial.println("MQTT Broker connection is down");
    if (mqttClient.connect("atmega_imu")) {
       //Serial.println("MQTT Broker Connection Restarted");
    }
  }
  /*switch (Ethernet.maintain()){
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
  }*/

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

  if(mqttClient.publish("atmega_imu/imu", packet)){
    //Serial.println("Publish imu succeded!");
  }
  else{
    //Serial.println("Publish imu failed!");
  }

  // Update pressure and temperature readings
  sensor.read();

  /** Pressure returned in mbar or mbar*conversion rate.
 */
  //Serial.print("Pressure: ");
  //Serial.print(sensor.pressure());
  //Serial.println(" mbar");

  /** Depth returned in meters (valid for operation in incompressible
 *  liquids only. Uses density that is set for fresh or seawater.
 */

  //Serial.print("Depth: ");         
  //Serial.print(sensor.depth());
  //Serial.println(" m");

  sprintf(pressure_packet,
          "{\"pressure\":%s,\"depth\":%s}",
          String(sensor.pressure()).c_str(),
          String(sensor.depth()).c_str());

  Serial.println(pressure_packet);

  if(mqttClient.publish("atmega_imu/pressure", pressure_packet)){
    ///Serial.println("Publish pressure succeded!");
  }
  else{
    //Serial.println("Publish pressure failed!");
  }

  //Serial.print(F("Temperature = "));
  //Serial.print(bmp.readTemperature());
  //Serial.println(" *C");

  sprintf(temperature_packet,
          "{\"temperature\":%s}",
          String(bmp.readTemperature()).c_str());

  Serial.println(temperature_packet);

  if(mqttClient.publish("atmega_imu/temperature", temperature_packet)){
    ///Serial.println("Publish pressure succeded!");
  }
  else{
    //Serial.println("Publish pressure failed!");
  }
}
