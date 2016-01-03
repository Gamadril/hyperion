// system includes
#include <stdexcept>

#include "Poco/Net/TCPServerConnectionFactory.h"

// project includes
#include <boblightserver/BoblightServer.h>
#include "BoblightClientConnection.h"


class BoblightServerConnectionFactory : public Poco::Net::TCPServerConnectionFactory
{
public:
	BoblightServerConnectionFactory(Hyperion *hyperion):
		_hyperion(hyperion)
	{
	}
	
	Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket)
	{
        std::cout << "New boblight connection" << std::endl;
		return new BoblightClientConnection((Poco::Net::DialogSocket *)&socket, _hyperion);
	}

private:
	Hyperion * _hyperion;
};


BoblightServer::BoblightServer(Hyperion *hyperion, uint16_t port) :
    _hyperion(hyperion)
{
    Poco::Net::ServerSocket serverSocket(port);
    _server = new Poco::Net::TCPServer(new BoblightServerConnectionFactory(_hyperion), serverSocket);
    _server->start();
}

BoblightServer::~BoblightServer()
{
    _server->stop();
    delete _server;
}

uint16_t BoblightServer::getPort() const
{
    return _server->port();
}
