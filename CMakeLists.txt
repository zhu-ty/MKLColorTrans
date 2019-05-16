# CMake file for MKL Color Transfer
# Author: zhu-ty
# E-mail: zhu.shadowk@gmail.com
#
message(
"
       
           ┌─┐       ┌─┐
        ┌──┘ ┴───────┘ ┴──┐
        │                 │
        │       ───       │
        │  ─┬┘       └┬─  │
        │                 │
        │       ─┴─       │
        │                 │
        └───┐         ┌───┘
            │         │
            │         │
            │         │
            │         └──────────────┐
            │                        │
            │                        ├─┐
            │                        ┌─┘    
            │                        │
            └─┐  ┐  ┌───────┬──┐  ┌──┘         
              │ ─┤ ─┤       │ ─┤ ─┤         
              └──┴──┘       └──┴──┘ 
                  神兽  保佑 
                  代码无BUG! 
  
")

cmake_minimum_required (VERSION 3.10)
set(name "MKLColorTrans")
project ( ${name} )


# cmake module
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# c++ version
set (CMAKE_CXX_STANDARD 11)
set (WIN_LIB_PATH "E:/libs")

#opencv 
if ( WIN32 )
	set(OpenCV_DIR ${WIN_LIB_PATH}/OpenCV/x64/vc14/lib)
	set(OpenCV_CONFIG_PATH ${WIN_LIB_PATH}/OpenCV)
endif()
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

#cuda
find_package(CUDA)
set(CUDA_NVCC_FLAGS
    ${CUDA_NVCC_FLAGS}
	-gencode=arch=compute_61,code=\"sm_61,compute_61\")

# opengl & Eigen
find_package(OpenGL REQUIRED)
set(GLM_INCLUDE_PATH ${WIN_LIB_PATH}/OpenGL/include)
include_directories(${GLM_INCLUDE_PATH})
set(EIGEN_INCLUDE_DIR ${WIN_LIB_PATH}/Eigen/include/eigen3)
include_directories(${EIGEN_INCLUDE_DIR})

#SKEncoder
set(SK_ENCODER_DIR ${PROJECT_SOURCE_DIR}/SKEncoder)
file (GLOB ske_h "${SK_ENCODER_DIR}/*.h")
file (GLOB ske_hpp "${SK_ENCODER_DIR}/*.hpp")
file (GLOB ske_cpp "${SK_ENCODER_DIR}/*.cpp")
file (GLOB ske_c "${SK_ENCODER_DIR}/*.c")
set (SK_ENCODER_FILES
    ${ske_h}
	${ske_hpp}
	${ske_c}
	${ske_cpp}
)
source_group("_SKENCODER_" FILES ${SK_ENCODER_FILES})
include_directories(${SK_ENCODER_DIR})



include_directories(${PROJECT_SOURCE_DIR})
file (GLOB this_h "./*.h")
file (GLOB this_hpp "./*.hpp")
file (GLOB this_cpp "./*.cpp")
file (GLOB this_c "./*.c")

set(MY_FILES
    ${SK_ENCODER_FILES}
	${this_h}
	${this_hpp}
	${this_c}
	${this_cpp}
)

cuda_add_executable(${name}
	${MY_FILES}
)

target_link_libraries(${name}
	${OpenCV_LIBS}
	${CUDA_LIBRARY}
	${CUDA_npp_LIBRARY}
	cuda
)

SET(MAJOR_VERSION "1")
SET(MINOR_VERSION "0")
add_definitions( -D__MAJOR_VERSION__=${MAJOR_VERSION})
add_definitions( -D__MINOR_VERSION__=${MINOR_VERSION})

FILE(WRITE ${CMAKE_BINARY_DIR}/version.h.in
"\#define __GIT_VERSION__ \"@VERSION@\"\n"
)
IF(EXISTS ${PROJECT_SOURCE_DIR}/.git)
FILE(WRITE ${CMAKE_BINARY_DIR}/version.cmake
"EXECUTE_PROCESS(
     COMMAND git log -1 --format=%ad --date=format:%y%m%d.%H%M%S
     OUTPUT_VARIABLE VERSION
     OUTPUT_STRIP_TRAILING_WHITESPACE
 )
 CONFIGURE_FILE(\${SRC} \${DST} @ONLY)
")
ELSE()
FILE(WRITE ${CMAKE_BINARY_DIR}/version.cmake
"EXECUTE_PROCESS(
     COMMAND echo unknown
     OUTPUT_VARIABLE VERSION
     OUTPUT_STRIP_TRAILING_WHITESPACE
 )
 CONFIGURE_FILE(\${SRC} \${DST} @ONLY)
")
ENDIF()
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
#ADD_EXECUTABLE(main main.c)
ADD_CUSTOM_TARGET(
    version
    ${CMAKE_COMMAND} -D SRC=${CMAKE_BINARY_DIR}/version.h.in
                     -D DST=${CMAKE_BINARY_DIR}/version.h
                     -P ${CMAKE_BINARY_DIR}/version.cmake
)

ADD_DEPENDENCIES(${name} version)




