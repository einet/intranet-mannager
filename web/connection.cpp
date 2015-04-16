//
// connection.cpp
// 文件上下载需要改成异步事件

//

#include "connection.hpp"
#include <utility>
#include <vector>
#include "connection_manager.hpp"
#include "request_handler.hpp"
#include <fstream>
#include "boost/format.hpp"
#include "boost/lexical_cast.hpp"
#include "mime_types.hpp"
#include "../CMcast.h"
#include "ParserProcess.h"
#include "md5.hpp"
#include "../JsonMsg.h"
#include "SeessionID.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>
#include <syslog.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include <unistd.h>
extern char hname[1024];
/*
 *
 * {"files": [
 {
 "picture1.jpg": true
 },
 {
 "picture2.jpg": true
 }
 ]}

 {"files": [
 {
 "name": "picture1.jpg",
 "size": 902604,
 "error": "Filetype not allowed"
 },
 {
 "name": "picture2.jpg",
 "size": 841946,
 "error": "Filetype not allowed"
 }
 ]}
 {"files": [
 {
 "name": "picture1.jpg",
 "size": 902604,
 "url": "http:\/\/example.org\/files\/picture1.jpg",
 "thumbnailUrl": "http:\/\/example.org\/files\/thumbnail\/picture1.jpg",
 "deleteUrl": "http:\/\/example.org\/files\/picture1.jpg",
 "deleteType": "DELETE"
 },
 {
 "name": "picture2.jpg",
 "size": 841946,
 "url": "http:\/\/example.org\/files\/picture2.jpg",
 "thumbnailUrl": "http:\/\/example.org\/files\/thumbnail\/picture2.jpg",
 "deleteUrl": "http:\/\/example.org\/files\/picture2.jpg",
 "deleteType": "DELETE"
 }
 ]}
 * */
using namespace boost::algorithm;

