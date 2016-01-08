// Stl includes
#include <string>
#include <algorithm>

#include "HyperionConfig.h"

// Leddevice includes
#include "leddevice/LedDeviceFactory.h"

#include "LedDeviceTest.h"

#ifdef ENABLE_LED_LPD6803
#include "LedDeviceLpd6803.h"
#endif
#ifdef ENABLE_LED_LPD8806
#include "LedDeviceLpd8806.h"
#endif
#ifdef ENABLE_LED_P9813
#include "LedDeviceP9813.h"
#endif
#ifdef ENABLE_LED_WS2801
#include "LedDeviceWs2801.h"
#endif
#ifdef ENABLE_LED_APA102
#include "LedDeviceAPA102.h"
#endif
#ifdef ENABLE_LED_TINKERFORGE
#include "LedDeviceTinkerforge.h"
#endif
#ifdef ENABLE_LED_ADALIGHT
#include "LedDeviceAdalight.h"
#endif
#ifdef ENABLE_LED_AMBILED
#include "LedDeviceAmbiLed.h"
#endif
#ifdef ENABLE_LED_LIGHTPACK
#include "LedDeviceLightpack.h"
#endif
#ifdef ENABLE_LED_MULTILIGHTPACK
#include "LedDeviceMultiLightpack.h"
#endif
#ifdef ENABLE_LED_PAINTPACK
#include "LedDevicePaintpack.h"
#endif
#ifdef ENABLE_LED_PIBLASTER
#include "LedDevicePiBlaster.h"
#endif
#ifdef ENABLE_LED_SEDU
#include "LedDeviceSedu.h"
#endif
#ifdef ENABLE_LED_USBASP
#include "LedDeviceHyperionUsbasp.h"
#endif
#ifdef ENABLE_LED_PHILIPSHUE
#include "LedDevicePhilipsHue.h"
#endif
#ifdef ENABLE_LED_SERIAL_TPM2
#include "LedDeviceTpm2Serial.h"
#endif
#ifdef ENABLE_LED_ATMO
#include "LedDeviceAtmo.h"
#endif
#ifdef ENABLE_LED_WS2812B_PWM
#include "LedDeviceWS2812b.h"
#endif
#ifdef ENABLE_LED_TCPSOCKET_TPM2
#include "LedDeviceTpm2TcpSocket.h"
#endif
#ifdef ENABLE_LED_UDPSOCKET_TPM2
#include "LedDeviceTpm2UdpSocket.h"
#endif

LedDevice * LedDeviceFactory::construct(const Poco::DynamicStruct & deviceConfig)
{
    std::cout << "Device configuration: " << ((Poco::DynamicStruct)deviceConfig).toString() << std::endl;

    std::string type = deviceConfig["type"];
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);

    LedDevice* device = nullptr;
	
	if (type == "test")
	{
		const std::string output = deviceConfig["output"];
	    device = new LedDeviceTest(output);
	}
#ifdef ENABLE_LED_ADALIGHT	
    else if (type == "adalight")
    {
        const std::string output = deviceConfig["output"];
        const unsigned rate      = deviceConfig["rate"];
        int delay_ms = 1000;
        if (deviceConfig.contains("delayAfterConnect")) {
            delay_ms = deviceConfig["delayAfterConnect"];
        }

        LedDeviceAdalight* deviceAdalight = new LedDeviceAdalight(output, rate, delay_ms);
        deviceAdalight->open();

        device = deviceAdalight;
    }
#endif
#ifdef ENABLE_LED_AMBILED	
    else if (type == "ambiled")
    {
        const std::string output = deviceConfig["output"];
        const unsigned rate      = deviceConfig["rate"];
        int delay_ms = 0;
        if (deviceConfig.contains("delayAfterConnect")) {
            delay_ms = deviceConfig["delayAfterConnect"];
        }

        LedDeviceAmbiLed* deviceAmbiLed = new LedDeviceAmbiLed(output, rate, delay_ms);
        deviceAmbiLed->open();

        device = deviceAmbiLed;
    }
