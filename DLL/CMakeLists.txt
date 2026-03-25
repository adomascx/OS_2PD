cmake_minimum_required(VERSION 3.20)
project(MyStudentDll LANGUAGES CXX)

add_library(MyStudentDll SHARED
    my_dll_api.cpp
)

target_compile_features(MyStudentDll PRIVATE cxx_std_17)
target_compile_definitions(MyStudentDll PRIVATE BUILDING_MY_DLL)

target_include_directories(MyStudentDll
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(MyStudentDll PROPERTIES
    OUTPUT_NAME "MyStudentDll"
)
