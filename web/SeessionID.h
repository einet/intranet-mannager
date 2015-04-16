/*
 * SeessionID.h
 *
 *  Created on: Apr 28, 2014
 *      Author: zlx
 */

#ifndef SEESSIONID_H_
#define SEESSIONID_H_
#include <string>
#include <map>
#include <boost/thread/locks.hpp>
#include <boost/thread/pthread/shared_mutex.hpp>
#include <boost/ptr_container/ptr_container.hpp>
#include <atomic>
typedef struct msgstruct
{
	int i;
	std::string msg;
}msgstruct;
typedef std::auto_ptr<msgstruct>msgstruct_ptr;
typedef boost::ptr_map<std::string, int> sidMap;

typedef boost::ptr_map<std::string, int>::iterator  s_ite;

typedef boost::ptr_vector<msgstruct> msgMap;
typedef boost::ptr_vector<msgstruct>::iterator  m_ite;


typedef std::vector<std::string> Ptr_strVector;
typedef boost::shared_mutex Lock;
typedef boost::unique_lock<Lock> WriteLock;
typedef boost::shared_lock<Lock> ReadLock;

typedef struct sessionStruct
{
	int i;
	std::string ipaddr;
	Ptr_strVector ptr_s;
	bool killflag;
	sessionStruct()
	{
		i = 0;
		killflag = false;
	}
}sessionStruct;

typedef boost::ptr_map<std::string, sessionStruct> Ptr_sidMap;

//查询主机的名称列表
//typedef boost::ptr_map<std::string, std::string> hostMap;

class SeessionID {
public:
	SeessionID();
	virtual ~SeessionID();
	//读取sid的消息没有发送的消息
	std::string ReadFunction(std::string sid);
	//存储sid和消息
	void WriteFunction(std::string msg);

	void WriteHostInfo(const std::string& sid,const std::string& hostname);
	Ptr_strVector GetHostInfo(const std::string& sid);
	void Clear();
	void Clear(const std::string & sid);
	void push_back(const std::string & sid,const std::string str);
	bool GetKillFlag(const std::string & sid);
	void SetKillFlag(const std::string & sid,bool flag);
	Ptr_strVector GetVect(const std::string & sid);
	std::string GetIPAddr(const std::string & sid);
private:
	//hostMap QueryHost;
	//sidMap smap;
	Ptr_sidMap ptrsmap;
	msgMap mmap;
	std::atomic<int> kki;
};

#endif /* SEESSIONID_H_ */
