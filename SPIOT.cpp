#include "SPIOT.h"
#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include <avr/sleep.h>

/* you can adjust the value below base on your condition
==========================================================*/
#define rxPIN 2
#define txPIN 3
#define maxDeviceNum 6  // max device number is 6
#define UPDATERATE	60   //rate for TX Dongle query , unit is mseconds.
#define BUFFERSIZE	90
/* Don't change any value below!
==========================================================*/
#define PIR		1
#define DOOR	2
#define TH		3
#define TH_T	3
#define TH_H	4
#define PLUG	5

byte deviceDataReturn[5] = {};   //Dongle回傳的為6個HEX碼

//第一層為devie ID號碼, 第2層: 0->status, 1->value1, 2-> value2
unsigned int pir[maxDeviceNum-1][1] = {};  
unsigned int door[maxDeviceNum-1][1] = {};
unsigned int th_t[maxDeviceNum-1][1] = {};
unsigned int th_h[maxDeviceNum-1][1] = {};
unsigned int plug[maxDeviceNum-1][1] = {};

//上次提供data的秒數, 計算是否離線用
unsigned long pirExists[maxDeviceNum-1] = {};
unsigned long doorExists[maxDeviceNum-1] = {};
unsigned long thExists[maxDeviceNum-1] = {};
unsigned long plugExists[maxDeviceNum-1] = {};

//Power電力警告
bool pirPower[maxDeviceNum-1] = {};
bool doorPower[maxDeviceNum-1] = {};
bool thPower[maxDeviceNum-1] = {};
bool plugPower[maxDeviceNum-1] = {};

unsigned int bufSerial[BUFFERSIZE];

SoftwareSerial spSerial(rxPIN, txPIN); // RX, TX


String toHEX(int valueRead) {
	String hexRead = String(valueRead, HEX);  //hexRead 為HEX 16進位
	if(hexRead.length()==1) hexRead = "0" + hexRead;
	return "0x"+hexRead;
}

String getReturnData() {
  String hexRead = "";
  String rtnString = "";

  while(spSerial.available()) {

	hexRead = String(spSerial.read(), HEX);
	if(hexRead.length()==1) hexRead = "0" + hexRead;
    rtnString += ("0x" + hexRead);
    if(spSerial.available()) rtnString += "-";
  }
  return rtnString;
}

void update2DB(unsigned int v0, unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4, unsigned int v5) {
	unsigned int deviceNum;
	
	Serial.print("update2DB received: "); Serial.print(toHEX(v0)); Serial.print("-"); Serial.print(toHEX(v1)); Serial.print("-"); Serial.print(toHEX(v2)); Serial.print("-");
	Serial.print(toHEX(v3)); Serial.print("-"); Serial.print(toHEX(v4)); Serial.print("-"); Serial.print(toHEX(v5)); Serial.println("");
				
	//if((v4==255 && v5 ==85) || (v4 ==85)) {
	//Serial.println("Accept!");
	switch (v0) {
		case 6:  // 十進位6 為0x06, 表回報資料
			
			if(v1>=16 && v1<(16+maxDeviceNum)) {   //若十進位介於16~32, 即0x10~ , 表示它是SmartPlug	
				deviceNum = v1-16;
				if(v2==1 or v2==0) plug[deviceNum][0] = v2;  //將值放入對應的PLUG device	
				Serial.print("TEST: save PLUG #"); Serial.print(deviceNum); Serial.print(" = ");Serial.println(toHEX(v2)); 
				plugExists[deviceNum] = millis()/1000;

			}else if(v1>=32 && v1<(32+maxDeviceNum)) {   //若十進位介於32~48, 即0x20~ , 表示它是PIR	
				deviceNum = v1-32;
				if(v2==1 or v2==0) pir[deviceNum][0] = v2;  //將值放入對應的PIR device	
				Serial.print("TEST: save PIR #"); Serial.print(deviceNum); Serial.print(" = ");Serial.println(toHEX(v2)); 
				pirExists[deviceNum] = millis()/1000;		

			}else if(v1>=48 && v1<(48+maxDeviceNum)) {   //若十進位介於48~64, 即0x30~ , 表示它是DOOR
				deviceNum = v1-48;
				if(v2==1 or v2==0) door[deviceNum][0] = v2;  //DOOR device	
				Serial.print("TEST: save DOOR #"); Serial.print(deviceNum); Serial.print(" = ");Serial.println(toHEX(v2)); 
				doorExists[deviceNum] = millis()/1000;

			}else if(v1>=64 && v1<(64+maxDeviceNum)) {   //若十進位介於64~80, 即0x50~ , 表示它是TH
				deviceNum = v1-64;
				if(v2<100) th_t[deviceNum][0] = v2;  //TH device					
				if(v3<100) th_h[deviceNum][0] = v3;  //TH device
				Serial.print("TEST: save TH #"); Serial.print(deviceNum); Serial.print(" = ");Serial.print(toHEX(v2)); Serial.print(" & "); Serial.println(toHEX(v3));
				thExists[deviceNum] = millis()/1000;

			}
			break;
			
		case 9:  // 十進位9 , 開頭為0x09表警告訊息, 但沒設alarm, 所以如果有收到0x09, 表示電力不夠了.
			//Serial.print("Power data received: "); Serial.print(v0); Serial.print("-"); Serial.print(v1); Serial.print("-"); Serial.print(v2); Serial.print("-");
			//Serial.print(v3); Serial.print("-"); Serial.print(v4); Serial.print("-"); Serial.print(v5); Serial.println("");
				
			if(v1>=16 && v1<(16+maxDeviceNum)) {   //若十進位介於16~32, 即0x20~ , 表示它是SmartPlug	, 但smartplug沒有電池電力問題
				deviceNum = v1-16;
				plugPower[deviceNum] = 1;  //true 1 表示電力不夠 
					
			}else if(v1>=32 && v1<(32+maxDeviceNum)) {   //若十進位介於32~48, 即0x20~ , 表示它是PIR	
				deviceNum = v1-32;
				pirPower[deviceNum] = 1;  //true 1 表示電力不夠	
					
			}else if(v1>=48 && v1<(48+maxDeviceNum)) {   //若十進位介於48~64, 即0x30~ , 表示它是DOOR
				deviceNum = v1-48;
				doorPower[deviceNum] = 1;  //true 1 表示電力不夠	
					
			}else if(v1>=64 && v1<(64+maxDeviceNum)) {   //若十進位介於64~80, 即0x50~ , 表示它是TH
				deviceNum = v1-64;
				thPower[deviceNum] = 1;  //true 1 表示電力不夠	
			}
			break;
		
		

	}
}

