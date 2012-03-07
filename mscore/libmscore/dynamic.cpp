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

#include "dynamic.h"
#include "xml.h"
#include "score.h"
#include "measure.h"
#include "system.h"
#include "segment.h"
#include "utils.h"
#include "style.h"
#include "mscore.h"

//
// see: http://en.wikipedia.org/wiki/File:Dynamic's_Note_Velocity.svg
//
Dyn dynList[] = {
      // dynamic:
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
      setSubtype(0);
      _dynType  = DYNAMIC_PART;
      setTextStyleType(TEXT_STYLE_DYNAMICS);
      }

Dynamic::Dynamic(const Dynamic& d)
   : Text(d)
      {
      setSubtype(d._subtype);
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
//   write
//---------------------------------------------------------

void Dynamic::write(Xml& xml) const
      {
      xml.stag("Dynamic");
      xml.tag("subtype", subtypeName());
      if (_velocity > 0)
            xml.tag("velocity", _velocity);
      if (_dynType != DYNAMIC_PART)
            xml.tag("dynType", _dynType);
      Text::writeProperties(xml, subtype() == 0);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Dynamic::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "subtype")
                  setSubtype(e.text());
            else if (e.tagName() == "velocity")
                  _velocity = e.text().toInt();
            else if (e.tagName() == "dynType")
                  _dynType = DynamicType(e.text().toInt());
            else if (!Text::readProperties(e))
                  domError(e);
            }
      if (score()->mscVersion() < 118)
            setTextStyleType(TEXT_STYLE_DYNAMICS);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Dynamic::setSubtype(int idx)
      {
      _subtype = idx;
      if (idx > 0) {
            if ((unsigned)idx >= sizeof(dynList)/sizeof(*dynList)) {
                  qDebug("Dynamic::setSubtype: bad type %d\n", idx);
                  idx = 1;
                  }
            setUnstyled();
            clear();
            QTextCursor* cursor = startCursorEdit();
            cursor->movePosition(QTextCursor::Start);
            QTextCharFormat tf = cursor->charFormat();
            const TextStyle& ts = score()->textStyle(TEXT_STYLE_DYNAMICS);
            qreal size = ts.size();
            qreal m = size;
            if (ts.sizeIsSpatiumDependent())
                  m *= (score()->spatium() / (SPATIUM20 * DPI));
            m *= mag();

            QFont font("MScore1");
            font.setPointSizeF(m);
            font.setKerning(true);
            font.setStyleStrategy(QFont::NoFontMerging);
            tf.setFont(font);
            tf.setProperty(QTextFormat::FontKerning, true);
            cursor->setBlockCharFormat(tf);
            cursor->insertText(dynList[idx].tag);
            endCursorEdit();
            }
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Dynamic::setSubtype(const QString& tag)
      {
      int n = sizeof(dynList)/sizeof(*dynList);
      for (int i = 0; i < n; ++i) {
            if (dynList[i].tag == tag) {
                  _subtype = i;
                  return;
                  }
            }
      qDebug("Dynamic: subtype not found <%s>\n", qPrintable(tag));
      _subtype = 0;
      setText(tag);
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

QString Dynamic::subtypeName() const
      {
      return dynList[subtype()].tag;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Dynamic::startEdit(MuseScoreView* v, const QPointF& p)
      {
      setSubtype(0);
      Text::startEdit(v, p);
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
      setTextStyle(score()->textStyle(TEXT_STYLE_DYNAMICS));
      setSubtype(idx);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Dynamic::layout()
      {
      setSubtype(subtype());  // re-apply style
      Text::layout();
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Dynamic::dragAnchor() const
      {
      qreal xp = 0.0;
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      qreal yp = measure()->system()->staffY(staffIdx());
      QPointF p(xp, yp);
      return QLineF(p, canvasPos());
      }

