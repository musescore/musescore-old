//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: dynamics.cpp,v 1.23 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#include "dynamics.h"
#include "xml.h"
#include "score.h"
#include "measure.h"
#include "system.h"
#include "segment.h"
#include "utils.h"
#include "style.h"
#include "layout.h"

Dyn dynList[] = {
      Dyn( -1,  "other-dynamics"),
      Dyn(  1,  "pppppp"),
      Dyn(  5,  "ppppp"),
      Dyn( 10,  "pppp"),
      Dyn( 30,  "ppp"),
      Dyn( 50,  "pp"),
      Dyn( 60,  "p"),
      Dyn( 70,  "mp"),
      Dyn( 80,  "mf"),
      Dyn( 90,  "f"),
      Dyn(100,  "ff"),
      Dyn(110,  "fff"),
      Dyn(120,  "ffff"),
      Dyn(125,  "fffff"),
      Dyn(127,  "ffffff"),
      Dyn( -1,  "fp"),
      Dyn( -1,  "sf"),
      Dyn( -1,  "sfz"),
      Dyn( -1,  "sff"),
      Dyn( -1,  "sffz"),
      Dyn( -1,  "sfp"),
      Dyn( -1,  "sfpp"),
      Dyn( -1,  "rfz"),
      Dyn( -1,  "rf"),
      Dyn( -1,  "fz"),
      Dyn( -1,  "m"),
      Dyn( -1,  "r"),
      Dyn( -1,  "s"),
      Dyn( -1,  "z"),
      };

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic(Score* s)
   : Text(s)
      {
      setTextStyle(TEXT_STYLE_DYNAMICS);
      }

Dynamic::Dynamic(const Dynamic& d)
   : Text(d)
      {
      setSubtype(subtype());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Dynamic::write(Xml& xml) const
      {
      xml.stag("Dynamic");
      Text::writeProperties(xml, false);
      if (subtype() == 0) {
            xml.stag("html-data");
            xml.writeHtml(doc()->toHtml("utf-8"));
            xml.etag();
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Dynamic::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!Text::readProperties(e))
                  domError(e);
            }
      setSubtype(subtype());
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Dynamic::setSubtype(int idx)
      {
      Element::setSubtype(idx);
      if (idx) {
            Element::setSubtype(idx);
            doc()->clear();
            QTextCursor cursor(doc());
            cursor.movePosition(QTextCursor::Start);
            QTextCharFormat tf = cursor.charFormat();
            TextStyle* ts = score()->textStyle(TEXT_STYLE_DYNAMICS);
            double size = ts->size;
            double mag = ::_spatium / (SPATIUM20 * DPI);
            double m = size * DPI / PPI;
            if (ts->sizeIsSpatiumDependent)
                  m *= mag;
            QFont font("MScore1");
            font.setPixelSize(lrint(m));
            tf.setFont(font);
            cursor.setBlockCharFormat(tf);
            cursor.insertText(dynList[idx].tag);
            }
      }

//---------------------------------------------------------
//   toSubtype
//---------------------------------------------------------

void Dynamic::setSubtype(const QString& tag)
      {
      int n = sizeof(dynList)/sizeof(*dynList);
      for (int i = 0; i < n; ++i) {
            if (dynList[i].tag == tag) {
                  setSubtype(i);
                  return;
                  }
            }
      Element::setSubtype(0);
      setText(tag);
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString Dynamic::subtypeName() const
      {
      return dynList[subtype()].tag;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Dynamic::endEdit()
      {
      Text::endEdit();
      setSubtype(0);
      }

//---------------------------------------------------------
//   resetType
//---------------------------------------------------------

void Dynamic::resetType()
      {
      QString tag = getText();
      int n = sizeof(dynList)/sizeof(*dynList);
      for (int i = 0; i < n; ++i) {
            if (dynList[i].tag == tag) {
                  setSubtype(i);
                  return;
                  }
            }
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Dynamic::toDefault()
      {
      QString tag = getText();
      int n = sizeof(dynList)/sizeof(*dynList);
      int idx = 0;
      for (int i = 0; i < n; ++i) {
            if (dynList[i].tag == tag) {
                  idx = i;
                  break;
                  }
            }
      Text::toDefault();
      setTextStyle(TEXT_STYLE_DYNAMICS);
      setSubtype(idx);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Dynamic::layout(ScoreLayout* sl)
      {
      setSubtype(subtype());  // re-apply style
      Text::layout(sl);
      }

