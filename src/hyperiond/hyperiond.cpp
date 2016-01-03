#include <cassert>

#include "Poco/Dynamic/Struct.h"
#include "Poco/FileStream.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Delegate.h"

#include "Poco/Thread.h"

// Json-Schema includes
//#include <utils/jsonschema/JsonFactory.h>

#include "HyperionConfig.h"
#include "hyperion/Hyperion.h"

#ifdef ENABLE_GRABBER
#include "grabber/FrameGrabberFactory.h"
#endif

// KODI Video checker includes
#ifdef ENABLE_KODI_CONNECTION
#include "kodiconnector/KodiConnector.h"
#endif

#ifdef ENABLE_SERVER_JSON
#include "jsonserver/JsonServer.h"
#endif

#ifdef ENABLE_SERVER_BOBLIGHT
#include "boblightserver/BoblightServer.h"
#endif

#define setDefault(dynstruct, key, value) if(!dynstruct.contains(key)){dynstruct[key]=value;}

Poco::DynamicStruct loadConfig(const std::string& configFile)
{
    Poco::FileInputStream fis(configFile);

    Poco::JSON::Parser parser;
    parser.setAllowComments(true);
    parser.parse(fis);
    Poco::DynamicStruct config = *(parser.result().extract<Poco::JSON::Object::Ptr>());

    return config;
}

class HyperionService : public Poco::Util::ServerApplication
{
public:
    HyperionService()
        : _helpRequested(false)
    {
    }

    ~HyperionService()
    {
    }

protected:
    void defineOptions(Poco::Util::OptionSet& options)
    {
        Poco::Util::ServerApplication::defineOptions(options);

        options.addOption(Poco::Util::Option("help", "h", "display help information on command line arguments")
                          .required(false)
                          .repeatable(false));
    }

    void handleOption(const std::string& name, const std::string& value)
    {
        Poco::Util::ServerApplication::handleOption(name, value);

        if(name == "help")
            _helpRequested = true;
    }

    void displayHelp()
    {
        Poco::Util::HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("config_file");
        helpFormatter.setHeader("Ambilight clone");
        helpFormatter.format(std::cout);
    }

