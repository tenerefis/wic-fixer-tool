#include "stdafx.h"
#include "resource.h"

#define MAX_LOADSTRING 100

enum ApplicationStateEnum : int
{
	DEFAULT,
	BROWSE_FOLDER,
	READY,
	NOT_INSTALLED,
	WORKING,
	INSTALLED
};

enum
{
	MPFIXINSTALLED = 0x0001,
	MAPSINSTALLED = 0x0002,
	TXTINSTALLED = 0x0004
	//DX10FLAG = 0x0008
};

enum
{
	DBGHELP_DLL,
	DBGHELP_OLD_DLL,
	WIC_CL_HOOK_DLL,
	WIC_DS_HOOK_DLL,
	WIC_BT_HOOK_DLL,
	WIC_EXE,
	WIC_ONLINE_EXE,
	WICAUTOEXEC_TXT
};

const WCHAR *szWicFixFiles[8] =
{
	L"dbghelp.dll",
	L"dbghelp_old.dll",
	L"wic_cl_hook.dll",
	L"wic_ds_hook.dll",
	L"wic_bt_hook.dll",
	L"wic.exe",
	L"wic_online.exe",
	L"wicautoexec.txt"
};

BOOL bMapTxtFilePresent = FALSE;
BOOL bWicAutoexecPresent = FALSE;
ULONGLONG ullTotalBytesTransferred = 0;
ULONGLONG ullLastBytesTransferred = 0;
ULONGLONG ullTotalSizeOfAllMaps = 0;

// map list
MAPSTRUCT szMapList[TOTAL_MAPS];

// string buffers for created paths
WCHAR szKnownPaths[5][MAX_PATH] = { L"", L"", L"", L"", L"" };
WCHAR szDrive[MAX_PATH] = L"";
WCHAR szProgramFiles[MAX_PATH] = L"";
WCHAR szMyDocuments[MAX_PATH] = L"";

// wicfix internal settings
WIC_Settings mySettings;

// TODO: ShellExecute URLS, put these in the string table when MUI is implemented
const WCHAR *szWicFixHelpURL = L"https://www.massgate.org/wicfix/redirect_help_page.php";
const WCHAR *szWicFixUpdateURL = L"https://www.massgate.org/wicfix/redirect_update_available.php";
const WCHAR *szWicGamePatchesURL = L"https://www.massgate.org/wicfix/redirect_patch_download.php";

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
WCHAR szMnuFile[MAX_LOADSTRING];
WCHAR szMnuItmChangeKey[MAX_LOADSTRING];
WCHAR szMnuItmExit[MAX_LOADSTRING];
WCHAR szMnuHelp[MAX_LOADSTRING];
WCHAR szMnuItmViewHelp[MAX_LOADSTRING];
WCHAR szMnuItmUpdate[MAX_LOADSTRING];
WCHAR szMnuItmAbout[MAX_LOADSTRING];

WCHAR szLblWicDir[MAX_LOADSTRING];
WCHAR szBtnBrowse[MAX_LOADSTRING];
WCHAR szFolderDialogTitle[MAX_LOADSTRING];
WCHAR szGroupBox[MAX_LOADSTRING];
WCHAR szStatusOK[MAX_LOADSTRING];
WCHAR szStatusBad[MAX_LOADSTRING];
WCHAR szInstallMPFix[MAX_LOADSTRING];
WCHAR szInstallMaps[MAX_LOADSTRING];
WCHAR szInstallTxt[MAX_LOADSTRING];
WCHAR szDXFlag[MAX_LOADSTRING];
WCHAR szBtnInstall[MAX_LOADSTRING];
WCHAR szBtnUnInstall[MAX_LOADSTRING];
WCHAR szLblLog[MAX_LOADSTRING];

WCHAR szMsgBoxCaptionError[MAX_LOADSTRING];
WCHAR szMsgBoxCaptionInformation[MAX_LOADSTRING];
WCHAR szMsgBoxCaptionQuestion[MAX_LOADSTRING];
WCHAR szMsgBoxGameFolderNotFound[MAX_LOADSTRING];
WCHAR szMsgBoxUninstallConfirmation[MAX_LOADSTRING];
WCHAR szMsgBoxHelpPageConfirmation[MAX_LOADSTRING];
WCHAR szMsgBoxUpdateAvailable[MAX_LOADSTRING];
WCHAR szMsgBoxNoUpdateAvailable[MAX_LOADSTRING];
WCHAR szMsgBoxIncorrectGameVersion[MAX_LOADSTRING];
WCHAR szMsgBoxCDKeyNotSaved[MAX_LOADSTRING];
WCHAR szMsgBoxCDKeyBadLength[MAX_LOADSTRING];

WCHAR szCDKeyDlgTitle[MAX_LOADSTRING];
WCHAR szCDKeyDlgBtnSave[MAX_LOADSTRING];
WCHAR szCDKeyDlgBtnCancel[MAX_LOADSTRING];

WCHAR szAboutDlgTitle[MAX_LOADSTRING];
WCHAR szAboutDlgLblAbout[MAX_LOADSTRING];
WCHAR szAboutDlgLblCopyright[MAX_LOADSTRING];
WCHAR szAboutDlgBtnOK[MAX_LOADSTRING];

// Global handles
HWND hWndTxtWicDir;
HWND hWndBtnBrowse;

HWND hWndChkInstallMPFix;
HWND hWndChkInstallMaps;
HWND hWndChkInstallTxt;
HWND hWndChkDXFlag;

HWND hWndLblInstallMPFixDone;
HWND hWndLblInstallMapsDone;
HWND hWndLblInstallTxtDone;
HWND hWndLblInstallMPFixNotDone;
HWND hWndLblInstallMapsNotDone;
HWND hWndLblInstallTxtNotDone;

HWND hWndBtnInstall;
HWND hWndBtnUnInstall;

HWND hWndProgressBar;

HWND hWndLogWindow;

// default window process pointer for returning from captured keypresses in the cd key dialog
WNDPROC DefCDKeyProc[5];

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

BOOL				SetApplicationState(ApplicationStateEnum, DWORD);
BOOL				LoadSettings(HWND);
BOOL				BuildSystemPaths();
BOOL				ReadMapFile();
BOOL				FindGameFolder(LPWSTR);
BOOL				HelpPage(HWND);
BOOL				UpdateCheck(HWND);
BOOL				FileOpenDialog(HWND, LPWSTR);
BOOL				BrowseFolderDialog(HWND, LPWSTR);
BOOL				InstallFixes(HWND);
BOOL				UninstallFixes();

DWORD CALLBACK		CopyProgressRoutine(LARGE_INTEGER, LARGE_INTEGER, LARGE_INTEGER, LARGE_INTEGER, DWORD, DWORD, HANDLE, HANDLE, LPVOID);
VOID				ShowProgressbar() { ShowWindow(hWndProgressBar, SW_SHOWNORMAL); ullTotalBytesTransferred = 0; };
VOID				HideProgressbar() { ShowWindow(hWndProgressBar, SW_HIDE); ullTotalBytesTransferred = 0; };

