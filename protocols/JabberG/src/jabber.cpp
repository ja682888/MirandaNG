/*

Jabber Protocol Plugin for Miranda IM
Copyright (C) 2002-04  Santithorn Bunchua
Copyright (C) 2005-12  George Hazan
Copyright (C) 2007     Maxim Mluhov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "jabber.h"
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "jabber_rc.h"

#include <locale.h>

#include <m_fontservice.h>
#include <m_icolib.h>

#include "m_assocmgr.h"
#include "m_folders.h"
#include "m_toptoolbar.h"
#include "m_extraicons.h"

HINSTANCE hInst;

int hLangpack;

int g_cbCountries;
struct CountryListEntry* g_countries;

TCHAR szCoreVersion[100];

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"Jabber Protocol",
	__VERSION_DWORD,
	"Jabber protocol plugin for Miranda NG.",
	"George Hazan, Maxim Mluhov, Victor Pavlychko, Artem Shpynov, Michael Stepura",
	"ghazan@miranda-im.org",
	"(c) 2005-2012 George Hazan, Maxim Mluhov, Victor Pavlychko, Artem Shpynov, Michael Stepura",
	"http://miranda-ng.org/",
	UNICODE_AWARE,
    {0x144e80a2, 0xd198, 0x428b, {0xac, 0xbe, 0x9d, 0x55, 0xda, 0xcc, 0x7f, 0xde}} // {144E80A2-D198-428b-ACBE-9D55DACC7FDE}
};

XML_API  xi;
TIME_API tmi;

CLIST_INTERFACE* pcli;

/////////////////////////////////////////////////////////////////////////////
// Theme API
BOOL (WINAPI *JabberAlphaBlend)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION) = NULL;
BOOL (WINAPI *JabberIsThemeActive)() = NULL;
HRESULT (WINAPI *JabberDrawThemeParentBackground)(HWND, HDC, RECT *) = NULL;
/////////////////////////////////////////////////////////////////////////////

BOOL   jabberChatDllPresent = FALSE;
HANDLE hModulesLoaded, hModulesLoadedTB;

HANDLE hExtraActivity = NULL;
HANDLE hExtraMood = NULL;

void JabberUserInfoInit(void);

int bSecureIM;

/////////////////////////////////////////////////////////////////////////////
// Protocol instances
static int sttCompareProtocols(const CJabberProto *p1, const CJabberProto *p2)
{
	return lstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

LIST<CJabberProto> g_Instances(1, sttCompareProtocols);
/////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD, LPVOID)
{
	hInst = hModule;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

///////////////////////////////////////////////////////////////////////////////
// OnPreShutdown - prepares Miranda to be shut down

int __cdecl CJabberProto::OnPreShutdown(WPARAM, LPARAM)
{
	UI_SAFE_CLOSE_HWND(m_hwndAgentRegInput);
	UI_SAFE_CLOSE_HWND(m_hwndRegProgress);
	UI_SAFE_CLOSE_HWND(m_hwndMucVoiceList);
	UI_SAFE_CLOSE_HWND(m_hwndMucMemberList);
	UI_SAFE_CLOSE_HWND(m_hwndMucModeratorList);
	UI_SAFE_CLOSE_HWND(m_hwndMucBanList);
	UI_SAFE_CLOSE_HWND(m_hwndMucAdminList);
	UI_SAFE_CLOSE_HWND(m_hwndMucOwnerList);
	UI_SAFE_CLOSE_HWND(m_hwndJabberChangePassword);
	UI_SAFE_CLOSE_HWND(m_hwndJabberAddBookmark);
	UI_SAFE_CLOSE_HWND(m_hwndPrivacyRule);

	UI_SAFE_CLOSE(m_pDlgPrivacyLists);
	UI_SAFE_CLOSE(m_pDlgBookmarks);
	UI_SAFE_CLOSE(m_pDlgServiceDiscovery);
	UI_SAFE_CLOSE(m_pDlgJabberJoinGroupchat);
	UI_SAFE_CLOSE(m_pDlgNotes);

	m_iqManager.ExpireAll();
	m_iqManager.Shutdown();
	m_messageManager.Shutdown();
	m_presenceManager.Shutdown();
	m_sendManager.Shutdown();
	ConsoleUninit();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// OnModulesLoaded - execute some code when all plugins are initialized

static INT_PTR g_SvcParseXmppUri(WPARAM w, LPARAM l)
{
	if (CJabberProto *ppro = JabberChooseInstance(true))
		return ppro->JabberServiceParseXmppURI(w, l);
	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	hModulesLoadedTB = HookEvent(ME_TTB_MODULELOADED, g_OnToolbarInit);

	bSecureIM = (ServiceExists("SecureIM/IsContactSecured"));

	// file associations manager plugin support
	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE)) {
		CreateServiceFunction("JABBER/*" JS_PARSE_XMPP_URI, g_SvcParseXmppUri);
		AssocMgr_AddNewUrlTypeT("xmpp:", TranslateT("Jabber Link Protocol"), hInst, IDI_JABBER, "JABBER/*" JS_PARSE_XMPP_URI, 0);
	}

	// init fontservice for info frame
	FontID fontid = {0};
	fontid.cbSize = sizeof(fontid);
	strcpy(fontid.group, "Jabber");
	strcpy(fontid.dbSettingsGroup, GLOBAL_SETTING_MODULE);
	strcpy(fontid.backgroundGroup, "Jabber");
	strcpy(fontid.backgroundName,"Background");
	fontid.flags = FIDF_DEFAULTVALID;

	fontid.deffontsettings.charset = DEFAULT_CHARSET;
	fontid.deffontsettings.colour = GetSysColor(COLOR_WINDOWTEXT);
	fontid.deffontsettings.size = -13;
	lstrcpyA(fontid.deffontsettings.szFace, "Verdana");
	fontid.deffontsettings.style = 0;

	strcpy(fontid.name, "Frame title");
	strcpy(fontid.prefix, "fntFrameTitle");
	fontid.deffontsettings.style = DBFONTF_BOLD;
	FontRegister(&fontid);

	strcpy(fontid.name, "Frame text");
	strcpy(fontid.prefix, "fntFrameClock");
	fontid.deffontsettings.style = 0;
	FontRegister(&fontid);

	ColourID colourid = {0};
	colourid.cbSize = sizeof(colourid);
	strcpy(colourid.group, "Jabber");
	strcpy(colourid.dbSettingsGroup, GLOBAL_SETTING_MODULE);

	strcpy(colourid.name, "Background");
	strcpy(colourid.setting, "clFrameBack");
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	ColourRegister(&colourid);

	// Init extra icons
	hExtraActivity = ExtraIcon_Register("activity", "Jabber Activity", "jabber_dancing");
	hExtraMood = ExtraIcon_Register("mood", "Jabber Mood", "jabber_contemplative");
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// OnLoad - initialize the plugin instance

static CJabberProto* jabberProtoInit(const char* pszProtoName, const TCHAR *tszUserName)
{
	CJabberProto *ppro = new CJabberProto(pszProtoName, tszUserName);
	g_Instances.insert(ppro);
	return ppro;
}

static int jabberProtoUninit(CJabberProto* ppro)
{
	g_Instances.remove(ppro);
	delete ppro;
	return 0;
}

extern "C" int __declspec(dllexport) Load()
{
	// set the memory, lists & utf8 managers
	mir_getXI(&xi);
	mir_getTMI(&tmi);
	mir_getLP(&pluginInfo);

	WORD v[4];
	CallService(MS_SYSTEM_GETFILEVERSION, 0, (LPARAM)v);
	mir_sntprintf(szCoreVersion, SIZEOF(szCoreVersion), _T("%d.%d.%d.%d"), v[0], v[1], v[2], v[3]);

	CallService(MS_UTILS_GETCOUNTRYLIST, (WPARAM)&g_cbCountries, (LPARAM)&g_countries);
	
	setlocale(LC_ALL, "");

	pcli = (CLIST_INTERFACE*)CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, (LPARAM)hInst);

	// Register protocol module
	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "JABBER";
	pd.fnInit = (pfnInitProto)jabberProtoInit;
	pd.fnUninit = (pfnUninitProto)jabberProtoUninit;
	pd.type = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	// Load some fuctions
	HMODULE hDll;
	if (hDll = GetModuleHandleA("gdi32.dll"))
		JabberAlphaBlend = (BOOL (WINAPI *)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION)) GetProcAddress(hDll, "GdiAlphaBlend");
	if (JabberAlphaBlend == NULL && (hDll = LoadLibraryA("msimg32.dll")))
		JabberAlphaBlend = (BOOL (WINAPI *)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION)) GetProcAddress(hDll, "AlphaBlend");

	if (IsWinVerXPPlus()) {
		if (hDll = GetModuleHandleA("uxtheme")) {
			JabberDrawThemeParentBackground = (HRESULT (WINAPI *)(HWND,HDC,RECT *))GetProcAddress(hDll, "DrawThemeParentBackground");
			JabberIsThemeActive = (BOOL (WINAPI *)())GetProcAddress(hDll, "IsThemeActive");
	}	}

	g_IconsInit();
	g_XstatusIconsInit();
	g_MenuInit();
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	JabberUserInfoInit();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Unload - destroy the plugin instance

extern "C" int __declspec(dllexport) Unload(void)
{
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hModulesLoadedTB);

	g_MenuUninit();

	g_Instances.destroy();
	return 0;
}