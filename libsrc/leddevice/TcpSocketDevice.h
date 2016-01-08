#pragma once

#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketAddress.h"

#include "leddevice/LedDevice.h"

///
/// The TcpSocketDevice implements an abstract base-class for LedDevices using a tcp socket.
///
class TcpSocketDevice : public LedDevice
{
public:
    ///
    /// Constructs the LedDevice which is connected over tcp socket
    ///
    /// @param[in] address The address of the output device incl. the port (eg 'localhost:10000')
    ///
    TcpSocketDevice(const std::string& address);

    ///
    /// Destructor of the LedDevice; closes the output device if it is open
    ///
    virtual ~TcpSocketDevice();

    ///
    /// Opens and configures the output device
    ///
    /// @param[in] timeout timeout in milliseconds
    /// @return Zero on succes else negative
    ///
    int open(const unsigned timeout = 2000);

    void close();

protected:
    /**
     * Writes the given bytes to the tcp socket
     *
     * @param[in[ size The length of the data
     * @param[in] data The data
     *
     * @return Zero on succes else negative
     */
    int writeBytes(const unsigned size, const uint8_t *data);

private:
    const Poco::Net::SocketAddress _address;
    Poco::Net::StreamSocket * _socket;
};
