TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += ophir_juno

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install ophir_juno.dbd into <top>/dbd
DBD += ophir_juno.dbd

# specify all source files to be compiled and added to the library

ophir_juno_SRCS += OphirJunoAsynPortDriver.cpp
ophir_juno_SRCS += Channel.cpp
ophir_juno_SRCS += UsbDriver.cpp
ophir_juno_SRCS += myTimeStampSource.cpp

USR_CFLAGS += -g -O2 -fno-strict-aliasing -DOS_LINUX -DHAVE_USB -DHAVE_LIBUSB10 -std=c++11
USR_CXXFLAGS += -g -O2 -fno-strict-aliasing -DOS_LINUX -DHAVE_USB -DHAVE_LIBUSB10 -std=c++11
#USR_CFLAGS += -g -O2 -fno-strict-aliasing
ophir_juno_LIBS += $(EPICS_BASE_IOC_LIBS)
ophir_juno_LIBS += asyn 
LIB_SYS_LIBS += usb-1.0
LIB_SYS_LIBS += pthread
LIB_SYS_LIBS += util
#LIB_SYS_LIBS += wx
#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