void readBuffer() {

	unsigned int nowIndex = 0;
	unsigned int ii = 0;
	
	while(ii+5 < BUFFERSIZE) {
		if(bufSerial[ii] == 6) {
			//Serial.print(ii); Serial.println("[readBuffer] then updateDB");
			if((ii+5)<=BUFFERSIZE) {
				//if( ((bufSerial[ii+1]>9 && bufSerial[ii+1]<80) && ( (bufSerial[ii+4]==255 && bufSerial[ii+5]==85) )) ) {  //;第二欄介於PLUG 0x10與TH 0x40+id之間, 且最後兩位HEX是0xff, 0x55 或 0x00和0x55
			if( ((bufSerial[ii+1]>9 && bufSerial[ii+1]<80) && ( bufSerial[ii+4]==255 || bufSerial[ii+4]==85)) ) {
					
					//Serial.println("");
					//Serial.print("readBuffer:"); Serial.print(ii); Serial.print("-> ");Serial.print(toHEX(bufSerial[ii]));Serial.print("-");Serial.print(toHEX(bufSerial[ii+1]));
					//Serial.print("-");Serial.print(toHEX(bufSerial[ii+2]));Serial.print("-");
					//Serial.print(toHEX(bufSerial[ii+3]));Serial.print("-");Serial.print(toHEX(bufSerial[ii+4]));Serial.print("-");Serial.print(toHEX(bufSerial[ii+5]));Serial.println("-");
					
					//if((bufSerial[ii+3]==0 && bufSerial[ii+4]==85)) {
					//if((bufSerial[ii+5]==6) || (bufSerial[ii+5]==9)) {
					//if(bufSerial[ii+5]==6) {
					//	update2DB(bufSerial[ii], bufSerial[ii+1], bufSerial[ii+2], bufSerial[ii+3], bufSerial[ii+4], 0);
					//	ii = ii+5;
					//}else{
					if(bufSerial[ii+5]==6 || bufSerial[ii+5]==0) {
						update2DB(bufSerial[ii], bufSerial[ii+1], bufSerial[ii+2], bufSerial[ii+3], bufSerial[ii+4], 0);
						ii = ii+5;
					}else{
						update2DB(bufSerial[ii], bufSerial[ii+1], bufSerial[ii+2], bufSerial[ii+3], bufSerial[ii+4], bufSerial[ii+5]);
						ii = ii+6;
					}
					
					//}
				}else{
					ii++;
				}
			}else{
				ii++;
			}
		}else if(bufSerial[ii] == 9) {
			if((ii+5)<BUFFERSIZE) {
				if((bufSerial[ii+1]>31 && bufSerial[ii+1]<80) && bufSerial[ii+4]==255 && bufSerial[ii+5]==85) {  //;第二欄介於PIR 0x20與TH 0x40+id之間, 且最後兩位HEX是0xff, 0x55
					update2DB(bufSerial[ii], bufSerial[ii+1], bufSerial[ii+2], bufSerial[ii+3], bufSerial[ii+4], bufSerial[ii+5]);
					ii = ii+6;
				}else{
					ii++;
				}
			}else{
				ii++;
			}
		}else{
			ii++;
		}
	}

}
/*
void getSerialData() {
	String rtnString = "";
	int valueRead[5] = {};  

  
	while(spSerial.available()) {
		String hexString = "";		
		valueRead[5] = {};
		
		valueRead[0] = (spSerial.read());  //valueRead 為10進位
		hexString = toHEX(valueRead[0]);
		
		if(valueRead[0]==6 || valueRead[0]==9) {    //0x06 , 0x09
		
			if(spSerial.available()) {
				valueRead[1] = (spSerial.read());
				hexString = hexString + toHEX(valueRead[1]);
				
				if(spSerial.available()) {
					valueRead[2] = (spSerial.read());
					hexString = hexString + toHEX(valueRead[2]);
					if(spSerial.available()) {
						valueRead[3] = (spSerial.read());
						hexString = hexString + toHEX(valueRead[3]);
						
						if(spSerial.available()) {
							valueRead[4] = (spSerial.read());
							hexString = hexString + toHEX(valueRead[4]);
							if(valueRead[4]==255) {
							
								if(spSerial.available()) {
									valueRead[5] = (spSerial.read());
									hexString = hexString + toHEX(valueRead[5]);
									
									update2DB(valueRead[0], valueRead[1], valueRead[2], valueRead[3], valueRead[4], valueRead[5], hexString);
								}
							}
						}
					}
				}
			}		
		}
	}

}
*/
void noEncrypt() {
  String rtnString = "";
  byte message[] = {0x99, 0x00, 0x00, 0x00, 0x00, 0x00 };
  spSerial.write(message, sizeof(message));
  delay(50);
}