INT_PTR CALLBACK	CDKeyDialog(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

BOOL				CreateFormObjects(HWND);
BOOL CALLBACK		EnumChildProc(HWND, LPARAM);
BOOL				SetCDKeyDialogProperties(HWND);
BOOL				SetAboutDialogProperties(HWND);

LRESULT CALLBACK	CDKey1EditDown(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	CDKey2EditDown(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	CDKey3EditDown(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	CDKey4EditDown(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	CDKey5EditDown(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					_In_opt_ HINSTANCE hPrevInstance,
					_In_ LPWSTR		lpCmdLine,
					_In_ int		nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	// Main application window
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WICFIX, szWindowClass, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MNUFILE, szMnuFile, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MNUITMCHANGEKEY, szMnuItmChangeKey, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MNUITMEXIT, szMnuItmExit, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MNUHELP, szMnuHelp, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MNUITMVIEWHELP, szMnuItmViewHelp, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MNUITMUPDATE, szMnuItmUpdate, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MNUITMABOUT, szMnuItmAbout, MAX_LOADSTRING);

	// main window strings
	LoadStringW(hInstance, IDS_LBLWICDIR, szLblWicDir, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_BTNBROWSE, szBtnBrowse, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_FOLDERDLG_TITLE, szFolderDialogTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_GROUPBOX, szGroupBox, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_LBLSTATUSOK, szStatusOK, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_LBLSTATUSBAD, szStatusBad, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_CHKINSTALLMPFIX, szInstallMPFix, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_CHKINSTALLMAPS, szInstallMaps, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_CHKINSTALLTXT, szInstallTxt, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_CHKDXFLAG, szDXFlag, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_BTNINSTALL, szBtnInstall, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_BTNUNINSTALL, szBtnUnInstall, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_LBLLOG, szLblLog, MAX_LOADSTRING);

	// messagebox strings
	LoadStringW(hInstance, IDS_MSGBOX_CAPTION_ERROR, szMsgBoxCaptionError, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MSGBOX_CAPTION_INFO, szMsgBoxCaptionInformation, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MSGBOX_CAPTION_QUESTION, szMsgBoxCaptionQuestion, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MSGBOX_GAMEFOLDER_NOTFOUND, szMsgBoxGameFolderNotFound, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MSGBOX_UNINSTALL_CONFIRMATION, szMsgBoxUninstallConfirmation, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MSGBOX_HELPPAGE_CONFIRMATION, szMsgBoxHelpPageConfirmation, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MSGBOX_UPDATE_AVAILABLE, szMsgBoxUpdateAvailable, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MSGBOX_NO_UPDATE_AVAILABLE, szMsgBoxNoUpdateAvailable, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MSGBOX_INCORRECT_GAME_VERSION, szMsgBoxIncorrectGameVersion, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MSGBOX_CDKEY_NOTSAVED, szMsgBoxCDKeyNotSaved, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_MSGBOX_CDKEY_BADLENGTH, szMsgBoxCDKeyBadLength, MAX_LOADSTRING);

	// cdkey entry dialog
	LoadStringW(hInstance, IDS_CDKEYDLG_TITLE, szCDKeyDlgTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_CDKEYDLG_BTNSAVE, szCDKeyDlgBtnSave, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_CDKEYDLG_BTNCANCEL, szCDKeyDlgBtnCancel, MAX_LOADSTRING);

	// about dialog
	LoadStringW(hInstance, IDS_ABOUTDLG_TITLE, szAboutDlgTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_ABOUTDLG_LBLABOUT, szAboutDlgLblAbout, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_ABOUTDLG_LBLCOPY, szAboutDlgLblCopyright, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_ABOUTDLG_BTNOK, szAboutDlgBtnOK, MAX_LOADSTRING);

	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WICFIX));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	
	wcex.cbSize = sizeof(WNDCLASSEX);
	
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WICFIX));
	wcex.hCursor		= LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground	= GetSysColorBrush(COLOR_3DFACE);
	wcex.lpszMenuName	= MAKEINTRESOURCEW(IDC_WICFIX);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	
	return RegisterClassExW(&wcex);
}

