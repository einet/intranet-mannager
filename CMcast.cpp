/*
 * CMcast.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: zlx
 *
 *  bugfix:修改popen打开文件read时无法终止的问题
 *  单独io时，持续执行脚本时存在无法接受数据的问题
 */

#include "CMcast.h"
#include <boost/algorithm/string/replace.hpp>
#include "JsonMsg.h"
#include <boost/algorithm/string.hpp>
#include "web/SeessionID.h"
using namespace std;
using namespace boost::algorithm;
char hname[1024];
extern boost::asio::io_service io_service;
extern boost::asio::io_service work_io_service;
//主机列表

ssmap mapHost;
std::string cmdcache;
std::string hostcache;
std::vector<std::string> hostvec;
extern SeessionID m_sessionID;

//增加对主机列表的读写锁，防止线程crash
Lock ssmapLock;
bool boolall = false;
#define READ 0
#define WRITE 1
#define ERR   2
/*pid_t popen2(const char **command, int *infp, int *outfp)
 {
 int p_stdin[2], p_stdout[2];
 pid_t pid;

 if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
 return -1;

 pid = fork();

 if (pid < 0)
 return pid;
 else if (pid == 0)
 {
 close(p_stdin[WRITE]);
 dup2(p_stdin[READ], READ);
 close(p_stdout[READ]);
 dup2(p_stdout[WRITE], WRITE);

 execvp(*command, command);
 perror("execvp");
 exit(1);
 }

 if (infp == NULL)
 close(p_stdin[WRITE]);
 else
 *infp = p_stdin[WRITE];

 if (outfp == NULL)
 close(p_stdout[READ]);
 else
 *outfp = p_stdout[READ];

 return pid;
 }*/
pid_t popen2(const char *command, int *infp, int *outfp) {
	int p_stdin[2], p_stdout[2];
	pid_t pid;

	if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
		return -1;

	pid = fork();
	//printf("child  %4d %4d %4d/n", getppid(), getpid(), pid);

	if (pid < 0)
		return pid;
	else if (pid == 0) {
		close(p_stdin[WRITE]);
		dup2(p_stdin[READ], READ);
		close(p_stdout[READ]);
		dup2(p_stdout[WRITE], WRITE);
		dup2(p_stdout[WRITE], ERR);

		execl("/bin/sh", "sh", "-c", command, NULL);
		perror("execl");
		exit(1);
	}

	if (infp == NULL)
		close(p_stdin[WRITE]);
	else
		*infp = p_stdin[WRITE];

	if (outfp == NULL)
		close(p_stdout[READ]);
	else
		*outfp = p_stdout[READ];

	return pid;
}

