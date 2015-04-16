//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request_handler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"
#include <sys/stat.h>
#include "boost/format.hpp"
#include "boost/lexical_cast.hpp"
namespace http {
namespace server {

request_handler::request_handler(const std::string& doc_root) :
		doc_root_(doc_root) {
}
std::string request_handler::get_fullpath(const request& req, reply& rep,std::string& extension) {
	// Decode url to path.
	std::string request_path;
	if (!url_decode(req.uri, request_path)) {
		rep = reply::stock_reply(reply::bad_request);
		return "";
	}

	// Request path must be absolute and not contain "..".
	if (request_path.empty() || request_path[0] != '/'
			|| request_path.find("..") != std::string::npos) {
		rep = reply::stock_reply(reply::bad_request);
		return "";
	}

	// If path ends in slash (i.e. is a directory) then add "index.html".
	if (request_path[request_path.size() - 1] == '/') {
		request_path += "index.html";
	}

	// Determine the file extension.
	std::size_t last_slash_pos = request_path.find_last_of("/");
	std::size_t last_dot_pos = request_path.find_last_of(".");

	if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos) {
		extension = request_path.substr(last_dot_pos + 1);
	}
	if(request_path.find(doc_root_)==0)
		return request_path;
	// return full_path
	return doc_root_ + request_path;
}
void request_handler::handle_request(const request& req, reply& rep) {
	// Decode url to path.
	std::string request_path;
	if (!url_decode(req.uri, request_path)) {
		rep = reply::stock_reply(reply::bad_request);
		return;
	}

	// Request path must be absolute and not contain "..".
	if (request_path.empty() || request_path[0] != '/'
			|| request_path.find("..") != std::string::npos) {
		rep = reply::stock_reply(reply::bad_request);
		return;
	}

	// If path ends in slash (i.e. is a directory) then add "index.html".
	if (request_path[request_path.size() - 1] == '/') {
		request_path += "index.html";
	}

	// Determine the file extension.
	std::size_t last_slash_pos = request_path.find_last_of("/");
	std::size_t last_dot_pos = request_path.find_last_of(".");
	std::string extension;
	if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos) {
		extension = request_path.substr(last_dot_pos + 1);
	}

	// Open the file to send back.
	std::string full_path = doc_root_ + request_path;
	std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
	if (!is) {
		rep = reply::stock_reply(reply::not_found);
		return;
	}
	//RANGE: bytes=2000070-
	//Content-Range=bytes 2000070-106786027/106786028
	//接受完所有参数信息
	int istart = -1;
	int iend = -1;
	for (int i = 0; i < (int) req.headers.size(); i++) {
		if (req.headers[i].name == "RANGE") {
			std::string mstr = req.headers[i].value;
			size_t so = mstr.find("bytes=");
			if (so != std::string::npos) {
				so = so + 6;
				size_t se = mstr.find("-");
				if (se != std::string::npos) {
					istart = atoi(req.headers[i].value.c_str() + so);
					iend = atoi(req.headers[i].value.c_str() + se + 1);
				}
			}
			break;
		}
	}
	struct stat st;
	stat(full_path.c_str(), &st);
	size_t size = st.st_size;

	//如果长度大于1M则启用断点续传
	if (istart == -1 || size <= 1024 * 1024) {
		rep.headers.resize(2);
		rep.status = reply::ok;
		char buf[1024];

		while (is.read(buf, sizeof(buf)).gcount() > 0)
			rep.content.append(buf, is.gcount());

		rep.headers[0].name = "Content-Length";
		rep.headers[0].value = std::to_string(rep.content.size());
		rep.headers[1].name = "Content-Type";
		rep.headers[1].value = mime_types::extension_to_type(extension);

	} else {
		boost::format fmter("bytes %d-%d/%d");

		fmter % "true" % 1 % size;
		char buf[1024];
		bool first = false;
		if (istart > 0)
			is.seekg(istart);
		rep.headers.resize(3);
		rep.status = reply::partial_content;
		rep.headers[0].name = "Content-Length";
		rep.headers[0].value = std::to_string(rep.content.size());
		rep.headers[1].name = "Content-Type";
		rep.headers[1].value = mime_types::extension_to_type(extension);
		rep.headers[2].name = "Content-Range";
		rep.headers[2].value = std::to_string(rep.content.size());
		int len = 0;
		int readlen = size;
		if (iend > 0) {
			readlen = iend - istart;
			if (readlen < 0)
				readlen = size;
		}
		while (readlen > 0) {
			int plen = 1024;
			if (readlen > 1024) {
				len = is.readsome(buf, 1024);
			} else {
				len = is.readsome(buf, readlen);
				plen = readlen;
			}
			readlen = readlen - len;
			if (!first) {
				rep.content.append(buf, is.gcount());
				first = true;
				if (iend <= 0) {
					fmter % istart % (size - 1) % size;
				} else {
					fmter % istart % (iend - istart) % size;
				}

			} else {

			}
		}

	}

}

bool request_handler::url_decode(const std::string& in, std::string& out) {
	out.clear();
	out.reserve(in.size());
	for (std::size_t i = 0; i < in.size(); ++i) {
		if (in[i] == '%') {
			if (i + 3 <= in.size()) {
				int value = 0;
				std::istringstream is(in.substr(i + 1, 2));
				if (is >> std::hex >> value) {
					out += static_cast<char>(value);
					i += 2;
				} else {
					return false;
				}
			} else {
				return false;
			}
		} else if (in[i] == '+') {
			out += ' ';
		} else {
			out += in[i];
		}
	}
	return true;
}

} // namespace server
} // namespace http
