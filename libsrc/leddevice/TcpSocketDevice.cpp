#include "Poco/StringTokenizer.h"
#include "Poco/Net/NetException.h"

#include "TcpSocketDevice.h"

TcpSocketDevice::TcpSocketDevice(const std::string &address) :
        _address(Poco::Net::SocketAddress(address)),
        _socket(new Poco::Net::StreamSocket()) {
}

TcpSocketDevice::~TcpSocketDevice() {
    close();
    delete(_socket);
}

int TcpSocketDevice::open(const unsigned timeout) {
    std::cout << "Opening TCP socket connection to " << _address.toString() << std::endl;

    try {
        _socket->connect(_address, timeout * 1000);
    } catch (const std::exception &e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
    }

    return 0;
}

void TcpSocketDevice::close() {
    try {
        _socket->close();
    } catch (...) { }
}

int TcpSocketDevice::writeBytes(const unsigned size, const uint8_t *data) {
    static int errorCounter = 0;

    try {
        _socket->sendBytes(data, size);
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
        _socket = new Poco::Net::StreamSocket();

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