std::string gethoststr() {
	hostcache.clear();
	BOOST_FOREACH( const std::vector<std::string>::value_type &iter, hostvec ) {
		hostcache += iter + " ";
	}
	return hostcache;
}
void console_input(CMcast_ptr m_ptr) {

	gethostname(hname, sizeof(hname));

	std::cout << "[" << hname << "]>(Ctrl+C)" << std::endl;
	while (true) {
		std::string cmd;

		std::getline(std::cin, cmd);
		boost::algorithm::trim(cmd);
		if (!cmd.empty()) {
			if (cmd == "get host") {
				ReadLock r_lock(ssmapLock);
				BOOST_FOREACH( const ssmap::value_type &iter, mapHost ) {
					std::cout << "host:" << iter.first << " IP:" << iter.second
							<< std::endl;
				}
				std::cout << "[" << hname << ":" << gethoststr() << "]>"
						<< std::endl;

			} else if (cmd == "set host") {
				std::cout << "Please hostname(Quit set host(quit)):";
				while (1) {
					std::cout << "[" << hname << ":" << gethoststr()
							<< "](Quit set host(quit))>" << std::endl;
					std::getline(std::cin, cmd);
					boost::algorithm::trim(cmd);
					if (cmd == "quit" || cmd == "q" || cmd == "Q") {
						std::cout << "[" << hname << ":" << gethoststr() << "]>"
								<< std::endl;
						break;
					} else if (cmd == "clear") {
						hostvec.clear();
					} else if (cmd == "all") {

						bool b = false;
						for (unsigned int i = 0; i < hostvec.size(); ++i) {
							if (hostvec[i] == cmd) {
								b = true;
							}

						}
						if (!b)
							hostvec.push_back(cmd);
					} else if (cmd == "del") {
						std::cout << "Please input (del) hostname:"
								<< std::endl;
						std::getline(std::cin, cmd);
						boost::algorithm::trim(cmd);
						for (vector<std::string>::iterator it = hostvec.begin();
								it != hostvec.end();) {
							if (*it == cmd) {
								it = hostvec.erase(it);
							} else {
								++it;
							}
						}
						std::cout << "[" << hname << ":" << gethoststr()
								<< "](Quit set host(quit))>" << std::endl;
					} else {
						bool f = false;
						ReadLock r_lock(ssmapLock);
						BOOST_FOREACH( const ssmap::value_type &iter, mapHost ) {
							if (iter.first == cmd) {
								f = true;
								break;
							}
						}
						if (!f) {
							std::cout
									<< "This host No exists!(Quit set host(quit))"
									<< std::endl;
							continue;
						}
						f = false;
						for (unsigned int i = 0; i < hostvec.size(); ++i) {
							if (hostvec[i] == cmd) {
								std::cout
										<< "This host exists!(Quit set host(quit))"
										<< std::endl;
								f = true;
								break;
							}
						}
						if (!f)
							hostvec.push_back(cmd);
					}
				}
			} else if (cmd == "msg") {
				std::cout << "Please input msg(Quit msg(quit)):";
				while (1) {
					std::cout << "[" << hname << ":" << gethoststr()
							<< "](Quit set host(quit))>" << std::endl;
					std::getline(std::cin, cmd);
					boost::algorithm::trim(cmd);
					if (cmd == "quit" || cmd == "q" || cmd == "Q") {
						std::cout << "[" << hname << ":" << gethoststr() << "]>"
								<< std::endl;
						break;
					} else {
						m_ptr->send_to(cmd, 2);
						std::cout << "[" << hname << ":" << gethoststr() << "]>"
								<< std::endl;
						std::cout << "Please input msg(Quit msg(quit)):"
								<< std::endl;

					}
				}
			} else {
				m_ptr->send_to(cmd, 0);
				std::cout << "[" << hname << ":" << gethoststr() << "]>"
						<< std::endl;
			}
		}
	}
}
void ztimeout(const boost::system::error_code& error, tmout_ptr m_ptr) {
	cout << "ztimeout:" << error << endl;
	if (!error) {
		m_ptr->set(true);
	}
}

