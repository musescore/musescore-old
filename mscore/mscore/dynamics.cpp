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
      }

Dynamic::Dynamic(const Dynamic& d)
   : Text(d)
      {
      setSubtype(subtype());
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Dynamic::setSubtype(int idx)
      {
      Element::setSubtype(idx);
      setStyle(score()->textStyle(TEXT_STYLE_DYNAMICS));
      if (idx != 0)
            setText(dynList[idx].tag);
      }

//---------------------------------------------------------
//   isMovable
//---------------------------------------------------------

bool Dynamic::isMovable() const
      {
      dragOffset = QPointF();
      return true;
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF Dynamic::drag(const QPointF& pos)
      {
      QRectF r(abbox());
      setUserOff((pos - dragOffset) / _spatium);
      score()->setLayoutAll(false);
      return abbox() | r;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Dynamic::write(Xml& xml) const
      {
      xml.stag("Dynamic");
      if (subtype() == 0)
            xml.tag("data", QVariant(doc()->toHtml("utf8")));
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Dynamic::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "data")
                  doc()->setHtml(e.text());
            else if (!Element::readProperties(e))
                  domError(e);
            }
      setSubtype(subtype());
      }

//---------------------------------------------------------
//   toSubtype
//---------------------------------------------------------

void Dynamic::setSubtype(const QString& tag)
      {
      QString d("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\"font-family:'Times New Roman'; font-weight:400; font-style:italic;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'Times New Roman'; font-style:normal; font-size:12pt;\"><span style=\"font-family:'MScore1'; font-size:20pt;\">%1</span></p></body></html></data>\n");

      setSubtype(0);
      int n = sizeof(dynList)/sizeof(*dynList);
      for (int i = 0; i < n; ++i) {
            if (dynList[i].tag == tag) {
                  setHtml(d.arg(tag));
                  return;
                  }
            }
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
//   canvasPos
//---------------------------------------------------------

QPointF Dynamic::canvasPos() const
      {
      if (!parent())
            return pos();
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
      double yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
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
//   resetUserOffsets
//---------------------------------------------------------

void Dynamic::resetUserOffsets()
      {
      Text::resetUserOffsets();
      setSubtype(subtype());        // (re) apply style
      }

