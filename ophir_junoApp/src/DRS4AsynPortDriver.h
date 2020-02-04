#include "asynPortDriver.h"

#include "strlcpy.h"
#include "DRS.h"

#define _USE_MATH_DEFINES



#define O_BINARY 0

#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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

class DRS4AsynPortDriver : public asynPortDriver {
public:
    DRS4AsynPortDriver(const char *portName, int maxArraySize);
                 
    /* These are the methods that we override from asynPortDriver */
	virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
	virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
	virtual asynStatus readFloat32Array(asynUser *pasynUser, epicsFloat32 *value, size_t nElements, size_t *nIn);

    /* These are the methods that are new to this class */
	void dataTask(void);

protected:
    /** Values used for pasynUser->reason, and indexes into the parameter library. */
    int P_Run;
	#define FIRST_SCOPE_COMMAND P_Run
    int P_Acq_Count;
	int P_Max_Points;
	
	int P_Trig_mV;
	int P_Trig_Ch;
	int P_Trig_Del_nS;
	
	int P_Ch_1_mV;
	int P_Ch_2_mV;
	int P_Ch_3_mV;
	int P_Ch_4_mV;
	
	int P_Ch_1_nS;
	int P_Ch_2_nS;
	int P_Ch_3_nS;
	int P_Ch_4_nS;
		
    #define LAST_SCOPE_COMMAND P_Ch_4_nS
 

	
private:
    /* Our data */
    epicsEventId eventId_;
    
    epicsFloat32 *wave_a_1;
	epicsFloat32 *wave_a_2;
	epicsFloat32 *wave_a_3;
	epicsFloat32 *wave_a_4;
	
	epicsFloat32 *time_a_1;
	epicsFloat32 *time_a_2;
	epicsFloat32 *time_a_3;
	epicsFloat32 *time_a_4;
	
	DRS *drs;
    DRSBoard *b;	
		
};


#define NUM_SCOPE_PARAMS (&LAST_SCOPE_COMMAND - &FIRST_SCOPE_COMMAND + 1)