void zpidtimeout(const boost::system::error_code& error, tpidmout_ptr m_ptr) {
	cout << "ztimeout:" << error.message() << endl;

	if (!error)
	{
		m_ptr->killpid();
	}
}
//设置为非阻塞读取数据，加上超时１秒的设计
extern void stopwebsvr();
void callpid_sh(const std::string& sid, const std::string& script,
		CMcast_ptr m_ptr) {
	//开启定时器
	timer_ptr m_t_p(new boost::asio::deadline_timer(io_service));

	int infp = 0;
	int fp = 0;
	pid_t pid = 0;

	char result_buf[2048  * 4];

	int timeout = 50;
	try {

		if (script.find("tail", 0) == 0 || script.find("tar", 0) == 0
				|| script.find("./package.sh", 0) == 0
				|| script.find("cat", 0) == 0 || script.find("find", 0) == 0) {
			timeout = 300;
			pid = popen2(script.c_str(), &infp, &fp);
		} else {

			pid = popen2(script.c_str(), &infp, &fp);
		}
		tpidmout_ptr m_tptr(new CPidTimeout(pid));

		m_t_p->expires_from_now(boost::posix_time::seconds(timeout));
		m_t_p->async_wait(
				boost::bind(&zpidtimeout, boost::asio::placeholders::error,
						m_tptr));
		if (fp == 0) {
			return;
		}

		int d = fp;

		int flags;
		flags = fcntl(d, F_GETFL);
		flags |= O_NONBLOCK;
		fcntl(d, F_SETFL, flags);
		ssize_t r = 0;
		std::cout << "Recv cmd:" << script << ",pid:" << pid << std::endl;
		while (true) {
			if (m_sessionID.GetUnlock()) {
				std::cout << " Recv unlock sig!" << script << "pid:" << pid
						<< std::endl;
				close(infp);
				close(fp);
				m_t_p->cancel();
				m_tptr->killpid();
				break;
			}
			if (m_sessionID.GetKillFlag(sid)) {
				std::cout << " Recv kill sig!" << script << "pid:" << pid
						<< std::endl;
				close(infp);
				close(fp);
				m_t_p->cancel();
				m_tptr->killpid();
				break;
			}
			::usleep(50);
			r = ::read(d, result_buf, sizeof(result_buf));

			if (r > 0) {
				result_buf[r] = '\0';
				m_ptr->send_to(sid, script, result_buf, 1);
				m_t_p->cancel();
				m_t_p->expires_from_now(boost::posix_time::seconds(timeout));
				m_t_p->async_wait(
						boost::bind(&zpidtimeout,
								boost::asio::placeholders::error, m_tptr));
				continue;
			} else if (r == -1 && errno != EAGAIN) {
				m_t_p->cancel();
				close(infp);
				close(fp);
				break;
			}
			int status = 0;
			//检查调用的线程是否运行结束
			if (pid == waitpid(pid, &status, WNOHANG))
			{
				printf("子进程:%d,end.\n", pid);
				m_t_p->cancel();
				close(infp);
				close(fp);
				break;
			}
			//检查定时器是否已经启动运行
			if (m_tptr->get()) {
				m_t_p->cancel();
				close(infp);
				close(fp);
				break;
			}
			if (errno == EAGAIN) {
				continue;
			}

		}

	} catch (...) {
		std::cout << "error!" << std::endl;
		return;
	}
}

#include "./web/SeessionID.h"
extern SeessionID m_sessionID;
void print_data(JsonMsg_ptr j_ptr, const std::string& addr, const int& port,
		CMcast_ptr m_ptr, int flag) {
	stringstream msg;

	if (flag == 0) {

		std::cout << "Recv from: " << j_ptr->GetHost() << " IP " << addr
				<< " port:" << port << ", cmd:" << std::endl << j_ptr->GetCmd()
				<< std::endl;
		if (j_ptr->invect(hname))
			callpid_sh(j_ptr->sid, j_ptr->GetCmd(), m_ptr);
		else
			std::cout << "[" << hname << "]>(Ctrl+C)" << std::endl;

		msg << "{" << "\"sid\":\"" << j_ptr->sid << "\",\"host\":\""
				<< j_ptr->GetHost() << "\",\"ip\":\"" << addr
				<< "\",\"port\":\"" << port << "\",\"flag\":0,\"cmd\":\""
				<< boost::property_tree::json_parser::create_escapes(
						j_ptr->GetCmd()) << "\",\"result\":\""
				<< boost::property_tree::json_parser::create_escapes(
						j_ptr->GetCmd()) << "\"}";

	} else if (flag == 1) {

		std::cout << "Recv from: " << j_ptr->GetHost() << " IP " << addr
				<< " port:" << port << ", result:" << std::endl
				<< j_ptr->GetResult() << std::endl;
		std::cout << "[" << hname << "]>(Ctrl+C)" << std::endl;

		msg << "{" << "\"sid\":\"" << j_ptr->sid << "\",\"host\":\""
				<< j_ptr->GetHost() << "\",\"ip\":\"" << addr
				<< "\",\"port\":\"" << port << "\",\"flag\":1,\"cmd\":\""
				<< boost::property_tree::json_parser::create_escapes(
						j_ptr->GetCmd()) << "\",\"result\":\""
				<< boost::property_tree::json_parser::create_escapes(
						j_ptr->GetResult()) << "\"}";
	} else if (flag == 2) {
		std::cout << "Recv from: " << j_ptr->GetHost() << " IP " << addr
				<< " port:" << port << ", msg:" << std::endl << "    "
				<< j_ptr->GetResult() << std::endl;

		msg << "{" << "\"sid\":\"" << j_ptr->sid << "\",\"host\":\""
				<< j_ptr->GetHost() << "\",\"ip\":\"" << addr
				<< "\",\"port\":\"" << port << "\",\"flag\":2,\"cmd\":\""
				<< boost::property_tree::json_parser::create_escapes(
						j_ptr->GetCmd()) << "\",\"result\":\""
				<< boost::property_tree::json_parser::create_escapes(
						j_ptr->GetResult()) << "\"}";

		std::cout << "[" << hname << ":" << gethoststr() << "]>"
				<< "Please input msg(Quit msg(quit)):" << std::endl;
	} else if (flag == 3) {

	} else {
		return;
	}
	if (flag != 3)
		m_sessionID.WriteFunction(msg.str());
}

