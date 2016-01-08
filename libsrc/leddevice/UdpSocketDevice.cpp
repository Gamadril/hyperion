#include "Poco/StringTokenizer.h"
#include "Poco/Net/NetException.h"

#include "UdpSocketDevice.h"

UdpSocketDevice::UdpSocketDevice(const std::string &address) :
        _address(Poco::Net::SocketAddress(address)),
        _socket(new Poco::Net::DatagramSocket()) {
}

UdpSocketDevice::~UdpSocketDevice() {
    close();
    delete(_socket);
}

void UdpSocketDevice::close() {
    try {
        _socket->close();
    } catch (...) { }
}

int UdpSocketDevice::writeBytes(const unsigned size, const uint8_t *data) {
    static int errorCounter = 0;

    try {
        _socket->sendTo(data, size, _address);
        errorCounter = 0;
    } catch (const std::exception &e) {
        if (errorCounter < 3) {
            std::cerr << "[ERROR] writing to " << _address.toString() << ": " << e.what() << std::endl;
            errorCounter++;
        }
        try {
            _socket->close();
        } catch (...) { }
        delete(_socket);
        _socket = new Poco::Net::DatagramSocket();

        try {
            _socket->connect(_address);
            _socket->sendBytes(data, size);
            errorCounter = 0;
        } catch (const std::exception &e) {
            if (errorCounter < 3) {
                std::cerr << "[ERROR] " << e.what() << std::endl;
            }
            return -1;
        }
    }

    return 0;
}
