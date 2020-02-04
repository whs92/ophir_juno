
#pragma once

#include <stdio.h>
#include <libusb.h>
#include <string>


struct CsCommand
{
    std::string start;
    std::string stop;

    CsCommand() {}

    CsCommand(std::string start, std::string stop)
    : start(start)
    , stop(stop)
    {
    }
};


struct UsbDevice
{
	int channelId;
	libusb_device_handle *devh;
    CsCommand csCommand;

	UsbDevice()
	: channelId(0)
	, devh(nullptr)
{
}


	UsbDevice(int channelId, libusb_device_handle *devh, CsCommand csCommand)
		: channelId(channelId)
		, devh(devh)
        , csCommand(csCommand)
	{
	}
};

struct CommunicationError : std::runtime_error
{
	explicit CommunicationError(const std::string& message = "")
		: runtime_error(message)
	{}
};

enum SupportedProducts {
    Nova2 = 0x333,
    Vega = 0x334,
    StarLite = 0x345,
    StarBright = 0x346,
    Juno = 0x777,
};

const int VENDOR_ID = 0xBD3;
const std::string CHANNEL_CLOSE = "CHANNEL_CLOSE";
