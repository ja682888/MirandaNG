/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci, Nightwish

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#ifndef __ACC_H__
# define __ACC_H__

class A2T
{
	TCHAR* buf;

public:
	A2T( const char* s ) : buf( mir_a2t( s )) {}
	A2T( const char* s, int cp ) : buf( mir_a2t_cp( s, cp )) {}
	~A2T() { mir_free(buf); }

	__forceinline operator TCHAR*() const
	{	return buf;
	}
};

INT_PTR avSetAvatar( HANDLE hContact, TCHAR* tszPath );
INT_PTR avSetMyAvatar( char* szProto, TCHAR* tszPath );

int LoadACC();


#endif // __ACC_H__