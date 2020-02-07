#pragma once

#include <thread>
#include <atomic>
#include <map>
#include "Definition.h"
#include "Queue.h"

class Channel
{
public:
    
    Channel();
    Channel(UsbDevice device);
    Channel(Channel&& source);
    ~Channel();
    Channel& operator=(Channel&& source);

    std::string executeCommand(const std::string& command);
    void startMeasuring();
    void stopMeasuring();
    void workThread();
    std::string getReading();
    void close();
    
private:
    
    std::thread mThread;
    std::unique_ptr<Queue<std::string>> mQueue;
    UsbDevice mUsbDevice;
    std::unique_ptr<std::atomic<bool>> mContinueMeasuring;
   
};