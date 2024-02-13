#pragma once

#include "Assert/Assert.h"
#include "Color/Color.h"
#include "Config/Config.h"
#include "Storage/Storage.h"
#include "Hash/Hash.h"
#include "HookManager/HookManager.h"
#include "InterfaceManager/InterfaceManager.h"
#include "Math/Math.h"
#include "Memory/Memory.h"
#include "SignatureManager/SignatureManager.h"
#include "Singleton/Singleton.h"
#include "Vector/Vector.h"

#include <intrin.h>
#include <random>
#include <chrono>
#include <filesystem>
#include <deque>
#include <regex>

namespace Utils
{
    static std::wstring ConvertUtf8ToWide(const std::string& ansi)
    {
        const int size = MultiByteToWideChar(CP_UTF8, 0, ansi.c_str(), -1, nullptr, 0);
		std::wstring result(size, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, ansi.c_str(), -1, result.data(), size);
		return result;
    }

    static std::string ConvertWideToUTF8(const std::wstring& unicode)
    {
        const int size = WideCharToMultiByte(CP_UTF8, 0, unicode.c_str(), -1, nullptr, 0, nullptr, nullptr);
		std::string result(size, '\0');
		WideCharToMultiByte(CP_UTF8, 0, unicode.c_str(), -1, result.data(), size, nullptr, nullptr);
		return result;
    }

    static int RandInt(int min, int max)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(min, max);
        return distr(gen);
    }
}