//
//  FUNCTION: InitInstance(HINSTANCE, int)
//
//  PURPOSE: Saves instance handle and creates main window
//
//  COMMENTS:
//
//		In this function, we save the instance handle in a global variable and
//		create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowEx(WS_EX_CONTROLPARENT, szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, 0, MAX_WIDTH, MAX_HEIGHT, nullptr, nullptr, hInstance, nullptr);
	
	if (!hWnd)
	{
		return FALSE;
	}
	
	// center the window
	int xPos = (GetSystemMetrics(SM_CXSCREEN) - MAX_WIDTH) / 2;
	int yPos = (GetSystemMetrics(SM_CYSCREEN) - MAX_HEIGHT) / 2;
	SetWindowPos(hWnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			// create the form objects and populate them with wic data
			if (!CreateFormObjects(hWnd))
				return DefWindowProc(hWnd, message, wParam, lParam);
			
			LoadSettings(hWnd);
			
			if (!mySettings.isFixInstalled())
			{
				WCHAR szGameDirectory[MAX_PATH] = L"";

				if (!FindGameFolder(szGameDirectory))
				{
					log_window(L"Game folder not found.");
					SetApplicationState(BROWSE_FOLDER, 0);
				}
				else
				{
					log_window(L"Found: " + std::wstring(szGameDirectory));
					SetWindowText(hWndTxtWicDir, szGameDirectory);
					SetApplicationState(READY, 0);
				}
			}
			else
			{
				DWORD dwFlag = 0;

				if (mySettings.mpFixInstalled)
					dwFlag |= MPFIXINSTALLED;

				if (mySettings.mapsInstalled)
					dwFlag |= MAPSINSTALLED;

				if (mySettings.wicautoexecInstalled)
					dwFlag |= TXTINSTALLED;

				log_window(L"Multiplayer Fix has been applied.");
				SetWindowText(hWndTxtWicDir, mySettings.myInstallPath);
				SetApplicationState(INSTALLED, dwFlag);
			}

			return 0;
		}
		break;
		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			switch (wmId)
			{
			case IDM_CHANGECDKEY:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_CDKEYBOX), hWnd, CDKeyDialog);
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			case IDM_VIEWHELP:
				HelpPage(hWnd);
				break;
			case IDM_UPDATE:
				UpdateCheck(hWnd);
				break;
			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case ID_BTNBROWSE:
			{
				WCHAR szSelectedPath[MAX_PATH] = L"";
				WCHAR szCheckPath[MAX_PATH] = L"";

				if (BrowseFolderDialog(hWnd, szSelectedPath))
				{
					wcscpy_s(szCheckPath, szSelectedPath);
					wcscat_s(szCheckPath, L"wic.exe");

					if (!file_exists(szCheckPath))
					{
						MessageBox(hWnd, szMsgBoxGameFolderNotFound, szMsgBoxCaptionError, MB_OK | MB_ICONERROR);
						SetWindowText(hWndTxtWicDir, L"");
						SetApplicationState(BROWSE_FOLDER, 0);
					}
					else
					{
						log_window(L"Found: " + std::wstring(szSelectedPath));
						SetWindowText(hWndTxtWicDir, szSelectedPath);
						SetApplicationState(READY, 0);
					}
				}
			}
			break;
			case ID_BTNINSTALL:
			{
				SetApplicationState(WORKING, 0);
				ShowProgressbar();
				InstallFixes(hWnd);

				DWORD dwFlag = 0;

				if (mySettings.mpFixInstalled)
					dwFlag |= MPFIXINSTALLED;

				if (mySettings.mapsInstalled)
					dwFlag |= MAPSINSTALLED;

				if (mySettings.wicautoexecInstalled)
					dwFlag |= TXTINSTALLED;

				log_window(L"Multiplayer Fix has been applied.");
				HideProgressbar();
				SetApplicationState(INSTALLED, dwFlag);
			}
			break;
			case ID_BTNUNINSTALL:
			{
				int iResult = MessageBox(hWnd, szMsgBoxUninstallConfirmation, szMsgBoxCaptionQuestion, MB_OKCANCEL | MB_ICONQUESTION);
				if (iResult == IDOK)
				{
					SetApplicationState(WORKING, 0);
					UninstallFixes();
					log_window(L"Multiplayer Fix has been removed.");
					SetApplicationState(NOT_INSTALLED, 0);
				}
			}
			break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;
		case WM_CTLCOLORSTATIC:
		{
			int id = GetDlgCtrlID(HWND(lParam));
			switch (id)
			{
				case ID_LBLINSTALLMPFIXDONE:
					SetBkMode((HDC)wParam, TRANSPARENT);
					SetTextColor((HDC)wParam, RGB(0, 200, 0));
					return (LRESULT)GetSysColorBrush(COLOR_MENU);
					break;
				case ID_LBLINSTALLMAPSDONE:
					SetBkMode((HDC)wParam, TRANSPARENT);
					SetTextColor((HDC)wParam, RGB(0, 200, 0));
					return (LRESULT)GetSysColorBrush(COLOR_MENU);
					break;
				case ID_LBLINSTALLTXTDONE:
					SetBkMode((HDC)wParam, TRANSPARENT);
					SetTextColor((HDC)wParam, RGB(0, 200, 0));
					return (LRESULT)GetSysColorBrush(COLOR_MENU);
					break;
				case ID_LBLINSTALLMPFIXNOTDONE:
					SetBkMode((HDC)wParam, TRANSPARENT);
					SetTextColor((HDC)wParam, RGB(200, 0, 0));
					return (LRESULT)GetSysColorBrush(COLOR_MENU);
					break;
				case ID_LBLINSTALLMAPSNOTDONE:
					SetBkMode((HDC)wParam, TRANSPARENT);
					SetTextColor((HDC)wParam, RGB(200, 0, 0));
					return (LRESULT)GetSysColorBrush(COLOR_MENU);
					break;
				case ID_LBLINSTALLTXTNOTDONE:
					SetBkMode((HDC)wParam, TRANSPARENT);
					SetTextColor((HDC)wParam, RGB(200, 0, 0));
					return (LRESULT)GetSysColorBrush(COLOR_MENU);
					break;
			}
			// apparantly the return value is supposed to be a HBRUSH
			// but this was the only way to fix the blurry text in the log window
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

BOOL SetApplicationState(ApplicationStateEnum iState, DWORD dwFlag)
{
	switch (iState)
	{
		case DEFAULT:
		{
			EnableWindow(hWndTxtWicDir, FALSE);
			EnableWindow(hWndBtnBrowse, FALSE);
			EnableWindow(hWndChkInstallMPFix, FALSE);
			EnableWindow(hWndChkInstallMaps, FALSE);
			EnableWindow(hWndChkInstallTxt, FALSE);
			//EnableWindow(hWndChkDXFlag, FALSE);
			EnableWindow(hWndBtnInstall, FALSE);
			EnableWindow(hWndBtnUnInstall, FALSE);

			SendMessage(hWndChkInstallMPFix, BM_SETCHECK, BST_CHECKED, 0);
			SendMessage(hWndChkInstallMaps, BM_SETCHECK, BST_CHECKED, 0);
			SendMessage(hWndChkInstallTxt, BM_SETCHECK, BST_CHECKED, 0);
			//SendMessage(hWndChkDXFlag, BM_SETCHECK, BST_UNCHECKED, 0);
		}
		break;
		case BROWSE_FOLDER:
		{
			EnableWindow(hWndTxtWicDir, TRUE);
			EnableWindow(hWndBtnBrowse, TRUE);
			EnableWindow(hWndChkInstallMPFix, FALSE);
			EnableWindow(hWndChkInstallMaps, TRUE);
			EnableWindow(hWndChkInstallTxt, TRUE);
			EnableWindow(hWndBtnInstall, FALSE);
			EnableWindow(hWndBtnUnInstall, FALSE);

			//SendMessage(hWndChkInstallMPFix, BM_SETCHECK, BST_CHECKED, 0);
			//SendMessage(hWndChkInstallMaps, BM_SETCHECK, BST_CHECKED, 0);
			//SendMessage(hWndChkInstallTxt, BM_SETCHECK, BST_CHECKED, 0);
		}
		break;
		case READY:
		{
			EnableWindow(hWndTxtWicDir, TRUE);
			EnableWindow(hWndBtnBrowse, TRUE);
			EnableWindow(hWndChkInstallMPFix, FALSE);
			EnableWindow(hWndChkInstallMaps, TRUE);
			EnableWindow(hWndChkInstallTxt, TRUE);
			EnableWindow(hWndBtnInstall, TRUE);
			EnableWindow(hWndBtnUnInstall, FALSE);

			//SendMessage(hWndChkInstallMPFix, BM_SETCHECK, BST_CHECKED, 0);
			//SendMessage(hWndChkInstallMaps, BM_SETCHECK, BST_CHECKED, 0);
			//SendMessage(hWndChkInstallTxt, BM_SETCHECK, BST_CHECKED, 0);
		}
		break;
		case NOT_INSTALLED:
		{
			EnableWindow(hWndTxtWicDir, TRUE);
			EnableWindow(hWndBtnBrowse, TRUE);
			EnableWindow(hWndChkInstallMPFix, FALSE);
			EnableWindow(hWndChkInstallMaps, TRUE);
			EnableWindow(hWndChkInstallTxt, TRUE);
			EnableWindow(hWndBtnInstall, TRUE);
			EnableWindow(hWndBtnUnInstall, FALSE);

			//SendMessage(hWndChkInstallMPFix, BM_SETCHECK, BST_CHECKED, 0);
			//SendMessage(hWndChkInstallMaps, BM_SETCHECK, BST_CHECKED, 0);
			//SendMessage(hWndChkInstallTxt, BM_SETCHECK, BST_CHECKED, 0);
		}
		break;
		case WORKING:
		{
			EnableWindow(hWndTxtWicDir, FALSE);
			EnableWindow(hWndBtnBrowse, FALSE);
			EnableWindow(hWndChkInstallMPFix, FALSE);
			EnableWindow(hWndChkInstallMaps, FALSE);
			EnableWindow(hWndChkInstallTxt, FALSE);
			EnableWindow(hWndBtnInstall, FALSE);
			EnableWindow(hWndBtnUnInstall, FALSE);
		}
		break;
		case INSTALLED:
		{
			EnableWindow(hWndTxtWicDir, FALSE);
			EnableWindow(hWndBtnBrowse, FALSE);
			EnableWindow(hWndChkInstallMPFix, FALSE);
			EnableWindow(hWndChkInstallMaps, FALSE);
			EnableWindow(hWndChkInstallTxt, FALSE);
			EnableWindow(hWndBtnInstall, FALSE);
			EnableWindow(hWndBtnUnInstall, TRUE);

			if (dwFlag & MPFIXINSTALLED)
			{
				SendMessage(hWndChkInstallMPFix, BM_SETCHECK, BST_CHECKED, 0);
				//ShowWindow(hWndLblInstallMPFixDone, SW_SHOWNORMAL);
				//ShowWindow(hWndLblInstallMPFixNotDone, SW_HIDE);
			}
			else
			{
				SendMessage(hWndChkInstallMPFix, BM_SETCHECK, BST_UNCHECKED, 0);
				//ShowWindow(hWndLblInstallMPFixDone, SW_HIDE);
				//ShowWindow(hWndLblInstallMPFixNotDone, SW_SHOWNORMAL);
			}

			if (dwFlag & MAPSINSTALLED)
			{
				SendMessage(hWndChkInstallMaps, BM_SETCHECK, BST_CHECKED, 0);
				//ShowWindow(hWndLblInstallMapsDone, SW_SHOWNORMAL);
				//ShowWindow(hWndLblInstallMapsNotDone, SW_HIDE);
			}
			else
			{
				SendMessage(hWndChkInstallMaps, BM_SETCHECK, BST_UNCHECKED, 0);
				//ShowWindow(hWndLblInstallMapsDone, SW_HIDE);
				//ShowWindow(hWndLblInstallMapsNotDone, SW_SHOWNORMAL);
			}

			if (dwFlag & TXTINSTALLED)
			{
				SendMessage(hWndChkInstallTxt, BM_SETCHECK, BST_CHECKED, 0);
				//ShowWindow(hWndLblInstallTxtDone, SW_SHOWNORMAL);
				//ShowWindow(hWndLblInstallTxtNotDone, SW_HIDE);
			}
			else
			{
				SendMessage(hWndChkInstallTxt, BM_SETCHECK, BST_UNCHECKED, 0);
				//ShowWindow(hWndLblInstallTxtDone, SW_HIDE);
				//ShowWindow(hWndLblInstallTxtNotDone, SW_SHOWNORMAL);
			}
		}
		break;
		default:
		{
		}
	}

	if (!bMapTxtFilePresent)
	{
		ShowWindow(hWndChkInstallMaps, SW_HIDE);
		SendMessage(hWndChkInstallMaps, BM_SETCHECK, BST_UNCHECKED, 0);
	}

	if (!bWicAutoexecPresent)
	{
		ShowWindow(hWndChkInstallTxt, SW_HIDE);
		SendMessage(hWndChkInstallTxt, BM_SETCHECK, BST_UNCHECKED, 0);
	}

	return TRUE;
}

BOOL LoadSettings(HWND hWnd)
{
	if (file_exists(L"data\\maps.txt"))
		bMapTxtFilePresent = TRUE;

	if (file_exists(L"data\\wicautoexec.txt"))
		bWicAutoexecPresent = TRUE;

	BuildSystemPaths();
	ReadMapFile();
	mySettings.Load();
	SetApplicationState(DEFAULT, 0);
	return TRUE;
}

BOOL BuildSystemPaths()
{
	HRESULT lResult = get_system_drive(szDrive);

	if (lResult != S_OK)
		return FALSE;

	lResult = get_program_files(szProgramFiles);

	if (lResult != S_OK)
		return FALSE;

	lResult = get_my_documents(szMyDocuments);

	if (lResult != S_OK)
		return FALSE;

	wcscat_s(szDrive, L"\\");
	wcscat_s(szProgramFiles, L"\\");
	wcscat_s(szMyDocuments, L"\\");

	return TRUE;
}

BOOL ReadMapFile()
{
	if (!bMapTxtFilePresent)
		return FALSE;

	std::wifstream mapFile(L"data\\maps.txt", std::wifstream::in);
	std::wstring line;

	int i = 0;
	while (std::getline(mapFile, line))
	{
		if (i < TOTAL_MAPS)
		{
			WCHAR szFilename[MAX_PATH] = L"";
			LARGE_INTEGER ullFileSize = { 0 };
			WIN32_FILE_ATTRIBUTE_DATA lpInfo;
			memset(&lpInfo, 0, sizeof(lpInfo));

			wcscpy_s(szMapList[i].mapname, MAX_PATH, line.c_str());

			wcscpy_s(szFilename, L"data\\");
			wcscat_s(szFilename, szMapList[i].mapname);

			GetFileAttributesEx(szFilename, GetFileExInfoStandard, &lpInfo);
			ullFileSize.HighPart = lpInfo.nFileSizeHigh;
			ullFileSize.LowPart = lpInfo.nFileSizeLow;

			ullTotalSizeOfAllMaps += ullFileSize.QuadPart;
			i++;
		}
	}

	return TRUE;
}

BOOL FindGameFolder(LPWSTR pszPath)
{
	BOOL bFound = FALSE;
	WCHAR szInstallExePath[MAX_PATH] = L"";
	WCHAR szInstallPath[MAX_PATH] = L"";
	WCHAR szVersion[MAX_STRING_LENGTH] = L"";

	// if the installpath is not in the registry, search known paths
	if (!wic_registry_installexepath(szInstallExePath)
		|| !wic_registry_installpath(szInstallPath)
		|| !wic_registry_version(szVersion))
	{
		log_window(L"Installation path not found in registry, searching known paths.");

		if (wcslen(szProgramFiles) > 0)
		{
			wcscpy_s(szKnownPaths[0], szProgramFiles);
			wcscpy_s(szKnownPaths[1], szProgramFiles);
			wcscpy_s(szKnownPaths[2], szProgramFiles);
			wcscpy_s(szKnownPaths[3], szProgramFiles);

			wcscat_s(szKnownPaths[0], L"Sierra Entertainment\\World in Conflict\\");
			wcscat_s(szKnownPaths[1], L"Ubisoft\\World in Conflict\\");
			wcscat_s(szKnownPaths[2], L"Ubisoft\\Ubisoft Game Launcher\\games\\World in Conflict\\");
			wcscat_s(szKnownPaths[3], L"Steam\\steamapps\\common\\World in Conflict\\");
		}
		else
		{
			// TODO: winxp?
			wcscpy_s(szKnownPaths[0], L"C:\\Program Files (x86)\\Sierra Entertainment\\World in Conflict\\");
			wcscpy_s(szKnownPaths[1], L"C:\\Program Files (x86)\\Ubisoft\\World in Conflict\\");
			wcscpy_s(szKnownPaths[2], L"C:\\Program Files (x86)\\Ubisoft\\Ubisoft Game Launcher\\games\\World in Conflict\\");
			wcscpy_s(szKnownPaths[3], L"C:\\Program Files (x86)\\Steam\\steamapps\\common\\World in Conflict\\");
		}

		/* \\GOG Galaxy\\Games\\World in Conflict - Complete Edition */
		if (wcslen(szDrive) > 0)
		{
			wcscpy_s(szKnownPaths[4], szDrive);
			wcscat_s(szKnownPaths[4], L"GOG Galaxy\\Games\\World in Conflict - Complete Edition\\");
		}
		else
		{
			wcscpy_s(szKnownPaths[4], L"C:\\GOG Galaxy\\Games\\World in Conflict - Complete Edition\\");
		}

		for (WCHAR* knownPath : szKnownPaths)
		{
			WCHAR szCheckExePath[MAX_PATH] = L"";

			wcscpy_s(szCheckExePath, knownPath);
			wcscat_s(szCheckExePath, L"wic.exe");

			if (file_exists(szCheckExePath))
			{
				//log_window(L"Found: " + std::wstring(knownPath));
				
				wcscpy_s(pszPath, MAX_PATH, knownPath);
				bFound = TRUE;
				break;
			}
		}
	}
	else
	{
		log_window(L"Installation path found in registry.");

		WCHAR szCheckExePath[MAX_PATH] = L"";

		wcscpy_s(szCheckExePath, szInstallPath);
		wcscat_s(szCheckExePath, L"wic.exe");

		if (file_exists(szCheckExePath))
		{
			wcscpy_s(pszPath, MAX_PATH, szInstallPath);
			bFound = TRUE;
		}
	}

	return bFound;
}

BOOL HelpPage(HWND hWnd)
{
	int iResult = MessageBox(hWnd, szMsgBoxHelpPageConfirmation, szMsgBoxCaptionQuestion, MB_OKCANCEL | MB_ICONQUESTION);
	if (iResult == IDOK)
	{
		HRESULT lResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (SUCCEEDED(lResult))
		{
			ShellExecute(NULL, L"open", szWicFixHelpURL, NULL, NULL, SW_SHOWNORMAL);
			CoUninitialize();
		}
	}

	return TRUE;
}

BOOL UpdateCheck(HWND hWnd)
{
	if (update_available())
	{
		int iResult = MessageBox(hWnd, szMsgBoxUpdateAvailable, szMsgBoxCaptionQuestion, MB_OKCANCEL | MB_ICONQUESTION);
		if (iResult == IDOK)
		{
			HRESULT lResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			if (SUCCEEDED(lResult))
			{
				ShellExecute(NULL, L"open", szWicFixUpdateURL, NULL, NULL, SW_SHOWNORMAL);
				CoUninitialize();
			}
		}
	}
	else
	{
		MessageBox(hWnd, szMsgBoxNoUpdateAvailable, szMsgBoxCaptionInformation, MB_OK | MB_ICONINFORMATION);
	}

	return TRUE;
}

BOOL FileOpenDialog(HWND hWnd, LPWSTR pszPath)
{
	BOOL bFound = FALSE;
	HRESULT lResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if (SUCCEEDED(lResult))
	{
		IFileDialog *pFileOpen = NULL;
		lResult = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_IFileDialog, (void**)&pFileOpen);
		
		if (SUCCEEDED(lResult))
		{
			DWORD dwFlags;

			if (SUCCEEDED(pFileOpen->GetOptions(&dwFlags)))
			{
				if (SUCCEEDED(pFileOpen->SetOptions(dwFlags | FOS_FORCEFILESYSTEM)))
				{
					COMDLG_FILTERSPEC fsFileTypes[] = { L"Application (*.exe)", L"*.exe" };

					if (SUCCEEDED(pFileOpen->SetFileTypes(ARRAYSIZE(fsFileTypes), fsFileTypes)))
					{
						if (SUCCEEDED(pFileOpen->SetFileTypeIndex(1)))
						{
							if (SUCCEEDED(pFileOpen->SetDefaultExtension(L"exe")))
							{
								if (SUCCEEDED(pFileOpen->Show(hWnd)))
								{
									IShellItem *pItem;

									if (SUCCEEDED(pFileOpen->GetResult(&pItem)))
									{
										LPWSTR pszFilePath;
										LPWSTR pszFileName;

										if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)) 
											&& SUCCEEDED(pItem->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &pszFileName)))
										{
											wcsncpy_s(pszPath, MAX_PATH, pszFilePath, wcslen(pszFilePath) - wcslen(pszFileName));
											bFound = TRUE;

											CoTaskMemFree(pszFilePath);
											CoTaskMemFree(pszFileName);
										}
										
										pItem->Release();
									}
								}
							}
						}
					}
				}
			}

			pFileOpen->Release();
		}
		else
		{
			WCHAR szFile[MAX_PATH] = L"";
			WCHAR szFileTitle[MAX_PATH] = L"";

			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = L"Application (*.exe)\0*.exe\0All Files (*.*)\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = szFileTitle;
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.lpstrInitialDir = NULL;
			ofn.lpstrTitle = NULL;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

			if (GetOpenFileName(&ofn))
			{
				wcsncpy_s(pszPath, MAX_PATH, szFile, ofn.nFileOffset);
				bFound = TRUE;
			}
		}

		CoUninitialize();
	}
	
	return bFound;
}

