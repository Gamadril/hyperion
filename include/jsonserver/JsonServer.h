#pragma once

// system includes
#include <cstdint>

#include "Poco/Net/HTTPServer.h"

// Hyperion includes
#include <hyperion/Hyperion.h>

class JsonClientConnection;

///
/// This class creates a TCP server which accepts connections wich can then send
/// in JSON encoded commands. This interface to Hyperion is used by hyperion-remote
/// to control the leds
///
class JsonServer
{
public:
    ///
    /// JsonServer constructor
    /// @param hyperion Hyperion instance
    /// @param port port number on which to start listening for connections
    ///
    JsonServer(Hyperion * hyperion, uint16_t port = 19444);
    ~JsonServer();

    ///
    /// @return the port number on which this TCP listens for incoming connections
    ///
    uint16_t getPort() const;

private:
    /// Hyperion instance
    Hyperion * _hyperion;

    /// The TCP server object
    Poco::Net::HTTPServer * _server;
};
