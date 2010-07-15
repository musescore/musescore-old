//=============================================================================
//  MuseScore
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

#include "ottava.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "score.h"
#include "text.h"
#include "staff.h"

//---------------------------------------------------------
//   Ottava
//---------------------------------------------------------

Ottava::Ottava(Score* s)
   : TextLine(s)
      {
      setSubtype(0);
      setOffsetType(OFFSET_SPATIUM);
      setYoff(-3.5);    // default position
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Ottava::layout()
      {
      Element::layout();
      SLine::layout();
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Ottava::setSubtype(int val)
      {
      setEndHook(true);
      Element::setSubtype(val);
      switch(val) {
            case 0:
                  setBeginText("8va", TEXT_STYLE_OTTAVA);
                  setContinueText("(8va)", TEXT_STYLE_OTTAVA);
                  setEndHookHeight(Spatium(1.5));
                  _pitchShift = 12;
                  break;
            case 1:
                  setBeginText("15ma", TEXT_STYLE_OTTAVA);
                  setContinueText("(15ma)", TEXT_STYLE_OTTAVA);
                  setEndHookHeight(Spatium(1.5));
                  _pitchShift = 24;
                  break;
            case 2:
                  setBeginText("8vb", TEXT_STYLE_OTTAVA);
                  setContinueText("(8vb)", TEXT_STYLE_OTTAVA);
                  setEndHookHeight(Spatium(-1.5));
                  _pitchShift = -12;
                  break;
            case 3:
                  setBeginText("15mb", TEXT_STYLE_OTTAVA);
                  setContinueText("(15mb)", TEXT_STYLE_OTTAVA);
                  setEndHookHeight(Spatium(-1.5));
                  _pitchShift = -24;
                  break;
            }
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Ottava::createLineSegment()
      {
      return new OttavaSegment(score());
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Ottava::endEdit()
      {
      if (oStartElement != startElement() || oEndElement != endElement()) {
            Staff* s = staff();
            int tick1 = static_cast<Segment*>(oStartElement)->tick();
            int tick2 = static_cast<Segment*>(oEndElement)->tick();
            s->pitchOffsets().remove(tick1);
            s->pitchOffsets().remove(tick2);

            tick1 = static_cast<Segment*>(startElement())->tick();
            tick2 = static_cast<Segment*>(endElement())->tick();
            s->pitchOffsets().setPitchOffset(tick1, _pitchShift);
            s->pitchOffsets().setPitchOffset(tick2, 0);

            score()->addLayoutFlag(LAYOUT_FIX_PITCH_VELO);
            }
      }

