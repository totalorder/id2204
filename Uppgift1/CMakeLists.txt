cmake_minimum_required(VERSION 3.7)
project(Uppgift1)

set(CMAKE_CXX_STANDARD 11)

# sudo apt-get install libqt4-dev
find_package(Qt4 REQUIRED QtGui)

set(SUDOKU_SOURCE_FILES sudoku.cpp)
set(SUDOKUSLOPPY_SOURCE_FILES sudokusloppy.cpp)
set(QUEENS_SOURCE_FILES queens.cpp)

add_executable(sudoku ${SUDOKU_SOURCE_FILES})
add_executable(sudokusloppy ${SUDOKUSLOPPY_SOURCE_FILES})
add_executable(queens ${QUEENS_SOURCE_FILES})

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
        Qt4::QtGui)
target_link_libraries(sudoku ${LIBRARIES})
target_link_libraries(sudokusloppy ${LIBRARIES})
target_link_libraries(queens ${LIBRARIES})
