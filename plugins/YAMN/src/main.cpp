/*
 * YAMN plugin main file
 * Miranda homepage: http://miranda-icq.sourceforge.net/
 * YAMN homepage: http://www.majvan.host.sk/Projekty/YAMN
 *
 * initializes all variables for further work
 *
 * (c) majvan 2002-2004
 */


#include "yamn.h"
#include "main.h"
#include <io.h>

#include "m_hotkeys.h"

//--------------------------------------------------------------------------------------------------

TCHAR ProfileName[MAX_PATH];
TCHAR UserDirectory[MAX_PATH];

TCHAR	szMirandaDir[MAX_PATH];
TCHAR	szProfileDir[MAX_PATH];

int YAMN_STATUS;

BOOL UninstallPlugins;

HANDLE hAccountFolder;

HINSTANCE *hDllPlugins;
static int iDllPlugins = 0;


YAMN_VARIABLES YAMNVar;

int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	YAMN_SHORTNAME,
	YAMN_VERSION,
	"Mail notifier and browser for Miranda NG. Included POP3 protocol.",
	"y_b tweety (majvan)",
	"francois.mean@skynet.be",
	"� (2002-2004 majvan) 2005-2007 tweety y_b Miranda community",
	"http://miranda-ng.org/", //"http://www.majvan.host.sk/Projekty/YAMN?fm = soft",
	UNICODE_AWARE,
	{ 0xb047a7e5, 0x27a, 0x4cfc, { 0x8b, 0x18, 0xed, 0xa8, 0x34, 0x5d, 0x27, 0x90 } } // {B047A7E5-027A-4cfc-8B18-EDA8345D2790}
};

HANDLE hNewMailHook;
HANDLE NoWriterEV;
HANDLE hTTButton;

UINT SecTimer;

HANDLE hMenuItemMain = 0;
HANDLE hMenuItemCont = 0;
HANDLE hMenuItemContApp = 0;

HMODULE hUxTheme = 0;
BOOL (WINAPI *MyEnableThemeDialogTexture)(HANDLE, DWORD) = 0;

// function pointers, use typedefs for casting to shut up the compiler when using GetProcAddress()

typedef BOOL (WINAPI *PITA)();
typedef HANDLE (WINAPI *POTD)(HWND, LPCWSTR);
typedef UINT (WINAPI *PDTB)(HANDLE, HDC, int, int, RECT *, RECT *);
typedef UINT (WINAPI *PCTD)(HANDLE);
typedef UINT (WINAPI *PDTT)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, RECT *);

PITA pfnIsThemeActive = 0;
POTD pfnOpenThemeData = 0;
PDTB pfnDrawThemeBackground = 0;
PCTD pfnCloseThemeData = 0;
PDTT pfnDrawThemeText = 0;

#define FIXED_TAB_SIZE 100                  // default value for fixed width tabs

/*
 * visual styles support (XP+)
 * returns 0 on failure
 */

int InitVSApi()
{
	if ((hUxTheme = LoadLibraryA("uxtheme.dll"))  ==  0)
		return 0;

	pfnIsThemeActive = (PITA)GetProcAddress(hUxTheme, "IsThemeActive");
	pfnOpenThemeData = (POTD)GetProcAddress(hUxTheme, "OpenThemeData");
	pfnDrawThemeBackground = (PDTB)GetProcAddress(hUxTheme, "DrawThemeBackground");
	pfnCloseThemeData = (PCTD)GetProcAddress(hUxTheme, "CloseThemeData");
	pfnDrawThemeText = (PDTT)GetProcAddress(hUxTheme, "DrawThemeText");

	MyEnableThemeDialogTexture = (BOOL (WINAPI *)(HANDLE, DWORD))GetProcAddress(hUxTheme, "EnableThemeDialogTexture");
	if (pfnIsThemeActive != 0 && pfnOpenThemeData != 0 && pfnDrawThemeBackground != 0 && pfnCloseThemeData != 0 && pfnDrawThemeText != 0)
		return 1;

	return 0;
}

/*
 * unload uxtheme.dll
 */

int FreeVSApi()
{
	if (hUxTheme != 0)
		FreeLibrary(hUxTheme);
	return 0;
}

//--------------------------------------------------------------------------------------------------

