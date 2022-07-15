cmake_minimum_required(VERSION 3.5)

project(WickrIOCSEDecryption LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#this will locate the aws sdk for c++ package so that we can use its targets
find_package(aws-sdk-cpp)

add_executable(WickrIOCSEDecryption main.cpp)
target_link_libraries(WickrIOCSEDecryption aws-cpp-sdk-s3 aws-cpp-sdk-s3-encryption aws-cpp-sdk-core)
