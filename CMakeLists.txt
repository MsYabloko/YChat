cmake_minimum_required(VERSION 3.24)
project(YChat)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/SQLiteCpp)

add_executable(YChat main.cpp src/tokens.h src/tokens.cpp src/messages.h src/messages.cpp libs/mongoose.c libs/src/bcrypt.cpp libs/src/blowfish.cpp)

target_include_directories(YChat PRIVATE includes)
target_link_libraries(YChat
  SQLiteCpp
  sqlite3
  pthread
  jsoncpp
  dl
  )
