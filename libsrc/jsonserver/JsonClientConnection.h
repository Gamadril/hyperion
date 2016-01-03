#pragma once

// stl includes
#include <string>

#include "Poco/BasicEvent.h"
#include "Poco/JSON/Object.h"
#include "Poco/Net/WebSocket.h"
#include "utils/DelegateArgs.h"

// Hyperion includes
#include <hyperion/Hyperion.h>

// util includes
//#include <utils/jsonschema/JsonSchemaChecker.h>

class ImageProcessor;

///
/// The Connection object created by \a JsonServer when a new connection is establshed
///
class JsonClientConnection
{
public:
    ///
    /// Constructor
    /// @param socket The Socket object for this connection
    /// @param hyperion The Hyperion server
    ///
    JsonClientConnection(Poco::Net::WebSocket * socket, Hyperion * hyperion);

    ///
    /// Destructor
    ///
    ~JsonClientConnection();

    int processRequest();

	Poco::BasicEvent<ColorSetArgs> setColorsEvent;
    Poco::BasicEvent<const int> clearEvent;
    Poco::BasicEvent<void> clearAllEvent;

private:
    ///
    /// Handle an incoming JSON message
    ///
    /// @param message the incoming message as string
    ///
    void handleMessage(const std::string & message);

    ///
    /// Handle an incoming JSON Color message
    ///
    /// @param message the incoming message
    ///
    void handleColorCommand(const Poco::JSON::Object::Ptr & message);

    ///
    /// Handle an incoming JSON Image message
    ///
    /// @param message the incoming message
    ///
    void handleImageCommand(const Poco::JSON::Object::Ptr & message);

    ///
    /// Handle an incoming JSON Effect message
    ///
    /// @param message the incoming message
    ///
    void handleEffectCommand(const Poco::JSON::Object::Ptr & message);

    ///
    /// Handle an incoming JSON Server info message
    ///
    void handleServerInfoCommand();

    ///
    /// Handle an incoming JSON Clear message
    ///
    /// @param message the incoming message
    ///
    void handleClearCommand(const Poco::JSON::Object::Ptr & message);

    ///
    /// Handle an incoming JSON Clearall message
    ///
    /// @param message the incoming message
    ///
    void handleClearallCommand();

    ///
    /// Handle an incoming JSON Transform message
    ///
    /// @param message the incoming message
    ///
    void handleTransformCommand(const Poco::JSON::Object::Ptr & message);

    ///
    /// Handle an incoming JSON message of unknown type
    ///
    void handleNotImplemented();

    ///
    /// Send a message to the connected client
    ///
    /// @param message The JSON message to send
    ///
    void sendMessage(const Poco::JSON::Object::Ptr & message);

    ///
    /// Send a standard reply indicating success
    ///
    void sendSuccessReply();

    ///
    /// Send an error message back to the client
    ///
    /// @param error String describing the error
    ///
    void sendErrorReply(const std::string & error);

private:
    ///
    /// Check if a JSON messag is valid according to a given JSON schema
    ///
    /// @param message JSON message which need to be checked
    /// @param schemaResource Qt esource identifier with the JSON schema
    /// @param errors Output error message
    ///
    /// @return true if message conforms the given JSON schema
    ///
    //bool checkJson(const Json::Value & message, const QString &schemaResource, std::string & errors);

private:
    /// The WebSocket that is connected tot the Json-client
    Poco::Net::WebSocket * _socket;

    /// The processor for translating images to led-values
    ImageProcessor * _imageProcessor;

    /// Link to Hyperion for writing led-values to a priority channel
    Hyperion * _hyperion;

    char _receiveBuffer[4096];
};
