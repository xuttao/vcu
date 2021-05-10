project(fservo)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

option(OPT_NEON_ENABLE OFF)

if(OPT_NEON_ENABLE)
        SET(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb -fPIC")
        SET(CMAKE_C_FLAGS_RELEASE  "$ENV{CFLAGS} -O3 -Wall -fPIC")

        SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -fPIC -std=c++17")
        SET(CMAKE_CXX_FLAGS_RELEASE  "$ENV{CXXFLAGS} -O3 -Wall -fPIC -std=c++17")
        
        add_definitions(-D_NEON)

else()

        SET(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb -fPIC")
        SET(CMAKE_C_FLAGS_RELEASE  "$ENV{CFLAGS} -O3 -Wall -fPIC")

        SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -fPIC -std=c++17")
        SET(CMAKE_CXX_FLAGS_RELEASE  "$ENV{CXXFLAGS} -O3 -Wall -fPIC -std=c++17")

endif()


ADD_DEFINITIONS(-DOPT_FSERVO_VERSION="snowlake_fservo_v1.0")

option(OPT_LOG_ENABLE  "option for log" ON)

option(OPT_BUILD_ARM64_ENABLE  "option for building target bin(ARM64)" ON)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    ADD_DEFINITIONS(-DOPT_DEBUG_ENABLE=1)
    ADD_DEFINITIONS(-D_DEBUG)
else()
    ADD_DEFINITIONS(-DOPT_DEBUG_ENABLE=0)
endif()

if (OPT_LOG_ENABLE)
    ADD_DEFINITIONS(-DOPT_LOG_ENABLE)
endif()

if (OPT_BUILD_ARM64_ENABLE)
        link_directories(${ARM_COMPILER_PATH}/lib)

        include_directories(${ARM_COMPILER_PATH}/include)

        ADD_DEFINITIONS(-DOPT_BUILD_ARM64_ENABLE)

endif()

include_directories(src/common)

add_definitions(-D_Unix)
add_definitions(-D_GPS)
#add_definitions(-D_Win32)

enable_language(ASM)

if(OPT_BUILD_ARM64_ENABLE)
        message(STATUS "--------------arm--------------")
        add_definitions(-D_ARM64)
        set(target_name fservo-vcu)
        
        link_directories(3rd/arm/opencv/lib)
        include_directories(3rd/arm/opencv/include)  

        include_directories(3rd/arm/libjpeg-turbo/include)
        link_directories(3rd/arm/libjpeg-turbo/lib)

        include_directories(3rd/arm/vcu/inc)
        link_directories(3rd/arm/vcu/lib)

        include_directories(3rd/arm/yuv/include)
        link_directories(3rd/arm/yuv/lib)

        set(CMAKE_PREFIX_PATH "3rd/arm/qt/lib/cmake")

        
else()

        message(STATUS "--------------x86 mode--------------")
        include_directories(3rd/arm/vcu/inc)
        
        link_directories(3rd/x86/opencv/lib)
        include_directories(3rd/x86/opencv/include)

        include_directories(/media/xtt/hdd/3rdlib/build/libjpeg-turbo-2.0.5_x86)
        link_directories(/media/xtt/hdd/3rdlib/build/libjpeg-turbo-2.0.5_x86/build)

        #     include_directories(/media/xtt/hdd/code_other/FreeImage-x86/Source)
        #     link_directories(/media/xtt/hdd/code_other/FreeImage-x86)
        include_directories(/media/xtt/hdd/3rdlib/ffmpeg/include)
        link_directories(/media/xtt/hdd/3rdlib/ffmpeg/lib)

        include_directories(/media/xtt/hdd/3rdlib/libyuv/include)
        link_directories(/media/xtt/hdd/3rdlib/libyuv/lib)

        # include_directories(/media/xtt/hdd/3rdlib/live555/include)
        # link_directories(/media/xtt/hdd/3rdlib/live555/lib)

        set(target_name fservo-x86)

        set(CMAKE_PREFIX_PATH "/media/xtt/hdd/3rdlib/Qt5.12.9/5.12.9/gcc_64/lib/cmake")

endif()


message(STATUS "Looking for Qt...")
find_package(Qt5 REQUIRED Core SerialPort) #REQUIRED表示一定要找到包，否则停止cmake
if (${Qt5_FOUND})
    message(STATUS "Found Qt " ${Qt5_VERSION})
else()
    message(STATUS "Couldn't find Qt")
endif()

include_directories(${Qt5Core_INCLUDE_DIRS} ${Qt5_INCLUDE_DIRS})
link_libraries(
    Qt5::Core
    Qt5::SerialPort
)
qt5_wrap_cpp(moc_list 
        src/serialPort.h
)

include_directories(./inc)
include_directories(./src/common)
include_directories(./src)
include_directories(./src/yuv2rgb)

aux_source_directory(./src src_list)
aux_source_directory(./src/common common_list)
aux_source_directory(./src/test   test_list)
aux_source_directory(./src/yuv2rgb   yuv_list)

link_libraries(pthread)

add_executable(${target_name} ${src_list} ${common_list} ${test_list} ${vcu_list} ${yuv_list} ${moc_list})
if(OPT_BUILD_ARM64_ENABLE)
target_link_libraries(${target_name} opencv_highgui opencv_imgproc opencv_core opencv_imgcodecs opencv_videoio turbojpeg vcu-2020.1 libyuv.a rt)
else()
target_link_libraries(${target_name} opencv_highgui opencv_imgproc opencv_core opencv_imgcodecs opencv_videoio turbojpeg libyuv.a 
                                                libBasicUsageEnvironment.a libgroupsock.a libliveMedia.a libUsageEnvironment.a)
endif()



