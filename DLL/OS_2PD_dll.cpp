#include "OS_2PD_dll.h"

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
    constexpr int TREEBRANCHCOUNT = 3;
    constexpr int LEAFCOUNT = TREEBRANCHCOUNT * TREEBRANCHCOUNT;
    constexpr const char *NAME = "Adomas";
    constexpr const char *LASTNAME = "Lukosevicius";

    struct Point
    {
        double x;
        double y;
    };

    // išveda DLL klaidas
    void printDllError(string functionName, string details) { cerr << "\n[DLL] " << functionName << " failed: " << details << endl; }

    // Vykdo sistemos komandą ir grąžina jos vykdymo būseną
    int runSystemCommand(const string &command) { return system(command.c_str()) == 1; }

    // Nustato root aplanką, kuriame yra sugeneruotas tree
    path getRootPath() { return current_path() / LASTNAME; }

    // Sudaro pirmo lygio aplankų pavadinimus root kataloge
    array<string, TREEBRANCHCOUNT> getL1FolderNames()
    {
        array<string, TREEBRANCHCOUNT> names{};

        for (int i = 0; i < TREEBRANCHCOUNT; i++)
        {
            names[i] = string(NAME) + to_string(i + 1);
        }
        return names;
    }

    // Grąžina visų teksto failų (leaf nodes), naudojamų tarpiniam saugojimui, sąrašą
    vector<path> getL3TextFilePaths()
    {
        const auto root = getRootPath();
        const auto L1FolderNames = getL1FolderNames();

        vector<path> L3Dirs;
        L3Dirs.reserve(LEAFCOUNT);

        for (int i = 0; i < TREEBRANCHCOUNT; i++)
        {
            const string parentName = string(NAME) + to_string(i + 1);

            for (int j = 0; j < TREEBRANCHCOUNT; j++)
            {
                const string leafName = parentName + string(LASTNAME) + to_string(j + 1);
                L3Dirs.push_back(root / parentName / leafName);
            }
        }

        vector<path> files;
        files.reserve(L3Dirs.size());

        for (const auto &leafDir : L3Dirs)
        {
            const string leafFolderName = leafDir.filename().string();
            files.push_back(leafDir / (leafFolderName + ".txt"));
        }

        return files;
    }

} // namespace

// Grąžina dabartinį laiką sekundėmis su vieno skaitmens po kablelio tikslumu
extern "C" OS_DLL float __cdecl Dll_GetCurrentTimeSeconds(void)
{
    // Paima dabartinį sistemos laika
    const auto now = system_clock::now();
    const auto nowMs = duration_cast<milliseconds>(now.time_since_epoch()).count();

    // Konvertuoja laiką į sekundes pagal grąžinimo formata
    const float seconds = static_cast<float>(nowMs) / 1000.0f;
    return floor(seconds * 10.0f) / 10.0f;
}

// Pritaiko nurodyto Windows vartotojo prisijungimo laiko apribojimus
extern "C" OS_DLL int __cdecl Dll_SetUserTimeRestriction(const char *userName, const char *weekDays, const char *timeInterval)
{
    // Sudaro Windows paskyros laiko apribojimo komandą nurodytam vartotojui
    ostringstream command;
    command << "net user \"" << userName << "\" /times:" << weekDays << "," << timeInterval;

    // Vykdo apribojimo komandą
    const int success = runSystemCommand(command.str());
    if (success != 0)
    {
        printDllError("Dll_SetUserTimeRestriction", "couldnt run command: " + command.str());
        return 1;
    }

    return 0;
}

// Sukuria katalogų medį ir paruošia teksto failus
extern "C" OS_DLL int __cdecl Dll_CreateWorkingFolderTree(void)
{
    // Nustato root path ir pirmo lygio aplanku pavadinimus (VardasX)
    const auto root = getRootPath();
    const auto L1FolderNames = getL1FolderNames();

    // Užtikrina, kad root katalogas egzistuoja
    const string cmd = "cmd /C if not exist \"" + root.string() + "\" mkdir \"" + root.string() + "\"";
    const int success = runSystemCommand(cmd);
    if (success != 0)
    {
            printDllError("Dll_CreateWorkingFolderTree", "couldnt run command: " + cmd);
        return 1;
    }

    for (int i = 0; i < TREEBRANCHCOUNT; i++)
    {
        // Sukuria kiekvieną pirmo lygio katalogą (VardasX)
        const string parentName = string(NAME) + to_string(i + 1);
        const auto firstLevel = root / parentName;
        const string firstCmd = "cmd /C if not exist \"" + firstLevel.string() + "\" mkdir \"" + firstLevel.string() + "\"";
        const int success = runSystemCommand(firstCmd);
        if (success != 0)
        {
            printDllError("Dll_CreateWorkingFolderTree", "couldnt run command: " + firstCmd);
            return 1;
        }

        for (int j = 0; j < TREEBRANCHCOUNT; j++)
        {
            // Sukuria kiekvieną antro lygio katalogą po dabartine šaka (VardasXPavardeX)
            const auto secondLevel = firstLevel / (parentName + string(LASTNAME) + to_string(j + 1));
            const string secondCmd = "cmd /C if not exist \"" + secondLevel.string() + "\" mkdir \"" + secondLevel.string() + "\"";
            const int success = runSystemCommand(secondCmd);
            if (success != 0)
            {
                printDllError("Dll_CreateWorkingFolderTree", "couldnt run command: " + secondCmd);
                return 1;
            }
        }
    }

    // Sukuria teksto failus, kurie bus naudojami Tšrinhauseno kilpos skaiciavimui
    const auto filePaths = getL3TextFilePaths();
    for (const auto &filePath : filePaths)
    {
        ofstream file(filePath, ios::app);
        if (!file.is_open())
        {
            printDllError("Dll_CreateWorkingFolderTree", "cannot open file: " + filePath.string());
            return 1;
        }
    }

    return 0;
}

