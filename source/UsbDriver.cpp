
#include "../source/UsbDriver.h"

#include <string>
#include <exception>
#include <regex>
#include "unistd.h"
#include <thread>



UsbDriver::UsbDriver()
{
    printf("UsbDriver. ctor\n");
    
    init();

    mSupportedProductsSet[Nova2] = CsCommand("CS 1 1 3", "CS 0");
    mSupportedProductsSet[Vega] = CsCommand("CS 1 1 3", "CS 0");
    mSupportedProductsSet[Juno] = CsCommand("CS 3", "CS 1");
    mSupportedProductsSet[StarLite] = CsCommand("CS 2", "CS 1");
    mSupportedProductsSet[StarBright] = CsCommand("CS 2", "CS 1");
}

UsbDriver::~UsbDriver()
{
    printf("~UsbDriver\n");
    
    close();
    exit();
}

void UsbDriver::init()
{
    int initResult = libusb_init(NULL);
    
    if (initResult < 0)
        throw CommunicationError("Init failed");
}

void UsbDriver::close()
{
    mChannels.clear();
}

void UsbDriver::close(int channelId)
{
    mChannels.erase(channelId);
}

void UsbDriver::exit()
{
    libusb_exit(NULL);
}

std::string UsbDriver::executeCommand(int channelId, const std::string & command)
{
    return mChannels[channelId].executeCommand(command);
}

void UsbDriver::startMeasuring(int channelId)
{
    return mChannels[channelId].startMeasuring();
}

void UsbDriver::stopMeasuring(int channelId)
{
    mChannels[channelId].stopMeasuring();
}

std::string UsbDriver::getReading(int channelId)
{
    return mChannels[channelId].getReading();
}

std::vector<int> UsbDriver::detect()
{
    
    // if exist, close previous opened channels.
    close();
    
    std::vector<int> devices;
    
    libusb_device **devs;
    libusb_device_handle *devh;
    ssize_t cnt;
    int channelId = 0;
    
    init();
    
    cnt = libusb_get_device_list(NULL, &devs);
    
    if(cnt >= 0)
    {
        libusb_device *dev;
        int i = 0;
        
        while ((dev = devs[i++]) != NULL) {
            struct libusb_device_descriptor desc;
            int r = libusb_get_device_descriptor(dev, &desc);
            if (r >= 0) {
                
                if(desc.idVendor == VENDOR_ID &&
                   mSupportedProductsSet.find(desc.idProduct) != mSupportedProductsSet.end())
                {
                	// http://ubuntuforums.org/showthread.php?t=901891
                	// http://stackoverflow.com/questions/22713834/libusb-cannot-open-usb-device-permission-isse-netbeans-ubuntu
                	// sudo adduser user group(root)
                	// sudo deluser user group(root)
                    r = libusb_open(dev, &devh);

                    if (r < 0)
                    	throw CommunicationError("libusb_open failed");
                    
                    // set configuration. to be on the safe side. not needed on ubuntu 14.04 LTS
                    libusb_set_configuration(devh, 1);
                    usleep(1000000); // 1 second

                    UsbDevice device(channelId, devh, mSupportedProductsSet[desc.idProduct]);
                    devices.push_back(channelId);
                    
                    // create channel
                    mChannels[channelId] = Channel(device);
                    
                    channelId++;
                }
            }
        }
    }
    libusb_free_device_list(devs, 1);
    
    return devices;
}