extern std::string webroot;
extern SeessionID m_sessionID;
extern CMcast_ptr mcast_r;
extern ssmap mapHost;
extern Lock ssmapLock;
extern bool process_sendcmd(const std::string& msg);
extern std::vector<std::string> hostvec;
namespace http {
namespace server {

connection::connection(boost::asio::ip::tcp::socket socket,
		connection_manager& manager, request_handler& handler) :
		socket_(std::move(socket)), connection_manager_(manager), request_handler_(
				handler), bkeepalive(true), f_offset(0) {

#if defined OS_WINDOWS
	int32_t timeout = 15000;
	setsockopt(socket_.native(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	setsockopt(socket_.native(), SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
#else
	struct timeval tv;
	tv.tv_sec = 30;
	tv.tv_usec = 0;
	setsockopt(socket_.native(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	setsockopt(socket_.native(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

	//检查socket的状态
	boost::asio::socket_base::keep_alive option(true);
	socket_.set_option(option);

	//int keepalive = 1; // 开启keepalive属性
	int keepidle = 60; // 如该连接在60秒内没有任何数据往来,则进行探测
	int keepinterval = 5; // 探测时发包的时间间隔为5 秒
	int keepcount = 3; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
	//setsockopt(socket.native(), SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive , sizeof(keepalive ));
	setsockopt(socket_.native(), SOL_TCP, TCP_KEEPIDLE, (void*) &keepidle,
			sizeof(keepidle));
	setsockopt(socket_.native(), SOL_TCP, TCP_KEEPINTVL, (void *) &keepinterval,
			sizeof(keepinterval));
	setsockopt(socket_.native(), SOL_TCP, TCP_KEEPCNT, (void *) &keepcount,
			sizeof(keepcount));

#endif
}

void connection::start() {
	do_read();
}

void connection::stop() {
	socket_.close();
}
//异步接受文件：用于文件的上传
void connection::asyncrecv_file() {
	auto self(shared_from_this());
	socket_.async_read_some(
			boost::asio::buffer(buffer_.data() + f_offset, 8192 - f_offset),
			[this, self](boost::system::error_code ec, std::size_t bytes_transferred)
			{
				if (!ec) {
					//处理文件上传事宜
					char *ss = buffer_.data();

					boost::format fmter(
							"{\"files\":[{\"name\":\"%s\",\"size\":%d,\"url\":\"%s\",\"thumbnailUrl\": \"%s\",\"deleteUrl\": \"%s\",\"deleteType\": \"DELETE\"}]}");
					char* pos = strstr(ss, "\r\n\r\n");
					if (!bheader) {

						if (pos > 0) {
							bheader = true;
							h_len = (size_t) (pos - ss);
						} else {
							f_offset += bytes_transferred;
							return asyncrecv_file();
						}
					}
					recv_datalen += bytes_transferred;
					if (!wfirst) {
						char *fpos = strstr(pos + 4, "Content-Disposition: form-data;");
						if (fpos != NULL) {
							wfirst = true;
							fpos = strstr(fpos, "filename=");
							char * d = strstr(fpos, "\r\n");
							filename = fpos + 10;
							filename.resize(d - fpos - 11);
							filename = "/update/" + filename;
							fullfilename = webroot + filename;
							std::cout<<"Upload file:"<<filename<<std::endl;;
							os.open(fullfilename.c_str(), std::ios::out | std::ios::binary);
							char * sd = strstr(fpos, "\r\n\r\n");
							size_t fl = recv_datalen - (int) (sd - ss + 4);

							f_len = fl;
							os.write(sd + 4, fl);
							f_offset = 0;
							os.flush();
							//return asyncrecv_file();
						} else {
							f_offset += bytes_transferred;
							//return asyncrecv_file();
						}
					} else
					{
						if (bytes_transferred > 0)
						{
							os.write(ss, bytes_transferred);
						}
					}

					if (recv_datalen - h_len == contentlen + 4) {
						os.close();
						//读取文件尾部数据
						size_t boffset = boundary.length() + 4;
						struct stat st;
						stat(fullfilename.c_str(), &st);
						size_t size = st.st_size;
						std::ifstream is(fullfilename.c_str(),
								std::ios::out | std::ios::binary);
						is.seekg(size - boffset);
						std::string mend;
						char *p = new char[boffset + 1];
						is.read(p, boffset);
						char* cpos = NULL;
						if ((cpos = strstr(p, boundary.c_str())) > 0)
						//找到边界
						{
							char tp = '-';
							boffset = boffset + 1;
							while (tp == '-') {
								is.seekg(size - boffset);
								is.read(&tp, 1);
								boffset = boffset + 1;
							}

							is.close();
							f_len = size - boffset;
							truncate(fullfilename.c_str(), f_len);
							fmter % filename % f_len % filename % "/pic/add.png" % filename;
							ansmsg = fmter.str();
						} else {
							boost::format fmter(
									"{\"files\":[{\"name\":\"%s\",\"size\":%s,\"error\":\"%s\")}]}");
							fmter % filename % f_len % "文件接受错误";
							ansmsg = fmter.str();
						}
						delete[] p;
						reply_.status = reply::ok;
						reply_.headers.resize(2);
						reply_.content = ansmsg;
						reply_.headers[0].name = "Content-Length";

						reply_.headers[0].value = boost::lexical_cast<std::string>(
								reply_.content.size());
						reply_.headers[1].name = "Content-Type";
						reply_.headers[1].value = mime_types::extension_to_type("cmd");
						return do_write();

					} else
					{
						return asyncrecv_file();
					}

				}
				else if (ec != boost::asio::error::operation_aborted)
				{
					connection_manager_.stop(shared_from_this());
					remove(fullfilename.c_str());
					return;
				}
			});

}
//异步发送文件：用于文件的下载
//异步发送文件，文件句柄复用bug
void connection::asyncsend_file(is_ptr is,int style,int ifirst, const boost::system::error_code& e) {
	int len = 0;

	if (!e) {
		if (style == 0) {
			is->seekg(ifirst);
			is->read(buf, sizeof(buf));
			len = is->gcount();
			if(len>0)
			{
				ifirst = ifirst + len;
				boost::asio::async_write(socket_, boost::asio::buffer(buf, len),
				boost::bind(&connection::asyncsend_file,
								shared_from_this(), is, style, ifirst,
								boost::asio::placeholders::error));
				return;

			}
		} else if (style == 1) {
			{
				is->seekg(ifirst);
				if (readlen > 0) {
					if (readlen <= 8192) {
						is->read(buf, readlen);
						len = is->gcount();

						if (len > 0) {
							ifirst = ifirst + len;
							readlen = readlen - len;
							boost::asio::async_write(socket_,
									boost::asio::buffer(buf, len),
									boost::bind(&connection::asyncsend_file,
											shared_from_this(), is, style,ifirst,
											boost::asio::placeholders::error));
							return;
						}
					} else {

						is->read(buf, sizeof(buf));
						len = is->gcount();
						{
							if ((len = is->gcount()) > 0) {
								ifirst = ifirst + len;
								readlen = readlen - len;
								boost::asio::async_write(socket_,
										boost::asio::buffer(buf, len),
										boost::bind(&connection::asyncsend_file,
												shared_from_this(), is, style,ifirst,
												boost::asio::placeholders::error));
								return;
							}
						}
					}

				}
			}
		}
		if (bkeepalive) {
			do_read();
		} else {
			boost::system::error_code ignored_ec;
			socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
					ignored_ec);
		}
	} else {
		std::cout << "error:" << e.message() << std::endl;
		if (e != boost::asio::error::operation_aborted) {
			connection_manager_.stop(shared_from_this());
		}
	}

}
void connection::do_read() {
	if (f_offset >= 8192)
		f_offset = 0;

	auto self(shared_from_this());
	socket_.async_read_some(
			boost::asio::buffer(buffer_.data() + f_offset, 8192 - f_offset),
			[this, self](boost::system::error_code ec, std::size_t bytes_transferred)
			{
				if (!ec)
				{
					bytes_transferred = f_offset+bytes_transferred;
					request_parser::result_type result;

					{
						//异步接受时需要重置：变量会累加
						request_.headers.clear();
						request_.method.clear();
						request_.uri.clear();
					}
					std::tie(result, std::ignore) = request_parser_.parse(
							request_, buffer_.data(), buffer_.data() + bytes_transferred);

					if (result == request_parser::good)
					{
						//接受完所有参数信息

						for (int i = 0; i < (int) request_.headers.size(); i++) {
							if (request_.headers[i].name == "Content-Length") {
								contentlen = boost::lexical_cast<long unsigned int>(request_.headers[i].value.c_str());
								break;
							}
						}
						int hsize = request_.headers.size();
						std::string cookies;
						parameterMap gmap;
						sessionid.clear();
						for (int i = 0; i < hsize; i++) {
							if (request_.headers[i].name == "Cookie") {
								cookies = request_.headers[i].value;
								gmap = getParameterMap(cookies);
								sessionid = gmap["sid"];
								break;
							}
						}

						if (request_.method == "POST")
						{

							for (int i = 0; i < (int) request_.headers.size(); i++) {
								if (request_.headers[i].name == "Content-Type") {
									ctype = request_.headers[i].value;
									if(ctype.find("multipart/form-data")!=std::string::npos)
									{
										size_t cpos= ctype.find("boundary=");
										if(cpos!=std::string::npos)
										{
											cpos = cpos+9;
											const char * p = ctype.c_str();
											while(p[cpos]=='-')
											{
												cpos = cpos +1;
											}
											boundary = ctype.c_str()+cpos;
										}
									}
									break;
								}
							}
							if(request_.uri == "/uploadfile.action" && !boundary.empty())
							//处理文件上传事宜
							{
								char *ss = buffer_.data();
								char* pos = 0;
								pos = strstr(ss, "\r\n\r\n");

								bool finished = false;
								if (pos > 0) {
									if (pos - ss != (int) bytes_transferred - contentlen - 4)
									finished = false;
									else
									finished = true;
								}

								boost::format fmter("{\"files\":[{\"name\":\"%s\",\"size\":%d,\"url\":\"%s\",\"thumbnailUrl\": \"%s\",\"deleteUrl\": \"%s\",\"deleteType\": \"DELETE\"}]}");

								if(finished)
								{
									char *fpos = strstr(pos+4,"Content-Disposition: form-data;");
									if(fpos!=NULL)
									{
										fpos = strstr(fpos,"filename=");
										char * d = strstr(fpos,"\r\n");
										filename = fpos+10;
										filename.resize(d-fpos-11);
										pos = strstr(d,"\r\n\r\n");
										pos = pos+4;
										int h_len = (size_t)(pos-ss);
										std::cout<<"Upload file:"<<filename<<std::endl;;
										filename = "/update/"+filename;
										fullfilename = webroot+filename;

										std::ofstream os(fullfilename.c_str(), std::ios::out | std::ios::binary);

										os.write(pos,bytes_transferred - h_len -4);
										os.flush();
										os.close();
										//读取文件尾部数据
										size_t boffset = boundary.length()+4;
										struct stat st;
										stat(fullfilename.c_str(), &st);
										size_t size = st.st_size;
										std::ifstream is(fullfilename.c_str(), std::ios::out | std::ios::binary);
										is.seekg(size-boffset);
										std::string mend;
										char *p = new char[boffset+1];
										is.read(p,boffset);
										char* cpos = NULL;
										if((cpos = strstr(p,boundary.c_str()))>0)
										//找到边界
										{
											char tp = '-';
											boffset = boffset +1;
											while(tp == '-')
											{
												is.seekg(size-boffset);
												is.read(&tp,1);
												boffset = boffset +1;
											}

											is.close();

											f_len = size - boffset;
											truncate(fullfilename.c_str(),f_len);
											fmter % filename % f_len % filename % "/pic/add.png" % filename;
											ansmsg = fmter.str();

										} else
										{
											remove(filename.c_str());
											boost::format fmter("{\"files\":[{\"name\":\"%s\",\"size\":%s,\"error\":\"%s\")}]}");
											fmter % filename % f_len % "文件接受错误";
											ansmsg = fmter.str();
										}
										delete [] p;
										reply_.status = reply::ok;
										reply_.headers.resize(2);
										reply_.content = ansmsg;
										reply_.headers[0].name = "Content-Length";

										reply_.headers[0].value = boost::lexical_cast<std::string>(
												reply_.content.size());
										reply_.headers[1].name = "Content-Type";
										reply_.headers[1].value = mime_types::extension_to_type("cmd");
										return do_write();

									}

								} else
								{

									ctype.clear();
									ansmsg="";
									filename.clear();
									fullfilename.clear();;
									f_len=0;
									bheader=false;
									h_len=0;
									wfirst=false;
									recv_datalen = bytes_transferred;
									f_offset = recv_datalen;
									return asyncrecv_file();
								}
							} else
							{
								char *ss = buffer_.data();
								char* pos = 0;
								pos = strstr(ss, "\r\n\r\n");
								bool finished = false;
								if (pos > 0) {
									if (pos - ss != (int) bytes_transferred - contentlen - 4)
									finished = false;
									else
									finished = true;
								}
								if (pos == 0 || !finished) {

									f_offset = bytes_transferred;
									//采用异步接受
									return do_read();

								} else
								{
									f_offset = 0;
								}
							}
						}
						std::string ppstr;

						if (request_.method == "POST" && request_.uri == "/saveconfig.asp") {
							char * p = buffer_.data();
							p[bytes_transferred] = '\0';
							char* pos = strstr(p, "\r\n\r\n");

							ppstr.append(pos + 4);
							std::ofstream os(webroot+"/lib/flare-data.js", std::ios::out | std::ios::binary);
							os <<"var flare_data ="<< ppstr<<";";
							os.close();
							boost::format fmter("{\"success\":%s,\"status\":\"%d\"}");
							fmter % "true" % 1;

							reply_.status = reply::ok;
							reply_.headers.resize(2);
							reply_.content = fmter.str();
							reply_.headers[0].name = "Content-Length";

							reply_.headers[0].value = boost::lexical_cast<std::string>(
									reply_.content.size());
							reply_.headers[1].name = "Content-Type";
							reply_.headers[1].value = mime_types::extension_to_type(
									"cmd");
							return do_write();

						} else if (request_.method == "POST" && request_.uri == "/getconfig.asp")
						{
							string str = "{\"name\":\"Ola Voice服务系统\",\"done\":false,\"id\":100,\"collapsed\":false,\"x0\":380,\"y0\":0,\"children\":[";
							boost::format fmtstr("{\"name\":\"%s\",\"ip\":\"%s\",\"done\":false,\"id\":%d,\"collapsed\":false},");
							int i= 101;
							{
								ReadLock r_lock(ssmapLock);
								BOOST_FOREACH( const ssmap::value_type &iter, mapHost ) {
									fmtstr % iter.first % iter.second % i++;
									str += fmtstr.str();
								}
							}
							if(i>101)
							{
								str = str.substr(0,str.length()-1);
							}
							str += "]}";
							boost::format fmter("{\"success\":%s,\"result\":%s}");
							fmter % "true" % str;

							reply_.status = reply::ok;
							reply_.headers.resize(2);
							reply_.content = fmter.str();
							reply_.headers[0].name = "Content-Length";

							reply_.headers[0].value = boost::lexical_cast<std::string>(
									reply_.content.size());
							reply_.headers[1].name = "Content-Type";
							reply_.headers[1].value = mime_types::extension_to_type(
									"cmd");
							return do_write();

						} else if (request_.method == "POST" && request_.uri == "/chat/sendmsg.asp")
						{
							//收到消息，通过组播发送，接受组播数据回传到ｗｅｂ端
							char * p = buffer_.data();
							p[bytes_transferred] = '\0';
							char* pos = strstr(p, "\r\n\r\n");
							ppstr.append(pos + 4);

							std::cout<<"recv msg:"<<ppstr<<std::endl;
							{
								//记载日志
								struct tm *newtime;
								char tmpbuf[80];
								time_t lt1;
								time(&lt1);
								newtime = ::localtime(&lt1);
								strftime(tmpbuf,80,"%Y-%m-%d %I:%M:%S",newtime);

								string ipstr;

								//nginx 传递的IP

								for (int i = 0; i < (int) request_.headers.size(); i++) {
									if (request_.headers[i].name == "X-Real-IP") {
										ipstr = request_.headers[i].value.c_str();

										break;
									}
								}

								try {
									if(ipstr.empty())
									ipstr = socket_.remote_endpoint().address().to_string();

									syslog(LOG_INFO, "%s IP:%s REQ: %s %s \r\n", tmpbuf,ipstr.c_str(),request_.uri.c_str(),ppstr.c_str());
								} catch(std::exception & ex)
								{
									std::cout<<"error:"<<ex.what()<<std::endl;
									connection_manager_.stop(shared_from_this());
									return;
								}
							}
							boost::format fmter("{\"success\":%s,\"id\":\"%s\",\"webhostname\":\"%s\",\"status\":\"%d\"}");
							{
								string md5str;
								if(sessionid.empty())
								{
									srand((unsigned) time(0));
									int num = rand();
									format fmter2("%s:%d:%d");
									fmter2 % socket_.remote_endpoint().address().to_string()
									% socket_.remote_endpoint().port() % num;

									md5str = zmd5::md5simpledigest(fmter2.str());
								}
								else
								{
									md5str = sessionid;
								}
								fmter % "true" % md5str % hname % 1;
								if(!ppstr.empty())
								{
									try {
										std::stringstream jsonStream;
										boost::property_tree::ptree jsonParse;
										jsonStream.str(ppstr);

										ppstr = boost::property_tree::json_parser::create_escapes(ppstr);
										boost::property_tree::json_parser::read_json(jsonStream, jsonParse);
										boost::property_tree::ptree::const_assoc_iterator it = jsonParse.find(
												"sendmsg");

										if (it != jsonParse.not_found())
										{
											ppstr = jsonParse.get<std::string>("sendmsg");
											if(!ppstr.empty())
											{
												if( ppstr.find("set host ") != string::npos )
												{
													string cmd = ppstr.substr(9);
													boost::algorithm::trim(cmd);
													if(cmd == "clear")
													{
														m_sessionID.Clear(sessionid);
													} else
													{
														m_sessionID.push_back(sessionid,cmd);
													}
												} else if( ppstr.find("clear all") != string::npos )
												{
													m_sessionID.Clear();
												} else
												{
													boost::property_tree::ptree::const_assoc_iterator itflag = jsonParse.find("flag");
													int flag=0;
													if (itflag != jsonParse.not_found())
													{
														flag = jsonParse.get<int>("flag");
													}
													//验证
													if(flag == 0)
													{
														if(process_sendcmd(ppstr))
														mcast_r->send_to(sessionid,ppstr,"",0);
														else
														mcast_r->send_to(sessionid,ppstr,"",1);
													} else
													mcast_r->send_to(sessionid,ppstr,"",flag);
												}
											}
										}
									} catch (const boost::property_tree::json_parser::json_parser_error& e) {
										cout << "sendmsg Invalid JSON:" << ppstr << " " <<e.what()<< endl; // Never gets here
									} catch (const std::runtime_error& e) {
										cout << "sendmsg Invalid JSON:" << ppstr << " " <<e.what() << endl; // Never gets here
									} catch (...) {
										cout << "sendmsg Invalid JSON:" << ppstr << endl; // Never gets here
									}
								}

							}

							reply_.status = reply::ok;
							reply_.headers.resize(2);
							reply_.content = fmter.str();
							reply_.headers[0].name = "Content-Length";

							reply_.headers[0].value = boost::lexical_cast<std::string>(
									reply_.content.size());
							reply_.headers[1].name = "Content-Type";
							reply_.headers[1].value = mime_types::extension_to_type(
									"cmd");
							return do_write();

						} else if (request_.method == "POST" && request_.uri == "/chat/recvmsg.asp")
						{
							//收到消息，通过组播发送，接受组播数据回传到ｗｅｂ端
							string msg;
							msg = m_sessionID.ReadFunction(sessionid);
							boost::format fmter("{\"success\":%s,\"webhostname\":\"%s\",\"status\":\"%d\",\"msg\":%s}");

							fmter % "true" % hname % 1 % msg;

							bool bgzip = false;
							for (int i = 0; i < (int) request_.headers.size(); i++) {
								if (request_.headers[i].name == "Accept-Encoding") {
									if (request_.headers[i].value.find_first_of("gzip") != string::npos) {
										bgzip = true;
										break;
									}
								}
							}
							if(bgzip && fmter.size()>=2048)
							{
								reply_.headers.resize(3);
								reply_.status = reply::ok;
								reply_.headers[0].name = "Content-Length";

								reply_.headers[1].name = "Content-Type";
								reply_.headers[1].value = mime_types::extension_to_type("cmd");

								reply_.headers[2].name = "Content-Encoding";
								reply_.headers[2].value = "gzip";
								CGzipProcess_ptr m_zipptr(new CGzipProcess());
								filtering_istream os;
								string msg = fmter.str();
								m_zipptr->GzipFromData((char*) msg.c_str(), (const int) msg.length(), os);

								size_t len = 0;
								char buf[8192];
								char *bt = buf;
								while (!os.eof()) {

									std::streamsize i = read(os, &bt[0], 8192);
									if (i < 0) {

										break;
									} else {
										reply_.content.append(bt, i);
										len += i;

									}
								}

								reply_.headers[0].value = boost::lexical_cast<std::string>(len);
							} else
							{

								reply_.status = reply::ok;
								reply_.headers.resize(2);
								reply_.content = fmter.str();
								reply_.headers[0].name = "Content-Length";

								reply_.headers[0].value = boost::lexical_cast<std::string>(
										reply_.content.size());
								reply_.headers[1].name = "Content-Type";
								reply_.headers[1].value = mime_types::extension_to_type(
										"cmd");
							}
							return do_write();

						} else if (request_.uri == "/chat/recvmsg.tes")
						{
							//收到消息，通过组播发送，接受组播数据回传到ｗｅｂ端

							string msg;
							msg = m_sessionID.ReadFunction(sessionid);

							boost::format fmter("data:{\"success\":%s,\"webhostname\":\"%s\",\"status\":\"%d\",\"msg\":%s}\r\n\r\n");

							fmter % "true" % hname % 1 % msg;
							bool bgzip = false;
							for (int i = 0; i < (int) request_.headers.size(); i++) {
								if (request_.headers[i].name == "Accept-Encoding") {
									if (request_.headers[i].value.find_first_of("gzip") != string::npos) {
										bgzip = true;
										break;
									}
								}
							}
							if(bgzip && fmter.size()>=2048)
							{
								reply_.headers.resize(3);
								reply_.status = reply::ok;
								reply_.headers[0].name = "Content-Length";

								reply_.headers[1].name = "Content-Type";
								reply_.headers[1].value = mime_types::extension_to_type("tes");

								reply_.headers[2].name = "Content-Encoding";
								reply_.headers[2].value = "gzip";

								CGzipProcess_ptr m_zipptr(new CGzipProcess());
								filtering_istream os;
								string msg = fmter.str();
								m_zipptr->GzipFromData((char*) msg.c_str(), (const int) msg.length(), os);

								size_t len = 0;
								char buf[8192];
								char *bt = buf;
								while (!os.eof()) {

									std::streamsize i = read(os, &bt[0], 8192);
									if (i < 0) {

										break;
									} else {
										reply_.content.append(bt, i);
										len += i;

									}
								}

								reply_.headers[0].value = boost::lexical_cast<std::string>(len);
							} else
							{
								reply_.status = reply::ok;
								reply_.headers.resize(3);
								reply_.content = fmter.str();
								reply_.headers[0].name = "Content-Length";

								reply_.headers[0].value = boost::lexical_cast<std::string>(
										reply_.content.size());
								reply_.headers[1].name = "Content-Type";
								reply_.headers[1].value = mime_types::extension_to_type(
										"tes");

								reply_.headers[2].name = "Cache-Control";
								reply_.headers[2].value = "no-cache";
							}

							return do_write();

						} else
						{
							std::string extension;
							full_path = request_handler_.get_fullpath(request_, reply_,extension);
							if(request_.method == "DELETE" && full_path.find("/update/")!=std::string::npos)
							{
								remove(full_path.c_str());

								reply_.status = reply::ok;
								reply_.headers.resize(3);
								reply_.content = "";
								reply_.headers[0].name = "Content-Length";

								reply_.headers[0].value = boost::lexical_cast<std::string>(
										reply_.content.size());
								reply_.headers[1].name = "Content-Type";
								reply_.headers[1].value = mime_types::extension_to_type(
										"tes");

								reply_.headers[2].name = "Cache-Control";
								reply_.headers[2].value = "no-cache";
								return do_write();

							} else
							{
								return do_file(request_, reply_,extension);
							}
						}
					}
					else if (result == request_parser::bad)
					{
						reply_ = reply::stock_reply(reply::bad_request);
						return do_write();
					}
					else
					{
						return do_read();
					}
				}
				else if (ec != boost::asio::error::operation_aborted)
				{
					connection_manager_.stop(shared_from_this());
				}
			});
}
void connection::do_zip_file(const request& req, reply& rep,
		std::string& extension) {
	replace_all(full_path, "\\", "/");
	struct stat st;
	stat(full_path.c_str(), &st);
	size_t size = st.st_size;
	//判断时间戳
	string filetmstr;
	char tm_str[29];
	bool bgzip = false;
	for (int i = 0; i < (int) req.headers.size(); i++) {
		if (req.headers[i].name == "Accept-Encoding") {
			if (req.headers[i].value.find_first_of("gzip") != string::npos) {
				bgzip = true;
				break;
			}
		}
	}
	for (int i = 0; i < (int) req.headers.size(); i++) {
		if (req.headers[i].name == "If-Modified-Since") {
			filetmstr = req.headers[i].value;
			namespace fs = boost::filesystem;
			try {
				time_t m_t = boost::filesystem::last_write_time(full_path);
				strftime(tm_str, 29, "%a,%d %b %Y %H:%M:%S GMT",
						::localtime(&m_t));
			} catch (...) {
				rep = reply::stock_reply(reply::not_found);
				return do_write();
			}
			replace_all(filetmstr, ", ", ",");
			if (strcasecmp(tm_str, filetmstr.c_str()) == 0) {
				rep.status = reply::not_modified;
				return do_write();
			}
		}
	}

	CGzipProcess mp;
	if (bgzip) {

		if (0 == mp.PushFileDataToZip(full_path.c_str())) {
			rep = reply::stock_reply(reply::not_found);
			do_write();
			return;
		}
		namespace fs = boost::filesystem;
		time_t m_t = fs::last_write_time(full_path);
		strftime(tm_str, 29, "%a,%d %b %Y %H:%M:%S GMT", ::localtime(&m_t));
		rep.status = reply::ok;
		char buf[8192];
		char *p = buf;
		int isize = 0;
		rep.headers.resize(4);
		rep.headers[0].name = "Content-Length";

		rep.headers[1].name = "Content-Type";
		rep.headers[1].value = mime_types::extension_to_type(extension);
		rep.headers[2].name = "Last-Modified";
		rep.headers[2].value = tm_str;
		rep.headers[3].name = "Content-Encoding";
		rep.headers[3].value = "gzip";
		size_t len = 0;
		while ((isize = mp.GetDataFromZip(&p, sizeof(buf))) > 0) {
			rep.content.append(buf, isize);
			len += isize;
		}
		rep.headers[0].value = boost::lexical_cast<std::string>(len);
		do_write();
		return;
	} else {

		is.reset(new std::ifstream(full_path.c_str(), std::ios::in | std::ios::binary));

		if (!is->is_open()) {
			rep = reply::stock_reply(reply::not_found);
			do_write();
			return;
		}

		namespace fs = boost::filesystem;
		time_t m_t = fs::last_write_time(full_path);
		strftime(tm_str, 29, "%a,%d %b %Y %H:%M:%S GMT", ::localtime(&m_t));
		rep.status = reply::ok;
		char buf[8192];
		rep.headers.resize(3);
		rep.headers[0].name = "Content-Length";
		rep.headers[0].value = boost::lexical_cast<std::string>(size);
		rep.headers[1].name = "Content-Type";
		rep.headers[1].value = mime_types::extension_to_type(extension);
		rep.headers[2].name = "Last-Modified";
		rep.headers[2].value = tm_str;

		while (is->read(buf, sizeof(buf)).gcount() > 0) {
			rep.content.append(buf, is->gcount());
		}
		rep.headers[0].value = boost::lexical_cast<std::string>(
				rep.to_buffers().size());
		do_write();
		return;
	}
	return;
}
void connection::do_file(const request& req, reply& rep,
		std::string& extension) {
	replace_all(full_path, "\\", "/");
	/*if(is.is_open())
	{
		is.close();
	}*/
	is.reset(new std::ifstream(full_path.c_str(), std::ios::in | std::ios::binary));
	//is.open(full_path.c_str(), std::ios::in | std::ios::binary);
	if (!is->is_open()) {
		rep = reply::stock_reply(reply::not_found);
		do_write();
		return;
	}
	//判断时间戳
	string filetmstr;
	char tm_str[29];
	namespace fs = boost::filesystem;
	try {
		time_t m_t = boost::filesystem::last_write_time(full_path);
		strftime(tm_str, 29, "%a,%d %b %Y %H:%M:%S GMT", ::localtime(&m_t));
	} catch (...) {
		rep = reply::stock_reply(reply::not_found);
		return do_write();
	}
	for (int i = 0; i < (int) req.headers.size(); i++) {
		if (req.headers[i].name == "If-Modified-Since") {
			filetmstr = req.headers[i].value;

			replace_all(filetmstr, ", ", ",");
			if (strcasecmp(tm_str, filetmstr.c_str()) == 0) {
				rep.status = reply::not_modified;
				return do_write();
			}
		}
	}

	struct stat st;
	stat(full_path.c_str(), &st);
	size_t size = st.st_size;

	bool brange = false;
	vector<m_data> v_m_data;
	for (int i = 0; i < (int) req.headers.size(); i++) {
		if (req.headers[i].name == "Range") {
			std::string mstr = req.headers[i].value;
			size_t so = mstr.find("bytes=");
			brange = true;
			if (so != std::string::npos) {
				so = so + 6;

				vector<string> strVec;
				string str = req.headers[i].value.c_str() + so;
				split(strVec, str, is_any_of(","));

				if (strVec.size() > 1) {
					vector<string>::iterator it = strVec.begin();
					for (; it != strVec.end(); it++) {
						cout << *it << endl;
						vector<string> str1Vec;
						split(str1Vec, *it, is_any_of("-"));

						if (str1Vec.size() == 2) {
							m_data m1;
							trim(str1Vec[0]);
							trim(str1Vec[1]);
							if (str1Vec[0].empty())
								m1.first = -1;
							else
								m1.first = atoi(str1Vec[0].c_str());

							if (str1Vec[1].empty())
								m1.end = -1;
							else
								m1.end = atoi(str1Vec[1].c_str());
							v_m_data.push_back(m1);
						}
					}
				} else {
					vector<string> str1Vec;
					split(str1Vec, str, is_any_of("-"));

					if (str1Vec.size() == 2) {
						m_data m1;
						if (str1Vec[0].empty())
							m1.first = -1;
						else
							m1.first = atoi(str1Vec[0].c_str());

						if (str1Vec[1].empty())
							m1.end = -1;
						else
							m1.end = atoi(str1Vec[1].c_str());
						v_m_data.push_back(m1);
					}
				}
				break;
			}
		}
	}

	if (!brange) {

		rep.headers.resize(3);
		rep.status = reply::ok;
		//char buf[1024];
		rep.headers[0].name = "Content-Length";
		rep.headers[0].value = std::to_string(size);
		rep.headers[1].name = "Content-Type";
		rep.headers[1].value = mime_types::extension_to_type(extension);
		rep.headers[2].name = "Last-Modified";
		rep.headers[2].value = tm_str;
		//if (is.read(buf, sizeof(buf)).gcount() >= 0) {
			//int len = is.gcount();
			//rep.content.append(buf, len);
			//ifirst = ifirst + len;
			boost::asio::async_write(socket_, rep.to_buffers(),
					boost::bind(&connection::asyncsend_file, shared_from_this(),
							is, 0, ifirst,
							boost::asio::placeholders::error));
		//}
		return;
	} else {

		boost::format fmter("bytes %d-%d/%d");

		if (v_m_data.size() == 1) {
			ifirst = 0;
			iend = 0;
			if (v_m_data[0].first == -1) {
				ifirst = size - v_m_data[0].end;
				iend = size - 1;
			} else {
				if (v_m_data[0].end == -1) {
					iend = size - 1;
				} else {
					iend = v_m_data[0].end;
				}
				ifirst = v_m_data[0].first;
			}

			rep.headers.resize(4);
			rep.status = reply::partial_content;
			rep.headers[0].name = "Content-Length";

			rep.headers[1].name = "Content-Type";
			rep.headers[1].value = mime_types::extension_to_type(extension);

			rep.headers[2].name = "Last-Modified";
			rep.headers[2].value = tm_str;
			rep.headers[3].name = "Content-Range";
			fmter % ifirst % iend % size;

			rep.headers[3].value = fmter.str();

			readlen = iend - ifirst + 1;

			rep.headers[0].value = std::to_string(readlen);
			//size_t len = 0;
			//char buf[1024];

			//if (is.read(buf, sizeof(buf)).gcount() > 0)
				//len = is.gcount();
			//rep.content.append(buf, len);

			//ifirst = ifirst + len;
			boost::asio::async_write(socket_, rep.to_buffers(),
					boost::bind(&connection::asyncsend_file, shared_from_this(),
							is, 1, ifirst,
							boost::asio::placeholders::error));
			return;
		} else {
			std::cout << "v_m_data.size:" << v_m_data.size() << std::endl;
		}

	}
	if (bkeepalive) {
		do_read();
	} else {
		boost::system::error_code ignored_ec;
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
				ignored_ec);
	}
}
void connection::do_write() {
	auto self(shared_from_this());
	boost::asio::async_write(socket_, reply_.to_buffers(),
			[this, self](boost::system::error_code ec, std::size_t)
			{
				if (!ec)
				{
					// Initiate graceful connection closure.
					if(bkeepalive)
					{
						do_read();
					} else
					{
						boost::system::error_code ignored_ec;
						socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
								ignored_ec);
					}
				}

				if (ec != boost::asio::error::operation_aborted)
				{
					connection_manager_.stop(shared_from_this());
				}
			});
}

}
// namespace server
}// namespace http
