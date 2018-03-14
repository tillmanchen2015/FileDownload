#ifndef __FILETRANSFER_H__
#define __FILETRANSFER_H__


#include <curl.h>
#include "downloadAPI.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/format.hpp>
#include <thread>
#include <memory>
#include <fstream>
#include <zlog.h>

using namespace std;

namespace talFD
{
	enum Http_Client_Response
	{
		HTTP_REQUEST_OK = CURLE_OK,

		HTTP_REQUEST_ERROR = -999,

	};

	struct FileInfo
	{
		std::string fileName;
		std::string lastModifiedTime;
		double fileLength = 0;
	};

	class FileDownload : public IDownloadFile
	{
	public:
		FileDownload();
		virtual ~FileDownload();

		virtual talFD::errorCode initialize(const char* requestURL, const char* saveDir, progress_info_callback cb);
		virtual void start_download();
		virtual void wait();
		virtual void stop();


		friend void download_thread_func(FileDownload* p);
		friend int curl_progress_callback(void *userdata, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & old_file_info.fileName;
			ar & old_file_info.fileLength;
			ar & old_file_info.lastModifiedTime;
		}

		void load_config(const char* configPath);
		void save_config(const char* configPath);

		talFD::errorCode get_app_dir(std::string &appDir);
		std::string get_config_path(const char* appDir, const char* fileName);
		

		static size_t receive_from_curl_callback(char *buffer, size_t size, size_t nmemb, void *userdata);

		long long get_local_file_length(string path);
		bool get_remote_file_info(string url, FileInfo* file);
		bool is_file_change(const char* url, FileInfo* oldFile);

		void log(const char* str, zlogLevel level= zlogLevel::debug);
		void log(boost::format& fmt, zlogLevel level = zlogLevel::debug);
		talFD::errorCode initialize_log(const char* fileName);

	private:
		std::string file_url;
		std::string save_dir;
		std::string file_name_download;
		std::string partfile_path;
		//std::string log_file_path;
		FileInfo old_file_info;
		std::string app_dir;
		std::string config_path;

		zlog zlog_inst;

		std::ofstream log_stream;
		std::mutex	log_mutex;

		progress_info_callback progress_callback_func;

		double download_file_length = 0;
		curl_off_t already_download_bytes = 0;
		time_t last_time_progress = 0;
		long long alreay_download_size = 0;

		bool is_stop_curl = false;
		bool download_from_beginning = false;
		bool use_config = false;

		std::shared_ptr<std::thread> download_thread;
	};
}//namespace FileDownload






#endif		// __FILETRANSFER_H__
