/*
// server.hpp
系统采用单IO处理，可以扩展为多IO操作

*/

#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include "connection.hpp"
#include "connection_manager.hpp"
#include "request_handler.hpp"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "io_service_pool.hpp"
namespace http {
namespace server {

typedef boost::shared_ptr< boost::asio::ip::tcp::socket > socket_ptr;
class server: public boost::enable_shared_from_this<server>,
private boost::noncopyable
{
public:
  server(const server&) = delete;
  server& operator=(const server&) = delete;


  explicit server(const std::string& address, const std::string& port,
      const std::string& doc_root,std::size_t pool_size);


  void run();
  void stop()
  {
	  io_service_pool_.stop();
  }
private:

  void do_accept();


  void do_await_stop();

  io_service_pool io_service_pool_;
  boost::asio::signal_set signals_;
  boost::asio::ip::tcp::acceptor acceptor_;
  connection_manager connection_manager_;

  request_handler request_handler_;
};

} // namespace server
} // namespace http
typedef boost::shared_ptr<http::server::server> CWebSVR_ptr;
#endif // HTTP_SERVER_HPP
