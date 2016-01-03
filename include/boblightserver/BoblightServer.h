#pragma once

// system includes
#include <cstdint>

// Qt includes
#include "Poco/Net/TCPServer.h"

// Hyperion includes
#include <hyperion/Hyperion.h>

class BoblightClientConnection;

///
/// This class creates a TCP server which accepts connections from boblight clients.
///
class BoblightServer
{
public:
    ///
    /// BoblightServer constructor
    /// @param hyperion Hyperion instance
    /// @param port port number on which to start listening for connections
    ///
    BoblightServer(Hyperion * hyperion, uint16_t port = 19333);
    ~BoblightServer();

    ///
    /// @return the port number on which this TCP listens for incoming connections
    ///
    uint16_t getPort() const;

private:
    /// Hyperion instance
    Hyperion * _hyperion;

    /// The TCP server object
    Poco::Net::TCPServer * _server;
};
