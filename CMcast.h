/*
 * CMcast.h
 *
 *  Created on: Apr 8, 2014
 *      Author: zlx
 */

#ifndef CMCAST_H_
#define CMCAST_H_

#include <iostream>
#include <sstream>
#include <string>
#include <boost/asio.hpp>
#include "boost/bind.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "boost/threadpool.hpp"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iomanip>
#include "GzipProcess.h"
#include <sys/types.h>
#include <sys/wait.h>

//const short multicast_port = 32001;
const int max_message_count = 10;
using namespace boost::threadpool;

typedef boost::shared_ptr<pool> pool_ptr;
typedef std::map<std::string, std::string> ssmap;
extern pool_ptr workthread_pool_ptr;
class CTimeout: public boost::enable_shared_from_this<CTimeout>,
		private boost::noncopyable {
public:
	CTimeout(bool b) {
		flag = b;

	}
	~CTimeout() {
	}
	bool get() {
		return flag;
	}
	void set(bool b) {
		flag = b;
	}

private:

	bool flag;
};

class CPidTimeout: public boost::enable_shared_from_this<CPidTimeout>,
		private boost::noncopyable {
public:
	CPidTimeout(pid_t p) {
		flag = false;
		pid = p;
	}
	~CPidTimeout() {
	}
	int killpid() {
		flag = true;
		printf("kill子进程:%d\n",pid);
		int res = kill(pid, SIGKILL);
		int status;
		printf("等待子进程退出status\n");
		if (waitpid(-1, &status, 0) != -1)     //!> 参数是-1就是等待所有的子进程
		{
			printf("子进程退出status：%d\n", status);
		}

		return res;
	}
	bool get() {
		return flag;
	}

private:

	bool flag;
	pid_t pid;
};
typedef boost::shared_ptr<CTimeout> tmout_ptr;
typedef boost::shared_ptr<CPidTimeout> tpidmout_ptr;
class CMcast: public boost::enable_shared_from_this<CMcast>,
		private boost::noncopyable {
public:
	CMcast(boost::asio::io_service& io_service,
			const boost::asio::ip::address& listen_address,
			const boost::asio::ip::address& multicast_address,
			const short multicast_port = 32001);
	std::string get_remote_addr();
	int get_remote_port();

	void handle_receive_from(const boost::system::error_code& error,
			size_t bytes_recvd);

	void handle_send_to(const boost::system::error_code& error);

	void handle_timeout(const boost::system::error_code& error);
	void handle_sendheartbeat(const boost::system::error_code& error);

	void handle_send(const boost::system::error_code& error);

	void send_to(std::string msg, int flag = 0);
	void send_to(const string & sid,const string & cmd, std::string msg, int flag = 0);

	void start();

private:
	boost::asio::ip::udp::socket socket_;
	boost::asio::ip::udp::endpoint sender_endpoint_;
	enum {
		max_length = 8 * 1024 * 1024
	};
	boost::asio::ip::udp::endpoint endpoint_;
	boost::asio::ip::udp::endpoint listen_endpoint;
	boost::asio::deadline_timer timer_;

	boost::asio::deadline_timer heartbeattimer_;
	std::size_t inbound_data_size = 0;
	enum {
		header_length = 8
	};

	std::string outbound_header_;
	std::vector<char> inbound_data_;
	CGzipProcess_ptr m_zipptr;

};
typedef boost::shared_ptr<boost::asio::deadline_timer> timer_ptr;
typedef boost::shared_ptr<CMcast> CMcast_ptr;
void print_data(const std::string& str, const std::string& addr,
		const int& port, CMcast_ptr m_ptr, int flag);
#endif /* CMCAST_H_ */
