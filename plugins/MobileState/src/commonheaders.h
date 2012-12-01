/*
   Mobile State plugin for Miranda NG (www.miranda-ng.org)
   (c) 2012 by Robert P�sel

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#define MIRANDA_VER    0x0A00

#include <windows.h>
#include <malloc.h>

// Miranda API headers
#include <newpluginapi.h>
#include <m_database.h>
#include <m_protocols.h>
#include <m_langpack.h>
#include <m_icolib.h>
#include <m_extraicons.h>

#include "resource.h"
#include "Version.h"

#define MODULENAME "MobileState"

extern HINSTANCE g_hInst;