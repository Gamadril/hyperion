// system includes
#include <stdexcept>

// project includes
#include <jsonserver/JsonServer.h>
#include "JsonClientConnection.h"

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Delegate.h"

class WebSocketRequestHandler: public Poco::Net::HTTPRequestHandler
{
public:
    WebSocketRequestHandler(Hyperion *hyperion) :
        _hyperion(hyperion)
    {
    }

    void handleRequest(Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response)
    {
        try
        {
            Poco::Net::WebSocket socket(request, response);

            std::cout << "New json connection from " << request.clientAddress().toString() << std::endl;
            JsonClientConnection connection(&socket, _hyperion);
//			connection.setColorsEvent += Poco::delegate(_hyperion, &Hyperion::setColorsD);
//            connection.clearEvent += Poco::delegate(_hyperion, &Hyperion::clearD);
//            connection.clearAllEvent += Poco::delegate(_hyperion, &Hyperion::clearAllD);

			int result;
			do
			{
				result = connection.processRequest();
			}
			while (result == 0);
			std::cout << "Connection to " << request.clientAddress().toString() << " closed" << std::endl;
//			connection.setColorsEvent -= Poco::delegate(_hyperion, &Hyperion::setColorsD);
//            connection.clearEvent -= Poco::delegate(_hyperion, &Hyperion::clearD);
//            connection.clearAllEvent -= Poco::delegate(_hyperion, &Hyperion::clearAllD);
        }
        catch (Poco::Net::WebSocketException & exc)
        {
			std::cout << exc.what() << std::endl;
            switch (exc.code())
            {
                case Poco::Net::WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
                    response.set("Sec-WebSocket-Version", Poco::Net::WebSocket::WEBSOCKET_VERSION);
                // fallthrough
                case Poco::Net::WebSocket::WS_ERR_NO_HANDSHAKE:
                case Poco::Net::WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
                case Poco::Net::WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
                    response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
                    response.setContentLength(0);
                    response.send();
                    break;
            }
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] handleRequest: " << e.what() << std::endl;
        }
    }
private:
    Hyperion * _hyperion;
};

class WebSocketRequestHandlerFactory: public Poco::Net::HTTPRequestHandlerFactory
{
public:
    WebSocketRequestHandlerFactory(Hyperion *hyperion) :
        _hyperion(hyperion)
    {
    }

    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request)
    {
        return new WebSocketRequestHandler(_hyperion);
    }
private:
    Hyperion * _hyperion;
};



JsonServer::JsonServer(Hyperion *hyperion, uint16_t port) :
    _hyperion(hyperion)
{
    Poco::Net::ServerSocket serverSocket(port);
    _server = new Poco::Net::HTTPServer(new WebSocketRequestHandlerFactory(_hyperion), serverSocket, new Poco::Net::HTTPServerParams);
    _server->start();
}

JsonServer::~JsonServer()
{
    _server->stop();
    delete _server;
}

uint16_t JsonServer::getPort() const
{
    return _server->port();
}
