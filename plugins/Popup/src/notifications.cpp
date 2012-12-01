/*
Popup Plus plugin for Miranda IM

Copyright	� 2002 Luca Santarelli,
			� 2004-2007 Victor Pavlychko
			� 2010 MPK
			� 2010 Merlin_de

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

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/src/notifications.cpp $
Revision       : $Revision: 1610 $
Last change on : $Date: 2010-06-23 00:55:13 +0300 (Ср, 23 июн 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#include "headers.h"

HANDLE g_hntfError, g_hntfWarning, g_hntfNotification;

#define PopupNotificationData_SIGNATURE		0x11BEDA1A


int TreeDataSortFunc(const POPUPTREEDATA *p1, const POPUPTREEDATA *p2)
{
	if (int cmp = lstrcmp(p1->pszTreeRoot, p2->pszTreeRoot))
		return cmp;
	return lstrcmp(p1->pszDescription, p2->pszDescription);

}

LIST<POPUPTREEDATA> gTreeData(20, TreeDataSortFunc);

// interface
void LoadNotifications()
{
	POPUPNOTIFICATION notification = {0};
	notification.cbSize = sizeof(notification);
	notification.actionCount = 0;
	notification.lpActions = 0;

	lstrcpynA(notification.lpzGroup, "Misc", sizeof(notification.lpzName));
	lstrcpynA(notification.lpzName, "Warning", sizeof(notification.lpzName));
	notification.lchIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MB_WARN), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR|LR_SHARED);
	notification.colorBack = RGB(210,210,150);
	notification.colorText = RGB(0,0,0);
	notification.iSeconds = 0;
	g_hntfWarning = RegisterNotification(&notification);

	lstrcpynA(notification.lpzGroup, "Misc", sizeof(notification.lpzName));
	lstrcpynA(notification.lpzName, "Notification", sizeof(notification.lpzName));
	notification.lchIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MB_INFO), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR|LR_SHARED);
	notification.colorBack = RGB(230,230,230);
	notification.colorText = RGB(0,0,0);
	notification.iSeconds = 0;
	g_hntfNotification = RegisterNotification(&notification);

	lstrcpynA(notification.lpzGroup, "Misc", sizeof(notification.lpzName));
	lstrcpynA(notification.lpzName, "Error", sizeof(notification.lpzName));
	notification.lchIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MB_STOP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR|LR_SHARED);
	notification.colorBack = RGB(191,0,0);
	notification.colorText = RGB(0,0,0);
	notification.iSeconds = 0;
	g_hntfError = RegisterNotification(&notification);
}

void UnloadTreeData()
{
	for (int i=0; i < gTreeData.getCount(); ++i) {
		if(gTreeData[i]->typ == 2) {
			mir_free(gTreeData[i]->pupClass.pszName);
			mir_free(gTreeData[i]->pupClass.pszDescription);
		}
		mir_free(gTreeData[i]->pszTreeRoot);
		mir_free(gTreeData[i]->pszDescription);
	}
	gTreeData.destroy();
}

void SaveNotificationSettings(POPUPTREEDATA *ptd, char* szModul)
{
	if(ptd->typ == 1) {
		char setting[2*MAXMODULELABELLENGTH];

		mir_snprintf(setting, sizeof(setting), "{%s/%s}Timeout",
			ptd->notification.lpzGroup,
			ptd->notification.lpzName);
		DBWriteContactSettingWord(NULL, szModul, setting, ptd->notification.iSeconds);

		mir_snprintf(setting, sizeof(setting), "{%s/%s}enabled", 
			ptd->notification.lpzGroup, 
			ptd->notification.lpzName);
		DBWriteContactSettingByte(NULL, szModul, setting, ptd->enabled);

		mir_snprintf(setting, sizeof(setting), "{%s/%s}TimeoutVal",
			ptd->notification.lpzGroup,
			ptd->notification.lpzName);
		DBWriteContactSettingWord(NULL, szModul, setting, ptd->timeoutValue);

		mir_snprintf(setting, sizeof(setting), "{%s/%s}disableWhen",
			ptd->notification.lpzGroup,
			ptd->notification.lpzName);
		DBWriteContactSettingByte(NULL, szModul, setting, ptd->disableWhen);

		mir_snprintf(setting, sizeof(setting), "{%s/%s}leftAction",
			ptd->notification.lpzGroup,
			ptd->notification.lpzName);
		DBWriteContactSettingString(NULL, szModul, setting, ptd->leftAction);

		mir_snprintf(setting, sizeof(setting), "{%s/%s}rightAction",
			ptd->notification.lpzGroup,
			ptd->notification.lpzName);
		DBWriteContactSettingString(NULL, szModul, setting, ptd->rightAction);

		for (int i=0; i < ptd->notification.actionCount; ++i)
		{
			if (!lstrcmpA(ptd->leftAction, ptd->notification.lpActions[i].lpzTitle))
			{
				DBCONTACTWRITESETTING dbcws = {0};
				dbcws.szModule = ptd->notification.lpActions[i].lpzLModule;
				dbcws.szModule = ptd->notification.lpActions[i].lpzLSetting;
				dbcws.value = ptd->notification.lpActions[i].dbvLData;
				CallService(MS_DB_CONTACT_WRITESETTING, 0, (LPARAM)&dbcws);
			}
			if (!lstrcmpA(ptd->rightAction, ptd->notification.lpActions[i].lpzTitle))
			{
				DBCONTACTWRITESETTING dbcws = {0};
				dbcws.szModule = ptd->notification.lpActions[i].lpzRModule;
				dbcws.szModule = ptd->notification.lpActions[i].lpzRSetting;
				dbcws.value = ptd->notification.lpActions[i].dbvRData;
				CallService(MS_DB_CONTACT_WRITESETTING, 0, (LPARAM)&dbcws);
			}
		}
	}
}

void LoadNotificationSettings(POPUPTREEDATA *ptd, char* szModul)
{
	if(ptd->typ == 1) {
		char setting[2*MAXMODULELABELLENGTH];
		char *szTmp = NULL;

		mir_snprintf(setting, sizeof(setting), "{%s/%s}enabled", ptd->notification.lpzGroup, ptd->notification.lpzName);
		ptd->enabled = 
			(signed char)DBGetContactSettingByte(NULL, szModul, setting, TRUE);

		mir_snprintf(setting, sizeof(setting), "{%s/%s}Timeout", ptd->notification.lpzGroup, ptd->notification.lpzName);
		ptd->notification.iSeconds = 
			(signed char)DBGetContactSettingWord(NULL, szModul, setting, ptd->notification.iSeconds);

		mir_snprintf(setting, sizeof(setting), "{%s/%s}TimeoutVal", ptd->notification.lpzGroup, ptd->notification.lpzName);
		ptd->timeoutValue =
			(signed char)DBGetContactSettingWord(NULL, szModul, setting,
				ptd->notification.iSeconds ? ptd->notification.iSeconds : 0);

		mir_snprintf(setting, sizeof(setting), "{%s/%s}disableWhen", ptd->notification.lpzGroup, ptd->notification.lpzName);
		ptd->disableWhen =
			DBGetContactSettingByte(NULL, szModul, setting, 0);

		mir_snprintf(setting, sizeof(setting), "{%s/%s}leftAction", ptd->notification.lpzGroup, ptd->notification.lpzName);
		szTmp = DBGetContactSettingString(NULL, szModul, setting, ptd->notification.lpzLAction);
		lstrcpynA(ptd->leftAction, szTmp, sizeof(ptd->leftAction));
		mir_free(szTmp); szTmp = NULL;

		mir_snprintf(setting, sizeof(setting), "{%s/%s}rightAction", ptd->notification.lpzGroup, ptd->notification.lpzName);
		szTmp = DBGetContactSettingString(NULL, szModul, setting, ptd->notification.lpzRAction);
		lstrcpynA(ptd->rightAction, szTmp, sizeof(ptd->rightAction));
		mir_free(szTmp); szTmp = NULL;
	}
}

HANDLE RegisterNotification(POPUPNOTIFICATION *notification)
{
	POPUPTREEDATA *ptd = (POPUPTREEDATA *)mir_alloc(sizeof(POPUPTREEDATA));
	ptd->signature			= PopupNotificationData_SIGNATURE;
	ptd->typ				= 1;
	ptd->pszTreeRoot		= mir_a2t(notification->lpzGroup);
	ptd->pszDescription		= mir_a2t(notification->lpzName);
	ptd->notification		= *notification;
	if (!ptd->notification.lpzLAction) ptd->notification.lpzLAction = POPUP_ACTION_NOTHING;
	if (!ptd->notification.lpzRAction) ptd->notification.lpzRAction = POPUP_ACTION_DISMISS;
	LoadNotificationSettings(ptd, "PopUpNotifications");

	// ugly hack to make reset always possible
	SaveNotificationSettings(ptd,"PopUpNotifications");

	FontID fontid = {0};
	fontid.cbSize = sizeof(fontid);
	mir_snprintf(fontid.group, sizeof(fontid.group), "%s/%s", PU_FNT_AND_COLOR, notification->lpzGroup);
	lstrcpyA(fontid.dbSettingsGroup, "PopUpNotifications");
	fontid.flags = FIDF_DEFAULTVALID;
	fontid.deffontsettings.charset = DEFAULT_CHARSET;
	fontid.deffontsettings.colour = ptd->notification.colorText;
	fontid.deffontsettings.size = -11;
	lstrcpyA(fontid.deffontsettings.szFace, "Verdana");
	fontid.deffontsettings.style = 0;
	mir_snprintf(fontid.name, SIZEOF(fontid.name), "%s (colors only)", notification->lpzName);
	mir_snprintf(fontid.prefix, SIZEOF(fontid.prefix), "{%s/%s}text", notification->lpzGroup, notification->lpzName);
	fontid.deffontsettings.style = 0;
	FontRegister(&fontid);

	ColourID colourid = {0};
	colourid.cbSize = sizeof(colourid);
	mir_snprintf(colourid.group, sizeof(colourid.group), "%s/%s", PU_FNT_AND_COLOR, notification->lpzGroup);
	lstrcpyA(colourid.dbSettingsGroup, "PopUpNotifications");
	mir_snprintf(colourid.name, SIZEOF(colourid.name), "%s (colors only)", notification->lpzName);
	mir_snprintf(colourid.setting, SIZEOF(colourid.setting), "{%s/%s}backColor", notification->lpzGroup, notification->lpzName);
	colourid.defcolour = ptd->notification.colorBack;
	ColourRegister(&colourid);

	char section[MAXMODULELABELLENGTH], setting[MAXMODULELABELLENGTH];
	mir_snprintf(section, sizeof(section), "PopUps/%s", notification->lpzGroup);
	mir_snprintf(setting, sizeof(setting), "%s_%s_%s", MODULNAME, notification->lpzGroup, notification->lpzName);

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(sid);
	sid.pszSection = section;
	sid.cx = sid.cy = 16;
	sid.pszName = setting;
	sid.pszDescription = notification->lpzName;
	sid.hDefaultIcon = notification->lchIcon;
	Skin_AddIcon(&sid);

	gTreeData.insert(ptd);
	return (HANDLE)ptd;
}

HANDLE FindTreeData(LPTSTR group, LPTSTR name, BYTE typ)
{
	for(int i=0; i < gTreeData.getCount(); i++) {
		if (	gTreeData[i]->typ == typ &&
				(!group || (_tcscmp(gTreeData[i]->pszTreeRoot,   group) == 0)) &&
				(!name  || (_tcscmp(gTreeData[i]->pszDescription, name) == 0)))
		{
			return gTreeData[i];
		}
	}
	return NULL;
}

void FillNotificationData(POPUPDATA2 *ppd, DWORD *disableWhen)
{
	if (!IsValidNotification(ppd->lchNotification)) {
		*disableWhen = 0;
		return;
	}

	POPUPTREEDATA *ptd = (POPUPTREEDATA *)ppd->lchNotification;

	ppd->iSeconds = ptd->timeoutValue;
	*disableWhen = ptd->enabled ? ptd->disableWhen : 0xFFFFFFFF;

	LOGFONTA lf;						//dummy to make FS happy (use LOGFONTA coz we use MS_FONT_GET)
	FontID fontid = {0};				//use ansi version of fontID coz POPUPNOTIFICATION use char
	fontid.cbSize = sizeof(fontid);
	mir_snprintf(fontid.group, sizeof(fontid.group), "%s/%s", PU_FNT_AND_COLOR, ptd->notification.lpzGroup);
	mir_snprintf(fontid.name, SIZEOF(fontid.name), "%s (colors only)", ptd->notification.lpzName);
	ppd->colorText = (COLORREF)CallService(MS_FONT_GET, (WPARAM)&fontid, (LPARAM)&lf);

	ColourID colourid = {0};			//use ansi version of ColourID coz POPUPNOTIFICATION use char
	colourid.cbSize = sizeof(colourid);
	mir_snprintf(colourid.group, sizeof(colourid.group), "%s/%s", PU_FNT_AND_COLOR, ptd->notification.lpzGroup);
	mir_snprintf(colourid.name, SIZEOF(colourid.name), "%s (colors only)", ptd->notification.lpzName);
	ppd->colorBack = (COLORREF)CallService(MS_COLOUR_GET, (WPARAM)&colourid, 0);

	char setting[MAXMODULELABELLENGTH];
	mir_snprintf(setting, sizeof(setting), "%s_%s_%s", MODULNAME, ptd->notification.lpzGroup, ptd->notification.lpzName);
	ppd->lchIcon = Skin_GetIcon(setting);
}

bool IsValidNotification(HANDLE hNotification)
{
	if (!hNotification) return false;

	bool res = false;
	__try
	{
		if (((POPUPTREEDATA *)hNotification)->signature == PopupNotificationData_SIGNATURE)
			res = true;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		res = false;
	}
	return res;
}

bool PerformAction(HANDLE hNotification, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (!IsValidNotification(hNotification))
		return false;

	POPUPTREEDATA *ptd = (POPUPTREEDATA *)hNotification;
	char *lpzAction = NULL;
	switch (message)
	{
		case WM_LBUTTONUP:
		case WM_COMMAND:
			lpzAction = ptd->leftAction;
			break;

		case WM_RBUTTONUP:
		case WM_CONTEXTMENU:
			lpzAction = ptd->rightAction;
			break;

		default:
			return false;
	}

	if (!lstrcmpA(lpzAction, POPUP_ACTION_NOTHING))
		return true;

	if (!lstrcmpA(lpzAction, POPUP_ACTION_DISMISS))
	{
		PUDeletePopUp(hwnd);
		return true;
	}

	for (int i=0; i < ptd->notification.actionCount; ++i)
	{
		if (!(ptd->notification.lpActions[i].dwFlags&PNAF_CALLBACK))
			continue;
		if (lstrcmpA(ptd->notification.lpActions[i].lpzTitle, lpzAction))
			continue;

		ptd->notification.lpActions[i].pfnCallback(hwnd, message, wparam, lparam,
			ptd->notification.lpActions[i].dwCookie);
		return true;
	}

	return false;
}