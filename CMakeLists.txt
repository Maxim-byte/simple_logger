cmake_minimum_required(VERSION 3.20)
project(simple_logger)

set(CMAKE_CXX_STANDARD 14)


set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.68 COMPONENTS program_options)
FIND_PACKAGE( Boost 1.40 COMPONENTS program_options REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )


add_executable(simple_logger main.cpp
        core.hpp
        )

TARGET_LINK_LIBRARIES( simple_logger LINK_PUBLIC ${Boost_LIBRARIES} )

