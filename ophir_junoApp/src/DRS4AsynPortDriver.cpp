#include "DRS4AsynPortDriver.h"

static const char *driverName="DRS4AsynPortDriver";
void dataTaskThread(void *drvPvt);

/** Constructor for the DRS4AsynPortDriver class.
  * Calls constructor for the asynPortDriver base class.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] maxPoints The maximum  number of points in the volt and time arrays */
  DRS4AsynPortDriver::DRS4AsynPortDriver(const char *portName, int maxPoints) 
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
	const char *functionName = "DRS4AsynPortDriver";
	//int i,j,nBoards;
	
	
   
	
	/* Make sure maxPoints is positive */
	if (maxPoints < 1) maxPoints = 1024;

	/* Allocate the waveform array */
	wave_a_1 = (epicsFloat32 *)calloc(maxPoints, sizeof(epicsFloat32));
	wave_a_2 = (epicsFloat32 *)calloc(maxPoints, sizeof(epicsFloat32));
	wave_a_3 = (epicsFloat32 *)calloc(maxPoints, sizeof(epicsFloat32));
	wave_a_4 = (epicsFloat32 *)calloc(maxPoints, sizeof(epicsFloat32));

	/* Allocate the waveform array */
	time_a_1 = (epicsFloat32 *)calloc(maxPoints, sizeof(epicsFloat32));
	time_a_2 = (epicsFloat32 *)calloc(maxPoints, sizeof(epicsFloat32));
	time_a_3 = (epicsFloat32 *)calloc(maxPoints, sizeof(epicsFloat32));
	time_a_4 = (epicsFloat32 *)calloc(maxPoints, sizeof(epicsFloat32));
	
	/*Create EPICS Parameters*/
	eventId_ = epicsEventCreate(epicsEventEmpty);
	createParam("H_Ch_1_mV",  	asynParamFloat32Array,	&P_Ch_1_mV);
	createParam("H_Ch_2_mV",  	asynParamFloat32Array,	&P_Ch_2_mV);
	createParam("H_Ch_3_mV",  	asynParamFloat32Array,	&P_Ch_3_mV);
	createParam("H_Ch_4_mV",  	asynParamFloat32Array,	&P_Ch_4_mV);
	
	createParam("H_Ch_1_nS",  	asynParamFloat32Array,	&P_Ch_1_nS);
	createParam("H_Ch_2_nS",  	asynParamFloat32Array,	&P_Ch_2_nS);
	createParam("H_Ch_3_nS",  	asynParamFloat32Array,	&P_Ch_3_nS);
	createParam("H_Ch_4_nS",  	asynParamFloat32Array,	&P_Ch_4_nS);

	createParam("H_Run", 		asynParamInt32,			&P_Run);
	createParam("H_Acq_Count",  asynParamInt32,			&P_Acq_Count);
	createParam("H_Max_Points", asynParamInt32,	    	&P_Max_Points);
	createParam("H_Trig_mV", 	asynParamFloat64,	    &P_Trig_mV);
	createParam("H_Trig_Ch", 	asynParamInt32,	    	&P_Trig_Ch);
	createParam("H_Trig_Del_nS",asynParamInt32,	    	&P_Trig_Del_nS);
	
	
	setIntegerParam(P_Acq_Count, 0);
	setIntegerParam(P_Run, 0);
	setIntegerParam(P_Max_Points,	maxPoints);
	setIntegerParam(P_Trig_Del_nS,	180);
	setDoubleParam(P_Trig_mV,	0.1);
	setIntegerParam(P_Trig_Ch,	1);
	
		/* Do callbacks so higher layers see any changes */
	status = (asynStatus) callParamCallbacks();


	/*Create Thread for Interrupt Handling */

	status = (asynStatus)(epicsThreadCreate("DRS4AsynPortDriverDataTask",	
						  epicsThreadPriorityHigh,
						  epicsThreadGetStackSize(epicsThreadStackMedium),
						  (EPICSTHREADFUNC)::dataTaskThread,
						  this) == NULL);

	if (status) {
		printf("%s:%s: epicsIntrThreadCreate failure\n", driverName, functionName);
		return;
	}
	
}

void dataTaskThread(void *drvPvt)
{
    DRS4AsynPortDriver *pPvt = (DRS4AsynPortDriver *)drvPvt;
    
    pPvt->dataTask();
}


