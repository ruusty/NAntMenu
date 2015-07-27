/* NAntMenu - Declaration of the context menu handler
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

#pragma once
#define UNICODE=1

#include <atlbase.h>

#include "Strsafe.h"
#include <atlcom.h>
#include <comdef.h>
#include <shlobj.h>
#include <msxml2.h>

//2A57BF13-CB2B-4769-B18C4133153F8D28
DEFINE_GUID(CLSID_CNAntContextMenu,
0x2A57BF13, 0xCB2B, 0x4769, 0xB1, 0x8C, 0x41, 0x33, 0x15, 0x3F, 0x8D, 0x28);

struct CNAntTarget
{
	bstr_t name, desc;
};

struct CNAntCommand
{
	LPCSTR strCaption, strVerb, strHelp;
	LPCWSTR wcsCaption, wcsVerb, wcsHelp;
};

class CNAntContextMenu :
public CComObjectRoot,
public CComCoClass<CNAntContextMenu, &CLSID_CNAntContextMenu>,
public IContextMenu3,
public IShellExtInit
{
private:
	enum IdCmd
	{
		IdCmd_Edit,
		IdCmd_Browse,
		IdCmd_Shell,
		IdCmd_Help,
		IdCmd_Default,
		IdCmd_Last
	};

	TCHAR filename[MAX_PATH];
	CComPtr<IXMLDOMDocument> script;

	UINT idCmdFirst;
	LONG targetCount;
	CNAntTarget * targets;
	CNAntTarget * defaultTarget;

	static CNAntCommand commands[IdCmd_Last];
	HICON hIcons[IdCmd_Last];

protected:
	HRESULT LoadScript(LPCTSTR filename);
	HRESULT BuildTarget(const CNAntTarget & target);

	template <class CharType>
	HRESULT CopyHelpString(UINT idCmd,
		CharType * (*copyFunc)(CharType *, const CharType *, UINT),
		bstr_t (*targetFunc)(const CNAntTarget & target),
		const CharType * (*sourceFunc)(const CNAntCommand & cmdInfo),
		CharType * strBuf, UINT cchMax);
	LPCTSTR GetMenuCaption(UINT itemId, HICON * hIcon = NULL);

public:
	CNAntContextMenu():
		targetCount(0), targets(0), defaultTarget(0) {}

	BEGIN_COM_MAP(CNAntContextMenu)
		COM_INTERFACE_ENTRY(IShellExtInit)
		COM_INTERFACE_ENTRY(IContextMenu3)
		COM_INTERFACE_ENTRY(IContextMenu2)
		COM_INTERFACE_ENTRY(IContextMenu)
	END_COM_MAP()

	DECLARE_NOT_AGGREGATABLE(CNAntContextMenu)

	static HRESULT STDMETHODCALLTYPE UpdateRegistry(BOOL bRegister);

	/////////////////////////////
	// IShellExtInit

public:
	STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder,
		LPDATAOBJECT dataObject, HKEY hkeyProgID);

	/////////////////////////////
	// IContextMenu

public:
	STDMETHOD(QueryContextMenu)(HMENU hMenu, UINT indexMenu,
		UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
  	STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
  	STDMETHOD(GetCommandString)(UINT idCmd, UINT uType,
		UINT * pwReserved, LPSTR pszName, UINT cchMax);

	/////////////////////////////
	// IContextMenu2

public:
	STDMETHOD(HandleMenuMsg)(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	HRESULT DrawItem(HMENU hMenu, LPDRAWITEMSTRUCT pDrawItem);
	HRESULT MeasureItem(HMENU hMenu, LPMEASUREITEMSTRUCT pMeasureItem);

	/////////////////////////////
	// IContextMenu3

public:
	STDMETHOD(HandleMenuMsg2)(UINT uMsg,
		WPARAM wParam, LPARAM lParam, LRESULT * plResult);
	HRESULT OnMenuChar(HMENU hMenu, WORD type, WORD charcode, LRESULT & result);
};

// vim:ts=4:sw=4
