//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: ottava.cpp,v 1.3 2006/03/28 14:58:58 wschweer Exp $
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

#include "ottava.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "layout.h"
#include "score.h"
#include "text.h"

//---------------------------------------------------------
//   Ottava
//---------------------------------------------------------

Ottava::Ottava(Score* s)
   : TextLine(s)
      {
      setSubtype(0);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Ottava::setSubtype(int val)
      {
      setEndHook(true);
      setEndHookHeight(Spatium(1.5));

      Element::setSubtype(val);
      switch(val) {
            case 0:
                  setBeginText("8va");
                  setContinueText("(8va)");
                  _pitchShift = 12;
                  break;
            case 1:
                  setBeginText("15ma");
                  setContinueText("(15ma)");
                  _pitchShift = 24;
                  break;
            case 2:
                  setBeginText("8vb");
                  setContinueText("(8vb)");
                  _pitchShift = -12;
                  break;
            case 3:
                  setBeginText("15mb");
                  setContinueText("(15mb)");
                  _pitchShift = -24;
                  break;
            }
      beginText()->setStyle(TEXT_STYLE_OTTAVA);
      continueText()->setStyle(TEXT_STYLE_OTTAVA);
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Ottava::createLineSegment()
      {
      return new OttavaSegment(score());
      }
