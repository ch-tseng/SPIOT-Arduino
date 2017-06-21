#ifndef SPIOT_H
#define SPIOT_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define PIR		1
#define DOOR	2
#define TH		3
#define TH_T	3
#define TH_H	4
#define PLUG	5

class SPIOT {
public:
        SPIOT(float baudrate);
        ~SPIOT();
		void begin();
        String readRaw();
		
		void flashDevice(unsigned int deviceType, unsigned int ID);
		
		void removeALlDevices();
		void removeGroupDevices(unsigned int deviceType);
		void removeTheDevice(unsigned int deviceType, unsigned int ID);
		void setSmartPlug(unsigned int ID, bool onoff);
		
		unsigned int getDeviceData(unsigned int dType, unsigned int idNum);
		bool devicePowerAlarm(unsigned int dType, unsigned int idNum);
		unsigned int lastUpdateTime(unsigned int dType, unsigned int idNum);
};
 
#endif