cmake_minimum_required(VERSION 3.20)
project(OS_2PD LANGUAGES CXX)

if(WIN32)
    enable_language(RC)
endif()

add_library(OS_Dll SHARED
    DLL/OS_2PD_dll.cpp
)

target_compile_features(OS_Dll PRIVATE cxx_std_17)
target_compile_definitions(OS_Dll PRIVATE BUILDING_MY_DLL)
target_include_directories(OS_Dll
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/DLL
)

set_target_properties(OS_Dll PROPERTIES
    OUTPUT_NAME "OS_Dll"
)

set(OS_PROGRAMA_SOURCES
    program/main.cpp
)

if(WIN32)
    list(APPEND OS_PROGRAMA_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/program/manifest/admin.rc
    )
endif()

add_executable(OS_Programa
    ${OS_PROGRAMA_SOURCES}
)

target_compile_features(OS_Programa PRIVATE cxx_std_17)
target_include_directories(OS_Programa PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/DLL
)
target_link_libraries(OS_Programa PRIVATE OS_Dll)

if(MINGW)
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set(MINGW_RUNTIME_DLLS
        libstdc++-6.dll
        libgcc_s_seh-1.dll
        libwinpthread-1.dll
    )

    foreach(runtime_dll IN LISTS MINGW_RUNTIME_DLLS)
        if(EXISTS "${MINGW_BIN_DIR}/${runtime_dll}")
            add_custom_command(TARGET OS_Programa POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${MINGW_BIN_DIR}/${runtime_dll}"
                    "$<TARGET_FILE_DIR:OS_Programa>/${runtime_dll}"
            )
        endif()
    endforeach()
endif()

install(TARGETS OS_Dll OS_Programa
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES DLL/OS_2PD_dll.h DESTINATION include)
