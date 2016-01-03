// stl includes
#include <iostream>
#include <sstream>
#include <thread>

#include "Poco/Timestamp.h"

// effect engin eincludes
#include "Effect.h"
#include "utils/HsvTransform.h"

namespace {

    static int lua_setColor(lua_State *state) {
        Effect **effect = static_cast<Effect **>(luaL_checkudata(state, 1, "EffectMT"));
        (*effect)->setColor(state);
        return 0;
    }

    static int lua_setImage(lua_State *state) {
        Effect **effect = static_cast<Effect **>(luaL_checkudata(state, 1, "EffectMT"));
        (*effect)->setImage(state);
        return 0;
    }

    static int lua_abort(lua_State *state) {
        Effect **effect = static_cast<Effect **>(luaL_checkudata(state, 1, "EffectMT"));
        bool toAbort = (*effect)->shouldFinish();
        lua_pushboolean(state, toAbort);
        return 1;
    }

    static int lua_sleep(lua_State *state) {
        int ms = static_cast<int> (luaL_checknumber(state, 2));
        std::this_thread::sleep_for(std::chrono::milliseconds{ms});
        return 0;
    }

    static int lua_rgb_to_hsv(lua_State *state) {
        uint16_t h;
        uint8_t s, v,
                r = static_cast<uint8_t> (luaL_checknumber(state, 1)),
                g = static_cast<uint8_t> (luaL_checknumber(state, 2)),
                b = static_cast<uint8_t> (luaL_checknumber(state, 3));

        HsvTransform::rgb2hsv(r, g, b, h, s, v);

        lua_newtable(state);

        lua_pushinteger(state, 1);
        lua_pushinteger(state, h);
        lua_settable(state, -3);

        lua_pushinteger(state, 2);
        lua_pushinteger(state, s);
        lua_settable(state, -3);

        lua_pushinteger(state, 3);
        lua_pushinteger(state, v);
        lua_settable(state, -3);

        return 1;
    }


    /**
     * Convert HSV value to RGB
     * H [0..359]
     * S [0..255]
     * V [0..255]
     */
    static int lua_hsv_to_rgb(lua_State *state) {
        uint16_t h = static_cast<uint16_t> (luaL_checknumber(state, 1));
        uint8_t r, g, b,
                s = static_cast<uint8_t> (luaL_checknumber(state, 2)),
                v = static_cast<uint8_t> (luaL_checknumber(state, 3));

        HsvTransform::hsv2rgb(h, s, v, r, g, b);

        lua_newtable(state);

        lua_pushinteger(state, 1);
        lua_pushinteger(state, r);
        lua_settable(state, -3);

        lua_pushinteger(state, 2);
        lua_pushinteger(state, g);
        lua_settable(state, -3);

        lua_pushinteger(state, 3);
        lua_pushinteger(state, b);
        lua_settable(state, -3);

        return 1;
    }



/*
void stackdump_g(lua_State* l)
{
    int i;
    int top = lua_gettop(l);

    printf("total in stack %d\n",top);

    for (i = 1; i <= top; i++)
    {
        int t = lua_type(l, i);
        switch (t) {
            case LUA_TSTRING:  
                printf("string: '%s'\n", lua_tostring(l, i));
                break;
            case LUA_TBOOLEAN:  
                printf("boolean %s\n",lua_toboolean(l, i) ? "true" : "false");
                break;
            case LUA_TNUMBER:  
                printf("number: %g\n", lua_tonumber(l, i));
                break;
            default:  
                printf("%s\n", lua_typename(l, t));
                break;
        }
        printf("  "); 
    }
    printf("\n"); 
}
*/
}

Effect::Effect(int priority, int timeout, const std::string &script, const Poco::Dynamic::Var &args) :
        _priority(priority),
        _timeout(timeout),
        _script(script),
        _args(args),
        _endTime(-1),
        _abortRequested(false),
        _imageProcessor(ImageProcessorFactory::getInstance().newImageProcessor()),
        _colors() {
    _colors.resize(_imageProcessor->getLedCount(), ColorRgb::BLACK);

    // disable the black border detector for effects
    _imageProcessor->enableBalckBorderDetector(false);
}

Effect::~Effect() {
}

void Effect::setColor(lua_State *state) {
    // check if we have aborted already
    if (shouldFinish())
    {
        return;
    }

    // check the number of arguments
    if (lua_gettop(state) == 4) {
        // three seperate arguments for red, green, and blue
        ColorRgb color;
        color.red = (uint8_t) luaL_checkinteger(state, 2);
        color.green = (uint8_t) luaL_checkinteger(state, 3);
        color.blue = (uint8_t) luaL_checkinteger(state, 4);

        std::fill(_colors.begin(), _colors.end(), color);
        ColorSetArgs args = {_priority, _colors, 0, false};
        setColorsEvent.notify(this, args);
    }
    else if (lua_gettop(state) == 2 && lua_istable(state, -1)) {
        // one argument with array/table of colors (also a table with RGB values)
        lua_len(state, -1);
        int count = (int) lua_tointeger(state, -1);
        lua_pop(state, 1);

        for (int i = 1; i <= count; i++) {
            lua_rawgeti(state, 2, i);

            ColorRgb color;
            lua_rawgeti(state, -1, 1);
            color.red = (uint8_t) luaL_checkinteger(state, -1);
            lua_pop(state, 1);

            lua_rawgeti(state, -1, 2);
            color.green = (uint8_t) luaL_checkinteger(state, -1);
            lua_pop(state, 1);

            lua_rawgeti(state, -1, 3);
            color.blue = (uint8_t) luaL_checkinteger(state, -1);
            lua_pop(state, 1);
            _colors[i-1] = color;
            //std::cout << i << ": " << color << std::endl;

            lua_pop(state, 1);
        }
        ColorSetArgs args = {_priority, _colors, 0, false};
        setColorsEvent.notify(this, args);
    }
}

