/*The Code is made by AI
Fucccccccccccccccccccccccccck yoooooooooou ai*/
#include <windows.h>
#include <string>
#include <vector>

void PlayRickroll();

const std::vector<std::wstring> deps = {
	L"SDL.dll", L"SDL2.dll", L"SDL3.dll", L"sqlite3.dll", L"raylib.dll", L"libvpx.dll", L"libpng16.dll", L"miniaudio.dll", L"libcairo-2.dll", L"libjpeg-8.dll", L"cjson.dll", L"libtiff.dll", L"zlib1.dll", L"libogg.dll", L"libvorbis.dll", L"libopus.dll", L"bass.dll", L"freetype.dll",
	L"libmysql.dll", L"alleg42.dll", L"fmod.dll", L"libzplay.dll", L"lua54.dll", L"libcurl.dll", L"libsndfile.dll", L"libzip.dll", L"tbb.dll", L"libzmq.dll", L"libxml2.dll", L".dll", L".dll", L".dll", L".dll", L".dll", L".dll", L".dll", L".dll", L".dll", L".dll", L".dll", L".dll",
	L".dll", L".dll", L".dll", L".dll", L".dll", L"avformat-60.dll", L"libFLAC++.dll"
};

bool FileExists(const std::wstring& name) {
	DWORD attr = GetFileAttributesW(name.c_str());
	return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

bool CheckAllDeps() {
	for (const auto& dll : deps) {
		if (!FileExists(dll)) {
			std::wstring msg = L"Missing " + dll;
			/*MessageBoxW(NULL, msg.c_str(), L"依赖检测", MB_OK | MB_ICONERROR);	/*Stupid fucking AI, Shit your head*/
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