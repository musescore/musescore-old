//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
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
      setFlag(ELEMENT_MOVABLE, false);
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

#if 0
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
#endif

