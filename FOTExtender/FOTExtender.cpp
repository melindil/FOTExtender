/* MIT License

Copyright (c) 2018 melindil

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// FOTExtender.cpp : Defines the exported functions for the DLL application.
//

#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

#include <Windows.h>
#include <strsafe.h>
#include "Logger.h"

#include "GenericPatcher.h"

#include "HookInstaller.h"
HookInstaller* g_hookinstaller;

#define FTE_VERSION "0.1a"

extern "C" _declspec(dllexport)  void _cdecl Bnd(WCHAR** firstparam, WCHAR** lastparam)
{
	Logger* log = new Logger("FTE.log");
	*log << "FTE version " << FTE_VERSION << " started." << std::endl;

	// Temporary dumping code
	std::ofstream outdump("BOS_FTE.EXE.DMP", std::ios::out | std::ios::binary);
	outdump.write((char*)0x400000, 0x51e000);
	outdump.close();

	while (firstparam != lastparam)
	{
		size_t len;
		if (SUCCEEDED(StringCchLengthW(*firstparam, 1024, &len)))
		{
			// UTF8: Max 4 bytes per UCS2 code, plus 1 byte for UTF8 null-term
			std::vector<char> converted(len * 2 + 1, 0);

			int newstrsize = WideCharToMultiByte(CP_UTF8,
				WC_ERR_INVALID_CHARS,
				*firstparam,
				-1,
				converted.data(),
				converted.size(),
				NULL,
				NULL);
			if (newstrsize > 0)
			{
				std::string paramstring(converted.data());
				if (paramstring != "")
					*log << "Detected parameter: " << paramstring << std::endl;
				if (paramstring == "-windex")
				{
					// Blank out the parameter if we are supposed to process it
					**firstparam = 0;
				}
			}
		}
		firstparam++;
	}

	GenericPatcher patcher(log, "FTE_config.json");
	patcher.apply();
	
	g_hookinstaller = new HookInstaller(log);
	g_hookinstaller->installHooks();

}