BOOL BrowseFolderDialog(HWND hWnd, LPWSTR pszPath)
{
	BOOL bFound = FALSE;
	HRESULT lResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if (SUCCEEDED(lResult))
	{
		IFileDialog *pFileOpen = NULL;
		lResult = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_IFileDialog, (void**)&pFileOpen);
		
		if (SUCCEEDED(lResult))
		{
			DWORD dwFlags;

			if (SUCCEEDED(pFileOpen->GetOptions(&dwFlags)))
			{
				if (SUCCEEDED(pFileOpen->SetOptions(dwFlags | FOS_PICKFOLDERS)))
				{
					if (SUCCEEDED(pFileOpen->Show(hWnd)))
					{
						IShellItem *pItem;

						if (SUCCEEDED(pFileOpen->GetResult(&pItem)))
						{
							LPWSTR pszFolderPath;

							if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath)))
							{
								wcscpy_s(pszPath, MAX_PATH, pszFolderPath);
								wcscat_s(pszPath, MAX_PATH, L"\\");
								bFound = TRUE;

								CoTaskMemFree(pszFolderPath);
							}

							pItem->Release();
						}
					}
				}
			}

			pFileOpen->Release();
		}
		else
		{
			WCHAR szFolderPath[MAX_PATH] = L"";

			BROWSEINFO bi;
			ZeroMemory(&bi, sizeof(bi));

			LPITEMIDLIST pidlRoot;

			if (FAILED(SHGetFolderLocation(hWnd, CSIDL_DRIVES, NULL, NULL, &pidlRoot)))
				pidlRoot = NULL;

			bi.hwndOwner = hWnd;
			bi.pidlRoot = pidlRoot;
			bi.pszDisplayName = NULL;
			bi.lpszTitle = szFolderDialogTitle;
			bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;

			LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

			if (pidl)
			{
				if (SHGetPathFromIDList(pidl, szFolderPath))
				{
					wcscpy_s(pszPath, MAX_PATH, szFolderPath);
					wcscat_s(pszPath, MAX_PATH, L"\\");
					bFound = TRUE;
				}

				CoTaskMemFree(pidl);
			}

			if (pidlRoot)
				CoTaskMemFree(pidlRoot);
		}

		CoUninitialize();
	}

	return bFound;
}

