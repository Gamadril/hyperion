#pragma once

#include "Poco/Dynamic/Struct.h"
#include "Poco/BasicEvent.h"
#include "Poco/Runnable.h"

// Hyperion includes
#include "hyperion/ImageProcessor.h"
#include "utils/DelegateArgs.h"
#include "utils/HyperionControlEvents.h"

extern "C"
{
   #include "lua.h"
   #include "lauxlib.h"
   #include "lualib.h"
}

class Effect: public Poco::Runnable, public HyperionControlEvents
{
public:
    Effect(int priority, int timeout, const std::string & script, const Poco::Dynamic::Var & args);
    virtual ~Effect();

    virtual void run();

    int getPriority() const;

    bool isAbortRequested() const;

    void abort();
	
	void setColor(lua_State *state);
    void setImage(lua_State *state);
	bool shouldFinish();

	Poco::BasicEvent<void> finishedEvent;
	
private:	
	void setArgumentsToLua(const Poco::Dynamic::Var & obj, lua_State *state);

private:
    const int _priority;

    const int _timeout;

    const std::string _script;

    const Poco::Dynamic::Var _args;

    int64_t _endTime;

    bool _abortRequested;

    /// The processor for translating images to led-values
    ImageProcessor * _imageProcessor;

    /// Buffer for colorData
    std::vector<ColorRgb> _colors;
};
