#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::wstring> split(const std::wstring &name, wchar_t delimiter) {
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