/* NAntMenu - Public DLL Interface
 * Copyright (C) 2003 Mathias Hasselmann <mathias@taschenorakel.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <objbase.h>
#include <initguid.h>
#include "ContextMenu.h"

#pragma comment(lib, "gdi32.lib")

CComModule module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_CNAntContextMenu, CNAntContextMenu)
END_OBJECT_MAP()

static bool IsWindowsNT()
{
	OSVERSIONINFO versionInfo;
  	versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);

	return
		GetVersionEx(&versionInfo) &&
 		VER_PLATFORM_WIN32_NT == versionInfo.dwPlatformId;
}

extern "C" BOOL WINAPI 
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
#ifdef UNICODE
    			if (!IsWindowsNT())
      				return FALSE;
#endif
    			module.Init(ObjectMap, hInstance);
			break;
			
		case DLL_PROCESS_DETACH:
    			module.Term();
			break;
	}
	
  	return TRUE;
}

STDAPI DllCanUnloadNow(void)
{
	return 0 == module.GetLockCount() ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return module.GetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer(void)
{
	return module.RegisterServer(FALSE);
}

STDAPI DllUnregisterServer(void)
{
	return module.UnregisterServer();
}

// vim:ts=4:sw=4
