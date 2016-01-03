
// Stl includes
#include <fstream>

#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Parser.h"
#include "Poco/File.h"
#include "Poco/Delegate.h"
#include "Poco/Thread.h"
// FIXME make it better
#include "Poco/Util/Application.h"

// effect engine includes
#include "effectengine/EffectEngine.h"
#include "Effect.h"

EffectEngine::EffectEngine(Hyperion * hyperion, const Poco::DynamicStruct & jsonEffectConfig) :
    _hyperion(hyperion),
    _availableEffects(),
    _activeEffects()
{
    std::vector<std::string> effectPaths;

    // look in the local effects subfolder
    std::string appPath = Poco::Util::Application::instance().commandPath();
    Poco::Path p(appPath);
    Poco::Path parent = p.parse(appPath).parent();
    effectPaths.push_back(parent.append("effects").toString());

    // read all effects from config
    Poco::Dynamic::Var paths = jsonEffectConfig["paths"];
    if (paths.isArray())
    {
        for (unsigned i = 0; i < paths.size(); i++)
        {
            std::string path = paths[i].toString();
            effectPaths.push_back(path);
        }
    }

    for (const std::string & effectsPath : effectPaths)
    {
        Poco::File directory(effectsPath);
        if (!directory.exists() || !directory.isDirectory())
        {
            std::cerr << "Effect directory can not be loaded: " << effectsPath << std::endl;
            continue;
        }

        std::vector<Poco::File> files;
        directory.list(files);

        for (const Poco::File & file : files)
        {
            Poco::Path fpath(file.path());

            if (fpath.getExtension() != "json" || !fpath.isFile())
                continue;

            EffectDefinition def;
            if (loadEffectDefinition(fpath, def))
            {
                _availableEffects.push_back(def);
            }
        }
    }
}

EffectEngine::~EffectEngine()
{
}

const std::list<EffectDefinition> &EffectEngine::getEffects() const
{
    return _availableEffects;
}

bool EffectEngine::loadEffectDefinition(const Poco::Path & effectConfigFile, EffectDefinition & effectDefinition)
{
	Poco::Path path(effectConfigFile);
    std::ifstream file(path.toString().c_str());

    if (!file.is_open())
    {
        std::cerr << "Effect file '" << path.getFileName() << "' could not be loaded" << std::endl;
        return false;
    }

    // Read the json config file
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result;
    try
    {
        result = parser.parse(file);
    }
    catch(Poco::JSON::JSONException& ex)
    {
        std::cerr << "Error while reading effect '" << path.getFileName() << "': " << ex.message() << std::endl;
        return false;
    }

    // setup the definition
    Poco::JSON::Object::Ptr effect = result.extract<Poco::JSON::Object::Ptr>();
	std::string script = effect->get("script").toString();
	path.setFileName(script);
    effectDefinition.name = effect->get("name").toString();
    effectDefinition.script = path.toString();
    effectDefinition.args = *(effect->get("args").extract<Poco::JSON::Object::Ptr>());

    // return succes
    std::cout << "Effect loaded: " + effectDefinition.name << std::endl;
    return true;
}

int EffectEngine::runEffect(const std::string & effectName, int priority, int timeout)
{
    return runEffect(effectName, Poco::DynamicStruct(), priority, timeout);
}

int EffectEngine::runEffect(const std::string & effectName, const Poco::DynamicStruct & args, int priority, int timeout)
{
    std::cout << "run effect " << effectName << " on channel " << priority << std::endl;

    const EffectDefinition * effectDefinition = nullptr;
    for (const EffectDefinition & e : _availableEffects)
    {
        if (e.name == effectName)
        {
            effectDefinition = &e;
            break;
        }
    }
    if (effectDefinition == nullptr)
    {
        // no such effect
        std::cerr << "effect " << effectName << " not found" << std::endl;
        return -1;
    }

    return runEffectScript(effectDefinition->script, args.empty() ? effectDefinition->args : args, priority, timeout);
}

int EffectEngine::runEffectScript(const std::string & script, const Poco::DynamicStruct & args, int priority, int timeout)
{
    // clear current effect on the channel
    clearChannel(priority);

    // create the effect
    Effect *effect = new Effect(priority, timeout, script, args);
	effect->setColorsEvent += Poco::delegate(this, &EffectEngine::onSetColors);
	effect->finishedEvent += Poco::delegate(this, &EffectEngine::onEffectFinished);
    _activeEffects.push_back(effect);
    //// start the effect
	Poco::ThreadPool::defaultPool().start(*effect);

    return 0;
}

void EffectEngine::clearChannel(int priority)
{
    for (Effect * effect : _activeEffects)
    {
        if (effect->getPriority() == priority)
        {
            effect->abort();
        }
    }
}

void EffectEngine::clearAllChannels()
{
    for (Effect * effect : _activeEffects)
    {
        effect->abort();
    }
}

void EffectEngine::onSetColors(const void *sender, ColorSetArgs &colorArgs) {
    _hyperion->setColors(colorArgs.priority, colorArgs.ledColors, colorArgs.timeout_ms, colorArgs.clearEffects);
}

void EffectEngine::onEffectFinished(const void* sender)
{
	Effect *effect = (Effect *)sender;

    effect->setColorsEvent -= Poco::delegate(this, &EffectEngine::onSetColors);
	effect->finishedEvent -= Poco::delegate(this, &EffectEngine::onEffectFinished);
	
	if (!effect->isAbortRequested())
	{
		// effect stopped by itself. Clear the channel
		_hyperion->clear(effect->getPriority());
	}
	
	_activeEffects.remove(effect);
	free(effect);
}