static void GetProfileDirectory(TCHAR *szPath, int cbPath)
//This is copied from Miranda's sources. In 0.2.1.0 it is needed, in newer vesions of Miranda use MS_DB_GETPROFILEPATH service
{
	TCHAR tszOldPath[MAX_PATH];
	CallService(MS_DB_GETPROFILEPATHT, SIZEOF(tszOldPath), (LPARAM)tszOldPath);
	_tcscat(tszOldPath, _T("\\*.book"));

	TCHAR* ptszNewPath = Utils_ReplaceVarsT( _T("%miranda_userdata%"));

	SHFILEOPSTRUCT file_op = {
		NULL,
		FO_MOVE,
		tszOldPath,
		ptszNewPath,
		FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_SILENT,
		false,
		0,
		_T("") };
	SHFileOperation(&file_op);

	_tcsncpy(szPath, ptszNewPath, cbPath);
	mir_free(ptszNewPath);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	YAMNVar.hInst = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MUUID_YAMN_FORCECHECK, MIID_LAST};

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef YAMN_DEBUG
static char unknownCP[1500] = {0};
#endif
// The callback function
BOOL CALLBACK EnumSystemCodePagesProc(LPTSTR cpStr)
{
    //Convert code page string to number
    UINT cp = _ttoi(cpStr);
    if (!IsValidCodePage(cp))
        return TRUE;

    //Get Code Page name
    CPINFOEX info;
    if (GetCPInfoEx(cp, 0, &info)) {
		#ifdef YAMN_DEBUG
		BOOLEAN found = FALSE;
		#endif
		for (int i = 1;i<CPLENALL;i++) if (CodePageNamesAll[i].CP == cp) {
			CodePageNamesAll[i].isValid = TRUE;
			CPLENSUPP++;
			#ifdef YAMN_DEBUG
			found = TRUE;
			#endif
			break;
		}
		#ifdef YAMN_DEBUG
		if (!found) {
			strcat(unknownCP, info.CodePageName);
			strcat(unknownCP, "\n");
		}
		#endif
	}
    return TRUE;
}

void CheckMenuItems()
{
	CLISTMENUITEM clmi = { sizeof(clmi) };
	clmi.flags = CMIM_FLAGS;
	if ( !DBGetContactSettingByte(NULL, YAMN_DBMODULE, YAMN_SHOWMAINMENU, 1))
		clmi.flags |= CMIF_HIDDEN;

	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuItemMain, ( LPARAM )&clmi );
}

