#include "my_dll_api.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr int kStatusSuccess = 0;
constexpr int kStatusInvalidInput = -1;
constexpr int kStatusSystemCommandFailed = -2;
constexpr int kStatusIoFailed = -3;

constexpr int kTreeBranchCount = 3;
constexpr int kLeafCount = kTreeBranchCount * kTreeBranchCount;

constexpr const char* kStudentName = "Adomas";
constexpr const char* kStudentSurname = "Lukosevicius";

struct Point {
    double x;
    double y;
};

bool IsNullOrEmpty(const char* value) {
    return value == nullptr || value[0] == '\0';
}

std::string QuoteForCmd(const std::filesystem::path& path) {
    return "\"" + path.string() + "\"";
}

int RunSystemCommand(const std::string& command) {
    const int rc = std::system(command.c_str());
    return rc == 0 ? kStatusSuccess : kStatusSystemCommandFailed;
}

std::filesystem::path GetWorkingRootPath() {
    return std::filesystem::current_path() / kStudentSurname;
}

std::array<std::string, kTreeBranchCount> GetFirstLevelNames() {
    std::array<std::string, kTreeBranchCount> names{};
    for (int i = 0; i < kTreeBranchCount; ++i) {
        names[static_cast<std::size_t>(i)] = std::string(kStudentName) + std::to_string(i + 1);
    }
    return names;
}

std::vector<std::filesystem::path> GetLeafDirectoryPaths() {
    const auto root = GetWorkingRootPath();
    const auto firstLevelNames = GetFirstLevelNames();

    std::vector<std::filesystem::path> result;
    result.reserve(kLeafCount);

    for (const auto& parentName : firstLevelNames) {
        for (const auto& suffixName : firstLevelNames) {
            const std::string leafName = parentName + suffixName;
            result.push_back(root / parentName / leafName);
        }
    }

    return result;
}

std::vector<std::filesystem::path> GetLeafTextFilePaths() {
    const auto leafDirs = GetLeafDirectoryPaths();

    std::vector<std::filesystem::path> files;
    files.reserve(leafDirs.size());

    for (const auto& leafDir : leafDirs) {
        const std::string leafFolderName = leafDir.filename().string();
        files.push_back(leafDir / (leafFolderName + ".txt"));
    }

    return files;
}

std::string FormatFloatForFileName(float value) {
    const double asDouble = static_cast<double>(value);
    const double nearestInteger = std::round(asDouble);
    if (std::fabs(asDouble - nearestInteger) < 1e-6) {
        return std::to_string(static_cast<long long>(nearestInteger));
    }

    std::ostringstream stream;
    stream << std::fixed << std::setprecision(3) << asDouble;
    std::string text = stream.str();

    while (!text.empty() && text.back() == '0') {
        text.pop_back();
    }
    if (!text.empty() && text.back() == '.') {
        text.pop_back();
    }

    return text;
}

int EnsureLeafNodeTextFilesExist() {
    const auto filePaths = GetLeafTextFilePaths();
    for (const auto& filePath : filePaths) {
        std::ofstream file(filePath, std::ios::app);
        if (!file.is_open()) {
            return kStatusIoFailed;
        }
    }

    return kStatusSuccess;
}

int CreateTreeWithSystem() {
    const auto root = GetWorkingRootPath();
    const auto firstLevelNames = GetFirstLevelNames();

    {
        const std::string cmd = "cmd /C if not exist " + QuoteForCmd(root) + " mkdir " + QuoteForCmd(root);
        const int status = RunSystemCommand(cmd);
        if (status != kStatusSuccess) {
            return status;
        }
    }

    for (const auto& parentName : firstLevelNames) {
        const auto firstLevel = root / parentName;
        const std::string firstCmd = "cmd /C if not exist " + QuoteForCmd(firstLevel) + " mkdir " + QuoteForCmd(firstLevel);
        const int firstStatus = RunSystemCommand(firstCmd);
        if (firstStatus != kStatusSuccess) {
            return firstStatus;
        }

        for (const auto& suffixName : firstLevelNames) {
            const auto secondLevel = firstLevel / (parentName + suffixName);
            const std::string secondCmd = "cmd /C if not exist " + QuoteForCmd(secondLevel) + " mkdir " + QuoteForCmd(secondLevel);
            const int secondStatus = RunSystemCommand(secondCmd);
            if (secondStatus != kStatusSuccess) {
                return secondStatus;
            }
        }
    }

    return kStatusSuccess;
}

}  // namespace

