#pragma once

#include <shlobj.h>
#include "../common.h"

class terminal
{
private:
	fs::path defaultWorkingDir;
	std::unordered_map<std::wstring, fs::path> knownPathMap;

	fs::path getAddress();
	void tryStoreKnownPath(IKnownFolder* pKnownFolder);
	void saveKnownPath(IKnownFolder *pKnownFolder);
	void iterateKnownPaths(KNOWNFOLDERID* pkfid, UINT cCount, IKnownFolderManager* pManager);
	void getKnownPaths();

public:
	void startTerminal();

public:
	terminal() {
		knownPathMap = {};
		defaultWorkingDir = std::getenv("USERPROFILE");

		std::thread(getKnownPaths).join();
		// getKnownPaths();
	};
};

