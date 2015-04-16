/*
 * JsonMsg.cpp
 *
 *  Created on: Apr 16, 2014
 *      Author: zlx
 */

#include "JsonMsg.h"
#include "CMcast.h"
#include "./web/SeessionID.h"
extern std::vector<std::string> hostvec;

extern ssmap mapHost;
extern Lock ssmapLock;
JsonMsg::JsonMsg() {
	Reset();
}

JsonMsg::~JsonMsg() {

}

void JsonMsg::AddHost(std::string host) {
	hostStr = host;
}
std::string JsonMsg::MakeJsonString() {
	if (cmddstVector.size() == 0) {
		if (hostvec.size() > 0) {
			boost::property_tree::ptree array;
			bool b = false;
			for (unsigned int i = 0; i < hostvec.size(); ++i) {
				if (hostvec[i] == "all") {
					b = true;
				}

			}
			if (!b) {
				for (unsigned int i = 0; i < hostvec.size(); ++i) {

					array.add("name", hostvec[i]);
				}
			} else {
				ReadLock r_lock(ssmapLock);
				BOOST_FOREACH( const ssmap::value_type &iter, mapHost ) {

					array.add("name", iter.first);
				}
			}
			jsonParse.push_back(std::make_pair("dst", array));
		}
	} else {
		if (cmddstVector.size() > 0) {
			boost::property_tree::ptree array;
			bool b = false;
			for (unsigned int i = 0; i < cmddstVector.size(); ++i) {
				if (cmddstVector[i] == "all") {
					b = true;
				}

			}
			if (!b) {
				for (unsigned int i = 0; i < cmddstVector.size(); ++i) {

					array.add("name", cmddstVector[i]);
				}
			} else {

				BOOST_FOREACH( const ssmap::value_type &iter, mapHost ) {

					array.add("name", iter.first);
				}
			}
			jsonParse.push_back(std::make_pair("dst", array));
		}
	}
	if (!hostStr.empty()) {

		jsonParse.add("host", hostStr);
	}
	if (!cmdString.empty())
	{

		jsonParse.add("cmd", cmdString);
	}
	if (!sid.empty()) {
			jsonParse.add("sid", sid);
		}
	if (!resultString.empty())
	{

		jsonParse.add("result", resultString);
	}

	std::stringstream ss;
	boost::property_tree::json_parser::write_json(ss, jsonParse);
	jsonStr = ss.str();
	return jsonStr;
}
std::string JsonMsg::GetJsonString() {
	return jsonStr;
}
void JsonMsg::AddCmd(std::string cmd) {
	cmdString = cmd;
}
void JsonMsg::AddResult(std::string result) {
	resultString = result;
}
bool JsonMsg::invect(std::string str) {
	for (unsigned int i = 0; i < dstVector.size(); ++i) {
		if (dstVector[i] == str) {
			return true;
		} else if (dstVector[i] == "all") {
			return true;
		}

	}
	return false;
}
void JsonMsg::Reset() {
	jsonStr.clear();
	cmdString.clear();

	hostStr.clear();
	resultString.clear();
	jsonStream.clear();
}
void JsonMsg::ParseJson(std::string str) {
	if(str.empty())
		return;
	try {

		Reset();
		jsonStr = str;
		jsonStream.str(str);

		//支持中文输入
		str = boost::property_tree::json_parser::create_escapes(str);

		boost::property_tree::json_parser::read_json(jsonStream, jsonParse);
		boost::property_tree::ptree::const_assoc_iterator it = jsonParse.find(
				"host");

		if (it != jsonParse.not_found())
			hostStr = jsonParse.get<std::string>("host");

		it = jsonParse.find("cmd");
		if (it != jsonParse.not_found())
			cmdString = jsonParse.get<std::string>("cmd");

		it = jsonParse.find("sid");
		if (it != jsonParse.not_found())
			sid = jsonParse.get<std::string>("sid");

		it = jsonParse.find("result");
		if (it != jsonParse.not_found())
			resultString = jsonParse.get<std::string>("result");

		it = jsonParse.find("dst");
		if (it != jsonParse.not_found()) {
			boost::property_tree::ptree pChild = jsonParse.get_child("dst");
			BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pChild) {
				dstVector.push_back(v.second.data());
			}
		}
	} catch (const boost::property_tree::json_parser::json_parser_error& e) {
		cout << "Invalid JSON:" << str << endl;  // Never gets here
	} catch (const std::runtime_error& e) {
		cout << "Invalid JSON:" << str << endl;  // Never gets here
	} catch (...) {
		cout << "Invalid JSON:" << str << endl;  // Never gets here
	}
}
std::string JsonMsg::GetHost() {
	return hostStr;
}
std::string JsonMsg::GetCmd() {
	return cmdString;
}
std::string JsonMsg::GetResult() {
	return resultString;
}

