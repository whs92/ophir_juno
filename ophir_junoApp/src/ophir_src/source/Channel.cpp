
#include "../source/Channel.h"

#include "unistd.h"

#include <libusb.h>

Channel::Channel()
: mQueue(new Queue<std::string>)
, mContinueMeasuring(new std::atomic<bool>(false))
{
}

Channel::Channel(UsbDevice device)
: Channel()
{
    mUsbDevice = device;
}

Channel::Channel(Channel&& source)
: mThread(std::move(source.mThread))
, mQueue(std::move(source.mQueue))
, mUsbDevice(std::move(source.mUsbDevice))
, mContinueMeasuring(std::move(source.mContinueMeasuring))
{}

Channel& Channel::operator=(Channel&& source)
{
    mThread = std::move(source.mThread);
    mQueue = std::move(source.mQueue);
    mUsbDevice = std::move(source.mUsbDevice);
    mContinueMeasuring = std::move(source.mContinueMeasuring);
    return *this;
}

Channel::~Channel()
{
    printf("~Channel[%d].\n", mUsbDevice.channelId);
    if (mQueue != nullptr)
    {
        stopMeasuring();
        close();
    }

}

std::string Channel::executeCommand(const std::string& command)
{
    std::string wrapCommand = "$" + command + "\r\n";
    int count = 1;
    int retryCount = 5;
    
    while (count < retryCount)
    {
        
        int responseSize;
        unsigned char response[80];
        
        responseSize = libusb_control_transfer(mUsbDevice.devh, 64, 2, 0, 0, (unsigned char *)wrapCommand.c_str(),wrapCommand.size(), 2000);
        
        if (responseSize > 0)
        {
            responseSize = libusb_control_transfer(mUsbDevice.devh, 192, 4, 0, 0, response, sizeof(response), 2000);
    
            if (responseSize > 0)
            {
                std::string strResponse(reinterpret_cast<const char *>(response), responseSize);
                
                if(!strResponse.empty() && (strResponse[0] == '*' || strResponse[0] == '?'))
                
                    return strResponse;
            }

        }
        usleep(500000); // 500 milliseconds
        count++;
    }
     throw CommunicationError("execute command: " + command + " failed");
}

void Channel::startMeasuring()
{
    
    try
    {
        if (!*mContinueMeasuring)
        {
            *mContinueMeasuring = true;
            int result;
            std::string response = executeCommand(mUsbDevice.csCommand.start);
            printf("startMeasuring[%d]. %s command = %s",  mUsbDevice.channelId, mUsbDevice.csCommand.start.c_str(), response.c_str());

            // detach kernel driver before claim interface
            libusb_detach_kernel_driver(mUsbDevice.devh, 0);
            
            result = libusb_claim_interface(mUsbDevice.devh, 0);
            if (result < 0)
            	throw CommunicationError("libusb_claim_interface failed");
            
            mThread = std::thread(&Channel::workThread, this);
        }
        
    }
    catch (...)
    {
        printf("startMeasuring[%d]. exception\n", mUsbDevice.channelId);
    }
}

void Channel::stopMeasuring()
{
    try
    {
        printf("stopMeasuring[%d].\n", mUsbDevice.channelId);
        
        if (*mContinueMeasuring)
        {
            *mContinueMeasuring = false;
            
            try {
                std::string response = executeCommand(mUsbDevice.csCommand.stop);
                printf("stopMeasuring[%d]. %s command = %s",  mUsbDevice.channelId, mUsbDevice.csCommand.stop.c_str(), response.c_str());

            } catch (...) {}


            if(mThread.joinable())
            {
                mThread.join();
            }
        }
    }
    catch(const std::runtime_error& re)
    {
         printf("stopMeasuring[%d]. exception=%s.\n", mUsbDevice.channelId, re.what());
    }
    catch(const std::exception& ex)
    {
        printf("stopMeasuring[%d]. exception=%s.\n", mUsbDevice.channelId, ex.what());

    }
    catch (...)
    {
        printf("stopMeasuring[%d]. exception.\n", mUsbDevice.channelId);
    }
}

void Channel::workThread()
{
    unsigned char data[64];
    int transferred;
    unsigned char endpoint = 0x82;
    int result;
    
    while (*mContinueMeasuring)
    {
        try
        {
            result = libusb_interrupt_transfer(mUsbDevice.devh,
                                               endpoint,
                                               data,
                                               sizeof(data),
                                               &transferred,
                                               1000 // timeout (in milliseconds) that this function should wait before giving up due to no response being received. For an unlimited timeout, use value 0.
                                               );
            
            if (result == LIBUSB_SUCCESS && transferred > 0)
            {
                std::string response(reinterpret_cast<const char *>(data), transferred);
                mQueue->push(response);
                printf("workThread[%d]. reading = %s", mUsbDevice.channelId, response.c_str());
            }
            else if(result != LIBUSB_SUCCESS && result != LIBUSB_ERROR_TIMEOUT)
            {
                printf("workThread[%d]. error = %d\n", mUsbDevice.channelId, result);
                mQueue->push(CHANNEL_CLOSE);
                break;
            }
        }
        catch (...)
        {
            printf("workThread[%d]. exception.\n", mUsbDevice.channelId);
        }
    }
}

std::string Channel::getReading()
{
    std::string result;
    mQueue->pop(result, std::chrono::milliseconds(100));
    return result;
}

void Channel::close()
{
    printf("close[%d].\n", mUsbDevice.channelId);
    libusb_release_interface(mUsbDevice.devh, 0);
    libusb_close(mUsbDevice.devh);
}