extern "C" MY_DLL_API float __cdecl Dll_GetCurrentTimeSeconds(void) {
    const auto now = std::chrono::system_clock::now();
    const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    const float seconds = static_cast<float>(nowMs) / 1000.0f;
    return std::floor(seconds * 10.0f) / 10.0f;
}

extern "C" MY_DLL_API int __cdecl Dll_SetUserTimeRestriction(
    const char* userName,
    const char* weekDays,
    const char* timeInterval) {
    if (IsNullOrEmpty(userName) || IsNullOrEmpty(weekDays) || IsNullOrEmpty(timeInterval)) {
        return kStatusInvalidInput;
    }

    std::ostringstream command;
    command << "net user \"" << userName << "\" /times:" << weekDays << "," << timeInterval;

    return RunSystemCommand(command.str());
}

extern "C" MY_DLL_API int __cdecl Dll_CreateWorkingFolderTree(void) {
    const int createStatus = CreateTreeWithSystem();
    if (createStatus != kStatusSuccess) {
        return createStatus;
    }

    return EnsureLeafNodeTextFilesExist();
}

extern "C" MY_DLL_API int __cdecl Dll_ComputeAndDistributePoints(
    float F,
    float x0,
    float xn,
    float dx) {
    if (dx <= 0.0f || xn < x0) {
        return kStatusInvalidInput;
    }

    const auto filePaths = GetLeafTextFilePaths();

    std::vector<std::ofstream> streams;
    streams.reserve(filePaths.size());
    for (const auto& path : filePaths) {
        streams.emplace_back(path, std::ios::app);
        if (!streams.back().is_open()) {
            return kStatusIoFailed;
        }
        streams.back() << std::fixed << std::setprecision(3);
    }

    std::size_t pointIndex = 0;
    const double xStart = static_cast<double>(x0);
    const double xEnd = static_cast<double>(xn);
    const double xStep = static_cast<double>(dx);
    const double fValue = static_cast<double>(F);
    const double epsilon = std::max(1e-12, std::fabs(xStep) * 1e-9);

    for (double x = xStart; x <= xEnd + epsilon; x += xStep) {
        const double radicand = (x * x * x) + (3.0 * x * x) - fValue;
        if (radicand < 0.0) {
            continue;
        }

        const double y = std::sqrt(radicand);
        std::ofstream& target = streams[pointIndex % streams.size()];
        target << x << " " << y << "\n";
        if (!target.good()) {
            return kStatusIoFailed;
        }

        ++pointIndex;
    }

    return kStatusSuccess;
}

extern "C" MY_DLL_API int __cdecl Dll_MergeSortAndFinalize(float F) {
    const auto filePaths = GetLeafTextFilePaths();

    std::vector<Point> points;
    for (const auto& path : filePaths) {
        std::ifstream input(path);
        if (!input.is_open()) {
            return kStatusIoFailed;
        }

        Point point{};
        while (input >> point.x >> point.y) {
            points.push_back(point);
        }

        if (!input.eof() && input.fail()) {
            return kStatusIoFailed;
        }
    }

    std::sort(points.begin(), points.end(), [](const Point& left, const Point& right) {
        if (left.x == right.x) {
            return left.y < right.y;
        }
        return left.x < right.x;
    });

    const std::filesystem::path outputPath = std::filesystem::current_path() /
        (std::string("F_") + FormatFloatForFileName(F) + ".txt");

    std::ofstream output(outputPath, std::ios::trunc);
    if (!output.is_open()) {
        return kStatusIoFailed;
    }

    output << std::fixed << std::setprecision(3);
    for (const auto& point : points) {
        output << point.x << " " << point.y << "\n";
        if (!output.good()) {
            return kStatusIoFailed;
        }
    }

    for (const auto& path : filePaths) {
        std::error_code ec;
        if (std::filesystem::exists(path, ec)) {
            std::filesystem::remove(path, ec);
            if (ec) {
                return kStatusIoFailed;
            }
        } else if (ec) {
            return kStatusIoFailed;
        }
    }

    return kStatusSuccess;
}

extern "C" MY_DLL_API int __cdecl Dll_DeleteWorkingFolderTree(void) {
    const auto root = GetWorkingRootPath();
    const std::string command = "cmd /C if exist " + QuoteForCmd(root) + " rmdir /S /Q " + QuoteForCmd(root);
    return RunSystemCommand(command);
}
