/* NAntMenu - Implementation of the context menu handler
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

#include "ContextMenu.h"

#define MODULENAME L##"NAntMenu.dll"

#define CMDINFO(Caption, Verb, Help) \
{ Caption, Verb, Help, L##Caption, L##Verb, L##Help }

#ifdef _UNICODE
#define T(Str) L##Str
#define Caption wcsCaption
#define Help wcsHelp
#define Verb wcsVerb
#else
#define T(Str) Str
#define Caption strCaption
#define Help strHelp
#define Verb strVerb
#endif

#define StrLen _tcslen

CNAntCommand CNAntContextMenu::commands[] =
{
	CMDINFO("Edit", "NAntMenuEdit",
		"Edit this build script"),
	CMDINFO("Explore", "NAntMenuBrowse",
		"Explore the project's folder"),
	CMDINFO("Command Prompt", "NAntMenuShell",
		"Open a command prompt for the project's folder"),
	CMDINFO("Help", "NAntMenuHelp",
		"Display help files for NAnt"),
	CMDINFO("Default", "NAntMenuDefault",
		"Build the project's default target")
};

static bool Failed(LONG result)
{
	if (NOERROR != result)
	{
		SetLastError(result);
		return true;
	}

	return false;
}

HRESULT STDMETHODCALLTYPE
CNAntContextMenu::UpdateRegistry(BOOL bRegister)
{
	OLECHAR clsid[64];
	HMODULE module;
	TCHAR filename[MAX_PATH];

	if (!StringFromGUID2(CLSID_CNAntContextMenu, clsid, sizeof(clsid)) ||
		NULL == (module = GetModuleHandle(MODULENAME)) ||
		!GetModuleFileName(module, filename, sizeof(filename)))
	{
		MessageBox(0, L"Cannot locate " MODULENAME L".", 0, 0);
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (bRegister)
	{
		//
		// Register the COM object
		//
		CRegKey rClass;
		CRegKey rInProc;

		bstr_t nClass(bstr_t("CLSID\\") + clsid);

		if (Failed(rClass.Create(HKEY_CLASSES_ROOT, nClass)) ||
			Failed(rClass.SetStringValue(NULL, L"NAntMenu Shell Extension")) ||

			Failed(rInProc.Create(rClass, L"InprocServer32")) ||
			Failed(rInProc.SetStringValue(NULL, filename)) ||
			Failed(rInProc.SetStringValue(L"ThreadingModel", L"Apartment")))
		{
			MessageBox(0, L"Cannot register COM object.", 0, 0);
			return HRESULT_FROM_WIN32(GetLastError());
		}

		//
		// Register the file type
		//
		CRegKey rDotBuild;
		CRegKey rNAntScript;
		CRegKey rDefaultIcon;
		CRegKey rNAntMenu;

		bstr_t defaultIcon(bstr_t(filename) + ",0");

		if (Failed(rDotBuild.Create(HKEY_CLASSES_ROOT, L".build")) ||
			Failed(rDotBuild.SetStringValue(NULL, L"NAntBuildScript")) ||

			Failed(rNAntScript.Create(HKEY_CLASSES_ROOT, L"NAntBuildScript")) ||
			Failed(rNAntScript.SetStringValue(NULL, L"NAnt Build Script")) ||
			Failed(rDefaultIcon.Create(rNAntScript, L"DefaultIcon")) ||
			Failed(rDefaultIcon.SetStringValue(NULL, defaultIcon)) ||

			Failed(rNAntMenu.Create(rNAntScript,
				L"shellex\\ContextMenuHandlers\\NAntMenu")) ||
			Failed(rNAntMenu.SetStringValue(NULL, bstr_t(clsid))))
		{
			MessageBox(0, L"Cannot register file type.", 0, 0);
			return HRESULT_FROM_WIN32(GetLastError());
		}
	}
	else
	{
		MessageBox(0, L"Not implemented.", 0, 0);
		return E_NOTIMPL;
	}

	return S_OK;
}

HRESULT CNAntContextMenu::LoadScript(LPCTSTR filename)
{
	HRESULT result = S_OK;

	if (!this->script)
		result = this->script.CoCreateInstance(L"Msxml2.DOMDocument");

	this->targetCount = 0;
	this->defaultTarget = 0;

	if (!this->targets)
	{
		CNAntTarget * t = targets;
		this->targets = 0;
		delete[] t;
	}

	VARIANT_BOOL scriptLoaded;
	CComPtr<IXMLDOMElement> project;
	variant_t defaultTarget;

	CComPtr<IXMLDOMNodeList> targetList;

	if (SUCCEEDED(result) &&
		SUCCEEDED(result = this->script->load(
			variant_t(filename), &scriptLoaded)) && scriptLoaded &&
		SUCCEEDED(this->script->get_documentElement(&project)) && project &&
		SUCCEEDED(project->getAttribute(bstr_t("default"), &defaultTarget)) &&
		SUCCEEDED(this->script->selectNodes(
			bstr_t("/project/target[@description]"),
			&targetList)) &&
		SUCCEEDED(targetList->get_length(&this->targetCount)))
	{
		bstr_t defaultTargetName(defaultTarget);
		CComPtr<IXMLDOMNode> node;
		CComPtr<IXMLDOMElement> target;
		variant_t name, desc;

		this->targets = new CNAntTarget[this->targetCount + 1];
		CNAntTarget * first = this->targets;

		while (SUCCEEDED(targetList->nextNode(&node)) && node &&
			   SUCCEEDED(node.QueryInterface(&target)) &&
			   SUCCEEDED(target->getAttribute(bstr_t("name"), &name)) &&
			   SUCCEEDED(target->getAttribute(bstr_t("description"), &desc)))
		{
			first->name = name;
			first->desc = desc;

			if (!this->defaultTarget && first->name == defaultTargetName)
				this->defaultTarget = first;

			first++;
		}

		if (!this->defaultTarget && defaultTargetName.length())
		{
			this->defaultTarget = this->targets + this->targetCount;
			first->name = defaultTargetName;
			first->desc = L"Build the default target";
		}
	}

	return result;
}

HRESULT STDMETHODCALLTYPE
CNAntContextMenu::Initialize(LPCITEMIDLIST pidlFolder,
	LPDATAOBJECT dataObject, HKEY hkeyProgID)
{
	HRESULT result = E_FAIL;

	if (dataObject)
	{
  		FORMATETC format =
		{
			CF_HDROP,  NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL
		};
		STGMEDIUM medium;

		result = dataObject->GetData(&format, &medium);

		if (S_OK == result)
		{
			HDROP hDrop = HDROP(GlobalLock(medium.hGlobal));

			if (1 == DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0) &&
				0 != DragQueryFile(hDrop, 0, this->filename,
					sizeof(this->filename)))
			{
				result = this->LoadScript(this->filename);
			}
			else
			{
				*this->filename = '\0';
				result = HRESULT_FROM_WIN32(GetLastError());
			}

			HMODULE module = GetModuleHandle(MODULENAME);
			const INT icx = GetSystemMetrics(SM_CXSMICON);
			const INT icy = GetSystemMetrics(SM_CXSMICON);

			this->hIcons[IdCmd_Edit] =
				HICON(LoadImage(module, L"edit", IMAGE_ICON, icx, icy, 0));
			this->hIcons[IdCmd_Browse] =
				HICON(LoadImage(module, L"explore", IMAGE_ICON, icx, icy, 0));
			this->hIcons[IdCmd_Shell] =
				HICON(LoadImage(module, L"shell", IMAGE_ICON, icx, icy, 0));
			this->hIcons[IdCmd_Help] =
				HICON(LoadImage(module, L"help", IMAGE_ICON, icx, icy, 0));
			this->hIcons[IdCmd_Default] =
				HICON(LoadImage(module, L"build", IMAGE_ICON, icx, icy, 0));

			GlobalUnlock(hDrop);
			GlobalFree(medium.hGlobal);
		}
	}


	return result;
}

static  BOOL MenuItemSetDefault(HMENU hMenu, UINT uItem,
	BOOL fByPosition, BOOL fDefault)
{
	MENUITEMINFO itemInfo;
	memset(&itemInfo, 0, sizeof(itemInfo));
	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.fMask = MIIM_STATE;

	if (GetMenuItemInfo(hMenu, uItem, fByPosition, &itemInfo))
	{
		itemInfo.fState = fDefault ?
			itemInfo.fState | MFS_DEFAULT :
			itemInfo.fState & ~MFS_DEFAULT;
		return SetMenuItemInfo(hMenu, uItem, fByPosition, &itemInfo);
	}

	return FALSE;
}

HRESULT STDMETHODCALLTYPE
CNAntContextMenu::QueryContextMenu(HMENU hMenu, UINT indexMenu,
	UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	if (!(uFlags & CMF_DEFAULTONLY))
	{
		HMENU hPopup = CreateMenu();

		this->idCmdFirst = idCmdFirst;
		CNAntTarget const * first = this->targets;
		CNAntTarget const * const last = first + this->targetCount;

		while (first < last &&
			AppendMenu(hPopup, MF_STRING, idCmdFirst, first->name) &&
			MenuItemSetDefault(hPopup, idCmdFirst, FALSE,
				first == this->defaultTarget))
		{
			idCmdFirst++;
			first++;
		}

		AppendMenu(hPopup, MF_SEPARATOR, 0, NULL);
		AppendMenu(hPopup, MF_OWNERDRAW | MF_STRING, idCmdFirst++, T("&Edit"));
		AppendMenu(hPopup, MF_OWNERDRAW | MF_STRING, idCmdFirst++, T("&Browse"));
		AppendMenu(hPopup, MF_OWNERDRAW | MF_STRING, idCmdFirst++, T("&Shell"));
		AppendMenu(hPopup, MF_SEPARATOR, 0, NULL);
		AppendMenu(hPopup, MF_OWNERDRAW | MF_STRING, idCmdFirst++, T("&Help"));

		if (!InsertMenu(hMenu, indexMenu,
			 MF_BYPOSITION | MF_STRING | MF_POPUP,
			 UINT_PTR(hPopup), T("NAnt")))
			return HRESULT_FROM_WIN32(GetLastError());

		if (this->defaultTarget)
		{
			bstr_t caption =
				T("NAnt (") + this->defaultTarget->name + T(")");

			MENUITEMINFO itemInfo;

			memset(&itemInfo, 0, sizeof(itemInfo));
			itemInfo.cbSize = sizeof(itemInfo);
			itemInfo.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
			itemInfo.wID = idCmdFirst++;
			itemInfo.fType = MFT_STRING | MFT_OWNERDRAW;
			itemInfo.fState = MFS_DEFAULT;
			itemInfo.dwTypeData = caption;

			if (!InsertMenuItem(hMenu, indexMenu, TRUE, &itemInfo))
				return HRESULT_FROM_WIN32(GetLastError());
		}

		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, idCmdFirst);
	}

	return E_FAIL;
}

inline bstr_t GetDescription(const CNAntTarget & target)
{
	return target.desc;
}

inline bstr_t GetVerb(const CNAntTarget & target)
{
	return "NAntMenuRun" + target.name;
}

inline LPCSTR GetHelpA(const CNAntCommand & cmdInfo)
{
	return cmdInfo.strHelp;
}

inline LPCWSTR GetHelpW(const CNAntCommand & cmdInfo)
{
	return cmdInfo.wcsHelp;
}

inline LPCSTR GetVerbA(const CNAntCommand & cmdInfo)
{
	return cmdInfo.strVerb;
}

inline LPCWSTR GetVerbW(const CNAntCommand & cmdInfo)
{
	return cmdInfo.wcsVerb;
}

template <class CharType>
HRESULT CNAntContextMenu::CopyHelpString(UINT idCmd,
	CharType * (*copyFunc)(CharType *, const CharType *, UINT),
	bstr_t (*targetFunc)(const CNAntTarget & target),
	const CharType * (*sourceFunc)(const CNAntCommand & cmdInfo),
	CharType * strBuf,
    UINT cchMax)
{
	if (idCmd < this->targetCount)
	{
		copyFunc(strBuf, targetFunc(this->targets[idCmd]), cchMax - 1);
		strBuf[cchMax - 1] = '\0';
		return S_OK;
	}

	idCmd-= this->targetCount;

	if (idCmd < IdCmd_Last)
	{
		copyFunc(strBuf, sourceFunc(commands[idCmd]), cchMax - 1);
		strBuf[cchMax - 1] = '\0';
		return S_OK;
	}

	return E_FAIL;
}

HRESULT STDMETHODCALLTYPE CNAntContextMenu::GetCommandString(
	UINT idCmd, UINT uType, UINT * pwReserved, LPSTR pszName, UINT cchMax)
{
	LPWSTR pwszName = LPWSTR(pszName);
	switch(uType)
	{
		case GCS_VALIDATEA:
		case GCS_VALIDATEW:
			return idCmd >= this->idCmdFirst &&
				idCmd < this->idCmdFirst + this->targetCount + 2 ?
				S_OK : S_FALSE;

		case GCS_HELPTEXTA:
			return this->CopyHelpString(idCmd, strncpy_s, GetDescription, GetHelpA, pszName, cchMax);

		case GCS_HELPTEXTW:
			return this->CopyHelpString(idCmd, wcsncpy_s, GetDescription, GetHelpW, pwszName, cchMax);

		case GCS_VERBA:
			return this->CopyHelpString(idCmd, strncpy_s, GetVerb, GetVerbA, pszName, cchMax);

		case GCS_VERBW:
			return this->CopyHelpString(idCmd, wcsncpy_s, GetVerb, GetVerbW, pwszName, cchMax);
	}
	return E_FAIL;
}

HRESULT CNAntContextMenu::BuildTarget(const CNAntTarget & target)
{
	TCHAR path[MAX_PATH];
	StrCpy(path, this->filename);
	LPTSTR filename = StrRChr(path, path + StrLen(path), '\\');

	if (filename)
	{
		*filename++ = '\0';

		bstr_t cmdline =
			"/c title NAnt - " + target.name + " - " + filename +
			" && nant -buildfile:\"" + filename + "\" " + target.name +
			" && pause || pause";

		ShellExecute(NULL, T("open"), T("cmd.exe"),
			cmdline, path, SW_SHOWNORMAL);

		return S_OK;
	}

	return E_FAIL;
}

HRESULT STDMETHODCALLTYPE CNAntContextMenu::InvokeCommand(
	LPCMINVOKECOMMANDINFO lpici)
{
	if (HIWORD(lpici->lpVerb))
	{
		MessageBox(0, lpici->lpVerb, __FUNCTION__, 0);//TODO:
	}
	else
	{
		//UINT idCmd = LOWORD(lpici->lpVerb);
        LONG idCmd = LOWORD(lpici->lpVerb);

		if (idCmd < this->targetCount)
		{
			return this->BuildTarget(this->targets[idCmd]);
		}
		else
		{
			idCmd-= this->targetCount;

			TCHAR path[MAX_PATH];
			StrCpy(path, this->filename);
			LPTSTR filename = StrRChr(path, path + StrLen(path), '\\');

			if (filename)
				*filename++ = '\0';
			else
			{
				filename= this->filename;
				*path = '\0';
			}

			switch(idCmd)
			{
				case IdCmd_Edit:
					if (ShellExecute(NULL, T("edit"), this->filename,
							NULL, NULL, SW_SHOWNORMAL))
						return S_OK;

					return HRESULT_FROM_WIN32(GetLastError());

				case IdCmd_Browse:
					if (ShellExecute(NULL, T("open"), path,
							NULL, NULL, SW_SHOWNORMAL))
						return S_OK;

					return HRESULT_FROM_WIN32(GetLastError());

				case IdCmd_Shell:
					if (ShellExecute(NULL, T("open"), T("cmd"),
							NULL, path, SW_SHOWNORMAL))
						return S_OK;

					return HRESULT_FROM_WIN32(GetLastError());

				case IdCmd_Help:
					if (ExpandEnvironmentStrings(
							T("%PROGRAMFILES%\\nant\\doc\\help\\index.html"),
							path, sizeof(path)) &&
						ShellExecute(NULL, T("open"), path,
							NULL, NULL, SW_SHOWNORMAL))
						return S_OK;

					return HRESULT_FROM_WIN32(GetLastError());

				case IdCmd_Default:
					if (this->defaultTarget)
						return this->BuildTarget(*this->defaultTarget);

					break;
			}
		}
	}

	return E_FAIL;
}

HRESULT STDMETHODCALLTYPE CNAntContextMenu::HandleMenuMsg(
	UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT noResult;
	return this->HandleMenuMsg2(uMsg, wParam, lParam, &noResult);
}

HRESULT STDMETHODCALLTYPE CNAntContextMenu::HandleMenuMsg2(UINT uMsg,
	WPARAM wParam, LPARAM lParam, LRESULT * plResult)
{
	if (NULL == plResult)
		return E_INVALIDARG;

	switch(uMsg)
	{
		case WM_INITMENUPOPUP:
			return NOERROR;

		case WM_DRAWITEM:
			return this->DrawItem(
				HMENU(wParam), LPDRAWITEMSTRUCT(lParam));

		case WM_MEASUREITEM:
			return this->MeasureItem(
				HMENU(wParam), LPMEASUREITEMSTRUCT(lParam));

		case WM_MENUCHAR:
			return this->OnMenuChar(
				HMENU(lParam), LOWORD(wParam), HIWORD(wParam), *plResult);
	}

	wchar_t str[100];
	wsprintf(str, L"Unexpected uMsg in HandleMenuMsg2: %d", uMsg);
	MessageBox(0, str, 0, 0);

	return E_NOTIMPL;
}

LPCTSTR CNAntContextMenu::GetMenuCaption(UINT itemId, HICON * hIcon)
{
	UINT cmdId = itemId - this->idCmdFirst;

	if (cmdId < this->targetCount)
	{
		if (hIcon) *hIcon = this->hIcons[IdCmd_Default];
		return this->targets[cmdId].name;
	}

	cmdId-= this->targetCount;

	if (cmdId < IdCmd_Last)
	{
		if (hIcon) *hIcon = this->hIcons[cmdId];
		return commands[cmdId].Caption;
	}

	if (hIcon) *hIcon = NULL;
	return L"Unknown item";
}

HRESULT CNAntContextMenu::DrawItem(
	HMENU hMenu, LPDRAWITEMSTRUCT pDrawItem)
{
	if (NULL == pDrawItem)
		return E_INVALIDARG;

	if (pDrawItem->itemAction & (ODA_DRAWENTIRE | ODA_SELECT))
	{
		COLORREF crText = GetTextColor(pDrawItem->hDC);
		COLORREF crBkgnd = GetBkColor(pDrawItem->hDC);

		if (pDrawItem->itemState & ODS_SELECTED)
		{
			crText = SetTextColor(pDrawItem->hDC,
				GetSysColor(pDrawItem->itemState & ODS_GRAYED ?
				COLOR_GRAYTEXT : COLOR_HIGHLIGHTTEXT));
			SetBkColor(pDrawItem->hDC, GetSysColor(COLOR_HIGHLIGHT));
		}

		HICON hIcon;
		LPCTSTR caption = this->GetMenuCaption(pDrawItem->itemID, &hIcon);
		HBRUSH bkBrush = CreateSolidBrush(GetBkColor(pDrawItem->hDC));

		FillRect(pDrawItem->hDC, &pDrawItem->rcItem, bkBrush);

		RECT rc = pDrawItem->rcItem;

		const DWORD wa = rc.right - rc.left + 1;
		const DWORD ha = rc.bottom - rc.top + 1;
		const DWORD wi = GetSystemMetrics(SM_CXSMICON);
		const DWORD hi = GetSystemMetrics(SM_CYSMICON);

		DrawIconEx(pDrawItem->hDC,
			rc.left - 1, rc.top + (ha - hi) / 2,
			hIcon, wi, hi, 0, NULL, DI_NORMAL | DI_COMPAT);
		DeleteObject(bkBrush);

		rc.left+= wi;

		DrawText(pDrawItem->hDC, caption, -1, &rc,
			DT_SINGLELINE | DT_LEFT | DT_VCENTER);
	}

	return NOERROR;
}

HRESULT CNAntContextMenu::MeasureItem(
	HMENU hMenu, LPMEASUREITEMSTRUCT pMeasureItem)
{
	if (NULL == pMeasureItem)
		return E_INVALIDARG;

	RECT rc;
	LPCTSTR caption = this->GetMenuCaption(pMeasureItem->itemID);

	if (DrawText(GetDC(NULL), caption, -1, &rc, DT_SINGLELINE | DT_CALCRECT))
	{
		pMeasureItem->itemWidth = rc.right - rc.left + 1;
		pMeasureItem->itemHeight = rc.bottom - rc.top + 1;

		return NOERROR;
	}

	return HRESULT_FROM_WIN32(GetLastError());
}

HRESULT CNAntContextMenu::OnMenuChar(
	HMENU hMenu, WORD type, WORD charcode, LRESULT & result)
{
	result = MNC_IGNORE;
	return NOERROR;
}

// vim:ts=4:sw=4