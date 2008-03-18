//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#include "lyrics.h"
#include "xml.h"
#include "system.h"
#include "measure.h"

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

Lyrics::Lyrics(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_LYRIC);
      _no        = 0;
      _syllabic  = SINGLE;
      _separator = 0;
      _endTick   = 0;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Lyrics::write(Xml& xml) const
      {
      xml.stag("Lyrics");
      xml.tag("data", getText());
      if (_no)
            xml.tag("no", _no);
      static const char* sl[] = {
            "single", "begin", "end", "middle"
            };
      if (_syllabic != SINGLE)
            xml.tag("syllabic", sl[_syllabic]);
      if (_endTick)
            xml.tag("endTick", _endTick);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Lyrics::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "data")
                  setText(val);
            else if (tag == "no")
                  _no = i;
            else if (tag == "syllabic") {
                  if (val == "single")
                        _syllabic = SINGLE;
                  else if (val == "begin")
                        _syllabic = BEGIN;
                  else if (val == "end")
                        _syllabic = END;
                  else if (val == "middle")
                        _syllabic = MIDDLE;
                  else
                        printf("bad syllabic property\n");
                  }
            else if (tag == "endTick")
                  _endTick = i;
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Lyrics::add(Element* el)
      {
      _separator = (Line*)el;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Lyrics::remove(Element*)
      {
      _separator = 0;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Lyrics::draw(QPainter& p) const
      {
      Text::draw(p);
      if (_separator) {
            QPointF pt(_separator->pos());
            p.translate(pt);
            _separator->draw(p);
            p.translate(-pt);
            }
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF Lyrics::canvasPos() const
      {
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
      if (debugMode && system == 0) {
            printf("Lyrics::canvasPos(): no system (%s)\n", qPrintable(getText()));
            return pos();
            }
      double yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }

