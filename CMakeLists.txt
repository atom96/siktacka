cmake_minimum_required(VERSION 3.5)
project(sik_duze)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -O2")

set(COMMON_SOURCE_FILES client_state.h client_state.cpp helper.h helper.cpp additional_exceptions.h udpmessenger.h udpmessenger.cpp tcpmessenger.h tcpmessenger.cpp messeges.h messeges.cpp events.h events.cpp additional_exceptions.cpp)
set(CLIENT_SOURCE_FILES client_main.cpp client.h client.cpp)
set(SERVER_SOURCE_FILES server.cpp server.h server_state.h server_state.cpp server_main.cpp)
add_executable(server ${SERVER_SOURCE_FILES} ${COMMON_SOURCE_FILES})

add_executable(client ${CLIENT_SOURCE_FILES} ${COMMON_SOURCE_FILES})

target_link_libraries(client z)
target_link_libraries(server z)
