#include <cassert>

#include "HyperionConfig.h"

#include "Poco/RegularExpression.h"
#include "Poco/StringTokenizer.h"
#include "Poco/Timestamp.h"
#include "Poco/Delegate.h"
#include "Poco/NumberParser.h"

#include "hyperion/Hyperion.h"
#include "hyperion/ImageProcessorFactory.h"

#include "leddevice/LedDevice.h"
#include "leddevice/LedDeviceFactory.h"

#include "MultiColorTransform.h"
#include "LinearColorSmoothing.h"

// effect engine includes
#ifdef ENABLE_EFFECT_ENGINE

#include "effectengine/EffectEngine.h"

#endif

ColorOrder Hyperion::createColorOrder(const Poco::DynamicStruct &deviceConfig) {
    std::string order = deviceConfig["colorOrder"];
    if (order == "rgb") {
        return ORDER_RGB;
    }
    else if (order == "bgr") {
        return ORDER_BGR;
    }
    else if (order == "rbg") {
        return ORDER_RBG;
    }
    else if (order == "brg") {
        return ORDER_BRG;
    }
    else if (order == "gbr") {
        return ORDER_GBR;
    }
    else if (order == "grb") {
        return ORDER_GRB;
    }
    else {
        std::cout << "Unknown color order defined (" << order << "). Using RGB." << std::endl;
    }

    return ORDER_RGB;
}

ColorTransform *Hyperion::createColorTransform(const Poco::DynamicStruct &transformConfig) {
    std::string id = "default";
    id = transformConfig["id"].toString();

    RgbChannelTransform *redTransform =
            createRgbChannelTransform(transformConfig["red"].extract<Poco::DynamicStruct>());
    RgbChannelTransform *greenTransform =
            createRgbChannelTransform(transformConfig["green"].extract<Poco::DynamicStruct>());
    RgbChannelTransform *blueTransform =
            createRgbChannelTransform(transformConfig["blue"].extract<Poco::DynamicStruct>());

    HsvTransform *hsvTransform = createHsvTransform(transformConfig["hsv"].extract<Poco::DynamicStruct>());

    ColorTransform *transform = new ColorTransform();
    transform->_id = id;
    transform->_rgbRedTransform = *redTransform;
    transform->_rgbGreenTransform = *greenTransform;
    transform->_rgbBlueTransform = *blueTransform;
    transform->_hsvTransform = *hsvTransform;

    // Cleanup the allocated individual transforms
    delete redTransform;
    delete greenTransform;
    delete blueTransform;
    delete hsvTransform;

    return transform;
}

MultiColorTransform *Hyperion::createLedColorsTransform(const unsigned ledCnt, const Poco::DynamicStruct &colorConfig) {
    // Create the result, the transforms are added to this
    MultiColorTransform *transform = new MultiColorTransform(ledCnt);

    Poco::Dynamic::Var transformConfig = colorConfig["transform"];
    if (!transformConfig.isArray()) {
        ColorTransform *colorTransform = createColorTransform(transformConfig.extract<Poco::DynamicStruct>());
        transform->addTransform(colorTransform);
        transform->setTransformForLed(colorTransform->_id, 0, ledCnt - 1);
    }
    else {
        const Poco::RegularExpression overallExp("([0-9]+(\\-[0-9]+)?)(,[ ]*([0-9]+(\\-[0-9]+)?))*");

        for (unsigned i = 0; i < transformConfig.size(); i++) {
            const Poco::DynamicStruct &config = transformConfig[i].extract<Poco::DynamicStruct>();
            ColorTransform *colorTransform = createColorTransform(config);
            transform->addTransform(colorTransform);

            const std::string ledIndicesStr = config["leds"].toString();
            if (ledIndicesStr.compare("*") == 0) {
                // Special case for indices '*' => all leds
                transform->setTransformForLed(colorTransform->_id, 0, ledCnt - 1);
                std::cout << "ColorTransform '" << colorTransform->_id << "' => [0; " << ledCnt - 1 << "]" << std::endl;
                continue;
            }

            if (!overallExp.match(ledIndicesStr)) {
                std::cerr << "Given led indices " << i << " not correct format: " << ledIndicesStr << std::endl;
                continue;
            }

            std::cout << "ColorTransform '" << colorTransform->_id << "' => [";

            Poco::StringTokenizer ledIndexList(ledIndicesStr, ",", Poco::StringTokenizer::TOK_TRIM);
            for (unsigned j = 0; j < ledIndexList.count(); ++j) {
                if (j > 0) {
                    std::cout << ", ";
                }
                if (ledIndexList[j].find("-") != std::string::npos) {
                    Poco::StringTokenizer ledIndices(ledIndexList[j], "-", Poco::StringTokenizer::TOK_TRIM);
                    const unsigned startInd = Poco::NumberParser::parseUnsigned(ledIndices[0]);
                    const unsigned endInd = Poco::NumberParser::parseUnsigned(ledIndices[1]);

                    transform->setTransformForLed(colorTransform->_id, startInd, endInd);
                    std::cout << startInd << "-" << endInd;
                }
                else {
                    const unsigned index = Poco::NumberParser::parseUnsigned(ledIndexList[j]);
                    transform->setTransformForLed(colorTransform->_id, index, index);
                    std::cout << index;
                }
            }
            std::cout << "]" << std::endl;
        }
    }
    return transform;
}

