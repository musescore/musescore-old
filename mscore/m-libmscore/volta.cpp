//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: volta.cpp 3592 2010-10-18 17:24:18Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
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

#include <QtCore/QStringList>

#include "volta.h"
#include "style.h"
#include "system.h"
#include "m-al/xml.h"
#include "score.h"
#include "text.h"

//---------------------------------------------------------
//   Volta
//---------------------------------------------------------

Volta::Volta(Score* s)
   : TextLine(s)
      {
      setLineWidth(Spatium(.18));
      setBeginText("1.", TEXT_STYLE_VOLTA);

      setBeginTextPlace(PLACE_BELOW);
      setContinueTextPlace(PLACE_BELOW);

      setBeginHook(true);
      setBeginHookHeight(Spatium(1.9));
      setYoff(-4.0);
      setEndHookHeight(Spatium(1.9));
      setAnchor(ANCHOR_MEASURE);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Volta::setSubtype(int val)
      {
      Element::setSubtype(val);
      switch(val) {
            case Volta::VOLTA_OPEN:
                  setEndHook(false);
                  break;
            case Volta::VOLTA_CLOSED:
                  setEndHook(true);
                  break;
            }
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick() to _tick2
//---------------------------------------------------------

void Volta::layout()
      {
      setPos(0.0, yoff() * spatium());
      TextLine::layout();
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void Volta::setText(const QString& s)
      {
      setBeginText(s, TEXT_STYLE_VOLTA);
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString Volta::text() const
      {
      return beginText()->getText();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Volta::read(XmlReader* r)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();

      int i= -1;
      while (r->readAttribute()) {
            if (r->tag() == "id")
                  i = r->intValue();
            }
      setId(i);
      while (r->readElement()) {
            MString8 tag(r->tag());
            if (tag == "endings") {
                  QString s = r->stringValue();
                  QStringList sl = s.split(",", QString::SkipEmptyParts);
                  _endings.clear();
                  foreach(const QString& l, sl) {
                        int i = l.simplified().toInt();
                        _endings.append(i);
                        }
                  }
            else if (!TextLine::readProperties(r))
                  r->unknown();
            }
//      setSubtype(subtype());
//      printf("readVolta: subtype %d\n", subtype());
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Volta::createLineSegment()
      {
      return new VoltaSegment(score());
      }

//---------------------------------------------------------
//   hasEnding
//---------------------------------------------------------

bool Volta::hasEnding(int repeat) const
      {
      foreach(int ending, endings()) {
            if (ending == repeat)
                  return true;
            }
      return false;
      }

