#pragma once

// lean and min windows

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <Psapi.h>
#include <iostream>
#include <stdio.h>
#include <conio.h>

#include <shlobj.h>
#include <atlcomcli.h>  // for COM smart pointers
#include <atlbase.h>    // for COM smart pointers
#include <vector>
#include <system_error>
#include <memory>
#include <comdef.h>
#include <format>
#include <filesystem>

#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <iterator>