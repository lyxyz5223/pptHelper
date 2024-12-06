#include "stringProcess.h"
#ifdef _WIN32
#include <Windows.h>
std::string wstr2str_2UTF8(std::wstring text)
{
	CHAR* str;
	int Tsize = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, 0, 0, 0, 0);
	str = new CHAR[Tsize];
	WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, str, Tsize, 0, 0);
	std::string str1 = str;
	delete[]str;
	return str1;
}
std::string wstr2str_2ANSI(std::wstring text)
{
	CHAR* str;
	int Tsize = WideCharToMultiByte(CP_ACP, 0, text.c_str(), -1, 0, 0, 0, 0);
	str = new CHAR[Tsize];
	WideCharToMultiByte(CP_ACP, 0, text.c_str(), -1, str, Tsize, 0, 0);
	std::string str1 = str;
	delete[]str;
	return str1;
}

std::wstring str2wstr_2UTF8(std::string text)
{
	WCHAR* str;
	int Tsize = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, 0, 0);
	str = new WCHAR[Tsize];
	MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, str, Tsize);
	std::wstring str1 = str;
	delete[]str;
	return str1;
}

std::wstring str2wstr_2ANSI(std::string text)
{
	WCHAR* str;
	int Tsize = MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, 0, 0);
	str = new WCHAR[Tsize];
	MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, str, Tsize);
	std::wstring str1 = str;
	delete[]str;
	return str1;

}

std::string UTF8ToANSI(std::string utf8Text)
{
	WCHAR* wstr;//中间量
	CHAR* str;//转换后的
	int Tsize = MultiByteToWideChar(CP_UTF8, 0, utf8Text.c_str(), -1, 0, 0);
	wstr = new WCHAR[Tsize];
	MultiByteToWideChar(CP_UTF8, 0, utf8Text.c_str(), -1, wstr, Tsize);
	Tsize = WideCharToMultiByte(CP_ACP, 0, wstr, -1, 0, 0, 0, 0);
	str = new CHAR[Tsize];
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, Tsize, 0, 0);
	std::string wstr1 = str;
	delete[]str;
	delete[]wstr;
	return wstr1;
}

std::string ANSIToUTF8(std::string ansiText)
{
	WCHAR* wstr;//中间量
	CHAR* str;//转换后的
	int Tsize = MultiByteToWideChar(CP_ACP, 0, ansiText.c_str(), -1, 0, 0);
	wstr = new WCHAR[Tsize];
	MultiByteToWideChar(CP_ACP, 0, ansiText.c_str(), -1, wstr, Tsize);
	Tsize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, 0, 0, 0, 0);
	str = new CHAR[Tsize];
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, Tsize, 0, 0);
	std::string wstr1 = str;
	delete[]str;
	delete[]wstr;
	return wstr1;
}

#endif // _WIN32
std::vector<std::string> split(std::string text, std::vector<std::string> delimiter/*separator,分隔符*/, std::string EscapeString /*char EscapeCharacter*/)
{
	std::vector<std::string> resultStrVec;
	for (std::vector<std::string>::iterator iter = delimiter.begin(); iter != delimiter.end(); iter++)
	{
		if ((iter + 1) != delimiter.end())
			text = join(split(text, *iter, EscapeString), *(iter + 1));
	}
	resultStrVec = split(text, delimiter.back(), EscapeString);
	return resultStrVec;
}
std::vector<std::string> split(std::string text, std::string delimiter/*separator,分隔符*/, std::string EscapeString /*char EscapeCharacter*/)
{
	text += delimiter;
	using namespace std;
	vector<string> resultStrVector;
	typedef long long llong;
	llong DelimiterCount = 0;
	string::size_type st1;
	string textProc = text;
	string textCut = text;
	string textTmpStorage;//临时储存转义的被剪裁的字符串
	while ((st1 = textCut.find(delimiter)) != string::npos)
	{//找到分隔符，接下来要判断分隔符前面是否有转义字符串
		string::size_type st2 = st1 - EscapeString.length();
		intptr_t aaa = st1 - EscapeString.length();
		if ((EscapeString != "") && (aaa < 0 ? false : (EscapeString == textCut.substr(st2, EscapeString.length()))))
		{//分隔符前面有转义字符串，接下来要判断转义字符串前面是否有转义字符串
			st2 = st1 - 2 * EscapeString.length();
			aaa = st1 - 2 * EscapeString.length();
			if (aaa < 0 ? false : (EscapeString == textCut.substr(st2, EscapeString.length())))
			{//转义字符串前面有转义字符串
				//去掉转义字符串，加入列表的字符串长度应该减小EscapeString.length()
				//先剪裁
				st1 -= EscapeString.length();//位置前移
				textCut = textCut.erase(st1 - EscapeString.length(), EscapeString.length());//删除转义字符串
				string textToAdd = (textTmpStorage == "" ? "" : textTmpStorage) + textCut.substr(0, st1);
				if (textToAdd != "")
					resultStrVector.push_back(textToAdd);//加入列表
				textTmpStorage = "";
				textCut = textCut.substr(st1 + delimiter.length());//剪裁字符串，去掉已经加入列表的部分
				//统计分隔符个数
				DelimiterCount += 1;
			}
			else {
				//先剪裁，去掉转义字符串
				//去掉转义字符串，加入列表的字符串长度应该减小EscapeString.length()
				st1 -= EscapeString.length();//位置前移
				textCut = textCut.erase(st1, EscapeString.length());//删除转义字符串
				textTmpStorage += textCut.substr(0, st1) + delimiter;//把转义字符串一并带上
				textCut = textCut.substr(st1 + delimiter.length());//剪裁字符串，去掉已经加入临时存储的字符串部分
				//统计分隔符个数，这里不是分隔符，所以不用DelimiterCount++;
			}
		}
		else {
			string textToAdd = (textTmpStorage == "" ? "" : textTmpStorage) + textCut.substr(0, st1);
			if (textToAdd != "")
				resultStrVector.push_back(textToAdd);
			textTmpStorage = "";
			textCut = textCut.substr(st1 + delimiter.length());//剪裁字符串，去掉已经加入列表的部分
			//统计分隔符个数
			DelimiterCount += 1;
		}
	}
	if (DelimiterCount == 0)
		resultStrVector.push_back(text);
	//"123456 789 4444 \ 55 \\ 11111 ";
	//123456;789;4444; 55;\;11111;
	return resultStrVector;
}
std::string join(std::vector<std::string> textVec, std::string delimiter)
{
	//using namespace std;
	using std::string;
	string resultString;
	for (string vecElement : textVec)
	{
		resultString += vecElement;
		resultString += delimiter;
	}
	resultString.pop_back();
	return resultString;
}
