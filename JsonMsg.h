/*
 * JsonMsg.h
 *
 *  Created on: Apr 16, 2014
 *      Author: zlx
 */
#include <boost/progress.hpp>
#include "sstream"
#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

// utf8转换用
#include <boost/program_options/detail/convert.hpp>
#include <boost/program_options/detail/utf8_codecvt_facet.hpp>

// 包含源代码（注意目录，在libs下，不是boost下），这样就不用库了，和前面的#define BOOST_ALL_NO_LIB配合使用
//#include <libs/program_options/src/utf8_codecvt_facet.cpp>
#ifndef JSONMSG_H_
#define JSONMSG_H_

class JsonMsg:public boost::enable_shared_from_this<JsonMsg>,
	private boost::noncopyable {
public:
	JsonMsg();
	virtual ~JsonMsg();
	void AddDst(std::string dst);
	void AddHost(std::string host);
	std::string GetJsonString();
	std::string MakeJsonString();
	void AddCmd(std::string cmd);
	void AddResult(std::string result);
	void Reset();
	void ParseJson(std::string str);
	std::string GetHost();
	std::string GetCmd();
	std::string GetResult();
	std::vector<std::string> GetDst();
	bool invect(std::string str);

	std::vector<std::string> cmddstVector;
	std::string sid;
	std::string cmdString;
private:
	std::string jsonStr;
	std::string hostStr;

	std::vector<std::string> dstVector;
	std::stringstream jsonStream;

	std::string resultString;
	boost::property_tree::ptree jsonParse;
};
typedef boost::shared_ptr<JsonMsg> JsonMsg_ptr;
#endif /* JSONMSG_H_ */
