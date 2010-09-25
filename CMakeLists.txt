# Mediabox
# Copyright (C) 2009 Jakob Truelsen

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
cmake_minimum_required(VERSION 2.6)
project(mediabox)


set(CPACK_PACKAGE_NAME "mediabox")
set(CPACK_PACKAGE_VERSION "0.1")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
set(CPACK_PACKAGE_CONTACT antialize@gmail.com)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Mediabox")
set(CPACK_PACKAGE_DESCRIPTION "mediabox")
set(CPACK_GENERATOR "DEB")

INCLUDE(CPack)

set(CMAKE_MODULE_PATH .)
find_package(Threads REQUIRED)
find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(SDL_ttf REQUIRED)
find_package(MySQL)

message(STATUS "CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_CXX_FLAGS "-Wall -ggdb")
ADD_DEFINITIONS(-DINSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})

set(src mplayer.cc main.cc sdlcanvas.cc sdlinput.cc browser.cc memorydb.cc config.cc videopart.cc systemmenu.cc musicpart.cc listbox.cc gamespart.cc)
set(lib ${CMAKE_THREAD_LIBS_INIT} ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY} ${SDLTTF_LIBRARY})

if(MYSQL_FOUND)
  include_directories(${MYSQL_INCLUDE_DIR})
  set(src ${src} mysqldb.cc)
  set(lib ${lib} ${MYSQL_LIBRARY})
  ADD_DEFINITIONS(-D__HAVE_MYSQL__)
endif(MYSQL_FOUND)

include_directories(${SDL_INCLUDE_DIR} ${SDLIMAGE_INCLUDE_DIR} ${SDLTTF_INCLUDE_DIR})
add_executable(mediabox ${src})
target_link_libraries(mediabox  ${lib} -lrt)

install(PROGRAMS mediabox DESTINATION bin)
install(FILES folder.png media.png stOrmblue-scaled.jpg exit.png video.png games.png music.png playing.png DESTINATION share/mediabox)
