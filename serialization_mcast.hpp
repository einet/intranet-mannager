//
// connection.hpp
// ~~~~~~~~~~~~~~

#ifndef SERIALIZATION_MCAST_HPP
#define SERIALIZATION_MCAST_HPP

#include <boost/asio.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/tuple/tuple.hpp>

#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include "boost/bind.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "boost/asio/placeholders.hpp"
#define MAXBUF 400

using namespace boost::asio::ip;

class mcast_connection:
	public boost::enable_shared_from_this<mcast_connection>,
	private boost::noncopyable {
public:
	mcast_connection(boost::asio::io_service& io_service, unsigned short port,
			const boost::asio::ip::address& listen_address,
			const boost::asio::ip::address& multicast_address) :
			socket_(io_service),
			endpoint_(listen_address, port),
			recv_len(0){
		inbound_data_.resize(MAXBUF);

		socket_.open(endpoint_.protocol());
		socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		socket_.bind(endpoint_);
		boost::asio::socket_base::receive_buffer_size receSize(2*1024*1024);
		socket_.set_option(receSize);
		socket_.set_option(
				boost::asio::ip::multicast::join_group(multicast_address));

	}

	boost::asio::ip::udp::socket& socket() {
		return socket_;
	}

	template<typename T, typename Handler>
	void async_send_to(const T& t, Handler handler) {

		std::ostringstream archive_stream;
		boost::archive::xml_oarchive archive(archive_stream);

		archive << BOOST_SERIALIZATION_NVP(t);
		outbound_data_ = archive_stream.str();

		std::ostringstream header_stream;
		header_stream << std::setw(header_length) << std::hex
				<< outbound_data_.size();

		if (!header_stream || header_stream.str().size() != header_length) {

			boost::system::error_code error(
					boost::asio::error::invalid_argument);
			socket_.get_io_service().post(boost::bind(handler, error));
			return;
		}
		outbound_header_ = header_stream.str();

		std::vector<boost::asio::const_buffer> buffers;
		buffers.push_back(boost::asio::buffer(outbound_header_));
		buffers.push_back(boost::asio::buffer(outbound_data_));

		socket_.async_send_to(buffers, endpoint_, handler);
	}

	template<typename T, typename Handler>
	void async_receive_from(T& t, Handler handler)
	{
		void (mcast_connection::*f)(const boost::system::error_code&,std::size_t,
				 T&,  boost::tuple<Handler>)
				= &mcast_connection::handle_read_data<T, Handler>;

		socket_.async_receive_from(boost::asio::buffer(inbound_data_.data()+recv_len,inbound_data_.size()-recv_len),
				remote_endpoint,
				boost::bind(f, shared_from_this(), boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred, boost::ref(t),
						boost::make_tuple(handler)));
	}

	template<typename T, typename Handler>
	void handle_read_data(const boost::system::error_code& e,std::size_t bytes_transferred,T& t,
			boost::tuple<Handler> handler )
	{
		if (e) {
			boost::get<0>(handler)(e);
		} else {
			std::istringstream is(
					std::string(inbound_data_.begin(),
							inbound_data_.begin() + header_length));

			std::size_t inbound_data_size = 0;
			if(recv_len == 0)
			{
				if (!(is >> std::hex >> inbound_data_size)) {

					std::clog << "Invalid header." << std::endl;
					boost::system::error_code error(
							boost::asio::error::invalid_argument);
					boost::get<0>(handler)(error);
					return;
				}
				inbound_data_size += 8;
			}else
			{
				inbound_data_size = inbound_data_.size();
			}
			recv_len += bytes_transferred;
			std::cout<<"len:"<<inbound_data_size<<"recv:"<<bytes_transferred<<" " <<std::endl;

			//判断包是否接受完整
			if(inbound_data_size > recv_len)
			{
				inbound_data_.resize(inbound_data_size);

				void (mcast_connection::*f)(const boost::system::error_code&,
						std::size_t, T&, boost::tuple<Handler>)
						= &mcast_connection::handle_read_data<T, Handler>;

				socket_.async_receive_from(
						boost::asio::buffer(inbound_data_.data() + recv_len,
								inbound_data_.size() - recv_len),
						remote_endpoint,
						boost::bind(f, shared_from_this(),
								boost::asio::placeholders::error,
								boost::asio::placeholders::bytes_transferred,
								boost::ref(t), handler));
				return;
			}else
			{
				recv_len = 0;
			}

			std::string archive_data(inbound_data_.begin() + header_length,
					(inbound_data_.end()));

			try {
				std::istringstream archive_stream(archive_data);
				boost::archive::xml_iarchive archive(archive_stream);

				archive >> BOOST_SERIALIZATION_NVP(t);
			} catch (std::exception& e) {
				// Unable to decode data.
				std::clog << "Unable to decode data" << std::endl;
#if DEBUG > 7
				std::cerr << "Incoming message" << std::endl
				<< archive_data << std::endl;
#endif

				boost::system::error_code error(
						boost::asio::error::invalid_argument);
				boost::get<0>(handler)(error);
				return;
			}

			// Inform caller that data has been received ok.
			boost::get<0>(handler)(e);
		}
	}
	std::string get_remote_addr()
	{
		return remote_endpoint.address().to_string();
	}
	int get_remote_port()
	{
		return remote_endpoint.port();
	}

private:

	boost::asio::ip::udp::socket socket_;
	boost::asio::ip::udp::endpoint endpoint_;
	boost::asio::ip::udp::endpoint remote_endpoint;

	enum {
		header_length = 8
	};

	std::string outbound_header_;

	std::string outbound_data_;

	std::vector<char> inbound_data_;

	int recv_len;
};

typedef boost::shared_ptr<mcast_connection> mcast_connection_ptr;

#endif // SERIALIZATION_MCAST_HPP