int SystemModulesLoaded(WPARAM, LPARAM)
{
	//Insert "Check mail (YAMN)" item to Miranda's menu
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 0xb0000000;
	mi.flags = CMIF_ICONFROMICOLIB;
	mi.icolibItem = g_GetIconHandle(0);
	mi.pszName = "Check &mail (All Account)";
	mi.pszPopupName = NULL;//YAMN_DBMODULE;
	mi.pszService = MS_YAMN_FORCECHECK;
	hMenuItemMain = Menu_AddMainMenuItem(&mi);

	mi.pszName = "Check &mail (This Account)";
	mi.pszContactOwner = YAMN_DBMODULE;
	mi.pszService = MS_YAMN_CLISTCONTEXT;
	hMenuItemCont = Menu_AddContactMenuItem(&mi);

	mi.icolibItem = g_GetIconHandle(1);
	mi.pszName = "Launch application";
	mi.pszContactOwner = YAMN_DBMODULE;
	mi.pszService = MS_YAMN_CLISTCONTEXTAPP;
	hMenuItemContApp = Menu_AddContactMenuItem(&mi);

	CheckMenuItems();

	if (ServiceExists(MS_FOLDERS_GET_PATH)) {
		hAccountFolder = FoldersRegisterCustomPathT(YAMN_DBMODULE, YAMN_DBMODULE" Account Folder", UserDirectory);
		FoldersGetCustomPathT(hAccountFolder, UserDirectory, MAX_PATH, UserDirectory);
	}

	RegisterPOP3Plugin(0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct TIconListItem
{
	TCHAR*  szDescr;
	char*  szName;
	int    defIconID;
	HANDLE hIcon;
};

static TIconListItem iconList[] =
{

	{LPGENT("Check mail"),			"YAMN_Check",		IDI_CHECKMAIL,	0},
	{LPGENT("Launch application"),	"YAMN_Launch",		IDI_LAUNCHAPP,	0},
	{LPGENT("New Mail"),			"YAMN_NewMail",		IDI_NEWMAIL,	0},
	{LPGENT("Connect Fail"),		"YAMN_ConnectFail",	IDI_BADCONNECT,	0},
};

static void LoadIcons()
{
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(YAMNVar.hInst, szFile, MAX_PATH);

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(SKINICONDESC);
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszDefaultFile = szFile;
	sid.ptszSection = _T("YAMN");

	for (int i = 0, k = 0; i < SIZEOF(iconList); i++)
	{
		sid.pszName = iconList[i].szName;
		sid.ptszDescription = iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		iconList[i].hIcon = Skin_AddIcon(&sid);
	}
}

HANDLE WINAPI g_GetIconHandle( int idx )
{
	if ( idx >= SIZEOF(iconList))
		return NULL;
	return iconList[idx].hIcon;
}

HICON WINAPI g_LoadIconEx( int idx, bool big )
{
	if ( idx >= SIZEOF(iconList))
		return NULL;
	return Skin_GetIcon(iconList[idx].szName, big);
}

void WINAPI g_ReleaseIcon( HICON hIcon )
{
	if ( hIcon ) Skin_ReleaseIcon(hIcon);
}

static void LoadPlugins()
{
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	TCHAR szSearchPath[MAX_PATH];
	TCHAR szPluginPath[MAX_PATH];
	lstrcpy(szSearchPath, szMirandaDir);
	lstrcat(szSearchPath, _T("\\Plugins\\YAMN\\*.dll"));
	typedef INT_PTR (*LOADFILTERFCN)(MIRANDASERVICE GetYAMNFcn);

	hDllPlugins = NULL;

	if (INVALID_HANDLE_VALUE!=(hFind = FindFirstFile(szSearchPath, &fd))) {
		do {
			//rewritten from Miranda sources... Needed because Win32 API has a bug in FindFirstFile, search is done for *.dlllllll... too
			TCHAR *dot = _tcsrchr(fd.cFileName, '.');
			if (dot == NULL )
				continue;

			// we have a dot
			int len = (int)lstrlen(fd.cFileName); // find the length of the string
			TCHAR* end = fd.cFileName+len; // get a pointer to the NULL
			int safe = (end-dot)-1;	// figure out how many chars after the dot are "safe", not including NULL

			if ((safe!=3) || (lstrcmpi(dot+1, _T("dll"))!=0)) //not bound, however the "dll" string should mean only 3 chars are compared
				continue;

			HINSTANCE hDll;
			LOADFILTERFCN LoadFilter;

			lstrcpy(szPluginPath, szMirandaDir);
			lstrcat(szPluginPath, _T("\\Plugins\\YAMN\\"));
			lstrcat(szPluginPath, fd.cFileName);
			if ((hDll = LoadLibrary(szPluginPath)) == NULL) continue;
			LoadFilter = (LOADFILTERFCN)GetProcAddress(hDll, "LoadFilter");
			if (NULL == LoadFilter) {
				FreeLibrary(hDll);
				hDll = NULL;
				continue;
			}

			if (!(*LoadFilter)(GetFcnPtrSvc)) {
				FreeLibrary(hDll);
				hDll = NULL;
			}

			if (hDll != NULL) {
				hDllPlugins = (HINSTANCE *)realloc((void *)hDllPlugins, (iDllPlugins+1)*sizeof(HINSTANCE));
				hDllPlugins[iDllPlugins++] = hDll;
			}
		}
			while(FindNextFile(hFind, &fd));

		FindClose(hFind);
	}
}

extern "C" int __declspec(dllexport) Load(void)
{
	int i, k;
	mir_getLP(&pluginInfo);

	YAMN_STATUS = ID_STATUS_OFFLINE;

	//	we get the Miranda Root Path
	if (ServiceExists(MS_UTILS_PATHTOABSOLUTET))
		CallService(MS_UTILS_PATHTOABSOLUTET, (WPARAM)_T("."), (LPARAM)szMirandaDir);
	else {
		GetModuleFileName(GetModuleHandle(NULL), szMirandaDir, MAX_PATH);
		TCHAR* str2 = _tcsrchr(szMirandaDir, '\\');
		if (str2!=NULL) *str2 = 0;
	}

	// retrieve the current profile name
	CallService(MS_DB_GETPROFILENAMET, (WPARAM)SIZEOF(ProfileName), (LPARAM)ProfileName);	//not to pass entire array to fcn
	TCHAR *fc = _tcsrchr(ProfileName, '.');
	if ( fc != NULL ) *fc = 0;

	//	we get the user path where our yamn-account.book.ini is stored from mirandaboot.ini file
	GetProfileDirectory(UserDirectory, SIZEOF(UserDirectory));

	// Enumerate all the code pages available for the System Locale
	EnumSystemCodePages(EnumSystemCodePagesProc, CP_INSTALLED);
	CodePageNamesSupp = new _tcptable[CPLENSUPP];
	for (i = 0, k = 0; i < CPLENALL; i++) {
		if (CodePageNamesAll[i].isValid){
			CodePageNamesSupp[k] = CodePageNamesAll[i];
			k++;
	}	}

	// Registering YAMN as protocol
	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = YAMN_DBMODULE;
	pd.type = PROTOTYPE_VIRTUAL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	InitializeCriticalSection(&AccountStatusCS);
	InitializeCriticalSection(&FileWritingCS);
	InitializeCriticalSection(&PluginRegCS);

	if (NULL == (NoWriterEV = CreateEvent(NULL, TRUE, TRUE, NULL)))
		return 1;
	if (NULL == (WriteToFileEV = CreateEvent(NULL, FALSE, FALSE, NULL)))
		return 1;
	if (NULL == (ExitEV = CreateEvent(NULL, TRUE, FALSE, NULL)))
		return 1;

	PosX = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBPOSX, 0);
	PosY = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBPOSY, 0);
	SizeX = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBSIZEX, 800);
	SizeY = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBSIZEY, 200);

	HeadPosX = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBMSGPOSX, 0);
	HeadPosY = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBMSGPOSY, 0);
	HeadSizeX = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBMSGSIZEX, 690);
	HeadSizeY = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBMSGSIZEY, 300);
	HeadSplitPos = DBGetContactSettingWord(NULL, YAMN_DBMODULE, YAMN_DBMSGPOSSPLIT, 250);

	optDateTime = DBGetContactSettingByte(NULL, YAMN_DBMODULE, YAMN_DBTIMEOPTIONS, optDateTime);

	// Create new window queues for broadcast messages
	YAMNVar.MessageWnds = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	YAMNVar.NewMailAccountWnd = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	YAMNVar.Shutdown = FALSE;

	hCurSplitNS = LoadCursor(NULL, IDC_SIZENS);
	hCurSplitWE = LoadCursor(NULL, IDC_SIZEWE);

