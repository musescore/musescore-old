//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  windows support routines
//  $Id: compat.cpp,v 1.4 2006/03/02 17:08:33 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "config.h"
#include "binreloc.h"

#ifdef MINGW32
#include <windows.h>

//---------------------------------------------------------
//   getSharePath
//---------------------------------------------------------

QString getSharePath()
      {
      char path[512];
      HKEY hKey;
      RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\MuseScore_02", 0, KEY_READ, &hKey);
      DWORD len  = 512;
      DWORD type = REG_SZ;
      RegQueryValueEx(hKey, "Install_Dir", 0, &type, (LPBYTE)&path, &len);
      return QString(path);
      }
#else

static int binRelocInitialized = false;

//---------------------------------------------------------
//   getSharePath
//---------------------------------------------------------

QString getSharePath()
      {
      if (!binRelocInitialized) {
	      BrInitError error;
      	if (br_init(&error) == 0 && error != BR_INIT_ERROR_DISABLED) {
            	printf("MScore: Warning: BinReloc failed to initialize(error code %d)\n",
	               error);
      	      printf("Will fallback to hardcoded default path.\n");
            	}
            binRelocInitialized = true;
            }

      QString s(br_find_data_dir(""));
      if (s.isEmpty() || !QDir(s + "/mscore").exists()) {
      	s = getenv("MSCORE");
	      if (s.isEmpty()) {
      	      s = INSTPREFIX;
	            s += "/share/mscore";
            	}
            }
      else {
	      s += "/mscore";
            }
      return s;
      }
#endif

