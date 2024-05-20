#pragma once
#include <string>
#include <unordered_map>
#include <iostream>

class knownFolder {
public:
	static std::unordered_map <std::string, knownFolder> knownFolderMap;

	std::string id;
	std::string name;
	std::string parentId;

	knownFolder* parent;
	
public:
	knownFolder(std::string id, std::string name, std::string parentId = "") : id(id), name(name), parentId(parentId), parent(nullptr) {
		knownFolderMap[id] = std::move(*this);
	}

	knownFolder() = default;
	knownFolder(const knownFolder& folder) {
		this->id = folder.id;
		this->name = folder.name;
		this->parentId = folder.parentId;
		this->parent = folder.parent;
	}

	static void enumKnownFolders();
	static void printKnownFolders();
};