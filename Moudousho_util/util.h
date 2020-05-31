#pragma once

#include <exception>
#include <fstream>
#include <wrl.h>
#include <vector>
#include <string>

namespace Util {
	inline void Log(const char* format, ...) {
#ifdef _DEBUG
		va_list valist;
		va_start(valist, format);
		vprintf(format, valist);
		va_end(valist);
#endif
	}

	inline void checkResult(HRESULT hr, std::string funcName) {
		if (FAILED(hr)) {
			std::string hrMsg = "HRESULT: " + std::to_string(hr) + "\n";
			Log(hrMsg.c_str());
			Log(funcName.c_str());
			throw std::runtime_error("Failed" + funcName + ".");
		}
	}
}