
#include <stdio.h>
#include <iostream>

#include <vector>
#include <string>

#include "unistd.h"
#include <sys/types.h>
#include <libusb.h>
#include "../source/UsbDriver.h"

int main(void)
{
   printf("main. start\n");

    UsbDriver usbDriver;
    
    // detect
    std::vector<int> devices = usbDriver.detect();
    
    printf("main. %d device(s) detected.\n", (int)devices.size());

    std::string mode, command;
    std::cout << "Do you want to enter interactive terminal mode? [y/n] ";
    std::getline(std::cin, mode);

    if(mode == "y")
    {
      	while(command != "exit")
    	{
      	  	std::cout << "Please enter command: [command/exit] ";
      	    std::getline(std::cin, command);
      	    if(command != "exit" )
      	    {
      	    	std::string response = usbDriver.executeCommand(0, command);
      	    	printf("response = %s", response.c_str());
      	    }
    	}
    }

    std::cout << "Do you want to enter continuous send mode? [y/n] ";
    std::cin >> mode;

     if(mode == "y")
     {
		for (std::vector<int>::iterator it = devices.begin(); it < devices.end(); ++it)
		{
		  int channelId = *it;

		  // continues send mode
		  usbDriver.startMeasuring(channelId);
		}

		int counter = 0;
		while (devices.size() > 0 && counter < 1000)
		{
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
			usleep(100000); // 100 milliseconds
			counter++;
		}

		for (std::vector<int>::iterator it = devices.begin(); it < devices.end(); ++it)
		{
			int channelId = *it;
			usbDriver.stopMeasuring(channelId);
		}
	}
    
	printf("main. end\n");
    return 0;
}