void Effect::setImage(lua_State *state) {
    // check if we have aborted already
    if (shouldFinish())
    {
        return;
    }

    // bytearray of values
    int width = (int) luaL_checkinteger(state, 2), height = (int) luaL_checkinteger(state, 3);

    if (lua_istable(state, 4)) {
        lua_len(state, -1);
        const unsigned count = (unsigned) lua_tointeger(state, -1);
        lua_pop(state, 1);

        if (count == unsigned(width * height))
        {
            uint8_t data[count * 3];

            for (unsigned i = 0; i < count; i++) {
                lua_rawgeti(state, 4, i+1);

                lua_rawgeti(state, -1, 1);
                data[i*3] = (uint8_t) luaL_checkinteger(state, -1);
                lua_pop(state, 1);

                lua_rawgeti(state, -1, 2);
                data[i*3+1] = (uint8_t) luaL_checkinteger(state, -1);
                lua_pop(state, 1);

                lua_rawgeti(state, -1, 3);
                data[i*3+2] = (uint8_t) luaL_checkinteger(state, -1);
                lua_pop(state, 1);

                lua_pop(state, 1);
            }
            Image<ColorRgb> image(width, height);
            memcpy(image.memptr(), data, count * 3);
            _imageProcessor->process(image, _colors);
            ColorSetArgs args = {_priority, _colors, 0, false};
            setColorsEvent.notify(this, args);
        }
        else
        {
            std::cerr << "[Effect] ledData size does not match image size" << std::endl;
        }
    }
}

void Effect::run() {
    if (_timeout > 0) {
        _endTime = (Poco::Timestamp().epochMicroseconds() / 1000) + _timeout;
    }

    // create new Lua state
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    luaL_newmetatable(L, "EffectMT");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lua_setColor);
    lua_setfield(L, -2, "setColor");
    lua_pushcfunction(L, lua_setImage);
    lua_setfield(L, -2, "setImage");
    lua_pushcfunction(L, lua_abort);
    lua_setfield(L, -2, "abort");
    lua_pushcfunction(L, lua_sleep);
    lua_setfield(L, -2, "sleep");

    // add ledCount variable
    lua_pushinteger(L, _imageProcessor->getLedCount());
    lua_setfield(L, -2, "ledCount");

    setArgumentsToLua(_args, L);
    lua_setfield(L, -2, "args");

    Effect **ud = static_cast<Effect **>(lua_newuserdata(L, sizeof(Effect *)));
    *(ud) = this;
    luaL_setmetatable(L, "EffectMT");
    lua_setglobal(L, "hyperion");

    luaL_newmetatable(L, "ColorsMT");
    lua_pushcfunction(L, lua_rgb_to_hsv);
    lua_setfield(L, -2, "rgb2hsv");
    lua_pushcfunction(L, lua_hsv_to_rgb);
    lua_setfield(L, -2, "hsv2rgb");
    luaL_setmetatable(L, "ColorsMT");
    lua_setglobal(L, "colors");

    lua_settop(L, 0); //empty the lua stack
    if (luaL_dofile(L, _script.c_str())) {
        fprintf(stderr, "error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    // close the Lua state
    lua_close(L);

    finishedEvent.notify(this);
}

int Effect::getPriority() const {
    return _priority;
}

bool Effect::isAbortRequested() const {
    return _abortRequested;
}

bool Effect::shouldFinish() {
    // Test if the effect has reached it end time
    if (_timeout > 0)
    {
        if ((Poco::Timestamp().epochMicroseconds() / 1000) > _endTime) {
            return true;
        }
    }
    return _abortRequested;
}

void Effect::abort() {
    _abortRequested = true;
}

void Effect::setArgumentsToLua(const Poco::Dynamic::Var &obj, lua_State *state) {
    //std::cout << obj.toString() << std::endl;

    if (obj.isEmpty())
        return;

    if (obj.isNumeric()) {
        lua_pushnumber(state, obj.convert<double>());
    }
    else if (obj.isInteger()) {
        lua_pushinteger(state, obj.convert<int>());
    }
    else if (obj.isBoolean()) {
        lua_pushboolean(state, obj.convert<bool>());
    }
    else if (obj.isString()) {
        lua_pushstring(state, obj.toString().c_str());
    }
    else if (obj.isStruct()) {
        lua_newtable(state);
        Poco::DynamicStruct str = obj.extract<Poco::DynamicStruct>();
        for (auto name : str.members()) {
            setArgumentsToLua(str[name], state);
            lua_setfield(state, -2, name.c_str());
        }
    }
    else if (obj.isArray()) {
        lua_newtable(state);
        for (unsigned i = 0; i < obj.size(); i++) {
            lua_pushinteger(state, i + 1); // lua's index starts at 1
            setArgumentsToLua(obj[i], state);
            lua_settable(state, -3);
        }
    }
}
