
cmake_minimum_required (VERSION 2.8)

# CMake disable -std=c++11 flag for C files
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")

set (PROJ_NAME realtinet)
project (realtinet)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


if(UNIX)

    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")  # Inhibit all warning messages.
        set(CXX_FLAGS
            -g
            # -DVALGRIND
            # -DMUDUO_STD_STRING
            -DCHECK_PTHREAD_RETURN_VALUE
            -D_FILE_OFFSET_BITS=64
            -Wall
            -Wextra
            -Werror
            # -Wconversion
            -Wno-unused-parameter
            # -Wold-style-cast
            -Wno-multichar
            -Wno-unused-function
            -Wno-reorder
            -Woverloaded-virtual
            -Wpointer-arith
            # -Wshadow
            -Wwrite-strings
            -march=native
            # -MMD
            -std=c++0x
            -rdynamic
        )
        string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
    endif()

  	# For gdb
    set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

elseif(WIN32)
      # message(SEND_ERROR "You are on an unsupported platform! (Not Unix)")
    macro(create_win_proj win_proj_name)
        if(MSVC) # Check if we are using the Visual Studio compiler

            file(GLOB_RECURSE project_headers *.h*)
            file(GLOB_RECURSE project_cpps *.c*)
            set(all_files ${project_headers} ${project_cpps})

            # 获取当前目录
            set(current_dir ${CMAKE_CURRENT_SOURCE_DIR})
            foreach(src_file ${all_files})
                # 求出相对路径
                string(REPLACE ${current_dir}/ "" rel_path_name ${src_file})
                # 删除相对路径中的文件名部分
                string(REGEX REPLACE "(.*)/.*" \\1 rel_path ${rel_path_name})
                # 比较是否是当前路径下的文件
                string(COMPARE EQUAL ${rel_path_name} ${rel_path} is_same_path)
                # 替换成Windows平台的路径分隔符
                string(REPLACE "/" "\\" rel_path ${rel_path})
                if(is_same_path)
                    set(rel_path "\\")
                endif(is_same_path)

                # CMake 命令
                source_group(${rel_path} FILES ${src_file})
            endforeach(src_file)
    
            # 指定生成目标
            add_executable(${win_proj_name} ${all_files})

            # set_target_properties(${win_proj_name} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS") # works for all build modes
            set_target_properties(${win_proj_name} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE") # works for all build modes
            set_target_properties(${win_proj_name} PROPERTIES FOLDER "realtime_srv")
            target_link_libraries(${win_proj_name} wsock32 ws2_32)
            set_target_properties(${win_proj_name}
                PROPERTIES
                COMPILE_FLAGS /wd"4819"
            )

        elseif(CMAKE_COMPILER_IS_GNUCXX)
            # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows") # Not tested
        else()
            message(SEND_ERROR "You are using an unsupported Windows compiler! (Not MSVC or GCC)")
        endif(MSVC)
    endmacro(create_win_proj)

    create_win_proj(${PROJ_NAME})
    
ENDIF()




include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/core)
include_directories(${PROJECT_SOURCE_DIR}/third)
include_directories(${PROJECT_SOURCE_DIR}/third/asio/asio/include)

add_subdirectory(core)
add_subdirectory(test)
# add_subdirectory(third/muduo)
add_subdirectory(third/kcpp)