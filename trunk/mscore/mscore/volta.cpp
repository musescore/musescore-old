//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: element.cpp,v 1.79 2006/04/12 14:58:10 wschweer Exp $
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

#include "volta.h"
#include "style.h"
#include "system.h"
#include "xml.h"
#include "score.h"
#include "voltaproperties.h"
#include "text.h"

//---------------------------------------------------------
//   Volta
//---------------------------------------------------------

Volta::Volta(Score* s)
   : TextLine(s)
      {
      setLineWidth(Spatium(.18));
      setBeginText("1.", TEXT_STYLE_VOLTA);

//      setBeginTextPlace(PLACE_ABOVE);
//      setContinueTextPlace(PLACE_ABOVE);
      setBeginTextPlace(PLACE_BELOW);
      setContinueTextPlace(PLACE_BELOW);

      setOffsetType(OFFSET_SPATIUM);
      setBeginHook(true);
      setBeginHookHeight(Spatium(1.9));
      setYoff(-2.0);
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
                  setEndHookHeight(Spatium(1.9));
                  break;
            }
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick() to _tick2
//---------------------------------------------------------

void Volta::layout(ScoreLayout* layout)
      {
      TextLine::layout(layout);
      Element::layout(layout);
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

void Volta::read(QDomElement e)
      {
      foreach(LineSegment* seg, segments)
            delete seg;
      segments.clear();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "text")            // obsolete
                  setText(e.text());
            else if (tag == "endings") {
                  QString s = e.text();
                  QStringList sl = s.split(",", QString::SkipEmptyParts);
                  _endings.clear();
                  foreach(const QString& l, sl) {
                        int i = l.simplified().toInt();
                        _endings.append(i);
                        }
                  }
            else if (!TextLine::readProperties(e))
                  domError(e);
            }
//      setSubtype(subtype());
//      printf("readVolta: subtype %d\n", subtype());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Volta::write(Xml& xml) const
      {
      Volta proto(score());
      proto.setSubtype(subtype());

      xml.stag(name());
      TextLine::writeProperties(xml, &proto);
      QString s;
      foreach(int i, _endings) {
            if (!s.isEmpty())
                  s += ", ";
            s += QString("%1").arg(i);
            }
      xml.tag("endings", s);
      xml.etag();
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Volta::createLineSegment()
      {
      return new VoltaSegment(score());
      }