HsvTransform *Hyperion::createHsvTransform(const Poco::DynamicStruct &hsvConfig) {
    double saturationGain, valueGain;

    saturationGain = hsvConfig["saturationGain"].convert<double>();
    valueGain = hsvConfig["valueGain"].convert<double>();

    return new HsvTransform(saturationGain, valueGain);
}

RgbChannelTransform *Hyperion::createRgbChannelTransform(const Poco::DynamicStruct &colorConfig) {
    double threshold, gamma, blacklevel, whitelevel;

    threshold = colorConfig["threshold"].convert<double>();
    gamma = colorConfig["gamma"].convert<double>();
    blacklevel = colorConfig["blacklevel"].convert<double>();
    whitelevel = colorConfig["whitelevel"].convert<double>();

    RgbChannelTransform *transform = new RgbChannelTransform(threshold, gamma, blacklevel, whitelevel);
    return transform;
}

LedString Hyperion::createLedString(const Poco::Dynamic::Var &ledsConfig, const ColorOrder deviceOrder) {
    LedString ledString;

    if (!ledsConfig.isArray()) {
        throw std::runtime_error("leds config is not valid");
    }

    const std::string deviceOrderStr = colorOrderToString(deviceOrder);
    Poco::DynamicStruct ledConfig;
    for (unsigned i = 0; i < ledsConfig.size(); i++) {
        ledConfig = ledsConfig[i].extract<Poco::DynamicStruct>();
        Led led;
        led.index = ledConfig["index"];

        const Poco::DynamicStruct &hscanConfig = ledConfig["hscan"].extract<Poco::DynamicStruct>();
        const Poco::DynamicStruct &vscanConfig = ledConfig["vscan"].extract<Poco::DynamicStruct>();
        led.minX_frac = std::max(0.0, std::min(1.0, hscanConfig["minimum"].extract<double>()));
        led.maxX_frac = std::max(0.0, std::min(1.0, hscanConfig["maximum"].extract<double>()));
        led.minY_frac = std::max(0.0, std::min(1.0, vscanConfig["minimum"].extract<double>()));
        led.maxY_frac = std::max(0.0, std::min(1.0, vscanConfig["maximum"].extract<double>()));

        // Fix if the user swapped min and max
        if (led.minX_frac > led.maxX_frac) {
            std::swap(led.minX_frac, led.maxX_frac);
        }
        if (led.minY_frac > led.maxY_frac) {
            std::swap(led.minY_frac, led.maxY_frac);
        }

        // Get the order of the rgb channels for this led (default is device order)
        std::string ledOrderStr = deviceOrderStr;
        if (ledConfig.contains("colorOrder"))
            ledOrderStr = ledConfig["colorOrder"].toString();
        led.colorOrder = stringToColorOrder(ledOrderStr);

        ledString.leds().push_back(led);
    }

    // Make sure the leds are sorted (on their indices)
    std::sort(ledString.leds().begin(), ledString.leds().end(), [](const Led &lhs, const Led &rhs) {
        return lhs.index < rhs.index;
    });

    return ledString;
}

