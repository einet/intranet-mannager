/*
 * CFrame.cpp


 *
 *  Created on: Mar 26, 2014
 *      Author: zlx
 */
#pragma once
#include <boost/progress.hpp>
#include "sstream"
#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

// utf8转换用
#include <boost/program_options/detail/convert.hpp>
#include <boost/program_options/detail/utf8_codecvt_facet.hpp>
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
#include "serialization_mcast.hpp"

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/export.hpp>

struct JsonMessage {
public:
	std::string getstring()
	{
		return json;
	}
	void setstring(std::string str)
	{
		json = str;
	}
private:
	friend class boost::serialization::access;
	std::string json;
	template<typename Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar &  BOOST_SERIALIZATION_NVP(json);
	}
};
typedef boost::shared_ptr<JsonMessage> JsonMessage_ptr;

class CFrame: public boost::enable_shared_from_this<CFrame>,
		private boost::noncopyable {
public:
	CFrame(boost::asio::io_service &p_ios, const std::string p_address,
			const std::string m_address, const std::string p_port);
	void send(JsonMessage_ptr p_mesg);
	void process_message(JsonMessage_ptr p_mesg);
	void start();
	void read();
private:
	mcast_connection_ptr m_con_ptr;
	std::vector< JsonMessage_ptr > recvmsgs_;
	void handle_write(const boost::system::error_code& err);
	void handle_read(const boost::system::error_code& err  );
	boost::asio::io_service& m_ios;

};
typedef boost::shared_ptr<CFrame> CFrame_ptr;