BOOL InstallFixes(HWND hWnd)
{
	WCHAR szGameDirectory[MAX_PATH] = L"";
	GetWindowText(hWndTxtWicDir, szGameDirectory, MAX_PATH);

	WCHAR szInstallExePath[MAX_PATH] = L"";
	WCHAR szInstallPath[MAX_PATH] = L"";
	WCHAR szVersion[MAX_STRING_LENGTH] = L"";

	wcscpy_s(szInstallExePath, szGameDirectory);
	wcscat_s(szInstallExePath, L"wic.exe");
	wcscpy_s(szInstallPath, szGameDirectory);

	if (!file_exists(szInstallExePath))
	{
		MessageBox(hWnd, szMsgBoxGameFolderNotFound, szMsgBoxCaptionError, MB_OK | MB_ICONERROR);
	}
	else
	{
		if (!wic_version_1011(szInstallExePath))
		{
			int iResult = MessageBox(hWnd, szMsgBoxIncorrectGameVersion, szMsgBoxCaptionError, MB_OKCANCEL | MB_ICONERROR);
			if (iResult == IDOK)
			{
				HRESULT lResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
				if (SUCCEEDED(lResult))
				{
					ShellExecute(NULL, L"open", szWicGamePatchesURL, NULL, NULL, SW_SHOWNORMAL);
					CoUninitialize();
				}
			}
		}
		else
		{
			//
			// remove .msi installer
			//
			remove_msi_installer(hWnd, szGameDirectory);

			//
			// copy patched client .dlls to game directory, overwrite existing files
			//
			WCHAR szInstallSrc[8][MAX_PATH];
			WCHAR szInstallDest[8][MAX_PATH];
			memset(szInstallSrc, 0, sizeof(szInstallSrc));
			memset(szInstallDest, 0, sizeof(szInstallDest));

			for (int i = DBGHELP_DLL; i <= WIC_ONLINE_EXE; i++)
			{
				wcscpy_s(szInstallSrc[i], L"data\\");
				wcscat_s(szInstallSrc[i], szWicFixFiles[i]);

				wcscpy_s(szInstallDest[i], szInstallPath);
				wcscat_s(szInstallDest[i], szWicFixFiles[i]);

				// should there be special cases for steam, gog and uplay?
				log_window(L"Copy: " + std::wstring(szWicFixFiles[i]) + L" to " + std::wstring(szInstallPath));
				file_copy(szInstallSrc[i], szInstallDest[i]);
			}

			mySettings.mpFixInstalled = true;

			//
			// copy maps to mydocuments
			//
			WCHAR szCreateMapsFolder[MAX_PATH];
			WCHAR szInstallMapsSrc[TOTAL_MAPS][MAX_PATH];
			WCHAR szInstallMapsDest[TOTAL_MAPS][MAX_PATH];
			memset(szCreateMapsFolder, 0, sizeof(szCreateMapsFolder));
			memset(szInstallMapsSrc, 0, sizeof(szInstallMapsSrc));
			memset(szInstallMapsDest, 0, sizeof(szInstallMapsDest));

			wcscpy_s(szCreateMapsFolder, szMyDocuments);
			wcscat_s(szCreateMapsFolder, L"World in Conflict\\Downloaded\\maps");

			if (SendMessage(hWndChkInstallMaps, BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				if (!folder_exists(szCreateMapsFolder))
					folder_create(szCreateMapsFolder);

				for (int i = 0; i < TOTAL_MAPS; i++)
				{
					wcscpy_s(szInstallMapsSrc[i], L"data\\");
					wcscat_s(szInstallMapsSrc[i], szMapList[i].mapname);

					wcscpy_s(szInstallMapsDest[i], szMyDocuments);
					wcscat_s(szInstallMapsDest[i], L"World in Conflict\\Downloaded\\maps\\");
					wcscat_s(szInstallMapsDest[i], szMapList[i].mapname);

					if (!file_exists(szInstallMapsSrc[i]) || !file_exists(szInstallMapsDest[i]))
					{
						log_window(L"Copy: " + std::wstring(szMapList[i].mapname) + L" to " + std::wstring(szCreateMapsFolder) + L"\\");
						CopyFileEx(szInstallMapsSrc[i], szInstallMapsDest[i], (LPPROGRESS_ROUTINE)CopyProgressRoutine, NULL, NULL, COPY_FILE_FAIL_IF_EXISTS);
						szMapList[i].installed = true;
					}
				}

				mySettings.mapsInstalled = true;
			}
			
			//
			// copy and open wicautoexec.txt
			//
			if (SendMessage(hWndChkInstallTxt, BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				wcscpy_s(szInstallSrc[WICAUTOEXEC_TXT], L"data\\");
				wcscat_s(szInstallSrc[WICAUTOEXEC_TXT], szWicFixFiles[WICAUTOEXEC_TXT]);

				wcscpy_s(szInstallDest[WICAUTOEXEC_TXT], szMyDocuments);
				wcscat_s(szInstallDest[WICAUTOEXEC_TXT], L"World in Conflict\\");
				wcscat_s(szInstallDest[WICAUTOEXEC_TXT], szWicFixFiles[WICAUTOEXEC_TXT]);
				
				if (!file_exists(szInstallDest[WICAUTOEXEC_TXT]))
				{
					log_window(L"Copy: " + std::wstring(szWicFixFiles[WICAUTOEXEC_TXT]) + L" to " + std::wstring(szMyDocuments) + L"World in Conflict\\");
					file_copy(szInstallSrc[WICAUTOEXEC_TXT], szInstallDest[WICAUTOEXEC_TXT]);
				}

				mySettings.wicautoexecInstalled = true;

				HRESULT lResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
				if (SUCCEEDED(lResult))
				{
					WCHAR szNotepadParams[MAX_PATH] = L"/A ";
					wcscat_s(szNotepadParams, szInstallDest[WICAUTOEXEC_TXT]);

					ShellExecute(NULL, L"open", L"notepad.exe", szNotepadParams, NULL, SW_SHOWNORMAL);
					CoUninitialize();
				}
			}

			//
			// fix for wiced and modkit installers
			//
			if (!wic_registry_version(szVersion))
			{
				write_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Massive Entertainment AB\\World in Conflict", L"Version", L"1.0.1.1");
				mySettings.registryFixed = true;
			}
			else
			{
				if (!wcsstr(szVersion, L"1.0.1.1"))
				{
					write_reg_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Massive Entertainment AB\\World in Conflict", L"Version", L"1.0.1.1");
					mySettings.registryFixed = true;
				}
			}

			//
			// delete serverfilters.mmg
			//
			WCHAR szServerFilters[MAX_PATH] = L"";

			wcscpy_s(szServerFilters, szMyDocuments);
			wcscat_s(szServerFilters, L"World in Conflict\\massgate\\serverfilters.mmg");
			file_delete(szServerFilters);

			//
			// save mySettings configuration
			//
			wcscpy_s(mySettings.myInstallExePath, szInstallExePath);
			wcscpy_s(mySettings.myInstallPath, szInstallPath);
			wcscpy_s(mySettings.myVersion, szVersion);
			
			mySettings.wicInstallDirFound = true;
			mySettings.Save();
		}
	}

	return TRUE;
}

BOOL UninstallFixes()
{
	WCHAR szFiles[5][MAX_PATH];
	memset(szFiles, 0, sizeof(szFiles));

	// only remove the .dlls from the game directory, any other modifications can be manually undone by the user
	wcscpy_s(szFiles[0], mySettings.myInstallPath);
	wcscpy_s(szFiles[1], mySettings.myInstallPath);
	wcscpy_s(szFiles[2], mySettings.myInstallPath);
	wcscpy_s(szFiles[3], mySettings.myInstallPath);
	wcscpy_s(szFiles[4], mySettings.myInstallPath);

	wcscat_s(szFiles[0], szWicFixFiles[DBGHELP_DLL]);
	wcscat_s(szFiles[1], szWicFixFiles[DBGHELP_OLD_DLL]);
	wcscat_s(szFiles[2], szWicFixFiles[WIC_CL_HOOK_DLL]);
	wcscat_s(szFiles[3], szWicFixFiles[WIC_DS_HOOK_DLL]);
	wcscat_s(szFiles[4], szWicFixFiles[WIC_BT_HOOK_DLL]);

	file_delete(szFiles[0]);
	file_copy(szFiles[1], szFiles[0]);
	file_delete(szFiles[1]);
	file_delete(szFiles[2]);
	file_delete(szFiles[3]);
	file_delete(szFiles[4]);

	mySettings.Delete();

	return TRUE;
}

DWORD CALLBACK CopyProgressRoutine(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred,
	DWORD dwStreamNumber, DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile, LPVOID lpData)
{
	ullTotalBytesTransferred += TotalBytesTransferred.QuadPart - ullLastBytesTransferred;
	ullLastBytesTransferred = TotalBytesTransferred.QuadPart;

	if (ullLastBytesTransferred == TotalFileSize.QuadPart)
		ullLastBytesTransferred = 0;
	
	double dblPercent = ((double)ullTotalBytesTransferred / (double)ullTotalSizeOfAllMaps) * 100;
	SendMessage(hWndProgressBar, PBM_SETPOS, (UINT)dblPercent, 0);
	
	return PROGRESS_CONTINUE;
}

INT_PTR CALLBACK CDKeyDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
		{
			// sets the window and button captions, and the textbox max lengths
			if (!SetCDKeyDialogProperties(hDlg))
				return (INT_PTR)FALSE;

			WCHAR szReadCDKey[MAX_CDKEY_LENGTH] = L"";

			// read cdkey from the registry, will return an empty string on any error
			if (!wic_read_cdkey(szReadCDKey))
				return (INT_PTR)FALSE;
			
			// if there is a cdkey populate the textboxes
			if (wcslen(szReadCDKey) == (MAX_CDKEY_LENGTH - 1))
			{
				WCHAR szCDKeys[5][5] =
				{
					{ szReadCDKey[0], szReadCDKey[1], szReadCDKey[2], szReadCDKey[3], '\0' },
					{ szReadCDKey[5], szReadCDKey[6], szReadCDKey[7], szReadCDKey[8], '\0' },
					{ szReadCDKey[10], szReadCDKey[11], szReadCDKey[12], szReadCDKey[13], '\0' },
					{ szReadCDKey[15], szReadCDKey[16], szReadCDKey[17], szReadCDKey[18], '\0' },
					{ szReadCDKey[20], szReadCDKey[21], szReadCDKey[22], szReadCDKey[23], '\0' }
				};

				SetDlgItemText(hDlg, ID_CDKEY1, szCDKeys[0]);
				SetDlgItemText(hDlg, ID_CDKEY2, szCDKeys[1]);
				SetDlgItemText(hDlg, ID_CDKEY3, szCDKeys[2]);
				SetDlgItemText(hDlg, ID_CDKEY4, szCDKeys[3]);
				SetDlgItemText(hDlg, ID_CDKEY5, szCDKeys[4]);
			}
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == ID_SAVE)
			{
				LRESULT lTextLength1 = SendMessage(GetDlgItem(hDlg, ID_CDKEY1), WM_GETTEXTLENGTH, NULL, NULL);
				LRESULT lTextLength2 = SendMessage(GetDlgItem(hDlg, ID_CDKEY2), WM_GETTEXTLENGTH, NULL, NULL);
				LRESULT lTextLength3 = SendMessage(GetDlgItem(hDlg, ID_CDKEY3), WM_GETTEXTLENGTH, NULL, NULL);
				LRESULT lTextLength4 = SendMessage(GetDlgItem(hDlg, ID_CDKEY4), WM_GETTEXTLENGTH, NULL, NULL);
				LRESULT lTextLength5 = SendMessage(GetDlgItem(hDlg, ID_CDKEY5), WM_GETTEXTLENGTH, NULL, NULL);

				// if cdkey is empty then clear the registry entry
				// if the length is correct, write it to the registry
				if ((lTextLength1 == 0 && lTextLength2 == 0 && lTextLength3 == 0 && lTextLength4 == 0 && lTextLength5 == 0) ||
					(lTextLength1 == 4 && lTextLength2 == 4 && lTextLength3 == 4 && lTextLength4 == 4 && lTextLength5 == 4))
				{
					WCHAR szWriteCDKey[5][5];
					memset(szWriteCDKey, 0, sizeof(szWriteCDKey));
					
					// read values from the textboxes
					GetDlgItemText(hDlg, ID_CDKEY1, szWriteCDKey[0], 5);
					GetDlgItemText(hDlg, ID_CDKEY2, szWriteCDKey[1], 5);
					GetDlgItemText(hDlg, ID_CDKEY3, szWriteCDKey[2], 5);
					GetDlgItemText(hDlg, ID_CDKEY4, szWriteCDKey[3], 5);
					GetDlgItemText(hDlg, ID_CDKEY5, szWriteCDKey[4], 5);
					
					WCHAR szCDKey[MAX_CDKEY_LENGTH] = L"";

					wcscpy_s(szCDKey, szWriteCDKey[0]);
					wcscat_s(szCDKey, L"-"); wcscat_s(szCDKey, szWriteCDKey[1]);
					wcscat_s(szCDKey, L"-"); wcscat_s(szCDKey, szWriteCDKey[2]);
					wcscat_s(szCDKey, L"-"); wcscat_s(szCDKey, szWriteCDKey[3]);
					wcscat_s(szCDKey, L"-"); wcscat_s(szCDKey, szWriteCDKey[4]);
					
					if (wcsstr(szCDKey, L"----"))
						memset(szCDKey, 0, sizeof(szCDKey));

					if (wcslen(szCDKey) == 0 || wcslen(szCDKey) == (MAX_CDKEY_LENGTH - 1))
					{
						if (!wic_write_cdkey(szCDKey))
						{
							MessageBox(hDlg, szMsgBoxCDKeyNotSaved, szMsgBoxCaptionError, MB_OK | MB_ICONERROR);
							return (INT_PTR)FALSE;
						}
					}
				}
				else
				{
					MessageBox(hDlg, szMsgBoxCDKeyBadLength, szMsgBoxCaptionError, MB_OK | MB_ICONERROR);
					return (INT_PTR)FALSE;
				}

				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			else
			{
				// advance focus to the next edit control when typing cdkey
				for (int ID_EditControl : { ID_CDKEY1, ID_CDKEY2, ID_CDKEY3, ID_CDKEY4 })
				{
					if (LOWORD(wParam) == ID_EditControl)
					{
						if (HIWORD(wParam) == EN_CHANGE)
						{
							LRESULT lTextLength = SendMessage(GetDlgItem(hDlg, ID_EditControl), WM_GETTEXTLENGTH, NULL, NULL);
							if (lTextLength == 4)
							{
								SetFocus(GetDlgItem(hDlg, ID_EditControl + 1));
								SendMessage(GetDlgItem(hDlg, ID_EditControl + 1), EM_SETSEL, 4, 4);
							}
						}
					}
				}

				if (LOWORD(wParam) == ID_CANCEL || LOWORD(wParam) == IDCANCEL)
				{
					EndDialog(hDlg, LOWORD(wParam));
					return (INT_PTR)TRUE;
				}

				return (INT_PTR)TRUE;
			}
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:

		if (!SetAboutDialogProperties(hDlg))
			return (INT_PTR)FALSE;

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == ID_OK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

BOOL CreateFormObjects(HWND hWnd)
{
	/* wic dir */
	create_static(szLblWicDir, 10, 10, 200, 20, hWnd, NULL);
	hWndTxtWicDir = create_edit(10, 30, 380, 20, hWnd, ID_TXTWICDIR);
	hWndBtnBrowse = create_button(szBtnBrowse, 395, 30, 80, 20, hWnd, ID_BTNBROWSE, BS_DEFPUSHBUTTON);
	
	/* install options */
	create_groupbox(szGroupBox, 10, 55, 465, 170, hWnd, NULL);
	hWndChkInstallMPFix = create_checkbox(szInstallMPFix, 20, 70, 200, 20, hWnd, ID_CHKINSTALLMPFIX);
	hWndChkInstallMaps = create_checkbox(szInstallMaps, 20, 90, 200, 20, hWnd, ID_CHKINSTALLMAPS);
	hWndChkInstallTxt = create_checkbox(szInstallTxt, 20, 110, 200, 20, hWnd, ID_CHKINSTALLTXT);
	//hWndChkDXFlag = create_checkbox(szDXFlag, 20, 130, 200, 20, hWnd, ID_CHKDXFLAG);

	/* status labels */
	hWndLblInstallMPFixDone = create_static(szStatusOK, 230, 70, 200, 20, hWnd, ID_LBLINSTALLMPFIXDONE);
	hWndLblInstallMapsDone = create_static(szStatusOK, 230, 90, 200, 20, hWnd, ID_LBLINSTALLMAPSDONE);
	hWndLblInstallTxtDone = create_static(szStatusOK, 230, 110, 200, 20, hWnd, ID_LBLINSTALLTXTDONE);
	hWndLblInstallMPFixNotDone = create_static(szStatusBad, 230, 70, 200, 20, hWnd, ID_LBLINSTALLMPFIXNOTDONE);
	hWndLblInstallMapsNotDone = create_static(szStatusBad, 230, 90, 200, 20, hWnd, ID_LBLINSTALLMAPSNOTDONE);
	hWndLblInstallTxtNotDone = create_static(szStatusBad, 230, 110, 200, 20, hWnd, ID_LBLINSTALLTXTNOTDONE);

	ShowWindow(hWndLblInstallMPFixDone, SW_HIDE);
	ShowWindow(hWndLblInstallMapsDone, SW_HIDE);
	ShowWindow(hWndLblInstallTxtDone, SW_HIDE);
	ShowWindow(hWndLblInstallMPFixNotDone, SW_HIDE);
	ShowWindow(hWndLblInstallMapsNotDone, SW_HIDE);
	ShowWindow(hWndLblInstallTxtNotDone, SW_HIDE);
	
	/* progress bar */
	hWndProgressBar = create_progressbar(20, 200, 275, 15, hWnd, ID_PROGRESSBAR);
	SendMessage(hWndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	SendMessage(hWndProgressBar, PBM_SETSTEP, 1, 0);
	HideProgressbar();

	hWndBtnInstall = create_button(szBtnInstall, 300, 195, 80, 20, hWnd, ID_BTNINSTALL, BS_PUSHBUTTON);
	hWndBtnUnInstall = create_button(szBtnUnInstall, 385, 195, 80, 20, hWnd, ID_BTNUNINSTALL, BS_PUSHBUTTON);
	
	/* log output window */
	create_static(szLblLog, 10, 230, 80, 20, hWnd, NULL);
	hWndLogWindow = create_edit_ml(10, 245, 465, 85, hWnd, ID_LOGWINDOW);
	set_log_window_handle(hWndLogWindow);

	// create font object to replace the default system font and apply it to all child windows
	LOGFONT lf;
	HFONT hFont;

	GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
	hFont = CreateFont(lf.lfHeight, lf.lfWidth,
		lf.lfEscapement, lf.lfOrientation, lf.lfWeight,
		lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet,
		lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality,
		lf.lfPitchAndFamily, lf.lfFaceName);

	// set font for all child windows
	EnumChildWindows(hWnd, EnumChildProc, (LPARAM)hFont);
	
	return TRUE;
}

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
	SendMessage(hWnd, WM_SETFONT, (WPARAM)lParam, TRUE);
	return TRUE;
}

BOOL SetCDKeyDialogProperties(HWND hWnd)
{
	HWND hWndCDKey[5];

	hWndCDKey[0] = GetDlgItem(hWnd, ID_CDKEY1);
	hWndCDKey[1] = GetDlgItem(hWnd, ID_CDKEY2);
	hWndCDKey[2] = GetDlgItem(hWnd, ID_CDKEY3);
	hWndCDKey[3] = GetDlgItem(hWnd, ID_CDKEY4);
	hWndCDKey[4] = GetDlgItem(hWnd, ID_CDKEY5);

	// set the dialog window caption
	SetWindowText(hWnd, szCDKeyDlgTitle);

	// set the maximum length of user input to four characters
	for (HWND hTemp : hWndCDKey)
		SendMessage(hTemp, EM_SETLIMITTEXT, 4, NULL);

	// capture WM_PASTE event for the first textbox so a cdkey can be pasted from the clipboard
	DefCDKeyProc[0] = (WNDPROC)SetWindowLongPtr(hWndCDKey[0], GWLP_WNDPROC, (LONG_PTR)CDKey1EditDown);

	// allow backspace to shift focus to other textboxes so the cdkey can be deleted easily
	DefCDKeyProc[1] = (WNDPROC)SetWindowLongPtr(hWndCDKey[1], GWLP_WNDPROC, (LONG_PTR)CDKey2EditDown);
	DefCDKeyProc[2] = (WNDPROC)SetWindowLongPtr(hWndCDKey[2], GWLP_WNDPROC, (LONG_PTR)CDKey3EditDown);
	DefCDKeyProc[3] = (WNDPROC)SetWindowLongPtr(hWndCDKey[3], GWLP_WNDPROC, (LONG_PTR)CDKey4EditDown);
	DefCDKeyProc[4] = (WNDPROC)SetWindowLongPtr(hWndCDKey[4], GWLP_WNDPROC, (LONG_PTR)CDKey5EditDown);
	
	// set button captions
	//SetWindowText(GetDlgItem(hWnd, ID_SAVE), szCDKeyDlgBtnSave);
	SetDlgItemText(hWnd, ID_SAVE, szCDKeyDlgBtnSave);
	SetDlgItemText(hWnd, ID_CANCEL, szCDKeyDlgBtnCancel);

	return TRUE;
}

BOOL SetAboutDialogProperties(HWND hWnd)
{
	SetWindowText(hWnd, szAboutDlgTitle);
	SetDlgItemText(hWnd, ID_LBLABOUT, szAboutDlgLblAbout);
	SetDlgItemText(hWnd, ID_LBLCOPY, szAboutDlgLblCopyright);
	SetDlgItemText(hWnd, ID_OK, szAboutDlgBtnOK);
	return TRUE;
}

LRESULT CALLBACK CDKey1EditDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_PASTE:
		{
			if (!IsClipboardFormatAvailable(CF_TEXT))
				return 0;

			if (!OpenClipboard(hWnd))
				return 0;

			HGLOBAL hGlbClipData = GetClipboardData(CF_TEXT);
			if (hGlbClipData != NULL)
			{
				char *sClipString = (char*)GlobalLock(hGlbClipData);
				if (sClipString != NULL)
				{
					HWND hDlg = GetParent(hWnd);

					std::string tmp(sClipString);
					std::wstring szCDKey(tmp.begin(), tmp.end());

					if (szCDKey.length() == (MAX_CDKEY_LENGTH - 1))
					{
						SetDlgItemText(hDlg, ID_CDKEY1, szCDKey.substr(0, 4).c_str());
						SetDlgItemText(hDlg, ID_CDKEY2, szCDKey.substr(5, 4).c_str());
						SetDlgItemText(hDlg, ID_CDKEY3, szCDKey.substr(10, 4).c_str());
						SetDlgItemText(hDlg, ID_CDKEY4, szCDKey.substr(15, 4).c_str());
						SetDlgItemText(hDlg, ID_CDKEY5, szCDKey.substr(20, 4).c_str());

						SetFocus(GetDlgItem(hDlg, ID_CDKEY5));
						SendMessage(GetDlgItem(hDlg, ID_CDKEY5), EM_SETSEL, 4, 4);
					}
				}
				GlobalUnlock(hGlbClipData);
			}
			CloseClipboard();
		}
		break;
		default:
			return CallWindowProc(DefCDKeyProc[0], hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK CDKey2EditDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_BACK:
				{
					LRESULT lTextLength = SendMessage(hWnd, WM_GETTEXTLENGTH, NULL, NULL);
					if (lTextLength <= 1)
					{
						HWND hDlg = GetParent(hWnd);
						HWND hWndEdit = GetDlgItem(hDlg, ID_CDKEY1);
						SetFocus(hWndEdit);
						SendMessage(hWndEdit, EM_SETSEL, 4, 4);
					}
				}
				break;
			}
		}
		default:
			return CallWindowProc(DefCDKeyProc[1], hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK CDKey3EditDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_BACK:
				{
					LRESULT lTextLength = SendMessage(hWnd, WM_GETTEXTLENGTH, NULL, NULL);
					if (lTextLength <= 1)
					{
						HWND hDlg = GetParent(hWnd);
						HWND hWndEdit = GetDlgItem(hDlg, ID_CDKEY2);
						SetFocus(hWndEdit);
						SendMessage(hWndEdit, EM_SETSEL, 4, 4);
					}
				}
				break;
			}
		}
		default:
			return CallWindowProc(DefCDKeyProc[2], hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK CDKey4EditDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_BACK:
				{
					LRESULT lTextLength = SendMessage(hWnd, WM_GETTEXTLENGTH, NULL, NULL);
					if (lTextLength <= 1)
					{
						HWND hDlg = GetParent(hWnd);
						HWND hWndEdit = GetDlgItem(hDlg, ID_CDKEY3);
						SetFocus(hWndEdit);
						SendMessage(hWndEdit, EM_SETSEL, 4, 4);
					}
				}
				break;
			}
		}
		default:
			return CallWindowProc(DefCDKeyProc[3], hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK CDKey5EditDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_BACK:
				{
					LRESULT lTextLength = SendMessage(hWnd, WM_GETTEXTLENGTH, NULL, NULL);
					if (lTextLength <= 1)
					{
						HWND hDlg = GetParent(hWnd);
						HWND hWndEdit = GetDlgItem(hDlg, ID_CDKEY4);
						SetFocus(hWndEdit);
						SendMessage(hWndEdit, EM_SETSEL, 4, 4);
					}
				}
				break;
			}
		}
		default:
			return CallWindowProc(DefCDKeyProc[4], hWnd, message, wParam, lParam);
	}
	return 0;
}