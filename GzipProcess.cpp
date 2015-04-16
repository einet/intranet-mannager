#include "GzipProcess.h"
using namespace std;

CGzipProcess::CGzipProcess(void)
{

	all_zip.push(gzip_compressor());
	all_unzip.push(gzip_decompressor());
}


CGzipProcess::~CGzipProcess(void)
{
}

int CGzipProcess::GzipFromFile( const string& infilename,const string& outfilename)
{
	using namespace std;    
	try
	{
		ifstream file(infilename.c_str(), ios_base::in | ios_base::binary);
		if(!file.good())
			return 1;
		filtering_istream  in;    
		in.push(gzip_compressor());    
		in.push(file);

		ofstream f(outfilename.c_str(), ios_base::out | ios_base::binary);
		boost::iostreams::copy(in, f);
	}
	catch(const boost::iostreams::gzip_error& e){  
		std::cout << e.what() << '\n';  
		return 1;
	}  
	return 0;
}



int CGzipProcess::UnGzipToFile( const string& infilename,const string& outfilename)
{
	using namespace std;    
	try{
		ifstream file(infilename.c_str(), ios_base::in | ios_base::binary);
		if(!file.good())
			return 1;
		filtering_istream  in;    
		in.push(gzip_decompressor());    
		in.push(file);

		ofstream f(outfilename.c_str(), ios_base::out | ios_base::binary);
		boost::iostreams::copy(in, f);
	}
	catch(const boost::iostreams::gzip_error& e){  
		std::cout << e.what() << '\n';  
		return 1;
	}  

	return 0;
}

int CGzipProcess::GzipFromData(const char * data,const int& len,filtering_istream & in)
{
	using namespace std;    
	try{
		in.push(gzip_compressor());    
		in.push(boost::make_iterator_range(&data[0],&data[len]));
	}
	catch(const boost::iostreams::gzip_error& e){  
		std::cout << e.what() << '\n';  
		return 1;
	}  

	return 0;
}


int CGzipProcess::UnGzipToData(const char * data,const int& len,filtering_istream & in)
{
	using namespace std;    
	try{
		in.push(gzip_decompressor());    
		in.push(boost::make_iterator_range(&data[0],&data[len]));
	}
	catch(const boost::iostreams::gzip_error& e){  
		std::cout << e.what() << '\n';  
		return 1;
	}  

	return 0;
}

void CGzipProcess::GzipAndCallback(const char * data,const int& len,const int& chunksize,Process_Stream callback)
{
	filtering_istream os;
	GzipFromData(data,len,os);

	while(!os.eof())
	{
		char *bt = new char[chunksize+1];
		std::streamsize i = read(os, &bt[0], chunksize); 
		if(i<0)
		{
			delete[]bt;
			break;
		}
		else
		{
			callback(bt,i);
			delete[]bt;
		}
	}
}

void CGzipProcess::UnGzipAndCallback(const char * data,const int& len,const int& chunksize,Process_Stream callback)
{
	filtering_istream os;
	UnGzipToData(data,len,os);

	while(!os.eof())
	{
		char *bt = new char[chunksize+1];
		std::streamsize i = read(os, &bt[0], chunksize); 
		if(i<0)
		{
			delete[]bt;
			break;
		}
		else
		{
			callback(bt,i);
			delete[]bt;
		}
	}
}

void CGzipProcess::GzipAndCallbackFromFile(const string& infilename, const int& chunksize,Process_Stream callback)
{
	using namespace std;    
	try
	{
		ifstream file(infilename.c_str(), ios_base::in | ios_base::binary);
		if(!file.good())
			return ;
		filtering_istream in;
		in.push(gzip_compressor());    
		in.push(file);

		while(!in.eof())
		{
			char *data = new char[chunksize+1];
			std::streamsize i = read(in, &data[0], chunksize); 
			if(i<0)
			{
				delete[]data;
				break;
			}
			else
			{
				callback(data,i);
				delete[]data;
			}
		}
	}
	catch(const boost::iostreams::gzip_error& e){  
		std::cout << e.what() << '\n';  
		return;
	}  
	return;
}

