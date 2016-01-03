#include <algorithm>
#include <stdexcept>

#include "hyperion/PriorityMuxer.h"

PriorityMuxer::PriorityMuxer(unsigned ledCount) :
        _currentPriority(LOWEST_PRIORITY),
        _activeInputs(),
        _lowestPriorityInfo() {
    _lowestPriorityInfo.priority = LOWEST_PRIORITY;
    _lowestPriorityInfo.timeoutTime_ms = -1;
    _lowestPriorityInfo.ledColors = std::vector<ColorRgb>(ledCount, {0, 0, 0});
}

PriorityMuxer::~PriorityMuxer() {
    // empty
}

int PriorityMuxer::getCurrentPriority() const {
    return _currentPriority;
}

std::vector<int> PriorityMuxer::getPriorities() const {
    std::vector<int> keys;
    for (auto imap: _activeInputs)
        keys.push_back(imap.first);

    return keys;
}

bool PriorityMuxer::hasPriority(const int priority) const {
    return _activeInputs.find(priority) != _activeInputs.end();
}

const PriorityMuxer::InputInfo &PriorityMuxer::getInputInfo(const int priority) const {
    if (priority == LOWEST_PRIORITY) {
        return _lowestPriorityInfo;
    }

    auto elemIt = _activeInputs.find(priority);
    if (elemIt == _activeInputs.end()) {
        throw std::runtime_error("no such priority");
    }
    return elemIt->second;
}

void PriorityMuxer::setInput(const int priority, const std::vector<ColorRgb> &ledColors, const int64_t timeoutTime_ms) {
    InputInfo &input = _activeInputs[priority];
    input.priority = priority;
    input.timeoutTime_ms = timeoutTime_ms;
    input.ledColors = ledColors;

    _currentPriority = std::min(_currentPriority, priority);
}

void PriorityMuxer::clearInput(const int priority) {
    _activeInputs.erase(_activeInputs.find(priority));
    if (_currentPriority == priority) {
        if (_activeInputs.empty()) {
            _currentPriority = LOWEST_PRIORITY;
        }
        else {
            _currentPriority = _activeInputs.begin()->first;
        }
    }
}

void PriorityMuxer::clearAll() {
    _activeInputs.clear();
    _currentPriority = LOWEST_PRIORITY;
}

void PriorityMuxer::setCurrentTime(const int64_t &now) {
    _currentPriority = LOWEST_PRIORITY;

    for (auto infoIt = _activeInputs.begin(); infoIt != _activeInputs.end();) {
        if (infoIt->second.timeoutTime_ms != -1 && infoIt->second.timeoutTime_ms <= now) {
            infoIt = _activeInputs.erase(infoIt);
        }
        else {
            _currentPriority = std::min(_currentPriority, infoIt->second.priority);
            ++infoIt;
        }
    }
}
