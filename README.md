# FileDownload - a library downloading file from breakpoint

### It's used for downloading http file with C++. You can resume downloading from last break point. I have referred https://github.com/junyux/HttpClient and re-wrapped it. Pass testing in windows and ubuntu. 

### api: FileDownload\FileDownload\include\downloadAPI.h

### demo: D:\src\FileDownload\test\main.cpp


    std::shared_ptr<talFD::IDownloadFile> p1 = std::shared_ptr<talFD::IDownloadFile>(talFD::get_interface());
    talFD::errorCode ec;
    ec = p1->initialize(url1, savedir, progress_func);
    if (ec != talFD::errorCode::initialize_finish)
    
    {
    
        printf("Initilize error: %d\n", ec);
     	return -1;
    }
    p1->start_download();