void DRS4AsynPortDriver::dataTask(void){
	
	int count = 0;
	int run =0;
	asynStatus status;
	int maxPoints;
	int i, nBoards,j;
	
#ifdef HAVE_USB
	printf("HAVE_USB\n");
#endif
#ifdef HAVE_LIBUSB
	printf("HAVE_LIBUSB\n");
#endif
#ifdef HAVE_LIBUSB10
	printf("HAVE_LIBUSB10\n");
#endif

	/* do initial scan */
	try{
		printf("before new drs = %d\n",drs);
		drs = new DRS();
		printf("after new drs = %d\n",drs);
	}catch(std::bad_alloc&){
		printf("drs pointer is broken\n");
	}
	b = drs->GetBoard(0);
	printf("I'm back in APD\n");
	
	//while(1);
	  printf("Found DRS4 evaluation board, serial #%d, firmware revision %d\n", 
		 b->GetBoardSerialNumber(), b->GetFirmwareVersion());
		 
	nBoards = drs->GetNumberOfBoards();
	if (nBoards == 0) {
	  printf("No DRS4 evaluation board found (My Support Module)\n");
	}
	/* show any found board(s) */
	for (i=0 ; i<nBoards ; i++) {
	  
	  b = drs->GetBoard(i);
	  printf("Found DRS4 evaluation board, serial #%d, firmware revision %d\n", 
		 b->GetBoardSerialNumber(), b->GetFirmwareVersion());
	}

		/* continue working with first board only */
	b = drs->GetBoard(0);

	/* initialize board */
	b->Init();

	/* set sampling frequency */
	b->SetFrequency(5, true);

	/* enable transparent mode needed for analog trigger */
	b->SetTranspMode(1);

	/* set input range to -0.5V ... +0.5V */
	b->SetInputRange(0);

	/* use following lines to enable hardware trigger on CH1 at 50 mV positive edge */
	b->EnableTrigger(1, 0);           // enable hardware trigger
	b->SetTriggerSource(1<<0);        // set CH1 as source
	b->SetTriggerLevel(0.1);            // 0.1 V
	b->SetTriggerPolarity(false);        // positive edge

	b->SetTriggerDelayNs(180);             // 230 ns trigger delay
	
	//lock();
	
	printf("finished setup of DRS, starting sampling\n");
    /* Loop forever */ 
	while(1){
		/* start board (activate domino wave) */
		b->StartDomino();
		//printf("Domino Started\n");
		
		getIntegerParam(P_Acq_Count,     &count);
		getIntegerParam(P_Run, &run);
		getIntegerParam(P_Max_Points, &maxPoints);
        // Release the lock while we wait for a command to start or wait for updateTime
        //printf("Run is %d\n",run);
		//setIntegerParam(P_Run, 0);
		//unlock();
        if (run){
			//lock();
			fflush(stdout);
			while (b->IsBusy());
			//setIntegerParam(P_Run, 1);
			count++;
			updateTimeStamp();
			setIntegerParam(P_Acq_Count, count);
			/* read all waveforms */
			b->TransferWaves(0, 8);

			/* read time (X) array of first channel in ns */
			b->GetTime(0, 0, b->GetTriggerCell(0), time_a_1);
			b->GetTime(0, 2, b->GetTriggerCell(0), time_a_2);
			b->GetTime(0, 4, b->GetTriggerCell(0), time_a_3);
			b->GetTime(0, 6, b->GetTriggerCell(0), time_a_4);

			/* decode waveform (Y) array of first channel in mV */
			b->GetWave(0, 0, wave_a_1);
			b->GetWave(0, 2, wave_a_2);
			b->GetWave(0, 4, wave_a_3);
			b->GetWave(0, 6, wave_a_4);

			doCallbacksFloat32Array(time_a_1, maxPoints, P_Ch_1_nS, 0);
			doCallbacksFloat32Array(time_a_2, maxPoints, P_Ch_2_nS, 0);
			doCallbacksFloat32Array(time_a_3, maxPoints, P_Ch_3_nS, 0);
			doCallbacksFloat32Array(time_a_4, maxPoints, P_Ch_4_nS, 0);

			doCallbacksFloat32Array(wave_a_1, maxPoints, P_Ch_1_mV, 0);
			doCallbacksFloat32Array(wave_a_2, maxPoints, P_Ch_2_mV, 0);
			doCallbacksFloat32Array(wave_a_3, maxPoints, P_Ch_3_mV, 0);
			doCallbacksFloat32Array(wave_a_4, maxPoints, P_Ch_4_mV, 0);
			
			
		}
		
        else{     
			//printf("Waiting For Run\n");
			(void) epicsEventWait(eventId_);
			// Take the lock again
			//lock(); 
     
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
asynStatus DRS4AsynPortDriver::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char *paramName;
    const char* functionName = "writeInt32";
	//DRSBoard *b;
	
	//b = drs->GetBoard(0);
    /* Set the parameter in the parameter library. */
    status = (asynStatus) setIntegerParam(function, value);
    
    /* Fetch the parameter string name for possible use in debugging */
    getParamName(function, &paramName);

    if (function == P_Run) {
        /* If run was set then wake up the simulation task */
        //printf("function P_Run Called\n");
		
		if (value){
			epicsEventSignal(eventId_);
		}
	}
	else if (function == P_Trig_Ch){
		b->SetTriggerSource(1<<(value-1));  
	}
	else if (function == P_Trig_Del_nS){
		b->SetTriggerDelayNs(value); 
	}

	/* Do callbacks so higher layers see any changes */
    status = (asynStatus) callParamCallbacks();
    
    if (status) 
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, name=%s, value=%d", 
                  driverName, functionName, status, function, paramName, value);
    else        
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, name=%s, value=%d\n", 
              driverName, functionName, function, paramName, value);
    return status;	
}
	
