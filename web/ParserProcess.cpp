/*
 *  模块名称：　ParserProcess
 *  功能　　：　处理字符串的转换
 *  作者　　：　张留学
 *  日期　　：　２０１３－０４－０７
 * */
#include "ParserProcess.h"
#include <iostream>
#include <cctype>
#include <algorithm>
#include <vector>
#include <string>
#include <boost/regex.hpp>
void trim2(string& str)
{
   string::size_type pos = str.find_last_not_of(' ');
   if(pos != string::npos)
   {
      str.erase(pos + 1);
      pos = str.find_first_not_of(' ');
      if(pos != string::npos) str.erase(0, pos);
   }
   else str.erase(str.begin(), str.end());
}

bool isdatestr(string str) {
	bool isdate = false;
	{
		//yyyy-MM-dd HH:mm:ss
		//str = "2103-10-03";
		try {
			boost::regex regex1(
					"(\\d{2}|\\d{4})(?:\\-)?([0]{1}\\d{1}|[1]{1}[0-2]{1})(?:\\-)?([0-2]{1}\\d{1}|[3]{1}[0-1]{1})(?:\\s)?([0-1]{1}\\d{1}|[2]{1}[0-3]{1})(?::)?([0-5]{1}\\d{1})(?::)?([0-5]{1}\\d{1})");
			boost::smatch what;

			if (boost::regex_search(str, what, regex1))
				isdate = true;
			else
				isdate = false;
		} catch (const std::exception & ex) {
			std::cerr << ex.what() << std::endl;
			isdate = false;

		}
		/*if(str.length()==19)
		 {
		 if(str.find("-")!=std::string::npos && str.find(":")!=std::string::npos)
		 {
		 isdate = true;
		 }else
		 isdate = false;
		 }*/

	}
	return !isdate;
}



//转换时间字符串
namespace http {
namespace server {
CParserProcess::CParserProcess(void) :
		regex0("&", boost::regbase::normal | boost::regbase::icase) {

}

CParserProcess::~CParserProcess(void) {
}
//得到参数列表
parameterVect CParserProcess::getParameterVect(string & strParameter) {
	std::vector<std::string> value;
	mp.clear();

	boost::regex_split(std::back_inserter(value), strParameter, regex0);
	for (int i = 0; i < (int) value.size(); i++) {
		header h;
		size_t sz = 0;
		sz = value[i].find_first_of("=");
		if (sz != std::string::npos) {
			h.name = value[i].substr(0, sz);
			h.value = value[i].substr(sz + 1);
			mp.push_back(h);
		}
	}
	return mp;
}

int processsdounry(char* p1, string& name, string& value, char *& fpos) {
	char * p = strstr(p1, "Content-Disposition: form-data; name=");
	if (p > 0) {
		p = p + 38;
		char * tp = strstr(p, "\"");
		char * rtp = strstr(p, "\r\n\r\n");
		if (tp > 0)
			tp[0] = '\0';
		name = p;
		if (name == "files[]") {
			char * filename = strstr(p + 8, "filename=");
			if (filename > 0) {
				filename = filename + 10;
				char * tp = strstr(filename, "\"");

				if (tp > 0) {
					tp[0] = '\0';
					name = "filename";
					char * p = strstr(filename, "\r\n");
					if (p > 0)
						p[0] = '\0';
					value = filename;
					fpos = rtp + 4;
					return 2;
				}
			}
		} else if (name == "filedata") {
			char * filename = strstr(p + 9, "filename=");
			if (filename > 0) {
				filename = filename + 10;
				char * tp = strstr(filename, "\"");

				if (tp > 0) {
					tp[0] = '\0';
					name = "filename";
					char * p = strstr(filename, "\r\n");
					if (p > 0)
						p[0] = '\0';
					value = filename;
					fpos = rtp + 4;
					return 2;
				}
			}
		} else {
			char * filename = strstr(p + 9, "filename=");
			if (filename > 0) {
				filename = filename + 10;
				char * tp = strstr(filename, "\"");

				if (tp > 0) {
					tp[0] = '\0';
					name = "filename";
					char * p = strstr(filename, "\r\n");
					if (p > 0)
						p[0] = '\0';
					value = filename;
					fpos = rtp + 4;
					return 2;
				}
			}
		}
		if (rtp > 0) {
			rtp = rtp + 4;
			char * p = strstr(rtp, "\r\n");
			if (p > 0)
				p[0] = '\0';
			value = rtp;
		}
		return 1;
	}
	return 0;
}
parameterVect CParserProcess::getUplodParameterVect(const string & strboundry,
		char* strParameter, const int len, char*& filestartpos) {
	//std::vector<std::string> value;
	//parameterVect mp;
	int size = strboundry.length();
	mp.clear();
	//按分解符号拆分
	char * pp = strstr(strParameter, strboundry.c_str());
	char * tp = NULL;

	if (pp > 0) {
		while (pp > 0) {
			pp = pp + size;
			tp = pp;
			pp = strstr(pp, strboundry.c_str());
			if (pp > 0) {
				pp[0] = '\0';
			}

			string name, value;
			header h;
//查找第一个\r\n\r\n
			if (1 <= processsdounry(tp, h.name, h.value, filestartpos))
				mp.push_back(h);
		}
	}

	return mp;
}

//获取cookie的值
parameterMap getParameterMap(string & strParameter) {
	std::vector<std::string> value;
	parameterMap mp;
	boost::regex regex1(";", boost::regbase::normal | boost::regbase::icase);
	boost::regex_split(std::back_inserter(value), strParameter, regex1);
	for (int i = 0; i < (int) value.size(); i++) {
		header h;
		size_t sz = 0;
		sz = value[i].find_first_of("=");
		if (sz != std::string::npos) {
			string str = value[i].substr(0, sz);
			string svalue = value[i].substr(sz + 1);
			trim2(str);
			trim2(svalue);
			mp[str] = svalue;
		}
	}
	return mp;
}
parameterMap getUplodParameterMap(string & strParameter) {
	std::vector<std::string> value;
	std::size_t last_dot_pos = strParameter.find_last_of("?");

	if (last_dot_pos != std::string::npos) {
		strParameter = strParameter.substr(last_dot_pos + 1);
	}
	parameterMap mp;
	boost::regex regex1("&", boost::regbase::normal | boost::regbase::icase);
	boost::regex_split(std::back_inserter(value), strParameter, regex1);
	for (int i = 0; i < (int) value.size(); i++) {
		header h;
		size_t sz = 0;
		sz = value[i].find_first_of("=");
		if (sz != std::string::npos) {
			string str = value[i].substr(0, sz);
			string svalue = value[i].substr(sz + 1);
			trim2(str);
			trim2(svalue);
			mp[str] = svalue;
		}
	}
	return mp;
}
}
}