void CGzipProcess::UnGzipAndCallbackFromFile( const string& infilename,const int& chunksize,Process_Stream callback)
{
	using namespace std;    
	try
	{
		ifstream file(infilename.c_str(), ios_base::in | ios_base::binary);
		if(!file.good())
			return ;
		filtering_istream in;
		in.push(gzip_decompressor());    
		in.push(file);

		while(!in.eof())
		{
			char *data = new char[chunksize+1];
			std::streamsize i = read(in, &data[0], chunksize); 
			if(i<0)
			{
				delete[]data;
				break;
			}
			else
			{
				callback(data,i);
				delete[]data;
			}
		}
	}
	catch(const boost::iostreams::gzip_error& e){  
		std::cout << e.what() << '\n';  
		return;
	}  
}


int CGzipProcess::GzipDataToFile(const string& outfilename,const char * data,const int & len)
{
	int res =0;
	filtering_istream os;
	res = GzipFromData(data,len,os);
	ofstream f(outfilename.c_str(), ios_base::out | ios_base::binary);
	if(!f.good())
		return 1;
	boost::iostreams::copy(os, f);
	return res;
}
int CGzipProcess::UnGzipDataToFile(const string& outfilename,const char * data,const int & len)
{
	int res =0;
	filtering_istream os;
	res = UnGzipToData(data,len,os);
	ofstream f(outfilename.c_str(), ios_base::out | ios_base::binary);
	if(!f.good())
		return 1;
	boost::iostreams::copy(os, f);
	return res;
}


void CGzipProcess::PushDataToZip(const char* data,const int & len)
{
	all_zip.push(boost::make_iterator_range(&data[0],&data[len]));
}
void CGzipProcess::PushDataToUnZip(const char* data,const int & len)
{
	all_unzip.push(boost::make_iterator_range(&data[0],&data[len]));
}

int CGzipProcess::PushFileDataToZip(const char* infilename)
{
	zipfile_ptr.reset(new ifstream(infilename, ios_base::in | ios_base::binary));
	if(!zipfile_ptr->good())
		return 0;
	all_zip.push(*zipfile_ptr);
	return 1;
}
int CGzipProcess::PushFileDataToUnZip(const char* infilename)
{
	unzipfile_ptr.reset(new ifstream(infilename, ios_base::in | ios_base::binary));
	if(!unzipfile_ptr->good())
		return 0;
	all_unzip.push(*unzipfile_ptr);
	return 1;
}

void CGzipProcess::GetDataCallbackFromZip(const int& chunksize,Process_Stream callback)
{
	try
	{
		while(!all_zip.eof())
		{
			char *data = new char[chunksize+1];
			std::streamsize i = read(all_zip, &data[0], chunksize); 
			if(i<0)
			{
				delete[]data;
				break;
			}
			else
			{
				callback(data,i);
				delete[]data;
			}
		}
	}
	catch(const boost::iostreams::gzip_error& e){  
		std::cout << e.what() << '\n';  
		return;
	}  
}
void CGzipProcess::GetDataCallbackFromUnZip(const int& chunksize,Process_Stream callback)
{
	try
	{
		while(!all_unzip.eof())
		{
			char *data = new char[chunksize+1];
			std::streamsize i = read(all_unzip, &data[0], chunksize); 
			if(i<0)
			{
				delete[]data;
				break;
			}
			else
			{
				callback(data,i);
				delete[]data;
			}
		}
	}
	catch(const boost::iostreams::gzip_error& e){  
		std::cout << e.what() << '\n';  
		return;
	} 
}


int CGzipProcess::GetDataFromZip(char ** data,const int & len)
{
	try
	{
		if(!all_zip.eof())
		{
			std::streamsize i = read(all_zip, *data, len); 
			return (int)i;
		}
	}
	catch(const boost::iostreams::gzip_error& e){  
		std::cout << e.what() << '\n';  

	}  
	return -1;
}
int CGzipProcess::GetDataFromUnZip(char ** data,const int & len)
{
	try
	{
		if(!all_unzip.eof())
		{
			std::streamsize i = read(all_unzip, *data, len); 
			return (int)i;
		}
	}
	catch(const boost::iostreams::gzip_error& e){  
		std::cout << e.what() << '\n';  

	}  
	return -1;
}