void getBindInformation() {
  String rtnString = "";	
  byte message[] = {0x10, 0x00, 0x00, 0x00, 0x00, 0x00 };
  spSerial.write(message, sizeof(message));
  //delay(50);
}

void pushDevice() {
  byte message[] = {0x08, 0x00, 0x00, 0x00, 0x00, 0x00 };
  spSerial.write(message, sizeof(message));
}

byte id2DeviceGroup(unsigned int id) {
	byte rtnValue = "";
	
	switch (id) {
		case 1:
			rtnValue = 0x20;  //PIR
			break;
		case 2:
			rtnValue = 0x30;  //DOOR
			break;	
		case 3:
			rtnValue = 0x40;  //TH
			break;
		case 4:
			rtnValue = 0x40;  //TH
			break;
		case 5:
			rtnValue = 0x10;  //PLUG
			break;
	}
	
	return rtnValue;
}

void serialToBuffer(){	
	//clear array
	memset(bufSerial, 0, sizeof(bufSerial));
	//for(int n = 0;n<sizeof(bufSerial);n++)  {  
	//	bufSerial[n] = 0; // copying data from buffer  
	//}  
		
	unsigned int i = 0;
	Serial.println("[serialToBuffer]");
	while(spSerial.available()) {
		bufSerial[i] = spSerial.read();	
		Serial.print(toHEX(bufSerial[i])); Serial.print("-");
		i++;
	}
	Serial.println("");
	
	readBuffer();
}

void updateStatus(){	
	if(spSerial.available()) {        
		serialToBuffer();   
		
	}else{
		pushDevice();
	}
  
}



//<<constructor>>
SPIOT::SPIOT(float baudrate) {
	spSerial.begin(baudrate);

}

//<<destructor>>
SPIOT::~SPIOT(){/*nothing to destruct*/}

void SPIOT::begin() {
	noEncrypt();
	delay(50);
	getBindInformation();
	MsTimer2::set(UPDATERATE, updateStatus);
	MsTimer2::start();
	
	//set_sleep_mode(SLEEP_MODE_IDLE);
	//sleep_enable();
}


