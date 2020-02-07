#include "asynPortDriver.h"

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <libusb.h>
#include "UsbDriver.h"

#define _USE_MATH_DEFINES



#define O_BINARY 0

#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <math.h>

#define DIR_SEPARATOR '/'

#include <epicsTypes.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <iocsh.h>

#include <epicsExport.h>

#define PI	M_PI	/* pi to machine precision, defined in math.h */
#define TWOPI	(2.0*PI)

class OphirJunoAsynPortDriver : public asynPortDriver {
public:
    OphirJunoAsynPortDriver(const char *portName, int maxArraySize);
                 
    /* These are the methods that we override from asynPortDriver */
	virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
	virtual asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);


    /* These are the methods that are new to this class */
	void dataTask(void);
	
	UsbDriver usbDriver;
	std::vector<int> devices;
	epicsFloat32 avBuff[10000]= {0};
	
protected:
    /** Values used for pasynUser->reason, and indexes into the parameter library. */
    int P_Range;
	#define FIRST_SCOPE_COMMAND P_Range
	int P_Run;
	int P_E;	
	int P_Over;
	int P_AvE;
	int P_AvN;
	int P_Freq;
    #define LAST_SCOPE_COMMAND P_Freq
 

	
private:
    /* Our data */
    epicsEventId eventId_;
    
    epicsFloat32 *wave_a_1;
	
	
	
		
};


#define NUM_SCOPE_PARAMS (&LAST_SCOPE_COMMAND - &FIRST_SCOPE_COMMAND + 1)