CMcast::CMcast(boost::asio::io_service& io_service,
		const boost::asio::ip::address& listen_address,
		const boost::asio::ip::address& multicast_address,
		const short multicast_port) :
		listen_endpoint(listen_address, multicast_port), socket_(io_service), timer_(
				io_service), heartbeattimer_(io_service), endpoint_(
				multicast_address, multicast_port), m_zipptr(new CGzipProcess()) {
	inbound_data_.resize(max_length);
	socket_.open(listen_endpoint.protocol().v4());
	socket_.set_option(
			boost::asio::ip::multicast::outbound_interface(
					listen_address.to_v4()));
	socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));

	boost::asio::ip::multicast::hops optiona(4);
	socket_.set_option(optiona);

	socket_.bind(listen_endpoint);

	socket_.set_option(
			boost::asio::ip::multicast::join_group(multicast_address.to_v4()));
}
void CMcast::start() {
	socket_.async_receive_from(
			boost::asio::buffer(inbound_data_.data(), max_length),
			sender_endpoint_,
			boost::bind(&CMcast::handle_receive_from, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));

	//启动console
	//workthread_pool_ptr->schedule(
	//	boost::bind(console_input, shared_from_this()));
	heartbeattimer_.expires_from_now(boost::posix_time::seconds(30));
	heartbeattimer_.async_wait(
			boost::bind(&CMcast::handle_sendheartbeat, shared_from_this(),
					boost::asio::placeholders::error));
	gethostname(hname, sizeof(hname));

}
void CMcast::handle_sendheartbeat(const boost::system::error_code& error) {
	if (!error) {
		send_to("heart beat", 3);
		heartbeattimer_.expires_from_now(boost::posix_time::seconds(30));
		heartbeattimer_.async_wait(
				boost::bind(&CMcast::handle_sendheartbeat, shared_from_this(),
						boost::asio::placeholders::error));
	}
}
std::string CMcast::get_remote_addr() {
	return sender_endpoint_.address().to_string();
}
int CMcast::get_remote_port() {
	return sender_endpoint_.port();
}

