#include "my_dll_api.h"

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

namespace
{

    // konstantos
    constexpr int treeBranchCount = 3;
    constexpr int leafCount = treeBranchCount * treeBranchCount;
    constexpr const char *name = "Adomas";
    constexpr const char *lastname = "Lukosevicius";

    struct Point
    {
        double x;
        double y;
    };

    // helperiai

    int printDllError(string functionName, string details) { cerr << "\n[DLL] " << functionName << " failed: " << details << endl; }

    string quoteForCmd(const path &path) { return "\"" + path.string() + "\""; }

    int runSystemCommand(const string &command) { return system(command.c_str()) == 1; }

    path getWorkingRootPath() { return current_path() / lastname; }

    string *getFirstLevelNames()
    {
        string names[treeBranchCount] = {};
        for (int i = 0; i < treeBranchCount; ++i)
        {
            names[i] = string(name) + to_string(i + 1);
        }
        return names;
    }

    vector<path> getLeafDirPaths()
    {
        const auto root = getWorkingRootPath();
        const auto firstLevelNames = getFirstLevelNames();

        vector<path> result;
        result.reserve(leafCount);

        for (const auto &parentName : firstLevelNames)
        {
            for (const auto &suffixName : firstLevelNames)
            {
                const string leafName = parentName + suffixName;
                result.push_back(root / parentName / leafName);
            }
        }

        return result;
    }

    vector<path> GetLeafTextFilePaths()
    {
        const auto leafDirs = getLeafDirPaths();

        vector<path> files;
        files.reserve(leafDirs.size());

        for (const auto &leafDir : leafDirs)
        {
            const string leafFolderName = leafDir.filename().string();
            files.push_back(leafDir / (leafFolderName + ".txt"));
        }

        return files;
    }

    string FormatFloatForFileName(float value)
    {
        const double asDouble = static_cast<double>(value);
        const double nearestInteger = round(asDouble);
        if (fabs(asDouble - nearestInteger) < 1e-6)
        {
            return to_string(static_cast<long long>(nearestInteger));
        }

        ostringstream stream;
        stream << fixed << setprecision(3) << asDouble;
        string text = stream.str();

        while (!text.empty() && text.back() == '0')
        {
            text.pop_back();
        }
        if (!text.empty() && text.back() == '.')
        {
            text.pop_back();
        }

        return text;
    }

    int EnsureLeafNodeTextFilesExist()
    {
        const auto filePaths = GetLeafTextFilePaths();
        for (const auto &filePath : filePaths)
        {
            ofstream file(filePath, ios::app);
            if (!file.is_open())
            {
                return printDllError("EnsureLeafNodeTextFilesExist", "cannot open file: " + filePath.string());
            }
        }

        return 0;
    }

    int CreateTreeWithSystem()
    {
        const auto root = getWorkingRootPath();
        const auto firstLevelNames = getFirstLevelNames();

        {
            const string cmd = "cmd /C if not exist \"" + root.string() + "\" mkdir \"" + root.string() + "\"";
            const int status = runSystemCommand(cmd);
            if (status != 0)
            {
                return printDllError("CreateTreeWithSystem", "command: " + cmd);
            }
        }

        for (const auto &parentName : firstLevelNames)
        {
            const auto firstLevel = root / parentName;
            const string firstCmd = "cmd /C if not exist \"" + firstLevel.string() + "\" mkdir \"" + firstLevel.string() + "\"";
            const int firstStatus = runSystemCommand(firstCmd);
            if (firstStatus != 0)
            {
                return printDllError("CreateTreeWithSystem", "command: " + firstCmd);
            }

            for (const auto &suffixName : firstLevelNames)
            {
                const auto secondLevel = firstLevel / (parentName + suffixName);
                const string secondCmd = "cmd /C if not exist \"" + secondLevel.string() + "\" mkdir \"" + secondLevel.string() + "\"";
                const int secondStatus = runSystemCommand(secondCmd);
                if (secondStatus != 0)
                {
                    return printDllError("CreateTreeWithSystem", "command: " + secondCmd);
                }
            }
        }

        return 0;
    }

} // namespace

extern "C" MY_DLL_API float __cdecl Dll_GetCurrentTimeSeconds(void)
{
    const auto now = system_clock::now();
    const auto nowMs = duration_cast<milliseconds>(now.time_since_epoch()).count();

    const float seconds = static_cast<float>(nowMs) / 1000.0f;
    return floor(seconds * 10.0f) / 10.0f;
}

