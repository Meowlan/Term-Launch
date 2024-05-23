#include "terminal.h"

fs::path terminal::getAddress() {
	HWND v_hwnd = GetForegroundWindow();
	if (!v_hwnd) return defaultWorkingDir;

	char className[256];
	if (GetClassNameA(v_hwnd, className, sizeof(className)) == 0) return defaultWorkingDir;

	if (strcmp(className, "WorkerW") == 0) return knownPathMap[L"Desktop"]; // If the desktop is focused, open terminal there
	if (strcmp(className, "CabinetWClass") != 0) return defaultWorkingDir;

	v_hwnd = FindWindowEx(v_hwnd, NULL, L"WorkerW", NULL);
	if (!v_hwnd) return defaultWorkingDir;

	v_hwnd = FindWindowEx(v_hwnd, NULL, L"ReBarWindow32", NULL);
	if (!v_hwnd) return defaultWorkingDir;

	v_hwnd = FindWindowEx(v_hwnd, NULL, L"Address Band Root", NULL);
	if (!v_hwnd) return defaultWorkingDir;

	v_hwnd = FindWindowEx(v_hwnd, NULL, L"msctls_progress32", NULL);
	if (!v_hwnd) return defaultWorkingDir;

	v_hwnd = FindWindowEx(v_hwnd, NULL, L"Breadcrumb Parent", NULL);
	if (!v_hwnd) return defaultWorkingDir;

	v_hwnd = FindWindowEx(v_hwnd, NULL, L"ToolbarWindow32", NULL);
	if (!v_hwnd) return defaultWorkingDir;

	wchar_t address[MAX_PATH + sizeof("Address:")] = { 0 };
	if (!GetWindowTextW(v_hwnd, address, MAX_PATH)) return defaultWorkingDir;

	std::wstring path(address + sizeof("Address:"));

	if (knownPathMap.find(path) != knownPathMap.end()) return knownPathMap[path];

	return path;
}

void terminal::startTerminal()
{
	fs::path path = getAddress();
	printf("Opening terminal in \"%ls\"\n", path.c_str());

	try {
		std::filesystem::current_path(path);
	}
	catch (std::exception& e) {
		path = defaultWorkingDir;
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

void terminal::tryStoreKnownPath(IKnownFolder* pKnownFolder)
{
	// not sure if this is a good idea
	static wchar_t nameBuf[MAX_PATH] = { 0 };
	static wchar_t pathBuf[MAX_PATH] = { 0 };

	bool gotName = false;
	bool gotPath = false;

	IShellItem* shellItem;
	HRESULT hr = pKnownFolder->GetShellItem(0, IID_PPV_ARGS(&shellItem));
	if (SUCCEEDED(hr))
	{
		LPWSTR szName = NULL;
		shellItem->GetDisplayName(SIGDN_NORMALDISPLAY, &szName);
		memcpy(nameBuf, szName, wcslen(szName) * sizeof(WCHAR));

		shellItem->Release();
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
		knownPathMap[nameBuf] = gotPath ? pathBuf : defaultWorkingDir;
	}
}

void terminal::iterateKnownPaths(KNOWNFOLDERID* pkfid, UINT cCount, IKnownFolderManager* pManager)
{
	for (UINT i = 0; i < cCount; i++)
	{
		IKnownFolder* pKnownFolder;
		HRESULT hr = pManager->GetFolder(pkfid[i], &pKnownFolder);
		if (SUCCEEDED(hr))
		{
			tryStoreKnownPath(pKnownFolder);
			pKnownFolder->Release();
		}
	}
}

void terminal::getKnownPaths() {
	::CoInitialize(NULL);

	IKnownFolderManager* pManager;
	HRESULT hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pManager));
	if (SUCCEEDED(hr))
	{
		UINT cCount;
		KNOWNFOLDERID* pkfid;

		hr = pManager->GetFolderIds(&pkfid, &cCount);
		if (SUCCEEDED(hr))
		{
			iterateKnownPaths(pkfid, cCount, pManager);
			CoTaskMemFree(pkfid);
		}

		pManager->Release();
	}

	::CoUninitialize();
}