unsigned int SPIOT::getDeviceData(unsigned int dType, unsigned int idNum) {
	unsigned int rtnValue;

	
	switch (dType) {
		case 1:
			rtnValue = pir[idNum][0];
			if(rtnValue>0) {
				rtnValue = 1;
			}else{
				rtnValue = 0;
			}
			break;
		case 2:
			rtnValue = door[idNum][0];	
			if(rtnValue>0) {
				rtnValue = 1;
			}else{
				rtnValue = 0;
			}
			break;
		case 3:
			rtnValue = th_t[idNum][0];	
			//Serial.print("Get ID #"); Serial.print(idNum); Serial.print("for TH_T:"); Serial.println(rtnValue);
			break;
		case 4:
			rtnValue = th_h[idNum][0];
			//Serial.print("Get ID #"); Serial.print(idNum); Serial.print("for TH_H:"); Serial.println(rtnValue);
			break;
		case 5:
			rtnValue = plug[idNum][0];
			if(rtnValue>0) {
				rtnValue = 1;
			}else{
				rtnValue = 0;
			}
			break;
	}
	
	return rtnValue;
}

bool SPIOT::devicePowerAlarm(unsigned int dType, unsigned int idNum) {
	unsigned int rtnValue;
	
	switch (dType) {
		case 1:
			rtnValue = pirPower[idNum];
			break;
		case 2:
			rtnValue = doorPower[idNum];	
			break;
		case 3:
			rtnValue = thPower[idNum];	
			break;
		case 4:
			rtnValue = thPower[idNum];
			break;
		case 5:
			rtnValue = plugPower[idNum];
			break;
	}
	
	return rtnValue;
}

String SPIOT::readRaw(){	
	String rtnString = "";
  
	if(spSerial.available()) {        
		rtnString = getReturnData();   
		
	}else{
		pushDevice();
		//delay(50);
	}
  
	return rtnString;
}

void SPIOT::removeALlDevices() {
	//String rtnString = "";
	byte message[] = {0x0F, 0xF0, 0x5A, 0xA5, 0x00, 0x00 };
	spSerial.write(message, sizeof(message));
	//delay(50);
}

void SPIOT::removeGroupDevices(unsigned int deviceType) {
	byte hexGroup = id2DeviceGroup(deviceType);	
	//String rtnString = "";
	
	byte message[] = {0x0E, hexGroup, 0x5A, 0xA5, 0x00, 0x00 };
	spSerial.write(message, sizeof(message));
	//delay(50);
	
}

void SPIOT::removeTheDevice(unsigned int deviceType, unsigned int ID) {
	byte hexNum = id2DeviceGroup(deviceType) + ID;	
	//String rtnString = "";
	
	byte message[] = {0x0D, hexNum, 0x5A, 0xA5, 0x00, 0x00 };
	spSerial.write(message, sizeof(message));
	//delay(50);
}

void SPIOT::flashDevice(unsigned int deviceType, unsigned int ID) {
	byte hexNum = id2DeviceGroup(deviceType) + ID;	
	
	byte message[] = {0x17, hexNum, 0x00, 0x00, 0x00, 0x00 };
	spSerial.write(message, sizeof(message));
}

void SPIOT::setSmartPlug(unsigned int ID, bool onoff) {
	byte hexNum = id2DeviceGroup(PLUG) + ID;	
	byte setValue;

	if(onoff==true) {
		setValue = 0x01;
	} else {
		setValue = 0x00;
	}
	
	byte message[] = {0x0A, hexNum, setValue, 0x00, 0x00, 0x00 };
	spSerial.write(message, sizeof(message));
	//Serial.print("PLUG-->"); Serial.print(toHEX(0x0A)); Serial.print("-"); Serial.print(toHEX(hexNum)); Serial.print("-"); Serial.print(toHEX(setValue)); Serial.print("-"); Serial.println(toHEX(0x00));
}

unsigned int SPIOT::lastUpdateTime(unsigned int dType, unsigned int idNum) {
	unsigned long rtnValue;
	unsigned long nowSeconds = (millis()/1000);
	
	switch (dType) {
		case 1:
			rtnValue = nowSeconds - pirExists[idNum];
			break;
		case 2:
			rtnValue = nowSeconds - doorExists[idNum];	
			break;
		case 3:
			rtnValue = nowSeconds - thExists[idNum];	
			break;
		case 4:
			rtnValue = nowSeconds - thExists[idNum];
			break;
		case 5:
			rtnValue = nowSeconds - plugExists[idNum];
			break;
	}
	
	return rtnValue;
}