#include "OS_2PD_dll.h"

// konstantos
constexpr int STUDENTO_ID = 2412939;
constexpr const char *WIN_USER = "user01";
constexpr const char *WEEK_DAYS = "M-F";
constexpr const char *TIME_INTERVAL = "08:00-20:00";
constexpr float F_VALUES[5] = {-2, -1, 0, 1, 2};

// error'u helper
void printProgramError(string functionName, string details) { cerr << "\n[Programa] " << functionName << " failed: " << details << endl; }

int main()
{
    const float x0 = (STUDENTO_ID % 19) * -1;
    const float xn = STUDENTO_ID % 25;
    const float dx = STUDENTO_ID / 1000000000000.0;

    cout << "Starting program... " << endl;
    const float startTime = Dll_GetCurrentTimeSeconds();

    cout << "Setting user restrictions... ";
    int success = Dll_SetUserTimeRestriction(WIN_USER, WEEK_DAYS, TIME_INTERVAL);
    if (success != 0)
    {
        printProgramError("Dll_SetUserTimeRestriction", string("Couldn't run command with params") + WIN_USER + WEEK_DAYS + TIME_INTERVAL);
        return 1;
    }

    cout << endl << "Creating folder tree... ";
    success = Dll_CreateWorkingFolderTree();
    if (success != 0)
    {
        printProgramError("Dll_CreateWorkingFolderTree", "unknown");
        return 1;
    }
    cout << "done" << endl;

    for (float fValue : F_VALUES)
    {
        cout << endl << "Computing and distributing points for F=" << fValue << "... ";
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

    cout << endl << "Deleting folder tree... ";
    success = Dll_DeleteWorkingFolderTree();
    if (success != 0)
    {
        printProgramError("Dll_DeleteWorkingFolderTree", "Couldnt run command");
        return 1;
    }
    cout << "done" << endl;

    const float endTime = Dll_GetCurrentTimeSeconds();
    const float totalTime = endTime - startTime;

    cout << endl << "Execution time: " << totalTime << " s" << endl;
    return 0;
}