// Apskaičiuoja tinkamus (x, y) taškus ir paskirsto juos per teksto failus
extern "C" OS_DLL int __cdecl Dll_ComputeAndDistributePoints(float F, float x0, float xn, float dx)
{

    // Nustato visus tarpinių rezultatų failus
    const auto filePaths = getL3TextFilePaths();

    // Atidaro write streams visiems failams
    vector<ofstream> streams;
    streams.reserve(filePaths.size());
    for (const auto &path : filePaths)
    {
        streams.emplace_back(path, ios::app);
        if (!streams.back().is_open())
        {
            printDllError("Dll_ComputeAndDistributePoints", "cannot open stream: " + path.string());
            return 1;
        }
        streams.back() << fixed << setprecision(3);
    }

    // Paruošia skaitines reikšmes, naudojamas taškų generavimui
    size_t pointIndex = 0;
    const double xStart = x0;
    const double xEnd = xn;
    const double xStep = dx;
    const double fValue = F;
    const double epsilon = max(1e-12, fabs(xStep) * 1e-9);

    // Sugeneruoja tinkamus taškus ir įrašo juos į failus
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
            printDllError("Dll_ComputeAndDistributePoints", "write failed while distributing points");
            return 1;
        }

        ++pointIndex;
    }

    return 0;
}

// Surenka visus taškus, juos surūšiuoja, įrašo galutinį rezultatą ir pašalina tarpinius failus
extern "C" OS_DLL int __cdecl Dll_MergeSortAndFinalize(float F)
{
    // Nustato visus tarpinius failus surinkimui ir vėlesniam salinimui
    const auto filePaths = getL3TextFilePaths();

    // Perskaito visus sugeneruotus taškus iš tarpinių failų
    vector<Point> points;
    for (const auto &path : filePaths)
    {
        ifstream input(path);
        if (!input.is_open())
        {
            printDllError("Dll_MergeSortAndFinalize", "cannot open input file: " + path.string());
            return 1;
        }

        Point point{};
        while (input >> point.x >> point.y)
        {
            points.push_back(point);
        }

        if (!input.eof() && input.fail())
        {
            printDllError("Dll_MergeSortAndFinalize", "parse failed for file: " + path.string());
            return 1;
        }
    }

    // Surūšiuoja taškus: pirma pagal x, tada pagal y
    sort(points.begin(), points.end(), [](const Point &left, const Point &right)
         {
        if (left.x == right.x) {
            return left.y < right.y;
        }
        return left.x < right.x; });

    // Suformuoja failo pavadinima pagal F reikšmę
    const double asDouble = F;
    const double nearestInteger = round(asDouble);
    string formattedF;
    if (fabs(asDouble - nearestInteger) < 1e-6)
    {
        formattedF = to_string(static_cast<long long>(nearestInteger));
    }
    else
    {
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

        formattedF = text;
    }

    // Atidaro galutinį rezultatų faila
    const path outputPath = current_path() /
                            (string("F_") + formattedF + ".txt");

    ofstream output(outputPath, ios::trunc);
    if (!output.is_open())
    {
        printDllError("Dll_MergeSortAndFinalize", "cannot open output file: " + outputPath.string());
        return 1;
    }

    // Įrašo surūšiuotų taškų sąrašą į galutinį rezultatų failą
    output << fixed << setprecision(3);
    for (const auto &point : points)
    {
        output << point.x << " " << point.y << "\n";
        if (!output.good())
        {
            printDllError("Dll_MergeSortAndFinalize", "write failed for output file: " + outputPath.string());
            return 1;
        }
    }

    // Pašalina tarpinius failus po to, kai sugeneruojamas galutinis rezultatas
    for (const auto &path : filePaths)
    {
        error_code ec;
        if (exists(path, ec))
        {
            remove(path, ec);
            if (ec)
            {
                printDllError("Dll_MergeSortAndFinalize", "failed to delete intermediate file: " + path.string());
                return 1;
            }
        }
        else if (ec)
        {
            printDllError("Dll_MergeSortAndFinalize", "filesystem check failed for: " + path.string());
            return 1;
        }
    }

    return 0;
}

// Ištrina visą sugeneruotą katalogų medį
extern "C" OS_DLL int __cdecl Dll_DeleteWorkingFolderTree(void)
{
    // Sudaro ir įvykdo komandą, kuri pašalina sugeneruotą medį
    const auto root = getRootPath();
    const string command = "cmd /C if exist \"" + root.string() + "\" rmdir /S /Q \"" + root.string() + "\"";
    const int success = runSystemCommand(command);
    if (success != 0)
    {
        printDllError("Dll_DeleteWorkingFolderTree", "couldnt run command: " + command);
        return 1;
    }

    return 0;
}
