#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef BUILDING_MY_DLL
#define MY_DLL_API __declspec(dllexport)
#else
#define MY_DLL_API __declspec(dllimport)
#endif
#else
#define MY_DLL_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

MY_DLL_API float __cdecl Dll_GetCurrentTimeSeconds(void);

MY_DLL_API int __cdecl Dll_SetUserTimeRestriction(
    const char* userName,
    const char* weekDays,
    const char* timeInterval);

MY_DLL_API int __cdecl Dll_CreateWorkingFolderTree(void);

MY_DLL_API int __cdecl Dll_ComputeAndDistributePoints(
    float F,
    float x0,
    float xn,
    float dx);

MY_DLL_API int __cdecl Dll_MergeSortAndFinalize(float F);

MY_DLL_API int __cdecl Dll_DeleteWorkingFolderTree(void);

#ifdef __cplusplus
}
#endif
