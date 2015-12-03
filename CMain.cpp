//#include "serialization_mcast.hpp"
//#include "CFrame.h"
//boost::asio::io_service m_ios;
//int main()
//{
//	CFrame_ptr m_ptr(new CFrame(m_ios,"192.168.244.128","225.0.0.1","8888"));
//	m_ptr->start();
//	m_ios.run();
//	return 1;
//}

#include "CMcast.h"
#include "boost/threadpool.hpp"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <execinfo.h>
using namespace boost::threadpool;
boost::asio::io_service io_service;
boost::asio::io_service work_io_service;

//工作线程池
pool_ptr workthread_pool_ptr;

//多播接受池
CMcast_ptr mcast_r;
std::string webroot;
extern std::string sendcmdexp;


void sig_term(int signo) {
	cout << "program terminated,wait for all threads over!" << endl;

	cout << "reason:" << signo << endl;
	std::ofstream fout("./Spider.bug", ios::out | ios::app);
	struct tm *newtime;
	char tmpbuf[128];
	time_t lt1;
	time(&lt1);
	newtime = ::localtime(&lt1);
	strftime(tmpbuf, 128, "Today is %A, day %d of %B in the year %Y %X./n",
			newtime);

	fout << "time:" << tmpbuf << endl << "sig_term:" << signo << endl;
	if (signo == SIGSEGV || signo == SIGFPE || signo == SIGINT
			|| signo == SIGABRT) {
		void *array[10];
		size_t size;
		char **strings;
		size_t i;

		size = backtrace(array, 10);
		strings = (char **) backtrace_symbols(array, size);

		fout << " Stack trace:\n";
		for (i = 0; i < size; i++) {
			fout << i << " " << strings[i] << endl;
		}

		free(strings);

		char cmd[256] = "addr2line -C -f -e";
		char* prog = cmd + strlen(cmd);
		readlink("/proc/self/exe", prog, sizeof(cmd) - strlen(cmd) - 1); // 获取进程的完整路径

		sprintf(cmd, "%s>>./dump.log", cmd);
		FILE* fp = popen(cmd, "w");
		if (fp != NULL) {
			for (i = 0; i < size; ++i) {
				fprintf(fp, "%p\n", array[i]);
			}
			pclose(fp);
		}
	}

	fout.close();

	cout << "program over!" << endl;
	exit(-1);
	return;
}
extern int webmain(const std::string& ip, const std::string& port,
		const std::string& doc);
void recv_thread(std::string ipdaar, std::string maddr,const short port) {

	mcast_r.reset(
			new CMcast(io_service,
					boost::asio::ip::address::from_string(ipdaar),
					boost::asio::ip::address::from_string(maddr),port));
	mcast_r->start();
	boost::shared_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(work_io_service));
	boost::shared_ptr<boost::thread> thread(
			new boost::thread(
					boost::bind(&boost::asio::io_service::run, &io_service)));

	boost::shared_ptr<boost::thread> thread1(
			new boost::thread(
					boost::bind(&boost::asio::io_service::run,
							&work_io_service)));


	thread->join();
	thread1->join();
}
void freshregx()
{
	std::string full_path = webroot + "/exp.regex";
	std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
	if (!is) {
		std::cout << "远程shell匹配规则文件：" << full_path << "不存在！" << std::endl;
		return;
	} else {
		string str;
		sendcmdexp.clear();
		while (is >> str) {
			sendcmdexp += str;
			;
		}
		is.close();
	}
}
#define NDEBUG
int main(int argc, char* argv[]) {
	if (argc < 6) {
		std::cerr
				<< "Usage: Waiter <listen_address> <multicast_address> <web port> <multicast port>  <web root_path>\n";
		std::cerr << "  For IPv4, try:\n";
		std::cerr << "    Waiter 0.0.0.0 224.0.0.18 8099 32000 /page\n";
		std::cerr << "  For IPv6, try:\n";
		std::cerr << "    Waiter 0::0ff31::8000:12348099 8099 32000 /page \n";
		return 1;
	}


	//防止产生僵尸进程
	//signal( SIGCHLD, SIG_IGN );
#ifdef NDEBUG

	int pid;

	if ((pid = fork()))

		exit(0);
	else if (pid < 0)
		exit(1);

	setsid();

	if ((pid = fork()))

		exit(0);
	else if (pid < 0)

		exit(1);

	signal(SIGTERM, sig_term); /* arrange to catch the signal */
	signal(SIGKILL, sig_term); /* arrange to catch the signal */
	signal(SIGSEGV, sig_term); /* arrange to catch the signal */
	signal(SIGFPE, sig_term); /* arrange to catch the signal */
	signal(SIGILL, sig_term); /* arrange to catch the signal */
	signal(SIGABRT, sig_term); /* arrange to catch the signal */
	signal(SIGINT, sig_term); /* arrange to catch the signal */
	signal(SIGQUIT, sig_term); /* arrange to catch the signal */

#endif
	try {

		std::string ipdaar = argv[1];
		std::string maddr = argv[2];
		std::string webport = argv[3];
		webroot = argv[5];


		//
		freshregx();

		short port = atoi(argv[4]);
		workthread_pool_ptr.reset(new pool(80));
		workthread_pool_ptr->schedule(boost::bind(recv_thread, ipdaar, maddr,port));
		workthread_pool_ptr->schedule(
					boost::bind(webmain, ipdaar, webport, webroot));
		workthread_pool_ptr->wait();

	} catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
