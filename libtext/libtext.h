#ifdef LIBTEXT_EXPORTS
#define LIBTEXT_API __declspec(dllexport)
#else
#define LIBTEXT_API __declspec(dllimport)
#endif

#include <string>
#include <vector>

namespace libtext
{
	using std::string;
	using std::wstring;
	using std::vector;

	LIBTEXT_API vector<wstring> split(const wstring &str, const wstring &delimiter);
	LIBTEXT_API wstring format(wstring fmt, ...);
	LIBTEXT_API string format(string fmt, ...);

	LIBTEXT_API wstring Int64ToWString(__int64 v);
	LIBTEXT_API __int64 wstringToInt64(const wstring& str);
	LIBTEXT_API wstring IntToWString(int v);
	LIBTEXT_API int wstringToInt(const wstring& str);
	LIBTEXT_API wstring UIntToWString(unsigned int v);
	//将wstring转换为unsinged int,此方法会将字符串"12345ddd"转换为整数12345，此时可使用下一个wstringToUInt方法
	LIBTEXT_API unsigned int wstringToUInt(const wstring& str);
	//将wstring转换为unsinged int，此方法转换字符串"12345ddd"时将返回失败false
	LIBTEXT_API bool wstringToUInt(const wstring& str, unsigned int* outResult);

	LIBTEXT_API wstring string2wstring(const string &str);
	LIBTEXT_API string wstring2string(const wstring &wstr);
	LIBTEXT_API string UTF8_To_string(const string & str);
	LIBTEXT_API string string_To_UTF8(const string & str);
	LIBTEXT_API wstring UTF8_To_wstring(const string & str);
	LIBTEXT_API string wstring_To_UTF8(const wstring & str);

	LIBTEXT_API bool contains(const wstring& str, const wstring& substr);
	LIBTEXT_API bool startWith(const wstring& str, const wstring& with);
	LIBTEXT_API bool endWith(const wstring& str, const wstring& with);
	LIBTEXT_API wstring trim(const wstring& str);
	LIBTEXT_API wstring trimLeft(const wstring& str, const wstring& trimstr);
	LIBTEXT_API wstring trimRight(const wstring& str, const wstring& trimstr);

	LIBTEXT_API wstring toUpper(wstring str);
	LIBTEXT_API wstring toLower(wstring str);

	LIBTEXT_API wstring replace(const wstring& str, const wstring& src, const wstring& dest);

	LIBTEXT_API wstring startWString(const wstring& str, const int num);
	LIBTEXT_API wstring endWString(const wstring& str, const int num);

	LIBTEXT_API wstring readFromFile(wstring path);
	LIBTEXT_API void writeToFile(wstring str, wstring path);

	LIBTEXT_API string string_to_URLEncode2(const string& str);
}