#endif
#ifdef ENABLE_LED_LPD6803
    else if (type == "lpd6803")
    {
        const std::string output = deviceConfig["output"];
        const unsigned rate      = deviceConfig["rate"];

        LedDeviceLpd6803* deviceLdp6803 = new LedDeviceLpd6803(output, rate);
        deviceLdp6803->open();

        device = deviceLdp6803;
    }
#endif
#ifdef ENABLE_LED_LPD8806
    else if (type == "lpd8806")
    {
        const std::string output = deviceConfig["output"];
        const unsigned rate      = deviceConfig["rate"];

        LedDeviceLpd8806* deviceLpd8806 = new LedDeviceLpd8806(output, rate);
        deviceLpd8806->open();

        device = deviceLpd8806;
    }
#endif
#ifdef ENABLE_LED_P9813
    else if (type == "p9813")
    {
        const std::string output = deviceConfig["output"];
        const unsigned rate      = deviceConfig["rate"];

        LedDeviceP9813* deviceP9813 = new LedDeviceP9813(output, rate);
        deviceP9813->open();

        device = deviceP9813;
    }
#endif
#ifdef ENABLE_LED_APA102
    else if (type == "apa102")
    {
        const std::string output = deviceConfig["output"];
        const unsigned rate      = deviceConfig["rate"];

        LedDeviceAPA102* deviceAPA102 = new LedDeviceAPA102(output, rate);
        deviceAPA102->open();

        device = deviceAPA102;
    }
#endif
#ifdef ENABLE_LED_WS2801
    else if (type == "ws2801" || type == "lightberry")
    {
        const std::string output = deviceConfig["output"];
        const unsigned rate      = deviceConfig["rate"];

        LedDeviceWs2801* deviceWs2801 = new LedDeviceWs2801(output, rate);
        deviceWs2801->open();

        device = deviceWs2801;
    }
#endif
#ifdef ENABLE_LED_TINKERFORGE
    else if (type=="tinkerforge")
    {
        std::string host = "127.0.0.1";
        if (!deviceConfig["output"].isEmpty())
            host = deviceConfig["output"].toString();
        uint16_t port = 4223;
        if (!deviceConfig["port"].isEmpty())
            port = deviceConfig["port"];
        std::string uid = deviceConfig["uid"];
        unsigned rate = deviceConfig["rate"];

        LedDeviceTinkerforge* deviceTinkerforge = new LedDeviceTinkerforge(host, port, uid, rate);
        deviceTinkerforge->open();

        device = deviceTinkerforge;
    }
#endif
#ifdef ENABLE_LED_LIGHTPACK
    else if (type == "lightpack")
    {
        const std::string output = deviceConfig["output"];

        LedDeviceLightpack* deviceLightpack = new LedDeviceLightpack();
        deviceLightpack->open(output);

        device = deviceLightpack;
    }
#endif
#ifdef ENABLE_LED_MULTILIGHTPACK
    else if (type == "multi-lightpack")
    {
        LedDeviceMultiLightpack* deviceLightpack = new LedDeviceMultiLightpack();
        deviceLightpack->open();

        device = deviceLightpack;
    }
#endif
#ifdef ENABLE_LED_PAINTPACK
    else if (type == "paintpack")
    {
        LedDevicePaintpack * devicePainLightpack = new LedDevicePaintpack();
        devicePainLightpack->open();

        device = devicePainLightpack;
    }
#endif
#ifdef ENABLE_LED_PIBLASTER
    else if (type == "piblaster")
    {
        const std::string output     = deviceConfig["output"];
        const std::string assignment = deviceConfig["assignment"];

        LedDevicePiBlaster * devicePiBlaster = new LedDevicePiBlaster(output, assignment);
        devicePiBlaster->open();

        device = devicePiBlaster;
    }
