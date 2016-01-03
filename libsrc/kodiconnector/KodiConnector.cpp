#include "kodiconnector/KodiConnector.h"


// Request player example:
// {"id":1,"jsonrpc":"2.0","method":"Player.GetActivePlayers"}
// {"id":1,"jsonrpc":"2.0","result":[{"playerid":1,"type":"video"}]}
// {"id":1,"jsonrpc":"2.0","result":[{"playerid":0,"type":"audio"},{"playerid":2,"type":"picture"}]}

// Request if screensaver is on
// {"id":1,"jsonrpc":"2.0","method":"XBMC.GetInfoBooleans","params":{"booleans":["System.ScreenSaverActive"]}}
// {"id":1,"jsonrpc":"2.0","result":{"System.ScreenSaverActive":false}}

// Request stereoscopicmode example:
// {"id":1,"jsonrpc":"2.0","method":"GUI.GetProperties","params":{"properties":["stereoscopicmode"]}}
// {"id":1,"jsonrpc":"2.0","result":{"stereoscopicmode":{"label":"Nebeneinander","mode":"split_vertical"}}}

KodiConnector::KodiConnector(const std::string &host, const std::string &port) :
        _address(Poco::Net::SocketAddress(host + ":" + port)),
        _socket(nullptr),
        _requestStub(),
        _previousGrabbingMode(GRABBINGMODE_OFF),
        _previousVideoMode(VIDEO_2D),
        _stop (false)
{
    if (_address.port() == 0) {
        throw std::invalid_argument("KodiConnector: missing port");
    }

    _requestStub["jsonrpc"] = "2.0";
    _requestStub["id"] = 1;
}

KodiConnector::~KodiConnector() {
    delete(_socket);
}

void KodiConnector::run() {
    GrabbingMode grabbingMode;
    VideoMode videoMode;

    while (!_stop) {
        if (checkScreensaver()) {
            grabbingMode = GRABBINGMODE_SCREENSAVER;
        } else {
            grabbingMode = checkActivePlayer();
        }

        if (grabbingMode != _previousGrabbingMode) {
            grabbingModeEvent.notifyAsync(this, grabbingMode);
            _previousGrabbingMode = grabbingMode;
        }

        videoMode = checkVideoMode();

        if (videoMode != _previousVideoMode) {
            videoModeEvent.notifyAsync(this, videoMode);
            _previousVideoMode = videoMode;
        }

        sleep(1);
    }
}

void KodiConnector::stop() {
    _stop = true;
}

Poco::Dynamic::Var KodiConnector::sendRequest(Poco::DynamicStruct &request) {
    static int errorCounter = 0;
    char buffer[2048];
    int read = 0;
    std::string reqStr;
    reqStr = request.toString();

    _socket = new Poco::Net::StreamSocket();
    try {
        _socket->connect(_address);
        _socket->setReceiveTimeout(Poco::Timespan(1, 0));
        _socket->sendBytes(reqStr.c_str(), (int) reqStr.size());
        errorCounter = 0;
        read = _socket->receiveBytes(buffer, sizeof(buffer));

        if (read == 0) {
            // graceful shutdown of the connection from the peer
            _socket->close();
            delete _socket;
            _socket = nullptr;
            return Poco::Dynamic::Var();
        }
    } catch (const std::exception &e) {
        if (errorCounter < 3) {
            std::cerr << "[ERROR] KodiConnector: " << e.what() << std::endl;
        }
        errorCounter++;
        _socket->close();
        delete _socket;
        _socket = nullptr;

        return Poco::Dynamic::Var();
    }
    _socket->close();
    delete _socket;
    _socket = nullptr;

    Poco::DynamicStruct response = Poco::Dynamic::Var::parse(
            std::string(buffer, (unsigned long) read)).extract<Poco::DynamicStruct>();

    return response["result"];
}

VideoMode KodiConnector::checkVideoMode() {
    VideoMode result = VIDEO_2D;
    Poco::DynamicStruct request = Poco::DynamicStruct(_requestStub);
    request["method"] = "GUI.GetProperties";

    Poco::DynamicStruct params;
    std::vector<Poco::Dynamic::Var> properties;
    properties.push_back("stereoscopicmode");
    params["properties"] = properties;
    request["params"] = params;

    Poco::Dynamic::Var response = sendRequest(request);

    if (!response.isEmpty()) {
        std::string mode = response["stereoscopicmode"]["mode"];

        if (mode == "split_horizontal") {
            result = VIDEO_3DTAB;
        } else if (mode == "split_vertical") {
            result = VIDEO_3DSBS;
        }
    }

    return result;
}

GrabbingMode KodiConnector::checkActivePlayer() {
    GrabbingMode result = GRABBINGMODE_OFF;
    Poco::DynamicStruct request = Poco::DynamicStruct(_requestStub);
    request["method"] = "Player.GetActivePlayers";

    Poco::Dynamic::Var response = sendRequest(request);

    if (!response.isEmpty()) {
        if (response.size() > 0) {
            Poco::DynamicStruct playerInfo = response[response.size() - 1].extract<Poco::DynamicStruct>();
            std::string playerType = playerInfo["type"];

            if (playerType == "video") {
                // video is playing
                result = GRABBINGMODE_VIDEO;
            } else if (playerType == "picture") {
                // picture viewer is playing
                result = GRABBINGMODE_PHOTO;
            } else if (playerType == "audio") {
                // audio is playing
                result = GRABBINGMODE_AUDIO;
            }
        } else {
            result = GRABBINGMODE_MENU;
        }
    }

    return result;
}

bool KodiConnector::checkScreensaver() {
    Poco::DynamicStruct request = Poco::DynamicStruct(_requestStub);
    request["method"] = "XBMC.GetInfoBooleans";

    Poco::DynamicStruct params;
    std::vector<Poco::Dynamic::Var> properties;
    properties.push_back("System.ScreenSaverActive");
    params["booleans"] = properties;
    request["params"] = params;

    Poco::Dynamic::Var response = sendRequest(request);

    if (response.isEmpty()) {
        return false;
    }

    return response["System.ScreenSaverActive"];
}
