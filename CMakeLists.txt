cmake_minimum_required(VERSION 2.6)
project(mediabox)
set(CMAKE_MODULE_PATH .)
find_package(Threads REQUIRED)
find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(SDL_ttf REQUIRED)
find_package(MySQL)

message(STATUS "CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_CXX_FLAGS "-Wall -O2")
ADD_DEFINITIONS(-DINSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})

set(src mplayer.cc main.cc sdlcanvas.cc sdlinput.cc browser.cc memorydb.cc config.cc)
set(lib ${CMAKE_THREAD_LIBS_INIT} ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY} ${SDLTTF_LIBRARY})

if(MYSQL_FOUND)
  include_directories(${MYSQL_INCLUDE_DIR})
  set(src ${src} mysqldb.cc)
  set(lib ${lib} ${MYSQL_LIBRARY})
  ADD_DEFINITIONS(-D__HAVE_MYSQL__)
endif(MYSQL_FOUND)

include_directories(${SDL_INCLUDE_DIR} ${SDLIMAGE_INCLUDE_DIR} ${SDLTTF_INCLUDE_DIR})
add_executable(mediabox ${src})
target_link_libraries(mediabox  ${lib})

install(PROGRAMS mediabox DESTINATION bin)
install(FILES folder.png media.png stOrmblue-scaled.jpg  DESTINATION share/mediabox)
