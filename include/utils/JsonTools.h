#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/Dynamic/Struct.h"

namespace jsontools
{
	void printJson(Poco::JSON::Object::Ptr obj);
	Poco::JSON::Array arrayToJsonObject(const Poco::Dynamic::Var & var);
	Poco::JSON::Object structToJsonObject(const Poco::DynamicStruct & ds);
}