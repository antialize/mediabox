cmake_minimum_required(VERSION 2.6)
project(mediabox)
set(CMAKE_MODULE_PATH .)
find_package(Threads REQUIRED)
find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(SDL_ttf REQUIRED)
find_package(MySQL)

set(CMAKE_CXX_FLAGS "-Wall -ggdb")

set(mysqlSources)
set(mysqlLibs)

if(MYSQL_FOUND)
  include_directories(${MYSQL_INCLUDE_DIR})
  set(mysqlSources mysqldb.cc)
  set(mysqlLibs ${MYSQL_LIBRARY})
  ADD_DEFINITIONS(-D__HAVE_MYSQL__)
endif(MYSQL_FOUND)

include_directories(${SDL_INCLUDE_DIR} ${SDLIMAGE_INCLUDE_DIR} ${SDLTTF_INCLUDE_DIR})
add_executable(mediabox mplayer.cc main.cc sdlcanvas.cc sdlinput.cc browser.cc memorydb.cc ${mysqlSources} config.cc)
target_link_libraries(mediabox ${CMAKE_THREAD_LIBS_INIT} ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY} ${SDLTTF_LIBRARY} ${mysqlLibs})
