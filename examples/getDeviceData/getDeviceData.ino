#include <SPIOT.h>
SPIOT iot(115200); // Default RX pin=2, TX pin = 3, initialize an instance of the class


void setup() {
  Serial.begin(115200);
  iot.begin();
  delay(500);
  
  //iot.flashDevice(DOOR, 1);

  //iot.removeALlDevices();
  //delay(5000);
  //iot.removeGroupDevices(DOOR);
  //iot.removeTheDevice(DOOR, 1);

  //iot.removeTheDevice(TH, 4);
}

boolean onoff = false;
unsigned int ii = 0;

void loop() {
  
  
  // 1: PIR, 2: DOOR, 3:TH-T, 4: TH-H


  if(ii > 90) {
    if(onoff==true) {
      iot.setSmartPlug(0, false);
      onoff=false;
    }else{
      iot.setSmartPlug(0, true);
      onoff=true;
    }
    ii = 0;    
  }
  ii += 1;

 
for (int i=0; i < 4; i++){
  Serial.println("------------------------------------------------------------------------------"); 
  Serial.print("#"); Serial.print(i); Serial.print(" PIR: "); Serial.print(iot.getDeviceData(PIR, i)); Serial.print(" ,Power alarm: "); Serial.print(iot.devicePowerAlarm(PIR, i)); Serial.print(" ,last: "); Serial.println(iot.lastUpdateTime(PIR, i));
  Serial.print("#"); Serial.print(i); Serial.print(" DOOR: "); Serial.print(iot.getDeviceData(DOOR, i)); Serial.print(" ,Power alarm: "); Serial.print(iot.devicePowerAlarm(DOOR, i)); Serial.print(" ,last: "); Serial.println(iot.lastUpdateTime(DOOR, i));
  Serial.print("#"); Serial.print(i); Serial.print(" TH_T: "); Serial.print(iot.getDeviceData(TH_T, i)); Serial.print(" ,Power alarm: "); Serial.print(iot.devicePowerAlarm(TH_T, i)); Serial.print(" ,last: "); Serial.println(iot.lastUpdateTime(TH_T, i));
  Serial.print("#"); Serial.print(i); Serial.print(" TH_H: "); Serial.print(iot.getDeviceData(TH_H, i)); Serial.print(" ,Power alarm: "); Serial.print(iot.devicePowerAlarm(TH_H, i)); Serial.print(" ,last: "); Serial.println(iot.lastUpdateTime(TH_H, i));
}
  Serial.print("#0 PLUG: "); Serial.print(iot.getDeviceData(PLUG, 0)); Serial.print(" ,last: "); Serial.println(iot.lastUpdateTime(PLUG, 0));
  Serial.println("===============================================================================");  

/*
  String rtnValue = iot.readRaw();
  if(rtnValue.length()>0) Serial.println(rtnValue);
*/
  //iot.update();
  delay(5000);

}