extern "C" MY_DLL_API int __cdecl Dll_SetUserTimeRestriction(string *userName, string *weekDays, string *timeInterval)
{
    ostringstream command;
    command << "net user \"" << userName << "\" /times:" << weekDays << "," << timeInterval;

    const int status = runSystemCommand(command.str());
    if (status != 0)
    {
        return printDllError("Dll_SetUserTimeRestriction", "command: " + command.str());
    }

    return 0;
}

extern "C" MY_DLL_API int __cdecl Dll_CreateWorkingFolderTree(void)
{
    const int createStatus = CreateTreeWithSystem();
    if (createStatus != 0)
    {
        return createStatus;
    }

    return EnsureLeafNodeTextFilesExist();
}

extern "C" MY_DLL_API int __cdecl Dll_ComputeAndDistributePoints(
    float F,
    float x0,
    float xn,
    float dx)
{
    if (dx <= 0.0f || xn < x0)
    {
        return printDllError("Dll_ComputeAndDistributePoints", "dx must be > 0 and xn must be >= x0");
    }

    const auto filePaths = GetLeafTextFilePaths();

    vector<ofstream> streams;
    streams.reserve(filePaths.size());
    for (const auto &path : filePaths)
    {
        streams.emplace_back(path, ios::app);
        if (!streams.back().is_open())
        {
            return printDllError("Dll_ComputeAndDistributePoints", "cannot open stream: " + path.string());
        }
        streams.back() << fixed << setprecision(3);
    }

    size_t pointIndex = 0;
    const double xStart = static_cast<double>(x0);
    const double xEnd = static_cast<double>(xn);
    const double xStep = static_cast<double>(dx);
    const double fValue = static_cast<double>(F);
    const double epsilon = max(1e-12, fabs(xStep) * 1e-9);

    for (double x = xStart; x <= xEnd + epsilon; x += xStep)
    {
        const double radicand = (x * x * x) + (3.0 * x * x) - fValue;
        if (radicand < 0.0)
        {
            continue;
        }

        const double y = sqrt(radicand);
        ofstream &target = streams[pointIndex % streams.size()];
        target << x << " " << y << "\n";
        if (!target.good())
        {
            return printDllError("Dll_ComputeAndDistributePoints", "write failed while distributing points");
        }

        ++pointIndex;
    }

    return 0;
}

extern "C" MY_DLL_API int __cdecl Dll_MergeSortAndFinalize(float F)
{
    const auto filePaths = GetLeafTextFilePaths();

    vector<Point> points;
    for (const auto &path : filePaths)
    {
        ifstream input(path);
        if (!input.is_open())
        {
            return printDllError("Dll_MergeSortAndFinalize", "cannot open input file: " + path.string());
        }

        Point point{};
        while (input >> point.x >> point.y)
        {
            points.push_back(point);
        }

        if (!input.eof() && input.fail())
        {
            return printDllError("Dll_MergeSortAndFinalize", "parse failed for file: " + path.string());
        }
    }

    sort(points.begin(), points.end(), [](const Point &left, const Point &right)
         {
        if (left.x == right.x) {
            return left.y < right.y;
        }
        return left.x < right.x; });

    const path outputPath = current_path() /
                            (string("F_") + FormatFloatForFileName(F) + ".txt");

    ofstream output(outputPath, ios::trunc);
    if (!output.is_open())
    {
        return printDllError("Dll_MergeSortAndFinalize", "cannot open output file: " + outputPath.string());
    }

    output << fixed << setprecision(3);
    for (const auto &point : points)
    {
        output << point.x << " " << point.y << "\n";
        if (!output.good())
        {
            return printDllError("Dll_MergeSortAndFinalize", "write failed for output file: " + outputPath.string());
        }
    }

    for (const auto &path : filePaths)
    {
        error_code ec;
        if (exists(path, ec))
        {
            remove(path, ec);
            if (ec)
            {
                return printDllError("Dll_MergeSortAndFinalize", "failed to delete intermediate file: " + path.string());
            }
        }
        else if (ec)
        {
            return printDllError("Dll_MergeSortAndFinalize", "filesystem check failed for: " + path.string());
        }
    }

    return 0;
}

extern "C" MY_DLL_API int __cdecl Dll_DeleteWorkingFolderTree(void)
{
    const auto root = getWorkingRootPath();
    const string command = "cmd /C if exist \"" + root.string() + "\" rmdir /S /Q \"" + root.string() + "\"";
    const int status = runSystemCommand(command);
    if (status != 0)
    {
        return printDllError("Dll_DeleteWorkingFolderTree", "command: " + command);
    }

    return 0;
}
