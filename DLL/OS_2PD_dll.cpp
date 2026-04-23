#include "OS_2PD_dll.h"

// konstantos
constexpr int TREE_BRANCH_COUNT = 3;
constexpr int LEAF_COUNT = TREE_BRANCH_COUNT * 3;
constexpr const char *VARDAS = "Adomas";
constexpr const char *PAVARDE = "Lukosevicius";

struct Point
{
    double x;
    double y;
};

// isveda DLL klaidas
void printDllError(string functionName, string details) { cerr << "\n[DLL] " << functionName << " failed: " << details << endl; }

// Vykdo sistemos komanda ir grazina jos vykdymo busena
bool runSystemCommand(const string &command) { return system(command.c_str()) == 0; }

// Nustato root aplanka, kuriame yra sugeneruotas tree
path getRootPath() { return current_path() / PAVARDE; }

// Sudaro pirmo lygio aplanku pavadinimus root kataloge
array<string, TREE_BRANCH_COUNT> getL1FolderNames()
{
    array<string, TREE_BRANCH_COUNT> names{};

    for (int i = 0; i < TREE_BRANCH_COUNT; i++)
    {
        names[i] = string(VARDAS) + to_string(i + 1);
    }
    return names;
}

// Grazina visu teksto failu (leaf nodes), naudojamu tarpiniam saugojimui, sarasa
vector<path> getL3TextFilePaths()
{
    const auto root = getRootPath();
    const auto L1FolderNames = getL1FolderNames();

    vector<path> L3Dirs;
    L3Dirs.reserve(LEAF_COUNT);

    for (int i = 0; i < TREE_BRANCH_COUNT; i++)
    {
        const string parentName = string(VARDAS) + to_string(i + 1);

        for (int j = 0; j < TREE_BRANCH_COUNT; j++)
        {
            const string leafName = parentName + string(PAVARDE) + to_string(j + 1);
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

// Grazina laika sekundemis nuo pirmo DLL kvietimo
extern "C" OS_DLL float __cdecl Dll_GetCurrentTimeSeconds()
{
    using Clock = steady_clock;

    // Fiksuoja atskaitos taska pirma karta iskvietus funkcija procese
    static const auto baseTime = Clock::now();

    // Apskaiciuoja praejusi laika nuo atskaitos tasko
    const auto now = Clock::now();
    const auto nowMs = duration_cast<milliseconds>(now - baseTime).count();

    // Konvertuoja laika i sekundes pagal grazinimo formata 'xx.x'
    const float seconds = static_cast<float>(nowMs) / 1000.0f;
    return floor(seconds * 10.0f) / 10.0f;
}

// Pritaiko nurodyto Windows vartotojo prisijungimo laiko apribojimus
extern "C" OS_DLL bool __cdecl Dll_SetUserTimeRestriction(const char *userName, const char *weekDays, const char *timeInterval)
{
    // Sudaro Windows paskyros laiko apribojimo komanda nurodytam vartotojui
    ostringstream command;
    command << "net user \"" << userName << "\" /times:" << weekDays << "," << timeInterval;

    // Vykdo apribojimo komanda
    const bool success = runSystemCommand(command.str());
    if (!success)
    {
        printDllError("Dll_SetUserTimeRestriction", "couldnt run command: " + command.str());
        return false;
    }

    return true;
}

// Sukuria katalogu medi ir paruosia teksto failus
extern "C" OS_DLL bool __cdecl Dll_CreateWorkingFolderTree()
{
    // Nustato root path ir pirmo lygio aplanku pavadinimus (VardasX)
    const auto root = getRootPath();
    const auto L1FolderNames = getL1FolderNames();

    // Uztikrina, kad root katalogas egzistuoja
    const string cmd = "cmd /C if not exist \"" + root.string() + "\" mkdir \"" + root.string() + "\"";
    const bool success = runSystemCommand(cmd);
    if (!success)
    {
        printDllError("Dll_CreateWorkingFolderTree", "couldnt run command: " + cmd);
        return false;
    }

    for (int i = 0; i < TREE_BRANCH_COUNT; i++)
    {
        // Sukuria kiekviena pirmo lygio kataloga (VardasX)
        const string parentName = string(VARDAS) + to_string(i + 1);
        const auto firstLevel = root / parentName;
        const string firstCmd = "cmd /C if not exist \"" + firstLevel.string() + "\" mkdir \"" + firstLevel.string() + "\"";
        const bool success = runSystemCommand(firstCmd);
        if (!success)
        {
            printDllError("Dll_CreateWorkingFolderTree", "couldnt run command: " + firstCmd);
            return false;
        }

        for (int j = 0; j < TREE_BRANCH_COUNT; j++)
        {
            // Sukuria kiekviena antro lygio kataloga po dabartine saka (VardasXPavardeX)
            const auto secondLevel = firstLevel / (parentName + string(PAVARDE) + to_string(j + 1));
            const string secondCmd = "cmd /C if not exist \"" + secondLevel.string() + "\" mkdir \"" + secondLevel.string() + "\"";
            const bool success = runSystemCommand(secondCmd);
            if (!success)
            {
                printDllError("Dll_CreateWorkingFolderTree", "couldnt run command: " + secondCmd);
                return false;
            }
        }
    }

    // Sukuria teksto failus, kurie bus naudojami Tschirnhauseno kilpos skaiciavimui
    const auto filePaths = getL3TextFilePaths();
    for (const auto &filePath : filePaths)
    {
        ofstream file(filePath, ios::app);
        if (!file.is_open())
        {
            printDllError("Dll_CreateWorkingFolderTree", "cannot open file: " + filePath.string());
            return false;
        }
    }

    return true;
}

// Apskaiciuoja tinkamus (x, y) taskus ir paskirsto juos per teksto failus
extern "C" OS_DLL bool __cdecl Dll_ComputeAndDistributePoints(float F, float x0, float xn, float dx)
{

    // Nustato visus tarpiniu rezultatu failus
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
            return false;
        }
        streams.back() << fixed << setprecision(3);
    }

    // Paruosia skaitines reiksmes, naudojamas tasku generavimui
    size_t pointIndex = 0;
    const double xStart = x0;
    const double xEnd = xn;
    const double xStep = dx;
    const double fValue = F;
    const double epsilon = max(1e-12, fabs(xStep) * 1e-9);

    // Sugeneruoja tinkamus taskus ir iraso juos i failus
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
            return false;
        }

        ++pointIndex;
    }

    return true;
}

