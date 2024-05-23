#pragma once
#include "common.h"

#include <unordered_set>
#include <thread>
#include "shortcutImpl/terminal.h"

class SHORTCUT {
public:
	std::unordered_set<DWORD> keySet;
	std::function<void()> shortcutFunction;
};

std::vector<SHORTCUT> shortcuts = {
	{ { VK_LWIN, 'T'}, "terminal" },
	{ { VK_RWIN, 'T'}, "terminal"}
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

int main(int argc, char* argv[])
{
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