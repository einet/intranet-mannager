/*
// connection.hpp
 * 功能：主要用于web数据接受和处理，主要相应消息和文件上下载等命令。
 * 文件的上下载采用异步方式，不会对阻塞系统运行
2014-10-22 异步发送文件时文件句柄可能被复用 ，tcp复用keepalive
系统没有考虑单个文件多分段下载要求
*/

#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include <array>
#include <memory>
#include <boost/asio.hpp>
#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"
#include <iostream>
#include <fstream>

using namespace std;
namespace http {
namespace server {

class connection_manager;
typedef struct m_data {
	int first;
	int end;
} m_data;
//读取文件句柄的智能指针
typedef boost::shared_ptr<std::ifstream> is_ptr;

class connection: public std::enable_shared_from_this<connection> {
public:
	connection(const connection&) = delete;
	connection& operator=(const connection&) = delete;

	explicit connection(boost::asio::ip::tcp::socket socket,
			connection_manager& manager, request_handler& handler);

	void start();

	void stop();

	std::string full_path;
	void do_zip_file(const request& req, reply& rep, std::string& extension);
private:

	void do_read();

	void do_write();

	void do_file(const request& req, reply& rep, std::string& extension);

	//接受数据长度
	size_t recv_datalen;
	std::string ctype;
	std::string boundary;

	std::string ansmsg;
	std::string filename;
	std::string fullfilename;
	size_t contentlen;
	size_t f_len;
	bool bheader;
	size_t h_len;
	std::ofstream os;
	bool wfirst;
	size_t f_offset;
	//异步接受文件：用于文件的上传
	void asyncrecv_file();
	//异步发送文件：用于文件的下载
	char buf[8192];
	is_ptr is;
	size_t ifirst;
	size_t iend;
	int readlen;

	void asyncsend_file(is_ptr is,int style,int ifirst, const boost::system::error_code& e);
	boost::asio::ip::tcp::socket socket_;

	connection_manager& connection_manager_;

	request_handler& request_handler_;

	std::array<char, 8192> buffer_;

	request request_;

	request_parser request_parser_;
	bool bkeepalive;

	reply reply_;
	std::string sessionid;
};

typedef std::shared_ptr<connection> connection_ptr;

} // namespace server
} // namespace http

#endif // HTTP_CONNECTION_HPP
