#pragma once
#include <string>
#include <mutex>
#include <boost/format.hpp>
#include <fstream>

enum zlogLevel
{
	debug,
	warning,
	error
};

class zlog
{
public:
	zlog();
	~zlog();

	std::string init(const char* logPath);
	void log(const char* msg, zlogLevel level);
	void log(boost::format& f, zlogLevel level);

private:
	std::string get_prefix(zlogLevel l);

	std::ofstream log_stream;
	std::mutex log_mutex;
};