LedDevice *Hyperion::createColorSmoothing(const Poco::DynamicStruct &smoothingConfig, LedDevice *ledDevice) {
    std::string type = smoothingConfig["type"].toString();
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);

    if (type == "none") {
        std::cout << "Not creating any smoothing" << std::endl;
        return ledDevice;
    }
    else if (type == "linear") {
        if (!smoothingConfig.contains("time_ms")) {
            std::cout << "Unable to create smoothing of type linear because of missing parameter 'time_ms'"
            << std::endl;
        }
        else if (!smoothingConfig.contains("updateFrequency")) {
            std::cout << "Unable to create smoothing of type linear because of missing parameter 'updateFrequency'"
            << std::endl;
        }
        else {
            unsigned updateDelay = 0;
            updateDelay = smoothingConfig["updateDelay"].extract<int>();
            std::cout << "Creating linear smoothing" << std::endl;
            return new LinearColorSmoothing(ledDevice,
                                            smoothingConfig["updateFrequency"].extract<double>(),
                                            smoothingConfig["time_ms"].extract<int>(),
                                            updateDelay);
        }
    }
    else {
        std::cout << "Unable to create smoothing of type " << type << std::endl;
    }

    return ledDevice;
}

Hyperion::Hyperion(const Poco::DynamicStruct &config)
        : _ledString(
        createLedString(config["leds"], createColorOrder(config["device"].extract<Poco::DynamicStruct>()))),
          _muxer(_ledString.count()), _raw2ledTransform(
                createLedColorsTransform(_ledString.count(), config["color"].extract<Poco::DynamicStruct>())),
          _device(LedDeviceFactory::construct(config["device"].extract<Poco::DynamicStruct>())), _timer(0, 0), _timerRunning() {
    if (_device == nullptr) {
        throw std::runtime_error("[ERROR] LED device could not be created");
    }

    if (!_raw2ledTransform->verifyTransforms()) {
        throw std::runtime_error("Color transformation incorrectly set");
    }
    // initialize the image processor factory
    ImageProcessorFactory::getInstance().init(
            _ledString,
            config["blackborderdetector"]["enable"].extract<bool>(),       // TODO default true
            config["blackborderdetector"]["threshold"].extract<double>()); // TODO default 0.01

    // initialize the color smoothing filter
    _device = createColorSmoothing(config["color"]["smoothing"].extract<Poco::DynamicStruct>(), _device);

#ifdef ENABLE_EFFECT_ENGINE
    // create the effect engine
    _effectEngine = new EffectEngine(this, config["effects"].extract<Poco::DynamicStruct>());
#endif

    stopTimerEvent += Poco::delegate(this, &Hyperion::stopTimer);

    // initialize the leds
    update();
}

Hyperion::~Hyperion() {
    // switch off all leds
    clearall();
    _device->switchOff();

    stopTimerEvent -= Poco::delegate(this, &Hyperion::stopTimer);

#ifdef ENABLE_EFFECT_ENGINE
    // delete the effect engine
    delete _effectEngine;
#endif

    // Delete the Led-Device object
    delete _device;

    // delete the color transform
    delete _raw2ledTransform;
}

unsigned Hyperion::getLedCount() const {
    return _ledString.count();
}

void Hyperion::setColor(int priority, const ColorRgb &color, const int timeout_ms, bool clearEffects) {
    // create led output
    std::vector<ColorRgb> ledColors(_ledString.count(), color);

    // set colors
    setColors(priority, ledColors, timeout_ms, clearEffects);
}

void Hyperion::setColors(int priority, const std::vector<ColorRgb> &ledColors, const int timeout_ms,
                         bool clearEffects) {
#ifdef ENABLE_EFFECT_ENGINE
    // clear effects if this call does not come from an effect
    if (clearEffects) {
        _effectEngine->clearChannel(priority);
    }
#endif

    if (timeout_ms > 0) {
        long timeoutTime = (Poco::Timestamp().epochMicroseconds() / 1000) + timeout_ms;
        _muxer.setInput(priority, ledColors, timeoutTime);
    }
    else {
        _muxer.setInput(priority, ledColors);
    }

    if (priority == _muxer.getCurrentPriority()) {
        update();
    }
}

const std::vector<std::string> &Hyperion::getTransformIds() const {
    return _raw2ledTransform->getTransformIds();
}