    int main(const std::vector<std::string>& args)
    {
        if(_helpRequested)
        {
            displayHelp();
        }
        else
        {
            std::string confgiFile = "./config/hyperion.config.json";
            if(args.size() < 1)
            {
                std::cout << "No configuration file provided, looking for default one" << std::endl;
            } else {
                confgiFile = args[0];
            }

            std::cout << "Using configuration file: " << confgiFile << std::endl;
            Poco::DynamicStruct config = loadConfig(confgiFile);

            _hyperion = new Hyperion(config);
            std::cout << "Hyperion created and initialised" << std::endl;

            // create boot sequence if the configuration is present
#ifdef ENABLE_EFFECT_ENGINE			
            if(config.contains("bootsequence"))
            {
                Poco::DynamicStruct effectConfig = config["bootsequence"].extract<Poco::DynamicStruct>();
                // Get the parameters for the bootsequence
                std::string effectName = effectConfig["effect"].toString();
                unsigned duration_ms = effectConfig["duration_ms"].convert<unsigned>();
                int priority = 0;

                if(effectConfig.contains("args"))
                {
                    Poco::DynamicStruct effectConfigArgs = effectConfig["args"].extract<Poco::DynamicStruct>();
                    if(_hyperion->setEffect(effectName, effectConfigArgs, priority, duration_ms) == 0)
                    {
                        std::cout << "Boot sequence(" << effectName
                                  << ") with user-defined arguments created and started" << std::endl;
                    }
                    else
                    {
                        std::cout << "Failed to start boot sequence: " << effectName << " with user-defined arguments"
                                  << std::endl;
                    }
                }
                else
                {
                    if(_hyperion->setEffect(effectName, priority, duration_ms) == 0)
                    {
                        std::cout << "Boot sequence(" << effectName << ") created and started" << std::endl;
                    }
                    else
                    {
                        std::cout << "Failed to start boot sequence: " << effectName << std::endl;
                    }
                }
            }
#endif			
			
#ifdef ENABLE_GRABBER
            // Construct and start the frame-grabber if the configuration is present
            if(config.contains("framegrabber"))
            {
                Poco::DynamicStruct frameGrabberConfig = config["framegrabber"].extract<Poco::DynamicStruct>();
				_frameGrabber = FrameGrabberFactory::createNewFrameGrabber(frameGrabberConfig, _hyperion->getLedCount());
				
				if (_frameGrabber != nullptr)
				{
                    _frameGrabber->clearEvent += Poco::delegate(this, &HyperionService::onClear);
                    _frameGrabber->setColorsEvent += Poco::delegate(this, &HyperionService::onSetColors);

                    _frameGrabber->start();

                    std::cout << "Frame grabber created and started" << std::endl;
                } else {
                    std::cerr << "[ERROR] Frame grabber " << frameGrabberConfig["type"].toString() << " could not be created" << std::endl;
                }
            }
#endif

#ifdef ENABLE_KODI_CONNECTION
            // create KODI connector if the configuration is present
            if(config.contains("kodiConnector"))
            {
                _kodiConnectorConfig = config["kodiConnector"].extract<Poco::DynamicStruct>();
                _kodiConnector = new KodiConnector(_kodiConnectorConfig["host"], _kodiConnectorConfig["port"]);

                if(_kodiConnector != nullptr)
                {
                    _kodiConnector->videoModeEvent += Poco::delegate(this, &HyperionService::onSetVideoMode);
                    _kodiConnector->grabbingModeEvent += Poco::delegate(this, &HyperionService::onSetGrabbingMode);
                }

                Poco::Thread kodiConnectorThread;
                kodiConnectorThread.start(*_kodiConnector);

                std::cout << "KODI connector created and started" << std::endl;
            }
#endif

#ifdef ENABLE_V4L2
            // construct and start the v4l2 grabber if the configuration is present
            V4L2Wrapper* v4l2Grabber = nullptr;
            if(config.contains("grabber-v4l2"))
            {
                Poco::DynamicStruct grabberConfig = config["grabber-v4l2"].extract<Poco::DynamicStruct>();

                setDefault(grabberConfig, "device", "/dev/video0");
                setDefault(grabberConfig, "input", 0);
                setDefault(grabberConfig, "standard", "no-change");
                setDefault(grabberConfig, "pixelFormat", "no-change");
                setDefault(grabberConfig, "width", -1);
                setDefault(grabberConfig, "height", -1);
                setDefault(grabberConfig, "frameDecimation", 2);
                setDefault(grabberConfig, "sizeDecimation", 8);
                setDefault(grabberConfig, "redSignalThreshold", 0.0);
                setDefault(grabberConfig, "greenSignalThreshold", 0.0);
                setDefault(grabberConfig, "blueSignalThreshold", 0.0);
                setDefault(grabberConfig, "priority", 800);
                setDefault(grabberConfig, "mode", "2D");
                setDefault(grabberConfig, "cropLeft", 0.0);
                setDefault(grabberConfig, "cropRight", 0.0);
                setDefault(grabberConfig, "cropTop", 0.0);
                setDefault(grabberConfig, "cropBottom", 0.0);

                v4l2Grabber =
                    new V4L2Wrapper(grabberConfig["device"],
                                    grabberConfig["input"],
                                    parseVideoStandard(grabberConfig["standard"]),
                                    parsePixelFormat(grabberConfig["pixelFormat"]),
                                    grabberConfig["width"],
                                    grabberConfig["height"],
                                    grabberConfig["frameDecimation"],
                                    grabberConfig["sizeDecimation"],
                                    grabberConfig["redSignalThreshold"],
                                    grabberConfig["greenSignalThreshold"],
                                    grabberConfig["blueSignalThreshold"],
                                    &hyperion,
                                    grabberConfig["priority"]);
                v4l2Grabber->set3D(parse3DMode(grabberConfig["mode"]));
                v4l2Grabber->setCropping(grabberConfig["cropLeft"],
                                         grabberConfig["cropRight"],
                                         grabberConfig["cropTop"],
                                         grabberConfig["cropBottom"]);

                v4l2Grabber->start();
                std::cout << "V4l2 grabber created and started" << std::endl;
            }
#endif


#ifdef ENABLE_SERVER_JSON
            // Create Json server if configuration is present
            if(config.contains("jsonServer"))
            {
                _jsonServer = new JsonServer(_hyperion, config["jsonServer"]["port"]);
                std::cout << "Json server created and started on port " << _jsonServer->getPort() << std::endl;
            }
#endif

#ifdef ENABLE_SERVER_BOBLIGHT

            // Create Boblight server if configuration is present
            BoblightServer* boblightServer = nullptr;
            if(config.contains("boblightServer"))
            {
                boblightServer = new BoblightServer(&hyperion, config["boblightServer"]["port"]);
                std::cout << "Boblight server created and started on port " << boblightServer->getPort() << std::endl;
            }
#endif			

            // wait for CTRL-C or kill
            waitForTerminationRequest();

            // Delete all component
#ifdef ENABLE_GRABBER
            if (_frameGrabber != nullptr) {
                _frameGrabber->clearEvent -= Poco::delegate(this, &HyperionService::onClear);
                _frameGrabber->setColorsEvent -= Poco::delegate(this, &HyperionService::onSetColors);
            }

            delete _frameGrabber;
#endif

#ifdef ENABLE_KODI_CONNECTION
            if (_kodiConnector != nullptr) {
                _kodiConnector->videoModeEvent -= Poco::delegate(this, &HyperionService::onSetVideoMode);
                _kodiConnector->grabbingModeEvent -= Poco::delegate(this, &HyperionService::onSetGrabbingMode);
            }

            delete _kodiConnector;
#endif

#ifdef ENABLE_SERVER_JSON
            delete _jsonServer;
#endif

#ifdef ENABLE_SERVER_BOBLIGHT
            delete boblightServer;
#endif
        }
        return ExitCode::EXIT_OK;
    }

protected:
#ifdef ENABLE_KODI_CONNECTION
    void onSetVideoMode (const void *sender, const VideoMode& mode) {
        if (_frameGrabber != nullptr && _kodiConnectorConfig["enable3DDetection"].extract<bool>()) {
            _frameGrabber->setVideoMode(mode);
        }
    }

