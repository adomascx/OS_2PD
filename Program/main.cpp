#include "my_dll_api.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

constexpr int kStatusSuccess = 0;
constexpr int kStudentId = 2412939;

// Hardcoded as requested by project constraints.
constexpr const char* kUserName = "user01";
constexpr const char* kWeekDays = "M-F";
constexpr const char* kTimeInterval = "08:00-20:00";

constexpr std::array<float, 5> kFValues = {-2.0f, -1.0f, 0.0f, 1.0f, 2.0f};

bool IsNullOrEmpty(const char* value) {
    return value == nullptr || value[0] == '\0';
}

bool ValidateInputs(float x0, float xn, float dx) {
    if (IsNullOrEmpty(kUserName) || IsNullOrEmpty(kWeekDays) || IsNullOrEmpty(kTimeInterval)) {
        std::cerr << "Input validation failed: restriction parameters must be non-empty." << std::endl;
        return false;
    }

    if (dx <= 0.0f) {
        std::cerr << "Input validation failed: dx must be > 0." << std::endl;
        return false;
    }

    if (xn < x0) {
        std::cerr << "Input validation failed: xn must be >= x0." << std::endl;
        return false;
    }

    return true;
}

int CheckStatus(const char* stepName, int status) {
    if (status == kStatusSuccess) {
        return kStatusSuccess;
    }

    std::cerr << stepName << " failed with status " << status << std::endl;
    return status;
}

}  // namespace

int main() {
    const float x0 = static_cast<float>(-(kStudentId % 19));
    const float xn = static_cast<float>(kStudentId % 25);
    const float dx = static_cast<float>(kStudentId / 1000000000000.0);

    if (!ValidateInputs(x0, xn, dx)) {
        return EXIT_FAILURE;
    }

    const float startTime = Dll_GetCurrentTimeSeconds();

    int status = CheckStatus(
        "Dll_SetUserTimeRestriction",
        Dll_SetUserTimeRestriction(kUserName, kWeekDays, kTimeInterval));
    if (status != kStatusSuccess) {
        return EXIT_FAILURE;
    }

    status = CheckStatus("Dll_CreateWorkingFolderTree", Dll_CreateWorkingFolderTree());
    if (status != kStatusSuccess) {
        return EXIT_FAILURE;
    }

    for (float fValue : kFValues) {
        status = CheckStatus(
            "Dll_ComputeAndDistributePoints",
            Dll_ComputeAndDistributePoints(fValue, x0, xn, dx));
        if (status != kStatusSuccess) {
            Dll_DeleteWorkingFolderTree();
            return EXIT_FAILURE;
        }

        status = CheckStatus("Dll_MergeSortAndFinalize", Dll_MergeSortAndFinalize(fValue));
        if (status != kStatusSuccess) {
            Dll_DeleteWorkingFolderTree();
            return EXIT_FAILURE;
        }
    }

    status = CheckStatus("Dll_DeleteWorkingFolderTree", Dll_DeleteWorkingFolderTree());
    if (status != kStatusSuccess) {
        return EXIT_FAILURE;
    }

    const float endTime = Dll_GetCurrentTimeSeconds();
    const float totalTime = endTime - startTime;

    std::cout << "Execution time: " << totalTime << " s" << std::endl;
    return EXIT_SUCCESS;
}
