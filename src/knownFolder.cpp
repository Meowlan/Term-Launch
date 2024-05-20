#include "knownFolder.h"
#include <wtypes.h>
#include <iostream>
#include <algorithm>
#include <ShlObj_core.h>

std::unordered_map <std::string, knownFolder> knownFolder::knownFolderMap = {};

#define REG_MAX 255

void knownFolder::enumKnownFolders() {
	const std::string baseKey = R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\FolderDescriptions\)";

	HKEY hkey;
	RegOpenKeyA(HKEY_LOCAL_MACHINE, baseKey.c_str(), &hkey);

	DWORD dwIndex = 0;
	char idData[REG_MAX];
	while (RegEnumKeyA(hkey, dwIndex, idData, REG_MAX) == ERROR_SUCCESS) {
		DWORD idSize = strlen(idData);
		
		HKEY tempkey;
		RegOpenKeyA(hkey, idData, &tempkey);

		DWORD nameSize = REG_MAX;
		CHAR nameData[REG_MAX] = { 0 };

		RegQueryValueExA(tempkey, "Name", NULL, NULL, (LPBYTE)nameData, &nameSize);

		DWORD parentSize = REG_MAX;
		CHAR parentData[REG_MAX] = { 0 };

		bool hasParent = (RegQueryValueExA(tempkey, "ParentFolder", NULL, NULL, (LPBYTE)parentData, &parentSize) == ERROR_SUCCESS);

		RegCloseKey(tempkey);

		parentSize--;

		knownFolder folder(
			std::string(idData, idSize),
			std::string(nameData, nameSize),
			std::string(parentData, hasParent ? parentSize : 0)
		);

		memset(idData, 0, REG_MAX);
		dwIndex++;
	}

	RegCloseKey(hkey);

	// Assign parents

	for (auto& [id, folder] : knownFolderMap)
	{
		if (!folder.parentId.empty())
		{
			if (!knownFolderMap.contains(folder.parentId))
			{
				continue;
			}

			folder.parent = &knownFolderMap[folder.parentId];
		}
	}
}

void knownFolder::printKnownFolders()
{
	for (const auto& [id, folder] : knownFolderMap)
	{
		std::vector<std::string> path = { "::" + folder.id};
		knownFolder curFolder = folder;

		std::cout << folder.name << " | ";

		while (curFolder.parent != nullptr)
		{
			curFolder = *curFolder.parent;
			path.insert(path.begin(), "::" + curFolder.id);
		}

		for (auto& p : path) {
			std::cout << p << "/";
		}

		std::cout << std::endl;
	}
}
