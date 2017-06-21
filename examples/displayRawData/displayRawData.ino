#include <SPIOT.h>

SPIOT iot(115200); // Default RX pin=2, TX pin = 3, initialize an instance of the class


void setup() {
  Serial.begin(115200);
  iot.begin();
  delay(5000);
  //iot.removeALlDevices();
}

void loop() {
  iot.updateStatus();
  // 1: PIR, 2: DOOR, 3:TH-T, 4: TH-H
  Serial.print("PIR: "); Serial.println(iot.getDeviceData(1, 0));
  Serial.print("DOOR: "); Serial.println(iot.getDeviceData(2, 0));
  Serial.print("TH-T: "); Serial.println(iot.getDeviceData(3, 0));
  Serial.print("TH-H: "); Serial.println(iot.getDeviceData(4, 0));

  delay(1000);
  //String rtnValue = iot.readRaw();
  //if(rtnValue.length()>0) Serial.println(rtnValue);
  
  /*
  int sizeValue = rtnValue.length();
  if(sizeValue>0) {
    Serial.println(rtnValue);
    Serial.println();
  }
  */
}