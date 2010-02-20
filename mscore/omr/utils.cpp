//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
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

#include "utils.h"

char bitsSetTable[256];

//---------------------------------------------------------
//   initUtils
//---------------------------------------------------------

void initUtils()
      {
      //
      // populate the bitsSetTable
      bitsSetTable[0] = 0;
      for (int i = 1; i < 256; i++)
            bitsSetTable[i] = (i & 1) + bitsSetTable[i/2];
      }