    void onSetGrabbingMode (const void *sender, const GrabbingMode& mode) {
        if (_frameGrabber != nullptr) {
            GrabbingMode modeToSet = GRABBINGMODE_OFF;

            if (mode == GRABBINGMODE_AUDIO && _kodiConnectorConfig["grabAudio"].extract<bool>()) {
                modeToSet = mode;
            } else if (mode == GRABBINGMODE_VIDEO && _kodiConnectorConfig["grabVideo"].extract<bool>()) {
                modeToSet = mode;
            } else if (mode == GRABBINGMODE_PHOTO && _kodiConnectorConfig["grabPictures"].extract<bool>()) {
                modeToSet = mode;
            } else if (mode == GRABBINGMODE_MENU && _kodiConnectorConfig["grabMenu"].extract<bool>()) {
                modeToSet = mode;
            } else if (mode == GRABBINGMODE_SCREENSAVER && _kodiConnectorConfig["grabScreensaver"].extract<bool>()) {
                modeToSet = mode;
            }
            _frameGrabber->setGrabbingMode(modeToSet);
        }
    }
#endif

    void onClear(const void *sender, const int &priority) {
        _hyperion->clear(priority);
    }

    void onSetColors(const void *sender, ColorSetArgs &colorArgs) {
        _hyperion->setColors(colorArgs.priority, colorArgs.ledColors, colorArgs.timeout_ms, colorArgs.clearEffects);
    }

    void onClearAll(const void *sender) {
        _hyperion->clearall();
    }

private:
    bool _helpRequested;
    Hyperion* _hyperion = nullptr;
#ifdef ENABLE_KODI_CONNECTION
    KodiConnector* _kodiConnector = nullptr;
    Poco::DynamicStruct _kodiConnectorConfig;
#endif
#ifdef ENABLE_GRABBER
    FrameGrabber* _frameGrabber = nullptr;
#endif
#ifdef ENABLE_SERVER_JSON
    JsonServer* _jsonServer = nullptr;
#endif
};

POCO_SERVER_MAIN(HyperionService)