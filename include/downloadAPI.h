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
	\brief ���ؽ��ȵĻص����������demo
	@param [out] filename ��ǰ���ص��ļ�����
	@param [out] progressPercentage ������ɰٷֱ�
	@param [out] ������. errorCode::download_inprogress �������ء�talFD::download_interrupt�������жϡ�talFD::download_finish��������ɡ�
	*/
	typedef void(*progress_info_callback)(std::string fileName, double progressPercentage, errorCode ec);

	class IDownloadFile
	{
	public:
		/*!
		\brief ���ʼ����飬���ش����롣
		@param [in] requestURL �ļ����ص�ַ��
		@param [in] saveDir ���ر����ļ���Ŀ¼��
		@param [in] cb ���Ȼص�������
		@return
		@retval �����롣
		@remarks �Ȳ�췵��ֵ��û�д����ٵ���start_Download
		*/
		virtual talFD::errorCode initialize(const char* requestURL, const char* saveDir, progress_info_callback cb) = 0;

		/*!
		\brief ��ʼ���ء�
		@remarks ���������첽���ã��������ء�������δ���֮ǰ��ر�֤���಻����������
		*/
		virtual void start_download() = 0;

		/*!
		\brief �ȴ����ؽ�����
		@remarks ������Ϊͬ�����á�ע�⣺ÿ���������Ҫ����wait(),���ᱣ�汾�������ļ����´�����ʱ�ж��Ƿ���Ҫ�ϵ�������
		*/
		virtual void wait() = 0;

		/*!
		\brief �ж����ڽ��е����أ�����ʹwait()���ء�
		*/
		virtual void stop() = 0;

		/*!
		\brief ����������ȴ��ڲ��߳̽�����
		*/

		virtual ~IDownloadFile() {};
	};
	
	/*!
	\brief ��������IDownloadFile��һ��ʵ����Ӧһ�����ء����������Ҫ���ʵ����
	*/
	 talFDAPI IDownloadFile* get_interface();
}