/** Called when asyn clients call pasynFloat32Array->read().
  * Returns the value of the wave a time arrays
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Pointer to the array to read.
  * \param[in] nElements Number of elements to read.
  * \param[out] nIn Number of elements actually read. */
asynStatus DRS4AsynPortDriver::readFloat32Array(asynUser *pasynUser, epicsFloat32 *value, 
                                         size_t nElements, size_t *nIn)
{
    int function = pasynUser->reason;
    size_t ncopy;
    asynStatus status = asynSuccess;
    const char *functionName = "readFloat32Array";
	//DRSBoard *b;

    status = getIntegerParam(P_Max_Points, (epicsInt32 *)&ncopy);
    if (nElements < ncopy) ncopy = nElements;
   

    if (function == P_Ch_1_nS ) {
        memcpy(value, time_a_1, ncopy*sizeof(epicsFloat32));
        *nIn = ncopy;
    }
	else if(function == P_Ch_2_nS ) {
        memcpy(value, time_a_2, ncopy*sizeof(epicsFloat32));
        *nIn = ncopy;
    }
	else if(function == P_Ch_3_nS ) {
        memcpy(value, time_a_3, ncopy*sizeof(epicsFloat32));
        *nIn = ncopy;
    }
	else if(function == P_Ch_4_nS ) {
        memcpy(value, time_a_4, ncopy*sizeof(epicsFloat32));
        *nIn = ncopy;
    }
	else if(function == P_Ch_1_mV ) {
        memcpy(value, wave_a_1, ncopy*sizeof(epicsFloat32));
        *nIn = ncopy;
    }
	else if(function == P_Ch_2_mV ) {
        memcpy(value, wave_a_2, ncopy*sizeof(epicsFloat32));
        *nIn = ncopy;
    }
	else if(function == P_Ch_3_mV ) {
        memcpy(value, wave_a_3, ncopy*sizeof(epicsFloat32));
        *nIn = ncopy;
    }
	else if(function == P_Ch_4_mV ) {
        memcpy(value, wave_a_4, ncopy*sizeof(epicsFloat32));
        *nIn = ncopy;
    }
	
	
    if (status) 
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d", 
                  driverName, functionName, status, function);
    else        
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d\n", 
              driverName, functionName, function);
    return status;
}

/** Called when asyn clients call pasynFloat64->write().
  * This function sends a signal to the simTask thread if the value of P_UpdateTime has changed.
  * For all  parameters it  sets the value in the parameter library and calls any registered callbacks.
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus DRS4AsynPortDriver::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    epicsInt32 run;
    const char *paramName;
    const char* functionName = "writeFloat64";

    /* Set the parameter in the parameter library. */
    status = (asynStatus) setDoubleParam(function, value);

    /* Fetch the parameter string name for possible use in debugging */
    getParamName(function, &paramName);
	
	if (function == P_Trig_mV){
		b->SetTriggerLevel(value);           
	}
    
    
    /* Do callbacks so higher layers see any changes */
    status = (asynStatus) callParamCallbacks();
    
    if (status) 
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, name=%s, value=%f", 
                  driverName, functionName, status, function, paramName, value);
    else        
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, name=%s, value=%f\n", 
              driverName, functionName, function, paramName, value);
    return status;
}



/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

/** EPICS iocsh callable function to call constructor for the DRS4AsynPortDriver class.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] maxPoints The maximum  number of points in the volt and time arrays */
int DRS4AsynPortDriverConfigure(const char *portName, int maxPoints)
{
    new DRS4AsynPortDriver(portName, maxPoints);
    return(asynSuccess);
}


/* EPICS iocsh shell commands */

static const iocshArg initArg0 = { "portName",iocshArgString};
static const iocshArg initArg1 = { "max points",iocshArgInt};
static const iocshArg * const initArgs[] = {&initArg0,
                                            &initArg1};
static const iocshFuncDef initFuncDef = {"DRS4AsynPortDriverConfigure",2,initArgs};
static void initCallFunc(const iocshArgBuf *args)
{
    DRS4AsynPortDriverConfigure(args[0].sval, args[1].ival);
}

void DRS4AsynPortDriverRegister(void)
{
    iocshRegister(&initFuncDef,initCallFunc);
}

epicsExportRegistrar(DRS4AsynPortDriverRegister);

}
