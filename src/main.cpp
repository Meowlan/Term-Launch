#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <iostream>
#include <shlobj.h>
#include <filesystem>
#include <functional>
#include <unordered_set>

namespace fs = std::filesystem;

const fs::path defaultPath = std::getenv("USERPROFILE");

std::unordered_map<std::wstring, fs::path> paths = {};

fs::path getAddress() {
	HWND v_hwnd = GetForegroundWindow();
	if (!v_hwnd) return defaultPath;

	char className[256];
	if (GetClassNameA(v_hwnd, className, sizeof(className)) == 0) return defaultPath;

	if (strcmp(className, "WorkerW") == 0) return paths[L"Desktop"];
	if (strcmp(className, "CabinetWClass") != 0) return defaultPath;

	v_hwnd = FindWindowEx(v_hwnd, NULL, L"WorkerW", NULL);
	if (!v_hwnd) return defaultPath;

	v_hwnd = FindWindowEx(v_hwnd, NULL, L"ReBarWindow32", NULL);
	if (!v_hwnd) return defaultPath;

	v_hwnd = FindWindowEx(v_hwnd, NULL, L"Address Band Root", NULL);
	if (!v_hwnd) return defaultPath;

	v_hwnd = FindWindowEx(v_hwnd, NULL, L"msctls_progress32", NULL);
	if (!v_hwnd) return defaultPath;

	v_hwnd = FindWindowEx(v_hwnd, NULL, L"Breadcrumb Parent", NULL);
	if (!v_hwnd) return defaultPath;

	v_hwnd = FindWindowEx(v_hwnd, NULL, L"ToolbarWindow32", NULL);
	if (!v_hwnd) return defaultPath;

	wchar_t address[MAX_PATH + sizeof("Address:")] = { 0 };
	if (!GetWindowTextW(v_hwnd, address, MAX_PATH)) return defaultPath;

	std::wstring path (address + sizeof("Address:"));

	if (paths.find(path) != paths.end()) return paths[path];

	return path;
}

void startTerminal()
{
	fs::path path = getAddress();
	printf("Opening terminal in \"%ls\"\n", path.c_str());

	try {
		std::filesystem::current_path(path);
	}
	catch (std::exception& e) {
		path = defaultPath;
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Create the process
	if (!CreateProcess(L"C:\\Windows\\System32\\cmd.exe", NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, path.c_str(), &si, &pi)) {
		std::cerr << "Failed to create process: " << GetLastError() << std::endl;
		return;
	}
}

class SHORTCUT {
public:
	std::unordered_set<DWORD> keySet;
	std::function<void()> func;
};

std::vector<SHORTCUT> shortcuts = {
	{ { VK_LWIN, 'T'}, startTerminal},
	{ { VK_RWIN, 'T'}, startTerminal}
};

HHOOK keyboardHook{ NULL };

std::unordered_set<DWORD> keysPressed;

LRESULT CALLBACK LowLevelKeyBoardProc(const int nCode, const WPARAM wParam, const LPARAM lParam) {
	if (nCode != HC_ACTION) return NULL;

	KBDLLHOOKSTRUCT* kbStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

	if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
	{
		keysPressed.insert(kbStruct->vkCode);

		for (const auto& [keySet, func] : shortcuts) {
			if (keySet != keysPressed) continue;

			func();
			return 1;
		}
	}
	else {
		keysPressed.erase(kbStruct->vkCode);
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
};

void initializePaths() {
	wchar_t nameBuf[MAX_PATH] = { 0 };
	wchar_t pathBuf[MAX_PATH] = { 0 };

	::CoInitialize(NULL);

	IKnownFolderManager *pManager;
    HRESULT hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pManager));
    if (SUCCEEDED(hr))
    {
        UINT cCount;
        KNOWNFOLDERID *pkfid;

        hr = pManager->GetFolderIds(&pkfid, &cCount);
        if (SUCCEEDED(hr))
        {
            for (UINT i = 0; i < cCount; i++)
            {
                IKnownFolder *pKnownFolder;
                hr = pManager->GetFolder(pkfid[i], &pKnownFolder);
                if (SUCCEEDED(hr))
                {
					bool gotName = false;
					bool gotPath = false;

                    IShellItem *psi;
                    hr = pKnownFolder->GetShellItem(0, IID_PPV_ARGS(&psi));
                    if (SUCCEEDED(hr))
                    {
						LPWSTR szName = NULL;
						psi->GetDisplayName(SIGDN_NORMALDISPLAY, &szName);
						memcpy(nameBuf, szName, wcslen(szName) * sizeof(WCHAR));

                        psi->Release();
						gotName = true;
					}

					LPWSTR path;
					hr = pKnownFolder->GetPath(KF_FLAG_DEFAULT, &path);
					if (SUCCEEDED(hr))
					{
						memcpy(pathBuf, path, wcslen(path) * sizeof(WCHAR));
						gotPath = true;
					}

					if (gotName) {
						paths[nameBuf] = gotPath ? pathBuf : defaultPath;
					}

                    pKnownFolder->Release();
                }

				memset(nameBuf, 0, sizeof(nameBuf));
				memset(pathBuf, 0, sizeof(pathBuf));
            }

			CoTaskMemFree(pkfid);
        }

        pManager->Release();
    }

	::CoUninitialize();
}

int main(int argc, char* argv[])
{
	std::thread(initializePaths).join();

	keyboardHook = SetWindowsHookEx(
		WH_KEYBOARD_LL,
		LowLevelKeyBoardProc,
		GetModuleHandle(NULL),
		NULL);

	MSG message;
	while (GetMessage(&message, NULL, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	::UnhookWindowsHookEx(keyboardHook);
	return 0;
}