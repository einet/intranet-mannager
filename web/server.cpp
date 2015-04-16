//
// server.cpp

//

#include "server.hpp"
#include <signal.h>
#include <utility>

namespace http {
namespace server {

server::server(const std::string& address, const std::string& port,
		const std::string& doc_root, std::size_t pool_size) :
		io_service_pool_(pool_size),
		signals_(io_service_pool_.get_io_service()),
		acceptor_(io_service_pool_.get_io_service()),
		connection_manager_(),
		request_handler_(doc_root) {

	signals_.add(SIGINT);
	signals_.add(SIGTERM);
#if defined(SIGQUIT)
	signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)

	do_await_stop();

	boost::asio::ip::tcp::resolver resolver(acceptor_.get_io_service());
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(
			{ address, port });
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(1));

	acceptor_.bind(endpoint);

	acceptor_.listen();

	do_accept();
}

void server::run() {

	io_service_pool_.run();
}
socket_ptr socket_;
void server::do_accept() {

	try {

		socket_.reset(
				new boost::asio::ip::tcp::socket(
						io_service_pool_.get_io_service()));

		acceptor_.async_accept(*socket_,
				[this](boost::system::error_code ec)
				{

					if (!acceptor_.is_open())
					{
						return;
					}

					if (!ec)
					{
						connection_manager_.start(std::make_shared<connection>(
										std::move(*socket_), connection_manager_, request_handler_));
					}

					do_accept();
				});
	} catch (std::exception & ex) {
		std::cout << "do_accept error:" << ex.what() << std::endl;

		return;
	}
}

void server::do_await_stop() {
	signals_.async_wait([this](boost::system::error_code /*ec*/, int /*signo*/)
	{

		acceptor_.close();
		connection_manager_.stop_all();
	});
}

} // namespace server
} // namespace http