// Surenka visus taskus, juos surusiuoja, iraso galutini rezultata ir pasalina tarpinius failus
extern "C" OS_DLL bool __cdecl Dll_MergeSortAndFinalize(float F)
{
    // Nustato visus tarpinius failus surinkimui ir velesniam salinimui
    const auto filePaths = getL3TextFilePaths();

    // Perskaito visus sugeneruotus taskus is tarpiniu failu
    vector<Point> points;
    for (const auto &path : filePaths)
    {
        ifstream input(path);
        if (!input.is_open())
        {
            printDllError("Dll_MergeSortAndFinalize", "cannot open input file: " + path.string());
            return false;
        }

        Point point{};
        while (input >> point.x >> point.y)
        {
            points.push_back(point);
        }

        if (!input.eof() && input.fail())
        {
            printDllError("Dll_MergeSortAndFinalize", "parse failed for file: " + path.string());
            return false;
        }
    }

    // Surusiuoja taskus: pirma pagal x, tada pagal y
    sort(points.begin(), points.end(), [](const Point &left, const Point &right)
         {
        if (left.x == right.x) {
            return left.y < right.y;
        }
        return left.x < right.x; });

    // Suformuoja failo pavadinima pagal F reiksme
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

    // Atidaro galutini rezultatu faila
    const path outputPath = current_path() /
                            (string("F_") + formattedF + ".txt");

    ofstream output(outputPath, ios::trunc);
    if (!output.is_open())
    {
        printDllError("Dll_MergeSortAndFinalize", "cannot open output file: " + outputPath.string());
        return false;
    }

    // Iraso surusiuotu tasku sarasa i galutini rezultatu faila
    output << fixed << setprecision(3);
    for (const auto &point : points)
    {
        output << point.x << " " << point.y << "\n";
        if (!output.good())
        {
            printDllError("Dll_MergeSortAndFinalize", "write failed for output file: " + outputPath.string());
            return false;
        }
    }

    // Pasalina tarpinius failus po to, kai sugeneruojamas galutinis rezultatas
    for (const auto &path : filePaths)
    {
        error_code ec;
        if (exists(path, ec))
        {
            remove(path, ec);
            if (ec)
            {
                printDllError("Dll_MergeSortAndFinalize", "failed to delete intermediate file: " + path.string());
                return false;
            }
        }
        else if (ec)
        {
            printDllError("Dll_MergeSortAndFinalize", "filesystem check failed for: " + path.string());
            return false;
        }
    }

    return true;
}

// Istrina visa sugeneruota katalogu medi
extern "C" OS_DLL bool __cdecl Dll_DeleteWorkingFolderTree()
{
    // Sudaro ir ivykdo komanda, kuri pasalina sugeneruota medi
    const auto root = getRootPath();
    const string command = "cmd /C if exist \"" + root.string() + "\" rmdir /S /Q \"" + root.string() + "\"";
    const bool success = runSystemCommand(command);
    if (!success)
    {
        printDllError("Dll_DeleteWorkingFolderTree", "couldnt run command: " + command);
        return false;
    }

    return true;
}
