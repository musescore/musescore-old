//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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
#include "chord.h"
#include "stem.h"

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

void Hook::setSubtype(int i)
      {
      Element::setSubtype(i);
      switch(i) {
            case 0:    break;
            case 1:    setSym(eighthflagSym);        break;
            case 2:    setSym(sixteenthflagSym);     break;
            case 3:    setSym(thirtysecondflagSym);  break;
            case 4:    setSym(sixtyfourthflagSym);   break;
            case 5:    setSym(flag128Sym);   break;
            case -1:   setSym(deighthflagSym);       break;
            case -2:   setSym(dsixteenthflagSym);    break;
            case -3:   setSym(dthirtysecondflagSym); break;
            case -4:   setSym(dsixtyfourthflagSym);  break;
            case -5:   setSym(dflag128Sym);  break;
            default:
                  printf("no hook for subtype %d\n", i);
                  break;
            }
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Hook::setVisible(bool f)
      {
      Element::setVisible(f);
      Chord* chord = static_cast<Chord*>(parent());
      if (chord && chord->stem() && chord->stem()->visible() != f)
            chord->stem()->setVisible(f);
      }
