cmake_minimum_required(VERSION 2.8)

project(SimpleRegionGrowing)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

add_executable(SimpleRegionGrowing MACOSX_BUNDLE simpleRegionGrowing.cxx)
target_link_libraries(SimpleRegionGrowing ${ITK_LIBRARIES})
