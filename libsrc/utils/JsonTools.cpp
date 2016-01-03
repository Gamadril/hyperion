#include "utils/JsonTools.h"

namespace jsontools
{
	void printJson(Poco::JSON::Object::Ptr obj)
	{
	    std::ostringstream stream;
	    obj->stringify(stream);
	    std::string serialized = stream.str();
		std::cout << serialized << std::endl;
	}
	
	Poco::JSON::Array arrayToJsonObject(const Poco::Dynamic::Var & var)
	{
		Poco::JSON::Array arr;
	
		Poco::Dynamic::Var::ConstIterator it  = var.begin();
		Poco::Dynamic::Var::ConstIterator end = var.end();
		for (; it != end; ++it)
		{
			if (it->isStruct())
			{
				Poco::JSON::Object sobj = jsontools::structToJsonObject((*it).extract<Poco::DynamicStruct>());
				arr.add(sobj);
			}
			else if (it->isArray())
			{
				Poco::JSON::Array la = jsontools::arrayToJsonObject(it);
				arr.add(la);
			}
			else if (it->isInteger())
				arr.add(it->convert<int>());
			else if (it->isBoolean())
				arr.add(it->convert<bool>());
			else if (it->isNumeric())
				arr.add(it->convert<double>());
			else if (it->isString())
				arr.add(it->convert<std::string>());
		}
	
		return arr;
	}


	Poco::JSON::Object structToJsonObject(const Poco::DynamicStruct & ds)
	{
		Poco::JSON::Object obj;
	
		Poco::DynamicStruct::ConstIterator it  = ds.begin();
		Poco::DynamicStruct::ConstIterator end = ds.end();
		for (; it != end; ++it)
		{
			if (it->second.isStruct())
			{
				Poco::JSON::Object sobj = jsontools::structToJsonObject(it->second.extract<Poco::DynamicStruct>());
				obj.set(it->first, sobj);
			}
			else if (it->second.isArray())
			{
				Poco::JSON::Array arr = jsontools::arrayToJsonObject(it->second);
				obj.set(it->first, arr);
			}
			else
				obj.set(it->first, it->second);
		}
	
		return obj;
	}
}