
#include <stdio.h>
#include <libusb.h>
#include <vector>
#include <thread>
#include "Channel.h"
#include "Definition.h"
#include "Queue.h"


class UsbDriver
{
    
public:
    
    UsbDriver();
    ~UsbDriver();

    std::vector<int> detect();
    void close(int channelId);
    std::string executeCommand(int channelId, const std::string& command);
    void startMeasuring(int channelId);
    void stopMeasuring(int channelId);
    std::string getReading(int channelId);
    
private:
    
     void init();
     void exit();
     void close();

     std::map<int, Channel> mChannels;
     std::map<int, CsCommand> mSupportedProductsSet;
    
};
