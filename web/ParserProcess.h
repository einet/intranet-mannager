#pragma once

#include <string>
#include <boost/lexical_cast.hpp>

#include <boost/regex.hpp>
#include "request_parser.hpp"
#include "request.hpp"
#include "boost/format.hpp"
#include <boost/shared_ptr.hpp>



using namespace std;

using namespace boost;

struct header {
	std::string name;
	std::string value;
};
void trim2(string& str);
//处理参数
namespace http {
namespace server {
typedef std::vector<header> parameterVect;
class CParserProcess {
public:
	CParserProcess(void);
	~CParserProcess(void);
	//得到参数列表
	parameterVect getParameterVect(string & strParameter);
	//文件上载参数获得
	parameterVect getUplodParameterVect(const string & strboundry,
			string& strParameter);

	parameterVect getUplodParameterVect(const string & strboundry,
			char* strParameter, const int len, char*& filestartpos);

private:
	//&分割
	boost::regex regex0;
	parameterVect mp;
};
typedef std::map<string, string> parameterMap;
parameterMap getParameterMap(string & strParameter);
parameterMap getUplodParameterMap(string & strParameter);
}
}
;

