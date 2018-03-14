MESSAGE("Begin to CMake demo")

include_directories(
	../../include
	../../include/curl
	${BOOST_DIR}
)

link_directories(
	${LIBDIR}
	${PROJECT_BINARY_DIR}/bin
	${BOOST_LIB}
)

MESSAGE("build demo exe")
add_executable(
	demo
	main.cpp
)

MESSAGE("link FileDownload.lib")
IF(WIN32)
	target_link_libraries(
	demo
	FileDownload.lib
	)
	MESSAGE("windows link FileDownload.lib")
ELSE()
	target_link_libraries(
	demo
	FileDownload.so
	)
	MESSAGE("linux link FileDownload.so")
ENDIF()

IF(WIN32)
	MESSAGE("Set /EHsc")
	SET(CMAKE_CXX_FLAGS "/EHsc")
ELSE()
ENDIF()



MESSAGE("End CMake demo")

