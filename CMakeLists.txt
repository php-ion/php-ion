cmake_minimum_required(VERSION 3.2)
project(ion)

set(CMAKE_SOURCE_DIR ./src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/framework.c src/php_ion.c)
add_executable(ion ${SOURCE_FILES} src/php_ion.h src/php_ion.c src/framework.h src/framework.c)
include_directories("/opt/local/include" "/opt/local/include" "/opt/local/include/php56/php" "/opt/local/include/php56/php/main" "/opt/local/include/php56/php/Zend")