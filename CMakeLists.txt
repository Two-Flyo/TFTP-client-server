cmake_minimum_required(VERSION 3.0.0)
project(TFTP VERSION 0.1.0 LANGUAGES C)

include(CTest)
enable_testing()

add_executable(tftp main.c tftp_base.c tftp_client.c tftp_server.c)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
