/*******************************************************************************
* Copyright 2016 ROBOTIS CO., LTD.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

//#include <SoftwareSerial.h>
#include <Ethernet.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define AIO_SERVER      "10.0.0.12"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "atmega_arm"

// Arm PCB v1_2560
// ###############
#include <Dynamixel2Arduino.h>
#define DXL_SERIAL   Serial
#define DEBUG_SERIAL Serial
const uint8_t DXL_DIR_PIN = 30;

// Arm PCB v2_328
// ##############
//#include <politocean_Dynamixel2Arduino.h>
//SoftwareSerial DXL_SERIAL = SoftwareSerial(2, 3); // pin fisici -> Rx = 32 (PD2), Tx = 1 (PD3); rikyru pinout corretto -> Rx = 3, Tx = 2
//#define DEBUG_SERIAL Serial
//const uint8_t DXL_DIR_PIN = A1; // pin fisico -> 24 (PC1)


// variabili del motore
const uint8_t DXL_ID = 1;
const float DXL_PROTOCOL_VERSION = 1.0;

Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);

//This namespace is required to use Control table item names
using namespace ControlTableItem;

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x06};
EthernetClient client;
IPAddress ip_arm(10,0,0,6);

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME/*, AIO_KEY*/);
Adafruit_MQTT_Subscribe opennipper = Adafruit_MQTT_Subscribe(&mqtt, "commands/");

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 0.015 seconds...");
       mqtt.disconnect();
       delay(15);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}

void setup() {
  // put your setup code here, to run once:
  Ethernet.init(10); // SCSn pin

  // start the Ethernet connection:
  Ethernet.begin(mac, ip_arm);
  delay(1000);
  
  mqtt.subscribe(&opennipper);
  // Use UART port of DYNAMIXEL Shield to debug.

  // Set Port baudrate to 57600bps. This has to match with DYNAMIXEL baudrate.
  dxl.begin(57600);
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  // Get DYNAMIXEL information
  dxl.ping(DXL_ID);

  // Turn off torque when configuring items in EEPROM area
   // dxl.torqueOff(DXL_ID);

  /*uint8_t new_id = 2;
    if(dxl.setID(DXL_ID, new_id) == true){
      //present_id = new_id;
      DEBUG_SERIAL.print("ID has been successfully changed to ");
      DEBUG_SERIAL.println(new_id);
    }
  */
  // Turn off torque when configuring items in EEPROM area
  dxl.torqueOff(DXL_ID);
  dxl.setOperatingMode(DXL_ID, OP_POSITION);
  dxl.torqueOn(DXL_ID);

  // Limit the maximum velocity in Position Control Mode. Use 0 for Max speed
  dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID, 30);

}

void loop() {  
  // Please refer to e-Manual(http://emanual.robotis.com/docs/en/parts/interface/dynamixel_shield/) for available range of value. 
  // Set Goal Position in RAW value

  MQTT_connect();

  dxl.setGoalPosition(DXL_ID, 45, UNIT_DEGREE);

  delay(2000);

  dxl.setGoalPosition(DXL_ID, 0, UNIT_DEGREE);

  delay(2000);

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(1000))) {
    if (subscription == &opennipper) {
      int dim = strlen((char *)opennipper.lastread);
      char cmd[dim+1];
      memcpy(cmd, (char *)opennipper.lastread, dim);
      cmd[dim]='\0';
      Serial.println(cmd);
      if(String(cmd) == "CW"){
        // Set Goal Position in DEGREE value
        dxl.setGoalPosition(DXL_ID, 45, UNIT_DEGREE);
      }
      else if(String(cmd) == "CCW"){
        // Set Goal Position in DEGREE value
        dxl.setGoalPosition(DXL_ID, 0, UNIT_DEGREE);
      }
     }
  }

  delay(15);
}
