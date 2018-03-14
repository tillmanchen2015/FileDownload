MESSAGE("Begin to CMake FileDownload")

include_directories(
	../../include
	../../include/curl
	${BOOST_DIR}
)
MESSAGE("boost dir is ${BOOST_DIR}")
link_directories(
	${LIBDIR}
	${BOOST_LIB}
)

file(
	GLOB src_list "*.cpp" 
)
MESSAGE("Filedownload src files:")
FOREACH(item ${src_list})
	MESSAGE("${item}")
ENDFOREACH(item ${src_list})

MESSAGE("build dll: FileDownload")
add_library(
	FileDownload
	SHARED
	${src_list}
)


IF(WIN32)
	MESSAGE("link libcurl.lib")
	target_link_libraries(
	FileDownload
	${LIBDIR}/libcurl.lib
	)
ELSE()
	MESSAGE("link libcurl.so, libboost_filesystem.so, libboost_system.so, libboost_serialization.so")
	target_link_libraries(
	FileDownload
	libcurl.so
	libboost_filesystem.so
	libboost_system.so
	libboost_serialization.so
	)
ENDIF()


IF(WIN32)
	MESSAGE("Set /EHsc")
	SET(CMAKE_CXX_FLAGS "/EHsc")
ELSE()

ENDIF()

MESSAGE("End CMake FileDownload")