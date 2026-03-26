#pragma once

#include <string>

#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef BUILDING_MY_DLL
        #define OS_DLL __declspec(dllexport)
    #else
        #define OS_DLL __declspec(dllimport)
    #endif
#else
    #define OS_DLL
#endif

#ifdef __cplusplus
extern "C"
{
#endif

OS_DLL float __cdecl Dll_GetCurrentTimeSeconds(void);
OS_DLL int __cdecl Dll_SetUserTimeRestriction(const char *userName, const char *weekDays, const char *timeInterval);
OS_DLL int __cdecl Dll_CreateWorkingFolderTree(void);
OS_DLL int __cdecl Dll_ComputeAndDistributePoints(float F, float x0, float xn, float dx);
OS_DLL int __cdecl Dll_MergeSortAndFinalize(float F);
OS_DLL int __cdecl Dll_DeleteWorkingFolderTree(void);

#ifdef __cplusplus
}
#endif

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::array;
using std::cerr;
using std::cout;
using std::endl;
using std::error_code;
using std::fabs;
using std::fixed;
using std::floor;
using std::ifstream;
using std::ios;
using std::max;
using std::ofstream;
using std::ostringstream;
using std::round;
using std::setprecision;
using std::size_t;
using std::sort;
using std::sqrt;
using std::string;
using std::system;
using std::to_string;
using std::vector;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::filesystem::current_path;
using std::filesystem::exists;
using std::filesystem::path;
using std::filesystem::remove;