#ifdef YAMN_DEBUG
	InitDebug();
#endif

	CreateServiceFunctions();

	SkinAddNewSoundEx(YAMN_NEWMAILSOUND, YAMN_DBMODULE, YAMN_NEWMAILSNDDESC);
	SkinAddNewSoundEx(YAMN_CONNECTFAILSOUND, YAMN_DBMODULE, YAMN_CONNECTFAILSNDDESC);

	HookEvents();

	LoadIcons();
	LoadPlugins();
	InitVSApi();

	HOTKEYDESC hkd = {0};
	hkd.cbSize = sizeof(hkd);
	hkd.pszService = MS_YAMN_FORCECHECK;
	hkd.pszSection = YAMN_DBMODULE;
	hkd.pszDescription = LPGEN("Check mail");
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, VK_F11);
	Hotkey_Register(&hkd);

	//Create thread that will be executed every second
	if (!(SecTimer = SetTimer(NULL, 0, 1000, (TIMERPROC)TimerProc)))
		return 1;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void UnloadPlugins()
{
	for (int i = iDllPlugins-1;i>=0;i--) {
		if (FreeLibrary(hDllPlugins[i])) {
			hDllPlugins[i] = NULL;				//for safety
			iDllPlugins --;
		}
	}
	if (hDllPlugins){
		free((void *)hDllPlugins);
		hDllPlugins = NULL;
	}
}

extern "C" int __declspec(dllexport) Unload(void)
{
#ifdef YAMN_DEBUG
	UnInitDebug();
#endif
	DestroyCursor(hCurSplitNS);
	DestroyCursor(hCurSplitWE);

	CloseHandle(NoWriterEV);
	CloseHandle(WriteToFileEV);
	CloseHandle(ExitEV);

	FreeVSApi();

	DeleteCriticalSection(&AccountStatusCS);
	DeleteCriticalSection(&FileWritingCS);
	DeleteCriticalSection(&PluginRegCS);

	UnhookEvents();
	DestroyServiceFunctions();

	UnloadPlugins();

	delete [] CodePageNamesSupp;
	return 0;
}