//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: dynamics.cpp 3692 2010-11-09 10:24:01Z wschweer $
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
#include "al/xml.h"
#include "score.h"
#include "measure.h"
#include "system.h"
#include "segment.h"
#include "utils.h"
#include "style.h"

//
// see: http://en.wikipedia.org/wiki/File:Dynamic's_Note_Velocity.svg
//
Dyn dynList[] = {
      // dynamics:
      Dyn( -1,  true,  "other-dynamics"),
      Dyn(  1,  false, "pppppp"),
      Dyn(  5,  false, "ppppp"),
      Dyn( 10,  false, "pppp"),
      Dyn( 16,  false, "ppp"),
      Dyn( 33,  false, "pp"),
      Dyn( 49,  false, "p"),
      Dyn( 64,  false, "mp"),
      Dyn( 80,  false, "mf"),
      Dyn( 96,  false, "f"),
      Dyn(112,  false, "ff"),
      Dyn(126,  false, "fff"),
      Dyn(127,  false, "ffff"),
      Dyn(127,  false, "fffff"),
      Dyn(127,  false, "ffffff"),

      // accents:
      Dyn( 0,   true,  "fp"),
      Dyn( 0,   true,  "sf"),
      Dyn( 0,   true,  "sfz"),
      Dyn( 0,   true,  "sff"),
      Dyn( 0,   true,  "sffz"),
      Dyn( 0,   true,  "sfp"),
      Dyn( 0,   true,  "sfpp"),
      Dyn( 0,   true,  "rfz"),
      Dyn( 0,   true,  "rf"),
      Dyn( 0,   true,  "fz"),
      Dyn( 0,   true,  "m"),
      Dyn( 0,   true,  "r"),
      Dyn( 0,   true,  "s"),
      Dyn( 0,   true,  "z"),
      };

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic(Score* s)
   : Text(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _velocity = -1;
      setTextStyle(TEXT_STYLE_DYNAMICS);
      _dynType  = DYNAMIC_PART;
      }

Dynamic::Dynamic(const Dynamic& d)
   : Text(d)
      {
      setSubtype(subtype());
      _velocity = d._velocity;
      _dynType  = d._dynType;
      }

//---------------------------------------------------------
//   setVelocity
//---------------------------------------------------------

void Dynamic::setVelocity(int v)
      {
      _velocity = v;
      }

//---------------------------------------------------------
//   velocity
//---------------------------------------------------------

int Dynamic::velocity() const
      {
      return _velocity <= 0 ? dynList[subtype()].velocity : _velocity;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Dynamic::read(XmlReader* r)
      {
      int i;
      while (r->readElement()) {
            if (r->readInt("velocity", &_velocity))
                  ;
            else if (r->readInt("dynType", &i))
                  _dynType = DynamicType(i);
            else if (!Text::readProperties(r))
                  r->unknown();
            }
      setSubtype(subtype());
      if (score()->mscVersion() < 118) {
            setTextStyle(TEXT_STYLE_DYNAMICS);
            setStyled(true);
            }
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Dynamic::setSubtype(int idx)
      {
      Element::setSubtype(idx);
      if (idx) {
#if 0
            doc()->clear();
            QTextCursor cursor(doc());
            cursor.movePosition(QTextCursor::Start);
            QTextCharFormat tf = cursor.charFormat();
            const TextStyle& ts = score()->textStyle(TEXT_STYLE_DYNAMICS);
            qreal size = ts.size();
            qreal m = size;
            if (ts.sizeIsSpatiumDependent())
                  m *= (score()->spatium() / (SPATIUM20 * DPI));
            m *= mag();

            QFont font("MScore1-test");
            font.setPointSizeF(m);
            font.setKerning(true);
            font.setStyleStrategy(QFont::NoFontMerging);
            tf.setFont(font);
            tf.setProperty(QTextFormat::FontKerning, true);
            // tf.setProperty(QTextFormat::FontLetterSpacing, 100);
            cursor.setBlockCharFormat(tf);
            cursor.insertText(dynList[idx].tag);
#endif
            setText(dynList[idx].tag);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Dynamic::draw(Painter* p) const
      {
      Font f("MScore1");
      f.setSize(12.0 * score()->spatium() / (SPATIUM20 * DPI));
      p->drawText(f, QPointF(), getText());
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
//   layout
//---------------------------------------------------------

void Dynamic::layout()
      {
      setSubtype(subtype());  // re-apply style
      Text::layout();
      }

