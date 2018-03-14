#include <downloadAPI.h>
#include <thread>
#include <chrono>
//using namespace std::literals::chrono_literals;

void progress_func(std::string fileName, double progressPercentage, talFD::errorCode ec)
{
		if (ec == talFD::errorCode::download_inprogress)
		{
			printf("file %s :progressPercentage is %f\n", fileName.c_str(), progressPercentage);
		}
		else if (ec == talFD::download_interrupt)
		{
			printf("download file is stopped!\n");
		}
		else if (ec == talFD::download_finish)
		{
			printf("download file Finish!\n");
		}
		else
		{
			printf("download error: %d\n", ec);
		}
}

#define url1 "http://download.skycn.com/hao123-soft-online-bcs/soft/B/2015-06-24_BaiduMusic-12345617.exe"
#define url2 "http://download.skycn.com/hao123-soft-online-bcs/soft/L/2014-08-20_GoldWave.zip"

#ifdef _WIN32
	#define savedir "d:\\tmp2"
#else
	#define savedir "/tmp/talFD"
#endif


int main()
{
	//测试多线程同时下载. 先用p1 p2两个实例分别下载两个文件4秒钟，然后再用另外两个实例将余部下完。
	std::shared_ptr<talFD::IDownloadFile> p1 = std::shared_ptr<talFD::IDownloadFile>(talFD::get_interface());
	talFD::errorCode ec;
	ec = p1->initialize(url1, savedir, progress_func);
	if (ec != talFD::errorCode::initialize_finish)
	{
		printf("Initilize error: %d\n", ec);
		return -1;
	}
	p1->start_download();

	std::shared_ptr<talFD::IDownloadFile> p2 = std::shared_ptr<talFD::IDownloadFile>(talFD::get_interface());
	p2->initialize(url2, savedir, progress_func);
	if (ec != talFD::errorCode::initialize_finish)
	{
		printf("Initilize error: %d\n", ec);
		return -1;
	}
	p2->start_download();

	//下载4秒，然后调用stop中断下载，造成断点续传案例。
	std::this_thread::sleep_for(std::chrono::seconds(4));
	p1->stop();
	p2->stop();
	p1->wait();
	p2->wait();

	p1.reset();
	p2.reset();

	//另起两个实例接着下载。
	std::shared_ptr<talFD::IDownloadFile> p11 = std::shared_ptr<talFD::IDownloadFile>(talFD::get_interface());
	p11->initialize(url1, savedir, progress_func);
	if (ec != talFD::errorCode::initialize_finish)
	{
		printf("Initilize error: %d\n", ec);
		return -1;
	}
	p11->start_download();

	std::shared_ptr<talFD::IDownloadFile> p22 = std::shared_ptr<talFD::IDownloadFile>(talFD::get_interface());
	p22->initialize(url2, savedir, progress_func);
	if (ec != talFD::errorCode::initialize_finish)
	{
		printf("Initilize error: %d\n", ec);
		return -1;
	}
	p22->start_download();
	std::this_thread::sleep_for(std::chrono::seconds(4));

	p11->wait();
	p22->wait();

	printf("downlaod finish");
	char a = getchar();
	return 0;
}