/*
 * CFrame.cpp
 *
 *  Created on: Mar 27, 2014
 *      Author: zlx
 */
#include "CFrame.h"
#include "logger.hpp"
#include <boost/foreach.hpp>
#define foreach         BOOST_FOREACH


CFrame::CFrame(boost::asio::io_service &p_ios,
			const std::string p_address,
			const std::string m_address,
			const std::string p_port ):
			m_ios(p_ios)
			{
	m_con_ptr = mcast_connection_ptr(new mcast_connection(p_ios,atoi(p_port.c_str()),address::from_string(p_address),address::from_string(m_address)));
			}
void CFrame::send(JsonMessage_ptr p_mesg)
{
	std::vector< JsonMessage_ptr > msgs_;
	msgs_.push_back( p_mesg );

	m_con_ptr->async_send_to(msgs_,boost::bind(&CFrame::handle_write, shared_from_this(),
			boost::asio::placeholders::error));
}
void CFrame::handle_write(const boost::system::error_code& err)
{

}
void CFrame::handle_read(const boost::system::error_code& err)
{

	if( !err )
		{
			foreach( JsonMessage_ptr msg, recvmsgs_ )
			{
				process_message( msg );
			}
		}
		else
		{
			throw boost::system::system_error(err);
		}
	m_con_ptr->async_receive_from(recvmsgs_,boost::bind(&CFrame::handle_read, shared_from_this(),boost::asio::placeholders::error));
}
void CFrame::process_message(JsonMessage_ptr m_mesg)
{
	std::cout<<"recv:"<<m_con_ptr->get_remote_addr()<<" port:"<<m_con_ptr->get_remote_port()<<m_mesg->getstring()<<std::endl;
	JsonMessage_ptr p_mesg(new JsonMessage());
	p_mesg->setstring("{2222send data:zxc;lkzxc;k!}");
	send(p_mesg);
}
void CFrame::start()
{

	JsonMessage_ptr p_mesg(new JsonMessage());
	p_mesg->setstring("send data!");
	send(p_mesg);
	read();
}
void CFrame::read()
{
	m_con_ptr->async_receive_from(recvmsgs_,boost::bind(&CFrame::handle_read, shared_from_this(),boost::asio::placeholders::error));
}