#endif
#ifdef ENABLE_LED_SEDU
    else if (type == "sedu")
    {
        const std::string output = deviceConfig["output"];
        const unsigned rate      = deviceConfig["rate"];

        LedDeviceSedu* deviceSedu = new LedDeviceSedu(output, rate);
        deviceSedu->open();

        device = deviceSedu;
    }
#endif
#ifdef ENABLE_LED_USBASP
    else if (type == "hyperion-usbasp-ws2801")
    {
        LedDeviceHyperionUsbasp * deviceHyperionUsbasp = new LedDeviceHyperionUsbasp(LedDeviceHyperionUsbasp::CMD_WRITE_WS2801);
        deviceHyperionUsbasp->open();
        device = deviceHyperionUsbasp;
    }
    else if (type == "hyperion-usbasp-ws2812")
    {
        LedDeviceHyperionUsbasp * deviceHyperionUsbasp = new LedDeviceHyperionUsbasp(LedDeviceHyperionUsbasp::CMD_WRITE_WS2812);
        deviceHyperionUsbasp->open();
        device = deviceHyperionUsbasp;
    }
#endif
#ifdef ENABLE_LED_PHILIPSHUE
    else if (type == "philipshue")
	{
		std::string output = deviceConfig["output"];
		std::string username = "newdeveloper";
		if (!deviceConfig["username"].isEmpty())
			username = deviceConfig["username"].toString();
		bool switchOffOnBlack = true;
		if (!deviceConfig["switchOffOnBlack"].isEmpty())
			 switchOffOnBlack = deviceConfig["switchOffOnBlack"];
		int transitiontime = 1;
		if (!deviceConfig["transitiontime"].isEmpty())
			 transitiontime = deviceConfig["transitiontime"];
		std::vector<unsigned int> lightIds;
		for (unsigned i = 0; i < deviceConfig["lightIds"].size(); i++) {
			lightIds.push_back(deviceConfig["lightIds"][i]);
		}
		device = new LedDevicePhilipsHue(output, username, switchOffOnBlack, transitiontime, lightIds);
	}
#endif
#ifdef ENABLE_LED_SERIAL_TPM2
    else if (type == "tpm2ser")
    {
        const std::string output = deviceConfig["output"];
        const unsigned rate = deviceConfig["rate"];

        LedDeviceTpm2Serial * deviceTpm2 = new LedDeviceTpm2Serial(output, rate);
        deviceTpm2->open();
		device = deviceTpm2;
    }
#endif
#ifdef ENABLE_LED_ATMO
    else if (type == "atmo")
    {
        const std::string output = deviceConfig["output"];
        const unsigned rate = 38400;

        LedDeviceAtmo * deviceAtmo = new LedDeviceAtmo(output, rate);
        deviceAtmo->open();
        device = deviceAtmo;
    }
#endif
#ifdef ENABLE_LED_WS2812B_PWM
    else if (type == "ws2812b")
    {
        LedDeviceWS2812b * ledDeviceWS2812b = new LedDeviceWS2812b();
        device = ledDeviceWS2812b;
    }
#endif
#ifdef ENABLE_LED_TCPSOCKET_TPM2
    else if (type == "tpm2tcp")
    {
        const std::string output = deviceConfig["output"];

        LedDeviceTpm2TcpSocket * deviceTpm2 = new LedDeviceTpm2TcpSocket(output);
        deviceTpm2->open();
        device = deviceTpm2;
    }
#endif
#ifdef ENABLE_LED_UDPSOCKET_TPM2
    else if (type == "tpm2udp")
    {
        const std::string output = deviceConfig["output"];

        LedDeviceTpm2UdpSocket * deviceTpm2 = new LedDeviceTpm2UdpSocket(output);
        device = deviceTpm2;
    }
#endif
    else
    {
        std::cout << "Unable to create device " << type << std::endl;
        // Unknown / Unimplemented device
    }
    return device;
}
