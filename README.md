HYPERION
========

Hyperion is an opensource 'AmbiLight' implementation.
More information can be found on the [wiki](https://github.com/tvdzwan/hyperion/wiki)

hyperion-poco was created to get rid of Qt4 dependency and make it running on more embedded platforms. Poco C++ library is used instead of Qt4.

Features:
- updated cmake build scripts and made parts of hyperion optional to create custom lightweight versions
- JSON server works only with WebSockets
- EffectEngine was rewrtitten to use Lua instead of Python
- No QT dependency

STATUS: HIGHLY EXPERIMENTAL!

Build for RPi:
`./build.sh rpi -DENABLE_GRABBER_DISPMANX=ON -DENABLE_EFFECT_ENGINE=ON -DENABLE_SERVER_JSON=ON`

add other options for output devices

The source is released under MIT-License (see http://opensource.org/licenses/MIT).
