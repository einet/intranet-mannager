//
// connection_manager.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//2015-12-07 增加写锁，发现有崩溃现象
#include "connection_manager.hpp"
#include <boost/thread/locks.hpp>
#include <boost/thread/pthread/shared_mutex.hpp>
typedef boost::shared_mutex connection_Lock;
typedef boost::unique_lock<connection_Lock> WriteConnLock;
typedef boost::shared_lock<connection_Lock> ReadConnLock;
connection_Lock connUnLock;
namespace http {
namespace server {

connection_manager::connection_manager() {
}

void connection_manager::start(connection_ptr c) {
	WriteConnLock W_lock(connUnLock);
	connections_.insert(c);
	c->start();
}

void connection_manager::stop(connection_ptr c) {
	WriteConnLock W_lock(connUnLock);
	connections_.erase(c);
	c->stop();
}

void connection_manager::stop_all() {
	WriteConnLock W_lock(connUnLock);
	for (auto c : connections_)
		c->stop();
	connections_.clear();
}

} // namespace server
} // namespace http
