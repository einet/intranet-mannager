//
// main.cpp
// ~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "server.hpp"
CWebSVR_ptr m_webSrv_Ptr;
int webmain(const std::string& ip,const std::string& port,const std::string& doc)
{
  try
  {
	  m_webSrv_Ptr.reset(new http::server::server(ip, port, doc,4));
	  m_webSrv_Ptr->run();
  }
  catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}
void  stopwebsvr()
{
	m_webSrv_Ptr->stop();
}
