//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: pedal.cpp,v 1.3 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include "pedal.h"
#include "textline.h"
#include "sym.h"

#include "score.h"
#include "layout.h"

//---------------------------------------------------------
//   Pedal
//---------------------------------------------------------

Pedal::Pedal(Score* s)
   : TextLine(s)
      {
      setBeginSymbol(pedalPedSym);
      setEndHook(true);
      setEndHookHeight(Spatium(-1.5));
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Pedal::read(QDomElement e)
      {
      if (score()->mscVersion() >= 110) {
            setBeginSymbol(-1);
            setEndHook(false);
            }
      TextLine::read(e);
      }

