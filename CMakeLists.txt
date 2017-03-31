cmake_minimum_required(VERSION 3.7)
project(id2204)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES sudoku.cpp)

add_executable(id2204 ${SOURCE_FILES})

set(GECODE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../gecode-5.0.0)
include_directories(${GECODE_DIR})
target_link_libraries(id2204
        ${GECODE_DIR}/libgecodedriver.so
        ${GECODE_DIR}/libgecodeflatzinc.so
        ${GECODE_DIR}/libgecodefloat.so
        ${GECODE_DIR}/libgecodegist.so
        ${GECODE_DIR}/libgecodeint.so
        ${GECODE_DIR}/libgecodekernel.so
        ${GECODE_DIR}/libgecodeminimodel.so
        ${GECODE_DIR}/libgecodesearch.so
        ${GECODE_DIR}/libgecodeset.so
        ${GECODE_DIR}/libgecodesupport.so)