/*
 * SeessionID.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: zlx
 */

#include "SeessionID.h"
#include <boost/regex.hpp>
#include "../JsonMsg.h"

std::map<std::string,Lock> myLock;
Lock msgLock;
SeessionID m_sessionID;


Lock cmdUnLock;
bool gUnlockflag = false;
SeessionID::SeessionID() {
	kki = 0;
}

SeessionID::~SeessionID() {

}
std::string SeessionID::GetIPAddr(const std::string & sid) {
	ReadLock r_lock(myLock[sid]);
	return ptrsmap[sid].ipaddr;
}
bool SeessionID::GetUnlock() {
	ReadLock r_lock(cmdUnLock);
	return gUnlockflag;
}
void SeessionID::SetUnlock(bool val) {
	WriteLock r_lock(cmdUnLock);
	std::cout<<"SetUnlock"<<val <<std::endl;
	gUnlockflag = val;
}
std::string SeessionID::ReadFunction(std::string sid)
{
	int id=0;
	{
		ReadLock r_lock(myLock[sid]);
		id = ptrsmap[sid].i;
	}

	std::string msg;
	bool num = false;
	ReadLock r_lock_msg(msgLock);
	if(mmap.size()>0)
	{
		int pknum = id;
		for (msgMap::const_iterator i = mmap.begin(), e = mmap.end(); i != e; ++i)
		{
			if (i->i <= id)
				continue;
			msg += i->msg;
			msg += ",";
			num = true;
			pknum = i->i;
		}
		{
			WriteLock w_lock(myLock[sid]);
			ptrsmap[sid].i = pknum;
		}

	}
	if (num)
	{
		msg = msg.substr(0, msg.length() - 1);

	}
	msg = "[" + msg;
	msg = msg + "]";

	return msg;
}
void SeessionID::Clear() {
	WriteLock w_lock(msgLock);
	mmap.clear();
	ptrsmap.clear();
}
void SeessionID::Clear(const std::string & sid) {
	WriteLock w_lock(myLock[sid]);
	ptrsmap[sid].ptr_s.clear();
}
Ptr_strVector SeessionID::GetVect(const std::string & sid) {
	ReadLock r_lock(myLock[sid]);
	return ptrsmap[sid].ptr_s;
}
void SeessionID::SetKillFlag(const std::string & sid, bool flag) {
	WriteLock w_lock(myLock[sid]);
	//std::cout<<"set killflag:"<<sid<<" "<<flag <<std::endl;
	ptrsmap[sid].killflag = flag;
}
bool SeessionID::GetKillFlag(const std::string & sid) {
	ReadLock r_lock(myLock[sid]);
	//std::cout<<"get killflag:"<<sid<<" "<< ptrsmap[sid].killflag<<std::endl;
	return ptrsmap[sid].killflag;
}
void SeessionID::push_back(const std::string & sid, const std::string str) {
	WriteLock w_lock(myLock[sid]);
	std::vector<std::string> value;
	boost::regex regex1(" ", boost::regbase::normal | boost::regbase::icase);
	std::string mstr = str;
	boost::regex_split(std::back_inserter(value), mstr, regex1);
	for (int i = 0; i < (int) value.size(); i++) {
		if (!value[i].empty()) {
			if (ptrsmap[sid].ptr_s.empty()) {
				ptrsmap[sid].ptr_s.push_back(value[i]);
			} else {
				bool find = false;
				for (unsigned int j = 0; j < ptrsmap[sid].ptr_s.size(); j++) {
					if (ptrsmap[sid].ptr_s[j] == value[i]) {
						find = true;
						break;
					}
				}
				if (!find) {
					ptrsmap[sid].ptr_s.push_back(value[i]);
				}
			}
		}
	}

}
void SeessionID::WriteFunction(std::string msg) {
	WriteLock w_lock(msgLock);
	//保留2000条消息，删除最早的1000条
	if (mmap.size() > 2000) {
		mmap.erase(mmap.begin(),mmap.begin()+1000);
	}

	msgstruct_ptr smsg(new msgstruct());
	smsg->i = kki;
	smsg->msg = msg;
	mmap.push_back(smsg);
	kki = kki + 1;
}
void SeessionID::WriteHostInfo(const std::string& sid,
		const std::string& hostname) {
	WriteLock w_lock(myLock[sid]);
	ptrsmap[sid].ptr_s.push_back(hostname);

}
Ptr_strVector SeessionID::GetHostInfo(const std::string& sid) {
	ReadLock r_lock(myLock[sid]);

	return ptrsmap[sid].ptr_s;
}
