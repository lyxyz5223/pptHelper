#pragma once
#include <string>
#include <vector>
std::vector<std::string> split(std::string text, std::vector<std::string> delimiter/*separator,分隔符*/, std::string EscapeString = "" /*char EscapeCharacter*/);
std::vector<std::string> split(std::string text, std::string delimiter = " "/*separator,分隔符*/, std::string EscapeString = "" /*char EscapeCharacter*/);
std::string join(std::vector<std::string> textVec, std::string delimiter);
std::string wstr2str_2UTF8(std::wstring text);
std::string wstr2str_2ANSI(std::wstring text);
std::wstring str2wstr_2UTF8(std::string text);
std::wstring str2wstr_2ANSI(std::string text);
std::string UTF8ToANSI(std::string utf8Text);
std::string ANSIToUTF8(std::string ansiText);
