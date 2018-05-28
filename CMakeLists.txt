cmake_minimum_required(VERSION 3.5)

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)

project(perfbench)
add_subdirectory (src)

enable_testing ()
add_subdirectory (tests)




