#include "OS_2PD_dll.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <string>

using std::array;
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::to_string;

namespace
{
    // konstantos
    constexpr int studentoId = 2412939;
    constexpr const char *userName = "user01";
    constexpr const char *weekDays = "M-F";
    constexpr const char *timeInterval = "08:00-20:00";
    constexpr float FValues[5] = {-2, -1, 0, 1, 2};

    // error'u helper
    void printProgramError(string functionName, string details) { cerr << "\n[Programa] " << functionName << " failed: " << details << endl; }

} // namespace

int main()
{
    const float x0 = (studentoId % 19) * -1;
    const float xn = studentoId % 25;
    const float dx = studentoId / 1000000000000.0;

    cout << "Starting program... " << endl;
    const float startTime = Dll_GetCurrentTimeSeconds();

    cout << "Setting user restrictions... ";
    int success = Dll_SetUserTimeRestriction(userName, weekDays, timeInterval);
    if (success != 0)
    {
        printProgramError("Dll_SetUserTimeRestriction", string("Couldn't run command with params") + userName + weekDays + timeInterval);
        return 1;
    }
    cout << "done" << endl;

    cout << "Creating folder tree... ";
    success = Dll_CreateWorkingFolderTree();
    if (success != 0)
    {
        printProgramError("Dll_CreateWorkingFolderTree", "unknown");
        return 1;
    }
    cout << "done" << endl;

    for (float fValue : FValues)
    {
        cout << "Computing and distributing points for" << fValue << "... ";
        success = Dll_ComputeAndDistributePoints(fValue, x0, xn, dx);
        if (success != 0)
        {
            printProgramError("Dll_ComputeAndDistributePoints", "Couldnt run command with params" + to_string(fValue) + to_string(x0) + to_string(xn) + to_string(dx));
            Dll_DeleteWorkingFolderTree();
            return 1;
        }

        success = Dll_MergeSortAndFinalize(fValue);
        if (success != 0)
        {
            printProgramError("Dll_MergeSortAndFinalize", "Couldnt run command with params" + to_string(fValue));
            Dll_DeleteWorkingFolderTree();
            return 1;
        }
        cout << "done" << endl;
    }

    cout << "Deleting folder tree... ";
    success = Dll_DeleteWorkingFolderTree();
    if (success != 0)
    {
        printProgramError("Dll_DeleteWorkingFolderTree", "Couldnt run command");
        return 1;
    }
    cout << "done" << endl;

    const float endTime = Dll_GetCurrentTimeSeconds();
    const float totalTime = endTime - startTime;

    cout << "Execution time: " << totalTime << " s" << endl;
    return 0;
}
