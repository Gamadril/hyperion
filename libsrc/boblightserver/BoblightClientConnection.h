#pragma once

// stl includes
#include <string>

#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/DialogSocket.h"

// Hyperion includes
#include <hyperion/Hyperion.h>

class ImageProcessor;

///
/// The Connection object created by a BoblightServer when a new connection is establshed
///
class BoblightClientConnection : public Poco::Net::TCPServerConnection
{
public:
    ///
    /// Constructor
    /// @param socket The socket
    /// @param hyperion The Hyperion server
    ///
    BoblightClientConnection(Poco::Net::DialogSocket * socket, Hyperion * hyperion);

    ///
    /// Destructor
    ///
    ~BoblightClientConnection();
    
    void run();

private:
    ///
    /// Handle an incoming boblight message
    ///
    /// @param message the incoming message as string
    ///
    void handleMessage(const std::string &message);

    ///
    /// Send a lights message the to connected client
    ///
    void sendLightMessage();

private:
    /// The TCP-Socket that is connected tot the boblight-client
    Poco::Net::DialogSocket * _socket;

    /// The processor for translating images to led-values
    ImageProcessor * _imageProcessor;

    /// Link to Hyperion for writing led-values to a priority channel
    Hyperion * _hyperion;

    /// The priority used by this connection
    int _priority;

    /// The latest led color data
    std::vector<ColorRgb> _ledColors;
};
