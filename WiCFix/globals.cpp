#include "stdafx.h"

HWND g_hWndLogWindow = NULL;
UINT g_uLineCount = 0;

BOOL set_log_window_handle(HWND hWnd)
{
	if (!hWnd)
		return FALSE;

	g_hWndLogWindow = hWnd;
	g_uLineCount = 0;

	return TRUE;
}

BOOL log_window(const std::wstring szLine)
{
	if (!g_hWndLogWindow)
		return FALSE;

	LRESULT lTextLength = SendMessage(g_hWndLogWindow, WM_GETTEXTLENGTH, NULL, NULL);
	std::wstring szOutput(lTextLength, 0);

	if (lTextLength > 0)
		GetWindowText(g_hWndLogWindow, (LPWSTR)szOutput.c_str(), lTextLength + 1);

	szOutput.append(szLine + L"\r\n");
	g_uLineCount++;

	SetWindowText(g_hWndLogWindow, szOutput.c_str());
	SendMessage(g_hWndLogWindow, EM_LINESCROLL, 0, g_uLineCount);

	return TRUE;
}

BOOL get_product_version(LPWSTR pszValue)
{
	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate = NULL;

	DWORD dwHandle = 0;
	WCHAR szPath[MAX_PATH] = L"";
	DWORD dwSize = 0;
	
	if (!GetModuleFileName(NULL, szPath, MAX_PATH))
		return FALSE;
	
	dwSize = GetFileVersionInfoSize(szPath, &dwHandle);

	if (dwSize > 0)
	{
		BYTE pBlock[1024]; // new/delete []
		ZeroMemory(pBlock, sizeof(pBlock));
		
		if (GetFileVersionInfo(szPath, NULL, dwSize, &pBlock))
		{
			VS_FIXEDFILEINFO *pFileInfo = NULL;
			UINT uSize = 0;

			if (VerQueryValue(pBlock, L"\\VarFileInfo\\Translation", (LPVOID*)&pFileInfo, &uSize))
			{
				lpTranslate = (LANGANDCODEPAGE*)pFileInfo;

				// **MUI**
				//WCHAR szSubBlock[1024];
				//ZeroMemory(szSubBlock, sizeof(szSubBlock));
				//swprintf_s(szSubBlock, L"\\StringFileInfo\\%04x%04x\\ProductVersion", lpTranslate->wLanguage, lpTranslate->wCodePage);
				//if (VerQueryValue(pBlock, szSubBlock, (LPVOID*)&pFileInfo, &uSize) && uSize > 0)

				if (VerQueryValue(pBlock, L"\\", (LPVOID*)&pFileInfo, &uSize) && uSize > 0)
				{
					swprintf_s(pszValue, MAX_STRING_LENGTH, L"%lu.%lu.%lu.%lu", 
						HIWORD(pFileInfo->dwFileVersionMS), 
						LOWORD(pFileInfo->dwFileVersionMS), 
						HIWORD(pFileInfo->dwFileVersionLS), 
						LOWORD(pFileInfo->dwFileVersionLS)
					);

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

BOOL update_available()
{
	WCHAR szAppVersion[MAX_STRING_LENGTH] = L"";
	CHAR szWebResponse[MAX_STRING_LENGTH] = "";
	
	if (!get_product_version(szAppVersion))
		return FALSE;

	HTTP_Request http;

	if (!http.Init("http://www.massgate.org/wicfix/version.php"))
		return FALSE;

	if (!http.SendGETRequest())
	{
		log_window(L"Could not contact server");
		return FALSE;
	}
	
	if (!http.Response(szWebResponse, MAX_STRING_LENGTH))
	{
		log_window(L"Could not find version file");
		return FALSE;
	}

	std::wstring szCurrentVersion(szAppVersion);
	
	std::string tmp(szWebResponse);
	std::wstring szLatestVersion(tmp.begin(), tmp.end());
	
	if (szCurrentVersion.empty() || szLatestVersion.empty())
		return FALSE;

	if (szCurrentVersion < szLatestVersion)
		return TRUE;
	else
		return FALSE;

	return FALSE;
}

HWND create_button(LPCWSTR szCaption, int x, int y, int width, int height, HWND hWnd, UINT ID, LONG DEFPUSHBUTTON)
{
	if ((DEFPUSHBUTTON < BS_PUSHBUTTON) || (DEFPUSHBUTTON > BS_DEFPUSHBUTTON))
		DEFPUSHBUTTON = BS_PUSHBUTTON;

	return CreateWindow(L"BUTTON", szCaption, WS_VISIBLE | WS_CHILD | DEFPUSHBUTTON,
		x, y, width, height,
		hWnd, (HMENU)ID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

HWND create_checkbox(LPCWSTR szCaption, int x, int y, int width, int height, HWND hWnd, UINT ID)
{
	return CreateWindow(L"BUTTON", szCaption, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		x, y, width, height,
		hWnd, (HMENU)ID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

HWND create_radio(LPCWSTR szCaption, int x, int y, int width, int height, HWND hWnd, UINT ID)
{
	return CreateWindow(L"BUTTON", szCaption, WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		x, y, width, height,
		hWnd, (HMENU)ID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

HWND create_groupbox(LPCWSTR szCaption, int x, int y, int width, int height, HWND hWnd, UINT ID)
{
	return CreateWindow(L"BUTTON", szCaption, WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
		x, y, width, height,
		hWnd, (HMENU)ID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

HWND create_edit(int x, int y, int width, int height, HWND hWnd, UINT ID)
{
	return CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
		x, y, width, height,
		hWnd, (HMENU)ID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

HWND create_edit_ml(int x, int y, int width, int height, HWND hWnd, UINT ID)
{
	return CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
		x, y, width, height,
		hWnd, (HMENU)ID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

HWND create_static(LPCWSTR szCaption, int x, int y, int width, int height, HWND hWnd, UINT ID)
{
	return CreateWindow(L"STATIC", szCaption, WS_CHILD | WS_VISIBLE | SS_LEFT,
		x, y, width, height,
		hWnd, (HMENU)ID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

HWND create_progressbar(int x, int y, int width, int height, HWND hWnd, UINT ID)
{
	return CreateWindow(PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 
		x, y, width, height, 
		hWnd, (HMENU)ID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

HRESULT get_system_drive(LPWSTR pszPath)
{
	WCHAR szPath[MAX_PATH] = L"";
	HRESULT lResult = SHGetFolderPath(NULL, CSIDL_WINDOWS, NULL, 0, szPath);
	
	if (lResult != S_OK)
		return S_FALSE;
	
	if (wcsstr(szPath, L"C:"))
		wcscpy_s(pszPath, MAX_PATH, L"C:");
	else
		wcsncpy_s(pszPath, MAX_PATH, szPath, 2);

	return S_OK;
}

HRESULT get_program_files(LPWSTR pszPath)
{
	// CSIDL_PROGRAM_FILES, FOLDERID_ProgramFilesX86
	return SHGetFolderPath(NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, pszPath);
}

HRESULT get_my_documents(LPWSTR pszPath)
{
	return SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, 0, pszPath);
}

BOOL file_exists(LPCWSTR pszFile)
{
	DWORD dwAttrib = GetFileAttributes(pszFile);
	
	if (dwAttrib == INVALID_FILE_ATTRIBUTES)
		return FALSE;

	return !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

BOOL folder_exists(LPCWSTR pszPath)
{
	DWORD dwAttrib = GetFileAttributes(pszPath);

	if (dwAttrib == INVALID_FILE_ATTRIBUTES)
		return FALSE;

	return (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

BOOL folder_create(LPCWSTR pszPath)
{
	if (folder_exists(pszPath))
		return FALSE;

	std::wstring folder(pszPath);
	size_t index = folder.find_last_of(L"\\");

	if (index != std::wstring::npos)
		folder_create(folder.substr(0, index).c_str());

	return CreateDirectory(pszPath, NULL);
}

BOOL file_copy(LPCWSTR pszPathSrc, LPCWSTR pszPathDest)
{
	if (!file_exists(pszPathSrc))
		return FALSE;

	return CopyFile(pszPathSrc, pszPathDest, FALSE);
}

BOOL file_delete(LPCWSTR pszPath)
{
	if (!file_exists(pszPath))
		return FALSE;

	if (!SetFileAttributes(pszPath, FILE_ATTRIBUTE_NORMAL))
		return FALSE;

	return DeleteFile(pszPath);
}

DWORD read_reg_dword(HKEY hKeyParent, LPCWSTR lpSubKey, LPCWSTR lpValue)
{
	DWORD dwDataLen = sizeof(DWORD);
	DWORD dwData;
	HKEY hKey;
	LSTATUS lResult = RegOpenKeyEx(hKeyParent, lpSubKey, 0, KEY_READ, &hKey);

	if (lResult == ERROR_SUCCESS)
	{
		lResult = RegQueryValueEx(hKey, lpValue, NULL, NULL, (LPBYTE)&dwData, &dwDataLen);

		if (lResult == ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return dwData;
		}

		RegCloseKey(hKey);
		return 0;
	}
	else
	{
		if (lResult == ERROR_FILE_NOT_FOUND)
		{
			RegCloseKey(hKey);
			return 0;
		}
		else
		{
			RegCloseKey(hKey);
			return 0;
		}
	}
}

BOOL read_reg_wstring(HKEY hKeyParent, LPCWSTR lpSubKey, LPCWSTR lpValue, LPWSTR pszData)
{
	DWORD dwDataLen = MAX_STRING_LENGTH;
	WCHAR szData[MAX_STRING_LENGTH] = L"";
	HKEY hKey;
	LSTATUS lResult = RegOpenKeyEx(hKeyParent, lpSubKey, 0, KEY_READ, &hKey);

	if (lResult == ERROR_SUCCESS)
	{
		lResult = RegQueryValueEx(hKey, lpValue, NULL, NULL, (LPBYTE)pszData, &dwDataLen);
		
		while (lResult == ERROR_MORE_DATA)
		{
			if (dwDataLen <= (MAX_STRING_LENGTH * 2))
				lResult = RegQueryValueEx(hKey, lpValue, NULL, NULL, (LPBYTE)pszData, &dwDataLen);
			else
				break;
		}

		if (lResult == ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return TRUE;
		}

		RegCloseKey(hKey);
		return FALSE;
	}
	else
	{
		if (lResult == ERROR_FILE_NOT_FOUND)
		{
			RegCloseKey(hKey);
			return FALSE;
		}
		else
		{
			RegCloseKey(hKey);
			return FALSE;
		}
	}
}

BOOL write_reg_dword(HKEY hKeyParent, LPCWSTR lpSubKey, LPCWSTR lpValue, DWORD dwData)
{
	DWORD dwDisposition;
	HKEY hKey;

	// LSTATUS lResult = RegOpenKeyEx(hKeyParent, lpSubKey, 0, KEY_WRITE, &hKey);

	LSTATUS lResult = RegCreateKeyEx(hKeyParent, lpSubKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);

	/*if (dwDisposition == REG_CREATED_NEW_KEY)
	{
	}
	else // REG_OPENED_EXISTING_KEY
	{
	}*/

	if (lResult == ERROR_SUCCESS)
	{
		lResult = RegSetValueEx(hKey, lpValue, NULL, REG_DWORD, (BYTE*)&dwData, sizeof(dwData));

		if (lResult != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return FALSE;
		}
	}
	RegCloseKey(hKey);
	return TRUE;
}

BOOL write_reg_wstring(HKEY hKeyParent, LPCWSTR lpSubKey, LPCWSTR lpValue, LPCWSTR pszData)
{
	DWORD dwDisposition;
	HKEY hKey;
	LSTATUS lResult = RegCreateKeyEx(hKeyParent, lpSubKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
	
	if (lResult == ERROR_SUCCESS)
	{
		lResult = RegSetValueEx(hKey, lpValue, NULL, REG_SZ, (BYTE*)pszData, wcslen(pszData) * 2);

		if (lResult != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return FALSE;
		}
	}
	RegCloseKey(hKey);
	return TRUE;
}

BOOL delete_reg_key(HKEY hKeyParent, LPCWSTR lpSubKey)
{
	HKEY hKey;
	LSTATUS lResult = RegOpenKeyEx(hKeyParent, lpSubKey, 0, KEY_ALL_ACCESS, &hKey);

	if (lResult == ERROR_SUCCESS)
	{
		lResult = RegDeleteTree(hKey, NULL);

		if (lResult != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return FALSE;
		}
	}
	RegCloseKey(hKey);
	return TRUE;
}

BOOL wic_read_cdkey(LPWSTR pszCDkey)
{
	read_reg_wstring(HKEY_CURRENT_USER, L"Software\\Massive Entertainment AB\\World in Conflict", L"CDKEY", pszCDkey);
	return (wcslen(pszCDkey) > 0);
}

BOOL wic_write_cdkey(LPCWSTR pszCDkey)
{
	return write_reg_wstring(HKEY_CURRENT_USER, L"Software\\Massive Entertainment AB\\World in Conflict", L"CDKEY", pszCDkey);
	//return (wcslen(pszCDkey) > 0);
}

BOOL wic_registry_installexepath(LPWSTR pszPath)
{
	read_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Massive Entertainment AB\\World in Conflict", L"InstallExePath", pszPath);
	return (wcslen(pszPath) > 0);
}

BOOL wic_registry_installpath(LPWSTR pszPath)
{
	read_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Massive Entertainment AB\\World in Conflict", L"InstallPath", pszPath);
	return (wcslen(pszPath) > 0);
}

BOOL wic_registry_version(LPWSTR pszPath)
{
	read_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Massive Entertainment AB\\World in Conflict", L"Version", pszPath);
	return (wcslen(pszPath) > 0);
}

BOOL wic_registry_complete()
{
	WCHAR szInstallExePath[MAX_STRING_LENGTH] = L"";
	WCHAR szInstallPath[MAX_STRING_LENGTH] = L"";
	WCHAR szLanguageCode[MAX_STRING_LENGTH] = L"";
	WCHAR szLocalized[MAX_STRING_LENGTH] = L"";
	WCHAR szVersion[MAX_STRING_LENGTH] = L"";
	
	return read_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Massive Entertainment AB\\World in Conflict", L"InstallExePath", szInstallExePath)
		&& read_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Massive Entertainment AB\\World in Conflict", L"InstallPath", szInstallPath)
		//&& read_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Massive Entertainment AB\\World in Conflict", L"LanguageCode", szLanguageCode)
		//&& read_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Massive Entertainment AB\\World in Conflict", L"Localized", szLocalized)
		//&& read_reg_dword(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Massive Entertainment AB\\World in Conflict", L"OnlineOnly")
		//&& read_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Massive Entertainment AB\\World in Conflict", L"SovietAssault", szSovietAssault)
		&& read_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Massive Entertainment AB\\World in Conflict", L"Version", szVersion);
}

BOOL gog_registry_installexepath(LPWSTR pszPath)
{
	read_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\GOG.com\\Games\\1438332414", L"EXE", pszPath);
	return (wcslen(pszPath) > 0);
}

BOOL gog_registry_installpath(LPWSTR pszPath)
{
	// EXE, PATH, WORKINGDIR
	read_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\GOG.com\\Games\\1438332414", L"WORKINGDIR", pszPath);
	return (wcslen(pszPath) > 0);
}

BOOL gog_registry_language(LPWSTR pszLanguageCode, LPWSTR pszLocalized)
{
	WCHAR szLanguage[MAX_STRING_LENGTH] = L"";

	read_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\GOG.com\\Games\\1438332414", L"LANGUAGE", szLanguage);

	if (wcslen(szLanguage) > 0)
	{
		if (wcsstr(szLanguage, L"english"))
			wcscpy_s(pszLanguageCode, MAX_STRING_LENGTH, L"EN");
		else if (wcsstr(szLanguage, L"french"))
			wcscpy_s(pszLanguageCode, MAX_STRING_LENGTH, L"FR");
		else if (wcsstr(szLanguage, L"german"))
			wcscpy_s(pszLanguageCode, MAX_STRING_LENGTH, L"DE");
		else if (wcsstr(szLanguage, L"italian"))
			wcscpy_s(pszLanguageCode, MAX_STRING_LENGTH, L"IT");
		else if (wcsstr(szLanguage, L"spanish"))
			wcscpy_s(pszLanguageCode, MAX_STRING_LENGTH, L"ES");
		else
			wcscpy_s(pszLanguageCode, MAX_STRING_LENGTH, L"EN");

		wcscpy_s(pszLocalized, MAX_STRING_LENGTH, szLanguage);
		pszLocalized[0] = toupper(pszLocalized[0]);
	}

	return ((wcslen(pszLanguageCode)) > 0 && (wcslen(pszLocalized) > 0));
}

BOOL wic_version_1011(LPCWSTR pszPath)
{
	char readBuffer[59] = "";
	DWORD readLength = 59;
	DWORD bytesRead = 0;

	HANDLE hFile = CreateFile(pszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	SetFilePointer(hFile, 9597836, NULL, FILE_BEGIN);

	if (!ReadFile(hFile, readBuffer, readLength - 1, &bytesRead, NULL))
		return FALSE;

	CloseHandle(hFile);

	if (strcmp(readBuffer, "henrik.davidsson/MSV-BUILD-04 at 10:51:42 on Jun 10 2009.\n") != 0)
		return FALSE;

	return TRUE;
}

BOOL wic_ds_version_1011(LPCWSTR pszPath)
{
	char readBuffer[59] = "";
	DWORD readLength = 59;
	DWORD bytesRead = 0;

	HANDLE hFile = CreateFile(pszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	SetFilePointer(hFile, 3488868, NULL, FILE_BEGIN);

	if (!ReadFile(hFile, readBuffer, readLength - 1, &bytesRead, NULL))
		return FALSE;

	CloseHandle(hFile);

	if (strcmp(readBuffer, "henrik.davidsson/MSV-BUILD-04 at 10:57:07 on Jun 10 2009.\n") != 0)
		return FALSE;

	return TRUE;
}

BOOL strip_zone_identifier(LPCWSTR pszPath)
{
	// removes the zone identifier from the file, i.e. click unblock button in the file property sheet

	HRESULT lResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if (SUCCEEDED(lResult))
	{
		IPersistFile* pFile = NULL;
		lResult = CoCreateInstance(CLSID_PersistentZoneIdentifier, NULL, CLSCTX_INPROC_SERVER, IID_IPersistFile, (void**)&pFile);

		if (SUCCEEDED(lResult))
		{
			IZoneIdentifier* pZoneId = NULL;
			lResult = CoCreateInstance(CLSID_PersistentZoneIdentifier, NULL, CLSCTX_INPROC_SERVER, IID_IZoneIdentifier, (void**)&pZoneId);

			if (SUCCEEDED(lResult))
			{
				lResult = pFile->QueryInterface(__uuidof(IZoneIdentifier), (void**)&pZoneId);

				if (SUCCEEDED(lResult))
				{
					pZoneId->Remove();
					pFile->Save(pszPath, TRUE);
				}

				pZoneId->Release();
			}

			pFile->Release();
		}

		CoUninitialize();
	}

	return TRUE;
}

BOOL remove_msi_installer(HWND hWnd, LPCWSTR pszPath)
{
	WCHAR szDisplayName[MAX_STRING_LENGTH] = L"";
	WCHAR szDisplayVersion[MAX_STRING_LENGTH] = L"";

	if (!read_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{8DA9EDCA-5977-42F1-937C-58AF5A79FABD}", L"DisplayName", szDisplayName)
		|| !read_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{8DA9EDCA-5977-42F1-937C-58AF5A79FABD}", L"DisplayVersion", szDisplayVersion))
		return FALSE;

	if (wcsstr(szDisplayName, L"World in Conflict: Multiplayer Fix") && wcsstr(szDisplayVersion, L"1.0.0"))
	{
		HRESULT lResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (SUCCEEDED(lResult))
		{
			//ShellExecute(hWnd, L"open", L"msiexec", L"/x {8DA9EDCA-5977-42F1-937C-58AF5A79FABD} /quiet", NULL, SW_HIDE);

			SHELLEXECUTEINFO pse;
			memset(&pse, 0, sizeof(pse));

			pse.cbSize = sizeof(SHELLEXECUTEINFO);
			pse.fMask = SEE_MASK_NOCLOSEPROCESS;
			pse.hwnd = hWnd;
			pse.lpVerb = L"open";
			pse.lpFile = L"msiexec";
			pse.lpParameters = L"/x {8DA9EDCA-5977-42F1-937C-58AF5A79FABD} /quiet";
			pse.lpDirectory = NULL;
			pse.nShow = SW_HIDE;
			pse.hInstApp = NULL;

			ShellExecuteEx(&pse);
			WaitForSingleObject(pse.hProcess, INFINITE);
			CloseHandle(pse.hProcess);

			CoUninitialize();
		}
	}

	// TODO: undo anything the msi installer did
	//file_delete(pszPath + L"dbghelp.dll.bak");
	//folder_delete(pszPath + "Backup")

	return TRUE;
}