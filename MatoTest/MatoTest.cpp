// MatoTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <win32/Win32Err.h>
#include <check.h>
#include <sstream>
#include <time.h>
#include <random>
#include <iostream>
#include <cassert>
#include <Time/TimeUtils.h>

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

enum LogLevel { Panic, Error, Info, Verbose, Status };


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
		SetTextColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
	}
private:
	WORD Colors[5] = { 
		FOREGROUND_RED | FOREGROUND_INTENSITY, 
		FOREGROUND_RED, 
		FOREGROUND_GREEN | FOREGROUND_INTENSITY, 
		FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
		FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY};
	CConsole m_Console;
};

mt19937 Engine{ random_device{}() };

int Pick(int Max)
{
	uniform_int_distribution<> u{ 0, Max-1 };
	return u(Engine);
}

char Operation[] = "+-./";

int Read()
{
	int x;
	cin >> x;
	return x;
}


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


CLogReporter r;

int PocetChyb = 0;
int PocetOK = 0;
Time::Point Start = Time::UTCNow();

void DoExample(int Num1, char Op, int Num2, int Res)
{
	while (cin)
	{
		r.OnLog(Status, Num1, " ", Op, " ", Num2, " = ");
		int Inp;
		cin >> Inp;
		if (Res == Inp)
		{
			r.OnLog(Info, "OK");
			PocetOK++;
			break;
		}
		else
		{
			PocetChyb++;
			r.OnLog(Panic, "Chyba - skus znovu:");
		}		
	}
	r.OnLog(Verbose, "OK=", PocetOK, " Chybne=", PocetChyb, " Vsetko=", PocetOK + PocetChyb, " Cas=", (Time::UTCNow()-Start).InSecs(), "s");
}

int main()
{
	while (cin)
	{
		char Op = Operation[Pick(sizeof(Operation)-1)];
		switch (Op)
		{
		case '+':
		{
			int Result = Pick(99) + 1;
			int Op1 = Pick(Result);
			int Op2 = Result - Op1;
			if (Pick(1) % 2)
				swap(Op1, Op2);
			DoExample(Op1, Op, Op2, Result);
			break;
		}
		case '-':
		{
			int Result = Pick(100);
			int Op1 = Pick(Result);
			int Op2 = Result - Op1;
			DoExample(Result, Op, Op2, Op1);
			break;
		}
		case '.':
		{
			int Op1 = Pick(5);
			int Op2 = Pick(10);
			if (Pick(1) % 2)
				swap(Op1, Op2);
			DoExample(Op1, Op, Op2, Op1*Op2);
			break;
		}
		case '/':
		{
			int Op1 = Pick(10);
			int Op2 = Pick(5) + 1;
			DoExample(Op1 * Op2, Op, Op2, Op1);
			break;
		}
		default:
			assert(false);
		}
	}

    return 0;
}

