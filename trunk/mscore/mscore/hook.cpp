//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: element.h,v 1.58 2006/04/12 14:58:10 wschweer Exp $
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

#include "hook.h"
#include "sym.h"

//---------------------------------------------------------
//   Hook
//---------------------------------------------------------

Hook::Hook(Score* s)
  : Symbol(s)
      {
      }

//---------------------------------------------------------
//   setIdx
//---------------------------------------------------------

void Hook::setIdx(int i, bool s)
      {
      val = i;
      _small = s;

      if (_small) {
            switch(i) {
                  case 0:    break;
                  case 1:    setSym(s_eighthflagSym); break;
                  case 2:    setSym(s_sixteenthflagSym); break;
                  case 3:    setSym(s_thirtysecondflagSym); break;
                  case 4:    setSym(s_sixtyfourthflagSym); break;
                  case 5:    break;
                  case -1:   setSym(s_deighthflagSym); break;
                  case -2:   setSym(s_dsixteenthflagSym); break;
                  case -3:   setSym(s_dthirtysecondflagSym); break;
                  case -4:   setSym(s_dsixtyfourthflagSym); break;
                  case -5:   break;
                  }
            }
      else {
            switch(i) {
                  case 0:    break;
                  case 1:    setSym(eighthflagSym); break;
                  case 2:    setSym(sixteenthflagSym); break;
                  case 3:    setSym(thirtysecondflagSym); break;
                  case 4:    setSym(sixtyfourthflagSym); break;
                  case 5:    break;
                  case -1:   setSym(deighthflagSym); break;
                  case -2:   setSym(dsixteenthflagSym); break;
                  case -3:   setSym(dthirtysecondflagSym); break;
                  case -4:   setSym(dsixtyfourthflagSym); break;
                  case -5:   break;
                  }
            }
      }


