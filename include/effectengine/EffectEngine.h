#pragma once

#include "Poco/Dynamic/Struct.h"
#include "Poco/Path.h"

#include "hyperion/Hyperion.h"
#include "EffectDefinition.h"

#include "utils/HyperionControlEvents.h"

// pre-declarioation
class Effect;

class EffectEngine : public HyperionControlEvents
{
public:
    EffectEngine(Hyperion * hyperion, const Poco::DynamicStruct & jsonEffectConfig);
    virtual ~EffectEngine();

    const std::list<EffectDefinition> & getEffects() const;

    static bool loadEffectDefinition(const Poco::Path & effectConfigFile, EffectDefinition & effectDefinition);

    /// Run the specified effect on the given priority channel and optionally specify a timeout
    int runEffect(const std::string &effectName, int priority, int timeout = -1);

    /// Run the specified effect on the given priority channel and optionally specify a timeout
    int runEffect(const std::string &effectName, const Poco::DynamicStruct & args, int priority, int timeout = -1);

    /// Clear any effect running on the provided channel
    void clearChannel(int priority);

    /// Clear all effects
    void clearAllChannels();

public:
    void onSetColors(const void *sender, ColorSetArgs &colorArgs);
	void onEffectFinished(const void* sender);

private:
    /// Run the specified effect on the given priority channel and optionally specify a timeout
    int runEffectScript(const std::string &script, const Poco::DynamicStruct & args, int priority, int timeout = -1);

private:
    Hyperion * _hyperion;

    std::list<EffectDefinition> _availableEffects;

    std::list<Effect *> _activeEffects;
};
