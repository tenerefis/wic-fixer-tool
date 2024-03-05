#pragma once

#define MAX_WIDTH				500			// max window width
#define MAX_HEIGHT				400			// max window height

#define MAX_STRING_LENGTH		1024
#define MAX_CDKEY_LENGTH		25			// includes NUL character

#define TOTAL_WIC_PATHS			7
#define TOTAL_FIX_FILES			9
#define TOTAL_MAPS				7

struct MAPSTRUCT
{
	WCHAR mapname[MAX_PATH];
	bool installed;
};

// log window
BOOL set_log_window_handle(HWND hWnd);
BOOL log_window(const std::wstring szLine);

// product version & update
BOOL get_product_version(LPWSTR pszValue);
BOOL update_available();

// Win32 api wrappers
HWND create_button(LPCWSTR szCaption, int x, int y, int width, int height, HWND hWnd, UINT ID, LONG DEFPUSHBUTTON = BS_PUSHBUTTON);
HWND create_checkbox(LPCWSTR szCaption, int x, int y, int width, int height, HWND hWnd, UINT ID);
HWND create_radio(LPCWSTR szCaption, int x, int y, int width, int height, HWND hWnd, UINT ID);
HWND create_groupbox(LPCWSTR szCaption, int x, int y, int width, int height, HWND hWnd, UINT ID);
HWND create_edit(int x, int y, int width, int height, HWND hWnd, UINT ID);
HWND create_edit_ml(int x, int y, int width, int height, HWND hWnd, UINT ID);
HWND create_static(LPCWSTR szCaption, int x, int y, int width, int height, HWND hWnd, UINT ID);
HWND create_progressbar(int x, int y, int width, int height, HWND hWnd, UINT ID);

// system folders
HRESULT get_system_drive(LPWSTR pszPath);
HRESULT get_program_files(LPWSTR pszPath);
HRESULT get_my_documents(LPWSTR pszPath);

// file i/o
BOOL file_exists(LPCWSTR pszFile);
BOOL folder_exists(LPCWSTR pszPath);
BOOL folder_create(LPCWSTR pszPath);
BOOL file_copy(LPCWSTR pszPathSrc, LPCWSTR pszPathDest);
BOOL file_delete(LPCWSTR pszPath);

// registry
DWORD read_reg_dword(HKEY hKeyParent, LPCWSTR lpSubKey, LPCWSTR lpValue);
BOOL read_reg_wstring(HKEY hKeyParent, LPCWSTR lpSubKey, LPCWSTR lpValue, LPWSTR pszData);
BOOL write_reg_dword(HKEY hKeyParent, LPCWSTR lpSubKey, LPCWSTR lpValue, DWORD dwData);
BOOL write_reg_wstring(HKEY hKeyParent, LPCWSTR lpSubKey, LPCWSTR lpValue, LPCWSTR pszData);
BOOL delete_reg_key(HKEY hKeyParent, LPCWSTR lpSubKey);

// wic cd key
BOOL wic_read_cdkey(LPWSTR pszCDkey);
BOOL wic_write_cdkey(LPCWSTR pszCDkey);

// wic registry install info
BOOL wic_registry_installexepath(LPWSTR pszPath);
BOOL wic_registry_installpath(LPWSTR pszPath);
BOOL wic_registry_version(LPWSTR pszPath);
BOOL wic_registry_complete();

// gog registry install info
BOOL gog_registry_installexepath(LPWSTR pszPath);
BOOL gog_registry_installpath(LPWSTR pszPath);
BOOL gog_registry_language(LPWSTR pszLanguageCode, LPWSTR pszLocalized);

// wic version check
BOOL wic_version_1011(LPCWSTR pszPath);
BOOL wic_ds_version_1011(LPCWSTR pszPath);

// questionable things
BOOL strip_zone_identifier(LPCWSTR);

// old dumb thing that should never have been made
BOOL remove_msi_installer(HWND hWnd, LPCWSTR pszPath);