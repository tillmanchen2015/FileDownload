#include <zlog.h>
#include <boost/date_time/posix_time/posix_time.hpp>


zlog::zlog()
{
}


zlog::~zlog()
{
	if (log_stream.is_open())
	{
		log_stream.close();
	}
}

std::string zlog::init(const char* logPath)
{
	if (!logPath || !*logPath)
	{
		return "logPath is empty";
	}

	log_stream.open(logPath);
	if (!log_stream.is_open())
	{
		return "log cannot open";
	}
	return "ok";
}

void zlog::log(const char* msg, zlogLevel level)
{
	std::string record = get_prefix(level);
	record += msg;

	std::lock_guard<std::mutex> lk(log_mutex);
	log_stream << record << std::endl;
}

void zlog::log(boost::format& f, zlogLevel level)
{
	std::string record = get_prefix(level);
	record += boost::str(f);

	std::lock_guard<std::mutex> lk(log_mutex);
	log_stream << record << std::endl;
}

std::string zlog::get_prefix(zlogLevel l)
{
	boost::posix_time::ptime p = boost::posix_time::second_clock::local_time();
	int month = p.date().month();
	int day = p.date().day();
	int hour = p.time_of_day().hours();
	int minute = p.time_of_day().minutes();
	int second = p.time_of_day().seconds();
	std::string level;
	if (zlogLevel::debug == l)
	{
		level = "debug";
	}
	else if(zlogLevel::warning == l)
	{
		level = "warning";
	}
	else if (zlogLevel::error == l)
	{
		level = "error";
	}

	return boost::str(boost::format("%02d/%02d %02d:%02d:%02d [%s] ")%month%day%hour%minute%second%level.c_str());
}
