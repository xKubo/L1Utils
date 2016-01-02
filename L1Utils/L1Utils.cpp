// L1Utils.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <vector>
#include <db/DBIntf.h>
#include <win32/Win32Err.h>
#include <Check.h>
#include <GSL/string_span.h>

using namespace std;

using TStringView = gsl::string_span<>;

inline std::ostream& operator<<(ostream& o, const TStringView& v)
{
	o.write(v.data(), v.size());
	return o;
}


using TParams = vector<pair<TStringView, TStringView>>;




struct CParameters : TParams
{
	using TParams::TParams;

	
};

// Parse parameters 
CParameters ParseParameters(int argc, char** argv)
{
	CParameters p;

	return p;
}


using namespace std;
using namespace Win32;
 
template <typename T, typename ... Args>
inline void ArgsToWString(wostringstream &woss, T&& t, Args&&... args...)
{
	woss << t;
	ArgsToWString(woss, forward<Args>(args)...);
}

inline void ArgsToWString(wostringstream &woss)
{

}

struct CConsole
{

	HANDLE Handle() { return m_hOut; }
private:
	HANDLE m_hOut = GetStdHandle(STD_OUTPUT_HANDLE);
};

CONSOLE_SCREEN_BUFFER_INFOEX GetConsoleInfo(CConsole& c)
{
	CONSOLE_SCREEN_BUFFER_INFOEX csbi = {};
	csbi.cbSize = sizeof(csbi);
	Win32Check(GetConsoleScreenBufferInfoEx(c.Handle(), &csbi), "ConsoleInfo");
	return csbi;
}

struct CScreen
{
	CScreen()
	{

	}

	void UpdateData(CConsole c)
	{
		m_Info = GetConsoleInfo(c);
		m_Data.resize(Height()*Width());
		SMALL_RECT sr = { 0, 0, Width(), Height() };
		Win32Check(ReadConsoleOutput(c.Handle(), m_Data.data(), m_Info.dwSize, { 0, 0 }, &sr), "ReadOutput");
	}

	short Width() const
	{
		return m_Info.dwSize.X;
	}

	short Height() const
	{
		return m_Info.dwSize.Y;
	}

	const CHAR_INFO *Data() const
	{
		return m_Data.data();
	}

	CONSOLE_SCREEN_BUFFER_INFOEX m_Info;
	vector<CHAR_INFO> m_Data;
};

enum LogLevel { Panic, Error, Info, Verbose };

struct CLogContext
{

};

enum TextColors : WORD
{
	Black = 0,
	Red = FOREGROUND_RED,
	Blue = FOREGROUND_BLUE,
	Green = FOREGROUND_GREEN,
	Yellow = FOREGROUND_RED | FOREGROUND_GREEN,
	Cyan = FOREGROUND_BLUE | FOREGROUND_GREEN,
	Magenta = FOREGROUND_BLUE | FOREGROUND_RED,
	Gray = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
	LightRed = Red | FOREGROUND_INTENSITY,
	LightBlue = Blue | FOREGROUND_INTENSITY,
	LightGreen = Green | FOREGROUND_INTENSITY,
	LightYellow = Yellow | FOREGROUND_INTENSITY,
	LightCyan = Cyan | FOREGROUND_INTENSITY,
	LightMagenta = Magenta | FOREGROUND_INTENSITY,
	White = Gray | FOREGROUND_INTENSITY,
};

struct CConsoleLogReporter
{
	void SetTextColor(WORD Color)
	{
		Win32::Win32Check(SetConsoleTextAttribute(m_Console.Handle(), Color), "SetAttr");
	}
	void OnLog(LogLevel Lvl, const wstring& ws)
	{
		SetTextColor(Colors[Lvl]);
		DWORD dwWritten;
		Win32::Win32Check(WriteConsole(m_Console.Handle(), ws.data(), ws.size(), &dwWritten, nullptr), "WriteConsole");
		Win32::Check(dwWritten == ws.size(), "AllWritten");
		SetTextColor(Gray);
	}
private:
	WORD Colors[4] = { LightRed, Red, LightCyan, Gray};
	CConsole m_Console;
};



struct CLogReporter
{
	template <typename ... TArgs>
	void OnLog(LogLevel Lvl, const TArgs&... args...)
	{
		wostringstream woss;
		ArgsToWString(woss, args...);
		woss << endl;
		m_Reporter.OnLog(Lvl, woss.str());
	}
private:
	CConsoleLogReporter m_Reporter;

};


int main(int argc, char* argv[])
{
	try
	{

		CParameters Params = ParseParameters(argc, argv);
		for (const auto& Param : Params)
		{
			cout << Param.first << " = " << Param.second << endl;
		}

		cout << "Test" << endl;
		CConsole c;
		SetConsoleTextAttribute(c.Handle(), FOREGROUND_GREEN | FOREGROUND_INTENSITY | 0);
		CLogReporter r;
		r.OnLog(Panic, L"TestP", 42);
		r.OnLog(Error, L"TestE");
		r.OnLog(Info, L"TestI");
		r.OnLog(Verbose, L"TestV");

		CScreen s;
		s.UpdateData(c);

		for (int i = 0; i < 5; ++i)
		{
			wcout << "Data = " << ((s.Data() + i)->Char.UnicodeChar) << endl;
		}

	}
	catch (const std::exception& e)
	{
		cerr << e.what() << endl;
	}


	return 0;
}



