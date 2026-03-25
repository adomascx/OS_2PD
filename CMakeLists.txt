cmake_minimum_required(VERSION 3.20)
project(OS_2PD LANGUAGES CXX)

add_library(MyStudentDll SHARED
    DLL/my_dll_api.cpp
)

target_compile_features(MyStudentDll PRIVATE cxx_std_17)
target_compile_definitions(MyStudentDll PRIVATE BUILDING_MY_DLL)
target_include_directories(MyStudentDll
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/DLL
)

set_target_properties(MyStudentDll PROPERTIES
    OUTPUT_NAME "MyStudentDll"
)

add_executable(OS_DLL_Program
    Program/main.cpp
)

target_compile_features(OS_DLL_Program PRIVATE cxx_std_17)
target_include_directories(OS_DLL_Program PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/DLL
)
target_link_libraries(OS_DLL_Program PRIVATE MyStudentDll)

install(TARGETS MyStudentDll OS_DLL_Program
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES DLL/my_dll_api.h DESTINATION include)
