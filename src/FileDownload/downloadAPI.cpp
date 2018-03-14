#include <downloadAPI.h>
#include <FileDownload.h>
namespace talFD
{
	IDownloadFile* get_interface()
	{
		return new FileDownload();
	}

	 //IDownloadFile::~IDownloadFile()
	 //{

	 //}

}

