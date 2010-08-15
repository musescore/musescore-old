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

#include "trill.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "sym.h"
#include "score.h"
#include "accidental.h"

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TrillSegment::draw(QPainter& p, ScoreView* v) const
      {
      double mags = magS();
      QRectF b1 = symbols[score()->symIdx()][trillSym].bbox(mags);
      QRectF b2 = symbols[score()->symIdx()][trillelementSym].bbox(mags);
      qreal w2  = symbols[score()->symIdx()][trillelementSym].width(mags);
      int n     = lrint((pos2().x() - b1.width()) / w2);

      QPointF a = symbols[score()->symIdx()][trillSym].attach(mags);

      symbols[score()->symIdx()][trillSym].draw(p, mags, -b1.x(), 0);
      symbols[score()->symIdx()][trillelementSym].draw(p, mags,  b1.width(), b2.y() * .9, n);

      if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_BEGIN) {
            if (trill()->accidental()) {
                  p.save();
                  p.translate(trill()->accidental()->canvasPos());
                  trill()->accidental()->draw(p, v);
                  p.restore();
                  }
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TrillSegment::layout()
      {
      QRectF rr(symbols[score()->symIdx()][trillSym].bbox(magS()));
      QRectF r(0.0, rr.y(), pos2().x(), rr.height());
      if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_BEGIN) {
            if (trill()->accidental()) {
                  r |= trill()->accidental()->bbox().translated(trill()->accidental()->pos());
                  }
            }
      setbbox(r);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool TrillSegment::acceptDrop(ScoreView*, const QPointF&, int type, int /*subtype*/) const
      {
      if (type == ACCIDENTAL)
            return true;
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* TrillSegment::drop(ScoreView*, const QPointF&, const QPointF&, Element* e)
      {
      switch(e->type()) {
            case ACCIDENTAL:
                  e->setParent(trill());
                  score()->undoAddElement(e);
                  break;

            default:
                  delete e;
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   Trill
//---------------------------------------------------------

Trill::Trill(Score* s)
  : SLine(s)
      {
      _accidental = 0;
      setLen(spatium() * 7);   // for use in palettes
      setOffsetType(OFFSET_SPATIUM);
      setYoff(-1.0);    // default position
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Trill::add(Element* e)
      {
      if (e->type() == ACCIDENTAL)
            _accidental = static_cast<Accidental*>(e);
      else
            SLine::add(e);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Trill::remove(Element* e)
      {
      if (e->type() == ACCIDENTAL)
            _accidental = 0;
      else
            SLine::remove(e);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Trill::layout()
      {
      Element::layout();
      SLine::layout();
      double _spatium = spatium();

      if (_accidental) {
            _accidental->setMag(.6);
            _accidental->layout();
            _accidental->setPos(_spatium*1.3, -2.2*_spatium);
            }
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Trill::createLineSegment()
      {
      TrillSegment* seg = new TrillSegment(score());
      seg->setTrack(track());
      return seg;
      }

//---------------------------------------------------------
//   Trill::write
//---------------------------------------------------------

void Trill::write(Xml& xml) const
      {
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
      if (_accidental)
            _accidental->write(xml);
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Trill::read
//---------------------------------------------------------

void Trill::read(QDomElement e)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();
      setId(e.attribute("id", "-1").toInt());
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "Accidental") {
                  _accidental = new Accidental(score());
                  _accidental->read(e);
                  }
            else if (!SLine::readProperties(e))
                  domError(e);
            }
      }

