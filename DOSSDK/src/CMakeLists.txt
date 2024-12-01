cmake_minimum_required(VERSION 3.10)
project(DOSSDK)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# if your header files directory is in a different folder from your CMakeList.txt file, use this and delete the one above
#include_directories(${PROJECT_SOURCE_DIR}/include)

include_directories(../ThirdParty/libcurl/include)
include_directories(../ThirdParty/socket-io/include)

find_library(SIOCLIENT_LIB NAMES sioclient PATHS ../ThirdParty/socket-io/lib)
find_library(LIBCURL_LIB NAMES libcurl_imp PATHS ../ThirdParty/libcurl/lib)

#Provided you need to add an executable, uncomment the two lines below as well as the part of the file for installing the executable line 25
#add_executable(${PROJECT_NAME} DenateTypes.cpp DOS_Achievement.cpp DOS_AppUser.cpp DOS_Connection.cpp DOS_Friends.cpp DOS_Leaderboard.cpp DOS_Local_Match.cpp DOS_Online_Save.cpp DOS_Voice_Chat.cpp)

#target_link_libraries(${PROJECT_NAME} ${SIOCLIENT_LIB} ${LIBCURL_LIB})

add_library(${PROJECT_NAME}_lib STATIC DenateTypes.cpp DOS_Achievement.cpp DOS_AppUser.cpp DOS_Connection.cpp DOS_Friends.cpp DOS_Leaderboard.cpp DOS_Local_Match.cpp DOS_Online_Save.cpp DOS_Voice_Chat.cpp) 

# Install the executable 
#install(TARGETS ${PROJECT_NAME} DESTINATION bin) 

# Install the project's library 
install(TARGETS ${PROJECT_NAME}_lib DESTINATION lib) 

# Install the project's include files 
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include)

install(FILES ../ThirdParty/libcurl/lib/libcurl_imp.lib DESTINATION lib) 
install(FILES ../ThirdParty/socket-io/lib/sioclient.lib DESTINATION lib) 
install(DIRECTORY ../ThirdParty/libcurl/include/ DESTINATION include) 
install(DIRECTORY ../ThirdParty/socket-io/include/ DESTINATION include)
