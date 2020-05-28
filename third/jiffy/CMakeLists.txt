cmake_minimum_required(VERSION 2.8.12)

set (PROJ_NAME jiffy)
project (${PROJ_NAME})

# support C++14
add_definitions(-std=c++1y)


set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

if (WIN32)
    add_definitions(
        -DNOMINMAX
        -DWIN32_LEAN_AND_MEAN
        -D_WIN32_WINNT=0x0600
        -D_CRT_SECURE_NO_WARNINGS
        -D_SCL_SECURE_NO_WARNINGS
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
    )
else()
    set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
endif()

file(GLOB LIB_HEADER_FILES ./*.h)
file(GLOB LIB_SOURCE_FILES ./*.c*)


macro(SOURCE_GROUP_BY_DIR source_files)
    set(sgbd_cur_dir ${CMAKE_CURRENT_SOURCE_DIR})
    foreach(sgbd_file ${${source_files}})
        string(REGEX REPLACE ${sgbd_cur_dir}/\(.*\) \\1 sgbd_fpath ${sgbd_file})
        string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
        string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
        string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
        if(sgbd_nogroup)
            set(sgbd_group_name "\\")
        endif(sgbd_nogroup)
        source_group(${sgbd_group_name} FILES ${sgbd_file})
    endforeach(sgbd_file)
endmacro(SOURCE_GROUP_BY_DIR)

SOURCE_GROUP_BY_DIR(LIB_HEADER_FILES)
SOURCE_GROUP_BY_DIR(LIB_SOURCE_FILES)


include_directories(${PROJECT_SOURCE_DIR})

# Creating a Header-Only CMake Target
# add_library(${PROJ_NAME} ${LIB_HEADER_FILES} ${LIB_SOURCE_FILES} )
# set_target_properties(${PROJ_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# add_library(${PROJ_NAME} INTERFACE
#  )
# #  target_include_directories(mylib INTERFACE my_include_dir1 my_include_dir2)
#  target_sources(${PROJ_NAME} INTERFACE 
#  ${LIB_HEADER_FILES}
#  ${LIB_SOURCE_FILES}
# ) 

#  set_target_properties(${PROJ_NAME} PROPERTIES LINKER_LANGUAGE CXX)




add_subdirectory(py)


# find_package(PythonLibs REQUIRED)    
# include_directories(${PYTHON_INCLUDE_DIRS})   
# include_directories(${PROJECT_SOURCE_DIR}/pybind11/include)
# add_subdirectory(pybind11)
# pybind11_add_module(_elapse ${LIB_HEADER_FILES} ${LIB_SOURCE_FILES})
