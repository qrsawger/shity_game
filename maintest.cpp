/*The Code is made by AI
Fucccccccccccccccccccccccccck yoooooooooou ai*/
#include <windows.h>
#include <string>
#include <vector>

void PlayRickroll();

const std::vector<std::wstring> deps = {
    L"SDL3.dll"
};

bool FileExists(const std::wstring& name) {
    DWORD attr = GetFileAttributesW(name.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

bool CheckAllDeps() {
    for (const auto& dll : deps) {
        if (!FileExists(dll)) {
            std::wstring msg = L"Missing " + dll;
            MessageBoxW(NULL, msg.c_str(), L"Error", MB_OK | MB_ICONERROR);    /*Stupid fucking AI, Shit your head*/
            return false;
        }
    }
    return true;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    if (!CheckAllDeps()) return 1;
    PlayRickroll();
    return 0;
}