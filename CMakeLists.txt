MESSAGE("Begin to CMake total:")

cmake_minimum_required(VERSION 3.2)
project(FileDownload)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_CONFIGURATION_TYPES Release)

MESSAGE("operation system is ${CMAKE_SYSTEM}")

IF(WIN32)
	set(LIBDIR ${PROJECT_SOURCE_DIR}/lib/win)
ELSE()
	set(LIBDIR ${PROJECT_SOURCE_DIR}/lib/ubuntu)
ENDIF()
MESSAGE("LIBDIR is ${LIBDIR}")

IF(WIN32)
	set(BOOST_LIB "D:/src/3rdParty/boost/boost_1_66_0/lib64-msvc-14.0")
	set(BOOST_DIR "D:/src/3rdParty/boost/boost_1_66_0")
ELSE()
	set(BOOST_LIB "/mnt/hgfs/shares/boost_1_66_0/lib64-ubuntu")
	set(BOOST_DIR "/mnt/hgfs/shares/boost_1_66_0")
ENDIF()


IF(WIN32)
	MESSAGE("Copy ${LIBDIR}/libcurl.dll to ${EXECUTABLE_OUTPUT_PATH}/release")
	file(COPY ${LIBDIR}/libcurl.dll DESTINATION ${EXECUTABLE_OUTPUT_PATH}/release)
ELSE()
	MESSAGE("Copy ${LIBDIR}/libcurl.so to ${EXECUTABLE_OUTPUT_PATH}")
	file(COPY ${LIBDIR}/libcurl.so DESTINATION ${EXECUTABLE_OUTPUT_PATH})
	MESSAGE("Copy libboost_filesystem.so to ${EXECUTABLE_OUTPUT_PATH}")
	file(COPY ${BOOST_LIB}/libboost_filesystem.so.1.66.0 DESTINATION ${EXECUTABLE_OUTPUT_PATH})
	MESSAGE("Copy libboost_serialization.so to ${EXECUTABLE_OUTPUT_PATH}")
	file(COPY ${BOOST_LIB}/libboost_serialization.so.1.66.0 DESTINATION ${EXECUTABLE_OUTPUT_PATH})
	MESSAGE("Copy libboost_system.so to ${EXECUTABLE_OUTPUT_PATH}")
	file(COPY ${BOOST_LIB}/libboost_system.so.1.66.0 DESTINATION ${EXECUTABLE_OUTPUT_PATH})
ENDIF()

add_subdirectory(
	src/FileDownload
)

add_subdirectory(
	src/test
)

MESSAGE("End CMake total")




