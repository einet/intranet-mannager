#pragma once
#include <fstream>
#include <iostream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filtering_stream.hpp>  
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/shared_ptr.hpp>
using namespace std;
using namespace boost;
using namespace boost::iostreams;
typedef void (*Process_Stream)(const char * data, const int& len);
typedef boost::shared_ptr<ifstream> ifstream_ptr;

class CGzipProcess {
public:
	CGzipProcess(void);
	~CGzipProcess(void);

	int GzipFromFile(const string& infilename, const string& outfilename);

	int UnGzipToFile(const string& infilename, const string& outfilename);

	int GzipFromData(const char * data, const int& len, filtering_istream & in);

	int UnGzipToData(const char * data, const int& len, filtering_istream & in);

	void GzipAndCallback(const char * data, const int& len,
			const int& chunksize, Process_Stream callback);

	void UnGzipAndCallback(const char * data, const int& len,
			const int& chunksize, Process_Stream callback);

	void GzipAndCallbackFromFile(const string& infilename, const int& chunksize,
			Process_Stream callback);

	void UnGzipAndCallbackFromFile(const string& infilename,
			const int& chunksize, Process_Stream callback);

	int GzipDataToFile(const string& outfilename, const char * data,
			const int & len);

	int UnGzipDataToFile(const string& outfilename, const char * data,
			const int & len);
private:

	filtering_istream all_zip;
	filtering_istream all_unzip;
	ifstream_ptr zipfile_ptr;
	ifstream_ptr unzipfile_ptr;
public:

	void PushDataToZip(const char* data, const int & len);
	void PushDataToUnZip(const char* data, const int & len);

	int PushFileDataToZip(const char* infilename);
	int PushFileDataToUnZip(const char* infilename);

	void GetDataCallbackFromZip(const int& chunksize, Process_Stream callback);
	void GetDataCallbackFromUnZip(const int& chunksize,
			Process_Stream callback);

	int GetDataFromZip(char ** data, const int & len);
	int GetDataFromUnZip(char ** data, const int & len);
};

typedef boost::shared_ptr<CGzipProcess> CGzipProcess_ptr;
