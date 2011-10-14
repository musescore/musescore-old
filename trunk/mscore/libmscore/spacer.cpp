//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "spacer.h"
#include "score.h"
#include "painter.h"
#include "mscore.h"

//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

Spacer::Spacer(Score* score)
   : Element(score)
      {
      _gap = 0.0;
      }

Spacer::Spacer(const Spacer& s)
   : Element(s)
      {
      _gap = s._gap;
      path = s.path;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Spacer::draw(Painter* painter) const
      {
      if (score()->printing() || !score()->showUnprintable())
            return;
      QPen pen;
      if (selected())
            painter->setPenColor(MScore::selectColor[0]);
      else
            painter->setPenColor(MScore::layoutBreakColor);

      painter->setLineWidth(spatium() * 0.4);
      painter->setNoBrush(true);
      painter->drawPath(path);
      }

//---------------------------------------------------------
//   layout0
//---------------------------------------------------------

void Spacer::layout0()
      {
      qreal _spatium = spatium();

      path    = QPainterPath();
      qreal w = _spatium;
      qreal b = w * .5;
      qreal h = _gap;

      if (subtype() == SPACER_DOWN) {
            path.lineTo(w, 0.0);
            path.moveTo(b, 0.0);
            path.lineTo(b, h);
            path.lineTo(0.0, h-b);
            path.moveTo(b, h);
            path.lineTo(w, h-b);
            }
      else if (subtype() == SPACER_UP) {
            path.moveTo(b, 0.0);
            path.lineTo(0.0, b);
            path.moveTo(b, 0.0);
            path.lineTo(w, b);
            path.moveTo(b, 0.0);
            path.lineTo(b, h);
            path.moveTo(0.0, h);
            path.lineTo(w, h);
            }
      qreal lw = _spatium * 0.4;
      QRectF bb(0, 0, w, h);
      bb.adjust(-lw, -lw, lw, lw);
      setbbox(bb);
      }

//---------------------------------------------------------
//   setGap
//---------------------------------------------------------

void Spacer::setGap(qreal sp)
      {
      _gap = sp;
      layout0();
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Spacer::spatiumChanged(qreal ov, qreal nv)
      {
      _gap = (_gap / ov) * nv;
      layout0();
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Spacer::setSubtype(int val)
      {
      Element::setSubtype(val);
      layout0();
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Spacer::editDrag(const EditData& ed)
      {
      qreal s = ed.delta.y();
      if (subtype() == SPACER_DOWN)
            _gap += s;
      else if (subtype() == SPACER_UP)
            _gap -= s;
      if (_gap < spatium() * 2.0)
            _gap = spatium() * 2;
      layout0();
      score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Spacer::updateGrips(int* grips, QRectF* grip) const
      {
      *grips         = 1;
      qreal _spatium = spatium();
      QPointF p;
      if (subtype() == SPACER_DOWN)
            p = QPointF(_spatium * .5, _gap);
      else if (subtype() == SPACER_UP)
            p = QPointF(_spatium * .5, 0.0);
      grip[0].translate(pagePos() + p);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Spacer::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      xml.tag("space", _gap / spatium());
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Spacer::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "space")
                  _gap = val.toDouble() * spatium();
            else if (!Element::readProperties(e))
                  domError(e);
            }
      layout0();
      }
