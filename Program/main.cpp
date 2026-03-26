#include "my_dll_api.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <string>

using std::array;
using std::cerr;
using std::cout;
using std::endl;
using std::string;

namespace
{
    // konstantos
    constexpr int studentoId = 2412939;
    constexpr const char *userName = "user01";
    constexpr const char *weekDays = "M-F";
    constexpr const char *timeInterval = "08:00-20:00";
    constexpr float FValues[5] = {-2, -1, 0, 1, 2};

    // error'u helper
    int printProgramError(string functionName, string details) { cerr << "\n[Programa] " << functionName << " failed: " << details << endl; }

} // namespace

int main()
{
    const float x0 = (studentoId % 19) * -1;
    const float xn = studentoId % 25;
    const float dx = studentoId / 1000000000000.0;

    cout << "Starting program..." << endl;
    const float startTime = Dll_GetCurrentTimeSeconds();

    int status = printProgramError(
        "Dll_SetUserTimeRestriction",
        Dll_SetUserTimeRestriction(userName, weekDays, timeInterval));
    if (status != 0)
    {
        cerr << "Warning: Dll_SetUserTimeRestriction failed, continuing." << endl;
    }

    cout << "Creating working folder tree..." << endl;
    status = printProgramError("Dll_CreateWorkingFolderTree", Dll_CreateWorkingFolderTree());
    if (status != 0)
    {
        return 1;
    }

    for (float fValue : FValues)
    {
        status = printProgramError(
            "Dll_ComputeAndDistributePoints",
            Dll_ComputeAndDistributePoints(fValue, x0, xn, dx));
        if (status != 0)
        {
            Dll_DeleteWorkingFolderTree();
            return 1;
        }

        status = printProgramError("Dll_MergeSortAndFinalize", Dll_MergeSortAndFinalize(fValue));
        if (status != 0)
        {
            Dll_DeleteWorkingFolderTree();
            return 1;
        }
    }

    status = printProgramError("Dll_DeleteWorkingFolderTree", Dll_DeleteWorkingFolderTree());
    if (status != 0)
    {
        return 1;
    }

    const float endTime = Dll_GetCurrentTimeSeconds();
    const float totalTime = endTime - startTime;

    cout << "Execution time: " << totalTime << " s" << endl;
    return 0;
}