ColorTransform *Hyperion::getTransform(const std::string &id) {
    return _raw2ledTransform->getTransform(id);
}

void Hyperion::transformsUpdated() {
    update();
}

void Hyperion::clear(int priority) {
    if (_muxer.hasPriority(priority)) {
        _muxer.clearInput(priority);

        // update leds if necessary
        if (priority < _muxer.getCurrentPriority()) {
            update();
        }
    }

#ifdef ENABLE_EFFECT_ENGINE
    // send clear signal to the effect engine
    // (outside the check so the effect gets cleared even when the effect is not sending colors)
    _effectEngine->clearChannel(priority);
#endif
}

void Hyperion::clearall() {
    _muxer.clearAll();

    // update leds
    update();

#ifdef ENABLE_EFFECT_ENGINE
    // send clearall signal to the effect engine
    _effectEngine->clearAllChannels();
#endif
}

std::vector<int> Hyperion::getActivePriorities() const {
    return _muxer.getPriorities();
}

const Hyperion::InputInfo &Hyperion::getPriorityInfo(const int priority) const {
    return _muxer.getInputInfo(priority);
}

#ifdef ENABLE_EFFECT_ENGINE

const std::list<EffectDefinition> &Hyperion::getEffects() const {
    return _effectEngine->getEffects();
}

int Hyperion::setEffect(const std::string &effectName, int priority, int timeout) {
    return _effectEngine->runEffect(effectName, priority, timeout);
}

int Hyperion::setEffect(const std::string &effectName, const Poco::DynamicStruct &args, int priority, int timeout) {
    return _effectEngine->runEffect(effectName, args, priority, timeout);
}

#endif

void Hyperion::startTimer(long timeout) {
    _timerRunning++;
    static Poco::TimerCallback<Hyperion> timerCallback(*this, &Hyperion::onTimer);
    _timer.setStartInterval(timeout);
    _timer.start(timerCallback);
}

void Hyperion::stopTimer() {
    _timerRunning--;
    _timer.stop();
    update();
}

void Hyperion::update() {
    static Poco::FastMutex mutex;

    if (!mutex.tryLock()) {
        return;
    }

    // Update the muxer, cleaning obsolete priorities
    int64_t now = Poco::Timestamp().epochMicroseconds() / 1000;
    _muxer.setCurrentTime(now);

    // Obtain the current priority channel
    int priority = _muxer.getCurrentPriority();
    PriorityMuxer::InputInfo priorityInfo = _muxer.getInputInfo(priority);
    long timeout_ms = priorityInfo.timeoutTime_ms > 0 ? (priorityInfo.timeoutTime_ms - now) : 0;
    //std::cout << "update() current priorityInfo: " << priorityInfo << " - TO: " << timeout_ms << std::endl;

    // Apply the transform to each led and color-channel
    std::vector<ColorRgb> ledColors = _raw2ledTransform->applyTransform(priorityInfo.ledColors);
    const std::vector<Led> &leds = _ledString.leds();
    unsigned long i = 0;
    for (ColorRgb &color : ledColors) {
        const ColorOrder ledColorOrder = leds.at(i).colorOrder;
        // correct the color byte order
        switch (ledColorOrder) {
            case ORDER_RGB:
                // leave as it is
                break;
            case ORDER_BGR:
                std::swap(color.red, color.blue);
                break;
            case ORDER_RBG:
                std::swap(color.green, color.blue);
                break;
            case ORDER_GRB:
                std::swap(color.red, color.green);
                break;
            case ORDER_GBR: {
                uint8_t temp = color.red;
                color.red = color.green;
                color.green = color.blue;
                color.blue = temp;
                break;
            }
            case ORDER_BRG: {
                uint8_t temp = color.red;
                color.red = color.blue;
                color.blue = color.green;
                color.green = temp;
                break;
            }
        }
        i++;
    }

    // Write the data to the device
    _device->write(ledColors);

    // Start the timeout-timer
    if (timeout_ms > 0) {
        if (_timerRunning == 0) {
            startTimer(timeout_ms);
        }
    } else if (_timerRunning > 0) {
        stopTimer();
    }

    mutex.unlock();
}

void Hyperion::onTimer(Poco::Timer &timer) {
    stopTimerEvent.notifyAsync(nullptr);
}
