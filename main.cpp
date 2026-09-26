/*The Code is made by AI
Fucccccccccccccccccccccccccck yoooooooooou ai*/
#include <windows.h>
#include <string>
#include <vector>

void PlayRickroll();

const std::vector<std::wstring> deps = {
    L"SDL.dll", L"SDL2.dll", L"SDL3.dll", L"sqlite3.dll", L"raylib.dll", L"libvpx.dll", L"libpng16.dll", L"miniaudio.dll", L"plutovg.dll", L"libjpeg-8.dll",
    L"cjson.dll", L"libspng.dll", L"zlib1.dll", L"libogg.dll", L"libvorbis.dll", L"libopus.dll", L"bass.dll", L"freetype.dll", L"lzokay.dll", L"alleg42.dll",
    L"fmod.dll", L"libsndwave.dll", L"lua54.dll", L"libcurl.dll", L"dr_flac.dll", L"libzip.dll", L"jsoncpp.dll", L"libzmq.dll", L"libxml2.dll", L"portaudio.dll",
    L"vulkan-1.dll", L"liblz4.dll", L"libzstd.dll", L"minizip.dll", L"box2d.dll", L"libdispatch.dll", L"rapidjson.dll", L"mbedtls.dll", L"libopenblas.dll", L"icuuc72.dll",
    L"msmpi.dll", L"PhysX3.dll", L"libssh2.dll", L"PopH264.dll", L"h264bsd.dll", L"SDL_net.dll", L"Chipmunk2D.dll", L"libFLAC++.dll", L"libopenmpt.dll", L"glfw3.dll"
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