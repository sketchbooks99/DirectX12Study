#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <exception>
#include <fstream>
#include <wrl.h>

#if _MSC_VER > 1922
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif
#include <experimental/filesystem>

namespace Util {
    inline std::vector<std::wstring> split(const std::wstring &name, wchar_t delimiter) {
        std::vector<std::wstring> elements;
        std::wstringstream wss(name);
        std::wstring item;
        while(getline(wss, item, delimiter))
        {
            if(!item.empty()) {
                elements.push_back(item);
            }
        }
        return elements;
    }

    inline void Log(const char* format, ...) {
#ifdef _DEBUG
		va_list valist;
		va_start(valist, format);
		vprintf(format, valist);
		va_end(valist);
#endif
	}

	inline void CheckResult(HRESULT hr, std::string funcName) {
		if (FAILED(hr)) {
			std::string hrMsg = "HRESULT: " + std::to_string(hr) + "\n";
			Log(hrMsg.c_str());
			Log(funcName.c_str());
			throw std::runtime_error("Failed" + funcName + ".");
		}
	}
}