TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += drs4

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install drs4.dbd into <top>/dbd
DBD += drs4.dbd

# specify all source files to be compiled and added to the library

drs4_SRCS += DRS4AsynPortDriver.cpp
drs4_SRCS += DRS.cpp
drs4_SRCS += strlcpy.c
drs4_SRCS += mxml.c
drs4_SRCS += musbstd.c
drs4_SRCS += averager.c
drs4_SRCS += myTimeStampSource.cpp

USR_CFLAGS += -g -O2 -fno-strict-aliasing -DOS_LINUX -DHAVE_USB -DHAVE_LIBUSB10 
USR_CXXFLAGS += -g -O2 -fno-strict-aliasing -DOS_LINUX -DHAVE_USB -DHAVE_LIBUSB10 
#USR_CFLAGS += -g -O2 -fno-strict-aliasing
drs4_LIBS += $(EPICS_BASE_IOC_LIBS)
drs4_LIBS += asyn 
LIB_SYS_LIBS += usb-1.0
LIB_SYS_LIBS += pthread
LIB_SYS_LIBS += util
#LIB_SYS_LIBS += wx
#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

