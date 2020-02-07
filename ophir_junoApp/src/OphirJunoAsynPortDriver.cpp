#include "OphirJunoAsynPortDriver.h"

static const char *driverName="OphirJunoAsynPortDriver";
void dataTaskThread(void *drvPvt);

/** Constructor for the OphirJunoAsynPortDriver class.
  * Calls constructor for the asynPortDriver base class.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] maxPoints The maximum  number of points in the volt and time arrays */
  OphirJunoAsynPortDriver::OphirJunoAsynPortDriver(const char *portName, int maxPoints) 
   : asynPortDriver(portName, 
                    1, /* maxAddr */ 
                    NUM_SCOPE_PARAMS,
                    asynInt32Mask | asynFloat64Mask | asynFloat32ArrayMask | asynDrvUserMask, /* Interface mask */
                    asynInt32Mask | asynFloat64Mask | asynFloat32ArrayMask,  /* Interrupt mask */
                    0, /* asynFlags.  This driver does not block and it is not multi-device, so flag is 0 */
                    1, /* Autoconnect */
                    0, /* Default priority */
                    0) /* Default stack size*/    
{
	asynStatus status;
	const char *functionName = "OphirJunoAsynPortDriver";
	//int i,j,nBoards;
		
	/*Create EPICS Parameters*/
	eventId_ = epicsEventCreate(epicsEventEmpty);
	

	createParam("H_Range", 		asynParamInt32,			&P_Range);
	createParam("H_Run", 		asynParamInt32,			&P_Run);
	
	/* Do callbacks so higher layers see any changes */
	status = (asynStatus) callParamCallbacks();


	/*Create Thread for Interrupt Handling */
	status = (asynStatus)(epicsThreadCreate("OphirJunoAsynPortDriverDataTask",	
						  epicsThreadPriorityHigh,
						  epicsThreadGetStackSize(epicsThreadStackMedium),
						  (EPICSTHREADFUNC)::dataTaskThread,
						  this) == NULL);

	if (status) {
		printf("%s:%s: epicsIntrThreadCreate failure\n", driverName, functionName);
		return;
	}
	
	// We've created the usbDriver of usbDriver type in the include file. 
	// It's part of this class. 
	    
    // detect
    devices = usbDriver.detect();
    printf("main. %d device(s) detected.\n", (int)devices.size());
	
	// Check that comms work, request device information
	std::string response = usbDriver.executeCommand(0, "HI");
    printf("Device info = %s", response.c_str());
	
}

void dataTaskThread(void *drvPvt)
{
    OphirJunoAsynPortDriver *pPvt = (OphirJunoAsynPortDriver *)drvPvt;
    
    pPvt->dataTask();
}
/*
In this task we respond to new messages from the device once we enter the continuous read back mode
*/

void OphirJunoAsynPortDriver::dataTask(void){
	int run =0;
	asynStatus status;

	while(1){
		getIntegerParam(P_Run, &run);
		if (run){

			for (std::vector<int>::iterator it = devices.begin(); it < devices.end();)
			{
				int channelId = *it;
				std::string reading = usbDriver.getReading(channelId);
				if(!reading.empty())
				{
					if(reading == CHANNEL_CLOSE)
					{
						// stop measuring
						usbDriver.stopMeasuring(channelId);

						// close channel
						usbDriver.close(channelId);

						// remove channel from devices list
						it = devices.erase(it);
						continue;
					}
					else
					{
						 printf("main[%d]. reading = %s", channelId, reading.c_str());
					}
				}
				++it;
			}
			updateTimeStamp();	
			
			
			
			
			
		}
		else{    
			/* Stop Measuring, iterating over all channels */
			for (std::vector<int>::iterator it = devices.begin(); it < devices.end(); ++it)
			{
				int channelId = *it;
				usbDriver.stopMeasuring(channelId);
			}
		
			//Waiting For Run
			(void) epicsEventWait(eventId_); 
			
			/* Start Measuring, iterating over all channels */
			for (std::vector<int>::iterator it = devices.begin(); it < devices.end(); ++it)
			{
				int channelId = *it;
				// continues send mode
			usbDriver.startMeasuring(channelId);
			}
			
		}
		
		/* Do callbacks so higher layers see any changes */
		status = (asynStatus) callParamCallbacks();
			
	}

		
}
	


/** Called when asyn clients call pasynInt32->write().
  * This function sends a signal to the simTask thread if the value of P_Run has changed.
  * For all parameters it sets the value in the parameter library and calls any registered callbacks..
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus OphirJunoAsynPortDriver::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char *paramName;
    const char* functionName = "writeInt32";
	std::string command;
	int rbv;
	
	
    /* Fetch the parameter string name for possible use in debugging */
    getParamName(function, &paramName);
	
	if (function == P_Range) {
        
		command = "WN " + std::to_string(value); // note space after command
		usbDriver.executeCommand(0, command);
		
		
		// for debug read the value
		std::string response = usbDriver.executeCommand(0, "RN");
		rbv = response[1]-'0'; //Take then second char and turn it into an integer
		
		//callbacks
		status = (asynStatus) setIntegerParam(function, rbv);
		
	    
	}
	else if( function == P_Run){
		
		/* If run was set then wake up the acquisition task */  	
		if (value){
			epicsEventSignal(eventId_);
		}
		
		//callbacks
		status = (asynStatus) setIntegerParam(function, value);
	}


	/* Do callbacks so higher layers see any changes */
    status = (asynStatus) callParamCallbacks();
	
    return status;	
}

/** Called when asyn clients call pasynInt32->read().
  * This function sends a signal to the simTask thread if the value of P_Run has changed.
  * For all parameters it sets the value in the parameter library and calls any registered callbacks..
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus OphirJunoAsynPortDriver::readInt32(asynUser *pasynUser, epicsInt32 *value)
{
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char *paramName;
    const char* functionName = "readInt32";
	
    return status;	
}
	



/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

/** EPICS iocsh callable function to call constructor for the OphirJunoAsynPortDriver class.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] maxPoints The maximum  number of points in the volt and time arrays */
int OphirJunoAsynPortDriverConfigure(const char *portName, int maxPoints)
{
    new OphirJunoAsynPortDriver(portName, maxPoints);
    return(asynSuccess);
}


/* EPICS iocsh shell commands */

static const iocshArg initArg0 = { "portName",iocshArgString};
static const iocshArg initArg1 = { "max points",iocshArgInt};
static const iocshArg * const initArgs[] = {&initArg0,
                                            &initArg1};
static const iocshFuncDef initFuncDef = {"OphirJunoAsynPortDriverConfigure",2,initArgs};
static void initCallFunc(const iocshArgBuf *args)
{
    OphirJunoAsynPortDriverConfigure(args[0].sval, args[1].ival);
}

void OphirJunoAsynPortDriverRegister(void)
{
    iocshRegister(&initFuncDef,initCallFunc);
}

epicsExportRegistrar(OphirJunoAsynPortDriverRegister);

}
