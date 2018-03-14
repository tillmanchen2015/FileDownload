#include <FileDownload.h>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>  
#include <fstream>

namespace talFD
{
	/*double FileDownload::downloadFileLength = 0;
	curl_off_t FileDownload::resumeByte = -1;
	time_t FileDownload::lastTime = 0;*/
	
	//bool FileDownload::is_stop_curl = false;
	int curl_progress_callback(void *userdata, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
	{

		FileDownload *p = static_cast<FileDownload*>(userdata);
		talFD::errorCode ec;
		if (p->is_stop_curl)
		{
			ec = talFD::errorCode::download_interrupt;
			p->log("FileDownload is stopped!");
			if (p->progress_callback_func != NULL)
				p->progress_callback_func(p->file_name_download, 0, ec);
			return 1;
		}

		double progress = 0;
		double downloadedBytes = p->already_download_bytes + dlnow;
		double total = p->download_file_length;
		progress = downloadedBytes / total;

		ec = errorCode::download_inprogress;
		if (p->progress_callback_func!= NULL)
			p->progress_callback_func(p->file_name_download, progress, ec);

		return 0;
	}

	void download_thread_func(FileDownload* p)
	{
		p->log("Start download file...");
		talFD::errorCode err = talFD::errorCode::no_error;

		
		std::string partPath = p->partfile_path;
		p->already_download_bytes = p->get_local_file_length(partPath);
		if (0 == p->already_download_bytes)
		{
			p->log(boost::format("cannot find local file: %1%")%partPath);
		}
		else
		{
			p->log(boost::format("find local file: %1%, length : %2%") %partPath % p->already_download_bytes);
		}

		CURL *easy_handle = NULL;
		FILE *fp = NULL;
		int ret = HTTP_REQUEST_ERROR;
		errorCode ec;
		do
		{
			easy_handle = curl_easy_init();
			if (!easy_handle)
			{
				p->log("curl_easy_init error");
				ec = errorCode::curl_easy_init_error;
				break;
			}

#ifdef _WIN32
			fopen_s(&fp, partPath.c_str(), "ab+");
#else
			fp = fopen(partPath.c_str(), "ab+");
#endif
			if (fp == NULL)
			{
				p->log(boost::format("cannot open part file: %1%, errno: %2%") % partPath%errno);
				ec = errorCode::fail_open_part_error;
				break;
			}

			// Set the url
			ret = curl_easy_setopt(easy_handle, CURLOPT_URL, p->file_url.c_str());

			// Save data from the server
			ret |= curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &FileDownload::receive_from_curl_callback);
			ret |= curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, fp);

			// Get the download progress
			ret |= curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 0L);
			ret |= curl_easy_setopt(easy_handle, CURLOPT_XFERINFOFUNCTION, &curl_progress_callback);
			ret |= curl_easy_setopt(easy_handle, CURLOPT_XFERINFODATA, p);

			// Fail the request if the HTTP code returned is equal to or larger than 400
			ret |= curl_easy_setopt(easy_handle, CURLOPT_FAILONERROR, 1L);

			// The maximum time that allow the connection parse to the server
			ret |= curl_easy_setopt(easy_handle, CURLOPT_CONNECTTIMEOUT, 30L);

			if (p->already_download_bytes > 0)
			{
				// Set a point to resume transfer
				ret |= curl_easy_setopt(easy_handle, CURLOPT_RESUME_FROM_LARGE, p->already_download_bytes);
			}

			if (ret != CURLE_OK)
			{
				ret = HTTP_REQUEST_ERROR;
				p->log("curl_easy_setopt error", zlogLevel::error);
				ec = errorCode::curl_set_opt_error;
				break;
			}

			ret = curl_easy_perform(easy_handle);

			if (ret != CURLE_OK)
			{
				ec = errorCode::curl_perform_error;
				char s[100] = { 0 };
#ifdef _WIN32
				sprintf_s(s, sizeof(s), "error:%d:%s", ret, curl_easy_strerror(static_cast<CURLcode>(ret)));
#else
				sprintf(s, "error:%d:%s", ret, curl_easy_strerror(static_cast<CURLcode>(ret)));
#endif

				p->log(s, zlogLevel::error);
				switch (ret)
				{
					case CURLE_HTTP_RETURNED_ERROR:
					{
						int code = 0;
						curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &code);

						char s[100] = { 0 };
	#ifdef _WIN32
						sprintf_s(s, sizeof(s), "HTTP error code:%d", code);
	#else
						sprintf(s, "HTTP error code:%d", code);
	#endif
						p->log(s, zlogLevel::error);
						break;
					}
				}
			}
		} while (0);

		if (fp != NULL)
		{
			fclose(fp);
			fp = NULL;
		}

		curl_easy_cleanup(easy_handle);
		easy_handle = NULL;

		if (ret == CURLE_OK)
		{
			boost::filesystem::path pathNew = p->save_dir;
			pathNew.append(p->file_name_download);

			boost::filesystem::path pathOld = partPath;
			boost::system::error_code er;
			boost::filesystem::rename(pathOld, pathNew, er);
			if (er.value() != boost::system::errc::success)
			{
				p->log(boost::format("failed to rename part file! ")%pathOld);
				ec = errorCode::rename_file_error;
			}
			else
			{
				p->log("succeeded in renaming part file");
				//check file size
				if (p->get_local_file_length(pathNew.string()) == p->download_file_length)
				{
					ec = errorCode::download_finish;
					//download finish then remove config file
					boost::filesystem::path pa(p->config_path);
					boost::filesystem::remove(pa, er);
					if (er != boost::system::errc::success)
					{
						p->log(boost::format("failed to remove config file, boost error: %1%")%er);
					}
					p->config_path = "";
					p->log("succeeded in removing config file");
				}
				else
				{
					ec = errorCode::file_length_diff_error;
				}
			}

			p->log(boost::format("finish file downloading : %1%") % pathNew.string());
			
		}
		p->progress_callback_func(p->file_name_download, 0, ec);
	}

	FileDownload::FileDownload()
	{
		
	}


	FileDownload::~FileDownload()
	{
		wait();
		curl_global_cleanup();
	}

	talFD::errorCode FileDownload::initialize(const char* requestURL, const char* saveDir, progress_info_callback cb)
	{

		talFD::errorCode err = get_app_dir(app_dir);
		if (err != talFD::no_error)
		{
			return err;
		}
				
		file_url = requestURL;
		std::string path = requestURL;
		std::size_t pos = path.rfind('/');
		if (pos == std::string::npos || pos == path.size()-1)
		{
			return request_url_no_filename;
		}
		file_name_download = path.substr(pos+1, path.size()-1-pos);
		

		err = initialize_log(file_name_download.c_str());
		if (err != talFD::not_find_log_flag && err != talFD::gen_log_path_succeed)
		{
			return err;
		}
		log(boost::format("download url is %1%, file_name_download is %2%") % file_url%file_name_download);

		config_path = get_config_path(app_dir.c_str(), file_name_download.c_str());
		if (boost::filesystem::exists(config_path.c_str()))
		{
			log("find config");
			load_config(config_path.c_str());
		}
		else
		{
			log("no config found. Download file from beginning.");
			download_from_beginning = true;
		}
		
		save_dir = saveDir;
		boost::system::error_code ec;
		if (!boost::filesystem::exists(save_dir, ec))
		{
			std::string str = "save_dir doesn't exist: ";
			str += save_dir;
			str += " ";
			str += ec.message();
			log(str.c_str());
			if (!boost::filesystem::create_directory(saveDir, ec))
			{
				std::string msg = boost::str(boost::format("Failed to create dir: %1, err: %2")%saveDir%ec.message());
				log(msg.c_str(), zlogLevel::error);
				return talFD::errorCode::save_dir_error;
			}
		}
		progress_callback_func = cb;

		log("curl global init");
		if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
			return talFD::errorCode::curl_glbal_init_error;

		FileInfo remoteFile;
		remoteFile.fileName = file_name_download;
		if (!get_remote_file_info(requestURL, &remoteFile))
		{
			return talFD::errorCode::get_remote_info_error;
		}
		download_file_length = remoteFile.fileLength;

		std::string partFileName = file_name_download + ".part";
		boost::filesystem::path partfilePath = save_dir;
		partfilePath.append(partFileName);
		partfile_path = partfilePath.string();

		if (use_config)
		{
			//compare if file has been changed
			if (remoteFile.fileLength != old_file_info.fileLength)
			{
				log(boost::format("file length has been changed! old length: %1%, now length: %2%")%old_file_info.fileLength%remoteFile.fileLength);
				download_from_beginning = true;
			}
			if (remoteFile.lastModifiedTime != old_file_info.lastModifiedTime)
			{
				log(boost::format("file modification time has been changed! old time : %1%, now time: %2%")%old_file_info.lastModifiedTime%remoteFile.lastModifiedTime);
				download_from_beginning = true;
			}
			if (download_from_beginning)
			{
				log("file will be downloaded from beginning! ");
				//remove 
				boost::system::error_code ec;
				boost::filesystem::remove(partfilePath, ec);
				if (ec != boost::system::errc::success)
				{
					log("unable to remove part file. ");
				}
				else
				{
					log("removed part file");
				}
			}
		}
		
		old_file_info = remoteFile;
		
		return talFD::errorCode::initialize_finish;
	}

	void FileDownload::log(const char* str, zlogLevel level)
	{
		zlog_inst.log(str, level);
	}

	void FileDownload::log(boost::format& fmt, zlogLevel level)
	{
		zlog_inst.log(fmt, level);
	}

	talFD::errorCode FileDownload::initialize_log(const char* fileName)
{
		boost::filesystem::path path = app_dir;
		boost::posix_time::ptime mst = boost::posix_time::second_clock::local_time();
		int month = mst.date().month();
		int day = mst.date().day();
		int hour = mst.time_of_day().hours();
		int minute = mst.time_of_day().minutes();
		int second = mst.time_of_day().seconds();
		
		std::string timeStr = boost::str(boost::format("%02d%02d%02d%02d%02d") % month % day % hour % minute % second);
		
		std::string logFileName = file_name_download;
		logFileName += "_";
		logFileName += timeStr;
		logFileName += ".log";

		path = app_dir;
		path.append(logFileName);

		std::string s = zlog_inst.init(path.string().c_str());
		if (s != "ok")
		{
			return errorCode::log_open_error;
		}

		return talFD::errorCode::gen_log_path_succeed;

	}

	// Return 0 if success, otherwise return error code
	void FileDownload::start_download()
	{
		download_thread = std::shared_ptr<std::thread>(new std::thread(download_thread_func, this));
	}

	void FileDownload::wait()
	{

		log("wait for thread to finish");
		if (download_thread->joinable())
		{
			download_thread->join();
		}
		save_config(config_path.c_str());
	}

	void FileDownload::stop()
	{
		log("Set stop flag");
		is_stop_curl = true;
	}

	bool FileDownload::is_file_change(const char* url, FileInfo* oldFile)
	{

		if (!oldFile)
		{
			return true;
		}
		FileInfo remoteFile;
		if (!get_remote_file_info(url, oldFile))
		{
			return true;
		}
		bool changed = false;
		if (remoteFile.fileLength != oldFile->fileLength)
		{
			log("File length changed!");
			changed = true;
		}
		if (remoteFile.lastModifiedTime != oldFile->lastModifiedTime)
		{
			log("Last modified time changed!");
			changed = true;
		}
		return changed;
	}

	void FileDownload::load_config(const char* configPath)
	{
		if (configPath != "")
		{
			std::ifstream ifs(configPath, std::fstream::in);
			if (ifs.is_open())
			{
				boost::archive::text_iarchive iar(ifs);
				iar >> *this;
				use_config = true;
				log(boost::format("config: fileName is %1%, fileLength is %2%, modified time is %3%") % old_file_info.fileName%old_file_info.fileName%old_file_info.lastModifiedTime);

				ifs.close();
			}
		}
	}

	void FileDownload::save_config(const char* configPath)
	{
		if (configPath != "")
		{
			std::ofstream ofs(configPath, std::fstream::out | std::fstream::trunc);
			if (ofs.is_open())
			{
				boost::archive::text_oarchive oar(ofs);
				oar << *this;

				ofs.close();
			}
			ofs.close();
		}
	}

	talFD::errorCode FileDownload::get_app_dir(std::string &appDir)
	{
#ifdef _WIN32
		char* appdata = getenv("APPDATA");
		appDir = boost::filesystem::path(appdata).append("talFD").string();
#else
		appDir = "/tmp/talFD";
#endif
		
		if (!boost::filesystem::exists(appDir))
		{
			if (!boost::filesystem::create_directories(appDir))
			{
				return talFD::errorCode::app_dir_error;
			}
		}
		return talFD::errorCode::no_error;
	}

	std::string FileDownload::get_config_path(const char* appDir, const char* fileName)
	{
		std::string configFileName = fileName;
		configFileName += "_talFD.config";
		std::string configPath = boost::filesystem::path(appDir).append(configFileName).string();
		return configPath;
	}

	size_t FileDownload::receive_from_curl_callback(char *buffer, size_t size, size_t nmemb, void *userdata)
	{
		FILE *fp = static_cast<FILE *>(userdata);
		size_t length = fwrite(buffer, size, nmemb, fp);

		return size * length;
	}

	

	// Get the local file size, return -1 if failed
	long long FileDownload::get_local_file_length(string path)
	{

		std::ifstream in(path, std::ifstream::ate | std::ifstream::binary);
		if (in.is_open())
		{
			return in.tellg();
		}
		return 0;
	}

	size_t nousecb(char *buffer, size_t x, size_t y, void *userdata)
	{
		(void)buffer;
		(void)userdata;
		return x * y;
	}

	// Get the file size on the server
	bool FileDownload::get_remote_file_info(string url, FileInfo* file)
	{

		log("try to get remote file info");
		CURL *easy_handle = NULL;
		int ret = CURLE_OK;
		double size = 0.0;
		do
		{
			easy_handle = curl_easy_init();
			if (!easy_handle)
			{
				log("curl_easy_init error", zlogLevel::error);
				break;
			}

			// Only get the header data
			ret = curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
			ret |= curl_easy_setopt(easy_handle, CURLOPT_HEADER, 1L);
			ret |= curl_easy_setopt(easy_handle, CURLOPT_NOBODY, 1L);
			// ret |= curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, nousecb);	// libcurl_a.lib will return error code 23 without this sentence on windows
			ret |= curl_easy_setopt(easy_handle, CURLOPT_FILETIME, 1L);

			if (ret != CURLE_OK)
			{
				log("getRemoteFileInfo curl_easy_setopt error", zlogLevel::error);
				break;
			}

			ret = curl_easy_perform(easy_handle);
			if (ret != CURLE_OK)
			{
				std::string str = boost::str(boost::format("get_remote_file_info::curl_easy_perform error:%d : %s") % ret%curl_easy_strerror(static_cast<CURLcode>(ret)));
				log(str.c_str(), zlogLevel::error);
				break;
			}

			ret = curl_easy_getinfo(easy_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
			if (ret != CURLE_OK)
			{
				log("curl_easy_getinfo error", zlogLevel::error);
				break;
			}
			file->fileLength = size;
			log(boost::format("Got file length: %1%")%size);

			long fileTime = -1;
			ret = curl_easy_getinfo(easy_handle, CURLINFO_FILETIME, &fileTime);
			if ((CURLE_OK == ret) && (fileTime >= 0)) {
				time_t file_time = (time_t)fileTime;
				char buf[256]{ 0 };
				#ifdef _WIN32
					ctime_s(buf, 256, &file_time);
				#else
					strcpy(buf, ctime(&file_time));
				#endif
				
				file->lastModifiedTime = buf;
				std::string msg = "File modified time is ";
				msg += file->lastModifiedTime;
				log(msg.c_str());
			}

		} while (0);

		curl_easy_cleanup(easy_handle);
		return true;
	}
}//namespace talFD

