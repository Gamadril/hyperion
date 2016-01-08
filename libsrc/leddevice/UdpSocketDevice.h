#pragma once

#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/SocketAddress.h"

#include "leddevice/LedDevice.h"

///
/// The UdpSocketDevice implements an abstract base-class for LedDevices using a udp socket.
///
class UdpSocketDevice : public LedDevice
{
public:
    ///
    /// Constructs the LedDevice which is connected over udp socket
    ///
    /// @param[in] address The address of the output device incl. the port (eg 'localhost:10000')
    ///
    UdpSocketDevice(const std::string& address);

    ///
    /// Destructor of the LedDevice; closes the output device if it is open
    ///
    virtual ~UdpSocketDevice();

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
    Poco::Net::DatagramSocket * _socket;
};