void CMcast::handle_receive_from(const boost::system::error_code& error,
		size_t bytes_recvd) {
	if (!error) {

		std::istringstream is(
				std::string(inbound_data_.begin(),
						inbound_data_.begin() + header_length + header_length));
		int flag = 1;
		if (!(is >> std::hex >> inbound_data_size >> flag)) {

			std::clog << "Invalid header." << std::endl;
			boost::system::error_code error(
					boost::asio::error::invalid_argument);

			inbound_data_size = 0;

			socket_.async_receive_from(
					boost::asio::buffer(inbound_data_.data(), max_length),
					sender_endpoint_,
					boost::bind(&CMcast::handle_receive_from,
							shared_from_this(),
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred));
			return;
		}

		//std::string msg = inbound_data_.data() + 16;

		//仅仅对消息做gzip处理
		string msg;

		{

			filtering_istream os;
			m_zipptr->UnGzipToData((char*) inbound_data_.data() + 16,
					inbound_data_size, os);

			while (!os.eof()) {
				char *bt = new char[1024 + 1];
				std::streamsize i = read(os, &bt[0], 1024);
				if (i < 0) {
					delete[] bt;
					break;
				} else {
					msg.append(bt, i);
					delete[] bt;
				}
			}
		}
		string addr = get_remote_addr();
		{
			if (msg.empty()) {
				socket_.async_receive_from(
						boost::asio::buffer(inbound_data_.data(), max_length),
						sender_endpoint_,
						boost::bind(&CMcast::handle_receive_from,
								shared_from_this(),
								boost::asio::placeholders::error,
								boost::asio::placeholders::bytes_transferred));
			}

			JsonMsg_ptr j_ptr(new JsonMsg());

			j_ptr->ParseJson(msg);
			if (!j_ptr->GetHost().empty()) {
				bool host = false;
				if (mapHost.size() > 0) {
					ReadLock w_lock(ssmapLock);
					map<string, string>::iterator iter = mapHost.find(
							j_ptr->GetHost());
					if (iter != mapHost.end() && iter->second != addr) {
						host = true;
					}
					if (iter == mapHost.end()) {
						host = true;
					}
				} else {
					host = true;
				}
				if (host) {
					WriteLock w_lock(ssmapLock);
					mapHost[j_ptr->GetHost()] = addr;
				}

			}
			//如果收到心跳包，则继续接受
			if (flag == 3) {
				socket_.async_receive_from(
						boost::asio::buffer(inbound_data_.data(), max_length),
						sender_endpoint_,
						boost::bind(&CMcast::handle_receive_from,
								shared_from_this(),
								boost::asio::placeholders::error,
								boost::asio::placeholders::bytes_transferred));
				return;
			}
			if (j_ptr->invect(hname) && flag == 0) {
				string script = j_ptr->cmdString;
				string sid = j_ptr->sid;
				std::cout << "Recv a script:" << script << std::endl;
				if (script.find("set host ") != string::npos) {
					string cmd = script.substr(9);
					boost::algorithm::trim(cmd);
					if (cmd == "clear") {
						m_sessionID.Clear(sid);
					} else {
						m_sessionID.push_back(sid, cmd);
					}
					//m_ptr->send_to(sid, script, script + ": set ok!", 1);
					goto endloop;
				} else if (script.find("clear all") != string::npos) {
					m_sessionID.Clear();
					//m_ptr->send_to(sid, script, script + ": set ok!", 1);
					goto endloop;
				} else if (script.find("lock", 0) == 0) {
					std::cout << "System normal state!" << std::endl;
					m_sessionID.SetUnlock(false);
					m_sessionID.SetKillFlag(sid, false);
					send_to(sid, script, "set ok!", 1);
					goto endloop;
				} else if (script.find("unblock", 0) == 0) {
					std::cout << "User normal state!" << std::endl;
					m_sessionID.SetKillFlag(sid, false);
					send_to(sid, script, "set ok!", 1);
					goto endloop;
				} else if (script.find("block", 0) == 0) {
					m_sessionID.SetKillFlag(sid, true);
					send_to(sid, script, "set ok!", 1);
					goto endloop;
				} else if (script.find("unlock", 0) == 0) {
					m_sessionID.SetUnlock(true);
					std::cout << "System unlock state!" << std::endl;
					send_to(sid, script, "set ok!", 1);
					goto endloop;
				}
				if (m_sessionID.GetKillFlag(sid)) {
					send_to(sid, script, ": 您的会话已经锁定!请选择\"启动查询\"!", 1);
					goto endloop;
				}
			}

			//启用工作io
			{

				work_io_service.post(
						boost::bind(&CMcast::process_data, shared_from_this(),
								j_ptr, addr, get_remote_port(), flag));

			}
		}
		endloop: socket_.async_receive_from(
				boost::asio::buffer(inbound_data_.data(), max_length),
				sender_endpoint_,
				boost::bind(&CMcast::handle_receive_from, shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
	}
}
void CMcast::process_data(JsonMsg_ptr j_ptr, const std::string& addr,
		const int& port, int flag) {

	workthread_pool_ptr->schedule(
			boost::bind(print_data, j_ptr, addr, get_remote_port(),
					shared_from_this(), flag));
}
void CMcast::handle_send_to(const boost::system::error_code& error) {
	if (!error) {
		timer_.expires_from_now(boost::posix_time::seconds(1));
		timer_.async_wait(
				boost::bind(&CMcast::handle_timeout, shared_from_this(),
						boost::asio::placeholders::error));
	}
}

void CMcast::handle_timeout(const boost::system::error_code& error) {
	if (!error) {
		//send_to(
		//	"{}1i09810483204{}{}1i09810483204{}{}1i09810483204{}{}1i09810483204{}{}1i09810483204{}{}1i09810483204{}{}1i09810483204{}");
	}
}

void CMcast::handle_send(const boost::system::error_code& error) {
	if (!error) {
		std::cout << "send suc!" << std::endl;
	}
}
void CMcast::send_to(const string & sid, const string & cmd, std::string msg,
		int flag) {
	std::ostringstream header_stream;

	JsonMsg_ptr m_json(new JsonMsg());

	m_json->cmddstVector = m_sessionID.GetVect(sid);
	m_json->sid = sid;
	m_json->AddCmd(cmd);
	if (flag == 1) {
		m_json->AddHost(hname);
		m_json->AddResult(msg);

	} else if (flag == 0) {
		m_json->AddHost(hname);
		//m_json->AddCmd(msg);
	} else if (flag == 2) {
		m_json->AddHost(hname);
		m_json->AddResult(msg);
	} else if (flag == 3) {
		m_json->AddHost(hname);
	}
	std::vector<boost::asio::const_buffer> buffers;
	m_json->MakeJsonString();
	msg = m_json->GetJsonString();
	size_t st = 0;
	std::string zipmsg;
	{
		filtering_istream os;
		m_zipptr->GzipFromData((char*) msg.c_str(), (const int) msg.size(), os);

		while (!os.eof()) {
			char bt[1024 + 1];
			std::streamsize i = read(os, &bt[0], 1024);
			if (i < 0) {

				break;
			} else {
				st += i;
				zipmsg.append(bt, i);

			}
		}
	}

	header_stream << std::setw(header_length) << std::hex << st
			<< std::setw(header_length) << std::hex << flag;
	outbound_header_ = header_stream.str();

	buffers.push_back(boost::asio::buffer(outbound_header_));

	buffers.push_back(boost::asio::buffer(zipmsg.c_str(), st));

	socket_.async_send_to(buffers, endpoint_,
			boost::bind(&CMcast::handle_send_to, shared_from_this(),
					boost::asio::placeholders::error));
}
void CMcast::send_to(std::string msg, int flag) {

	std::ostringstream header_stream;

	JsonMsg_ptr m_json(new JsonMsg());

	if (flag == 1) {
		m_json->AddHost(hname);
		m_json->AddResult(msg);

	} else if (flag == 0) {
		m_json->AddHost(hname);
		m_json->AddCmd(msg);
	} else if (flag == 2) {
		m_json->AddHost(hname);
		m_json->AddResult(msg);
	} else if (flag == 3) {
		m_json->AddHost(hname);
	}
	std::vector<boost::asio::const_buffer> buffers;
	m_json->MakeJsonString();
	msg = m_json->GetJsonString();
	size_t st = 0;
	std::string zipmsg;
	{
		filtering_istream os;
		m_zipptr->GzipFromData((char*) msg.c_str(), (const int) msg.size(), os);

		while (!os.eof()) {
			char *bt = new char[1024 + 1];
			std::streamsize i = read(os, &bt[0], 1024);
			if (i < 0) {
				delete[] bt;
				break;
			} else {
				st += i;
				zipmsg.append(bt, i);
				delete[] bt;
			}
		}
	}

	header_stream << std::setw(header_length) << std::hex << st
			<< std::setw(header_length) << std::hex << flag;
	outbound_header_ = header_stream.str();

	buffers.push_back(boost::asio::buffer(outbound_header_));

	buffers.push_back(boost::asio::buffer(zipmsg.c_str(), st));

	socket_.async_send_to(buffers, endpoint_,
			boost::bind(&CMcast::handle_send_to, shared_from_this(),
					boost::asio::placeholders::error));

}
