//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2007 Werner Schweer (ws@seh.de)
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
      Element::writeProperties(xml);
      xml.etag("Lyrics");
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Lyrics::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
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
            else if (!Element::readProperties(node))
                  domError(node);
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

void Lyrics::draw(QPainter& p)
      {
      Text::draw(p);
      if (_separator) {
            QPointF pt(_separator->pos());
            p.translate(pt);
            _separator->draw(p);
            p.translate(-pt);
            }
      }

