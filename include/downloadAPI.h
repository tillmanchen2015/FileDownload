#pragma once
#define talFD_EXPORT

#ifdef _WIN32
	#if defined(talFD_EXPORT) // inside DLL
		#define talFDAPI   __declspec(dllexport)
	#else 
		#define talFDAPI   __declspec(dllimport)
	#endif  
#else
	#define talFDAPI
#endif


#include <string>

namespace talFD
{
	enum errorCode
	{
		no_error,
		curl_glbal_init_error,
		curl_easy_init_error,
		curl_set_opt_error,
		curl_perform_error,
		fail_open_part_error,
		file_length_changed,
		file_modified_time_change,
		initialize_finish,
		get_remote_info_error,
		download_succeed, //10
		request_url_no_filename,
		not_find_log_flag,
		gen_log_path_succeed,
		save_dir_error,
		app_dir_error,
		download_interrupt,
		download_inprogress,
		download_finish,
		file_length_diff_error,
		rename_file_error,
		log_open_error,
		errorMax
	};
	
	/*!
	\brief 下载进度的回调函数。详见demo
	@param [out] filename 当前下载的文件名。
	@param [out] progressPercentage 下载完成百分比
	@param [out] 错误码. errorCode::download_inprogress 正在下载。talFD::download_interrupt：下载中断。talFD::download_finish：下载完成。
	*/
	typedef void(*progress_info_callback)(std::string fileName, double progressPercentage, errorCode ec);

	class IDownloadFile
	{
	public:
		/*!
		\brief 类初始化检查，返回错误码。
		@param [in] requestURL 文件下载地址。
		@param [in] saveDir 本地保存文件的目录。
		@param [in] cb 进度回调函数。
		@return
		@retval 错误码。
		@remarks 先查检返回值，没有错误再调用start_Download
		*/
		virtual talFD::errorCode initialize(const char* requestURL, const char* saveDir, progress_info_callback cb) = 0;

		/*!
		\brief 开始下载。
		@remarks 本函数是异步调用，立即返回。在下载未完成之前务必保证本类不被析构掉。
		*/
		virtual void start_download() = 0;

		/*!
		\brief 等待下载结束。
		@remarks 本函数为同步调用。注意：每次下载务必要调用wait(),它会保存本地配置文件供下次下载时判断是否需要断点续传！
		*/
		virtual void wait() = 0;

		/*!
		\brief 中断正在进行的下载，将会使wait()返回。
		*/
		virtual void stop() = 0;

		/*!
		\brief 析构函数会等待内部线程结束。
		*/

		virtual ~IDownloadFile() {};
	};
	
	/*!
	\brief 用来构造IDownloadFile。一个实例对应一次下载。多次下载需要多个实例。
	*/
	 talFDAPI IDownloadFile* get_interface();
}

