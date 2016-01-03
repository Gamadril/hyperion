
#pragma once

// system includes
#include <cstdint>
#include <string>

#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/BasicEvent.h"
#include "Poco/DynamicStruct.h"
#include "Poco/Runnable.h"

// Hyperion includes
#include "hyperion/Hyperion.h"
#include "utils/KodiConnectorEvents.h"

///
/// This class will check if Kodi is playing something. When it does not, this class will send all black data to Hyperion.
/// This allows grabbed screen data to be overriden while in the Kodi menus.
///
/// Note: The json TCP server needs to be enabled manually in Kodi in System/Settings/Network/Services
///
class KodiConnector: public Poco::Runnable, public KodiConnectorEvents
{
public:
    ///
    /// Constructor
    ///
    /// @param host host name of the device running KODI
    /// @param port port number of the RPC service on the device running KODI
    ///
    KodiConnector(const std::string & host, const std::string & port);
    ~KodiConnector();

    virtual void run();

    void stop();

private:
    GrabbingMode checkActivePlayer();
    bool checkScreensaver();
    VideoMode checkVideoMode();

    /// Set the grabbing mode
    void setGrabbingMode(GrabbingMode grabbingMode);

    /// Set the video mode
    void setVideoMode(VideoMode videoMode);
private:
    Poco::Dynamic::Var sendRequest(Poco::DynamicStruct & request);

private:
    /// The network address of the Kodi instance
    Poco::Net::SocketAddress _address;

    /// The socket with connection to Kodi
    Poco::Net::StreamSocket * _socket;

    Poco::DynamicStruct _requestStub;

    /// Previous emitted grab mode
    GrabbingMode _previousGrabbingMode;

    /// Previous emitted video mode
    VideoMode _previousVideoMode;

    bool _stop;
};
