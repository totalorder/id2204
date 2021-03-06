cmake_minimum_required(VERSION 3.7)
project(Uppgift3)

set(CMAKE_CXX_STANDARD 11)

set(SQUARE_SOURCE_FILES square.cpp)
set(NO-OVERLAP_SOURCE_FILES no-overlap.cpp)
set(SOURCE_FILES square.cpp)

add_executable(square ${SQUARE_SOURCE_FILES})
add_executable(no-overlap ${NO-OVERLAP_SOURCE_FILES})

add_executable(Uppgift3 ${SOURCE_FILES})
add_executable(Slask slask.cpp)

set(GECODE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../gecode-5.0.0)
include_directories(${GECODE_DIR})

set(LIBRARIES
        ${GECODE_DIR}/libgecodedriver.so
        ${GECODE_DIR}/libgecodeflatzinc.so
        ${GECODE_DIR}/libgecodefloat.so
        ${GECODE_DIR}/libgecodegist.so
        ${GECODE_DIR}/libgecodeint.so
        ${GECODE_DIR}/libgecodekernel.so
        ${GECODE_DIR}/libgecodeminimodel.so
        ${GECODE_DIR}/libgecodesearch.so
        ${GECODE_DIR}/libgecodeset.so
        ${GECODE_DIR}/libgecodesupport.so
        )


target_link_libraries(Uppgift3 ${LIBRARIES})
target_link_libraries(square ${LIBRARIES})
target_link_libraries(no-overlap ${LIBRARIES})