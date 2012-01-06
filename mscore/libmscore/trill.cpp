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

#include "trill.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "sym.h"
#include "score.h"
#include "accidental.h"
#include "segment.h"

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TrillSegment::draw(QPainter* painter) const
      {
      qreal mag  = magS();
      int idx    = score()->symIdx();
      QRectF b2(symbols[idx][trillelementSym].bbox(mag));
      qreal w2   = symbols[idx][trillelementSym].width(mag);

      qreal x2   = pos2().x();

      painter->setPen(curColor());
      if (subtype() == SEGMENT_SINGLE || subtype() == SEGMENT_BEGIN) {
            int sym = 0;
            qreal x0 = 0.0, x1 = 0.0, y = 0.0;
            int n = 0;
            QRectF b1;

            switch(trill()->subtype()) {
                  case TRILL_LINE:
                        sym = trillSym;
                        b1  = symbols[idx][sym].bbox(mag);
                        x0   = -b1.x();
                        x1   = x0 + b1.width();
                        n    = int(floor((x2-x1) / w2));
                        y    = 0.0;
                        break;

                  case UPPRALL_LINE:
                        sym = upprallSym;
                        b1  = symbols[idx][sym].bbox(mag);
                        x0   = -b1.x();
                        x1   = b1.width();
                        n    = int(floor((x2-x1) / w2));
                        y    = -b1.height();
                        break;
                  case DOWNPRALL_LINE:
                        sym  = downprallSym;
                        b1   = symbols[idx][sym].bbox(mag);
                        x0   = -b1.x();
                        x1   = b1.width();
                        n    = int(floor((x2-x1) / w2));
                        y    = -b1.height();
                        break;
                  case PRALLPRALL_LINE:
                        sym = prallprallSym;
                        b1  = symbols[idx][sym].bbox(mag);
                        x0   = -b1.x();
                        x1   = b1.width();
                        n    = int(floor((x2-x1) / w2));
                        y    = -b1.height();
                        break;
                  }
            if (n <= 0)
                  n = 1;

            symbols[idx][sym].draw(painter, mag, QPointF(x0, y));
            symbols[idx][trillelementSym].draw(painter, mag,  QPointF(x1, b2.y() * .9), n);

            if (trill()->accidental()) {
                  QPointF pos(trill()->accidental()->pagePos());
                  painter->translate(pos);
                  trill()->accidental()->draw(painter);
                  painter->translate(-pos);
                  }
            }
      else {
            qreal x1 = 0.0;
            int n = int(floor((x2-x1) / w2));
            symbols[idx][trillelementSym].draw(painter, mag,  QPointF(x1, b2.y() * .9), n);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TrillSegment::layout()
      {
      qreal mag = magS();
      int idx    = score()->symIdx();
      QRectF b1(symbols[idx][trillSym].bbox(mag));
      QRectF rr(b1.translated(-b1.x(), 0.0));
      rr |= QRectF(0.0, rr.y(), pos2().x(), rr.height());
      if (subtype() == SEGMENT_SINGLE || subtype() == SEGMENT_BEGIN) {
            if (trill()->accidental()) {
                  rr |= trill()->accidental()->bbox().translated(trill()->accidental()->pos());
                  }
            }
      setbbox(rr);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool TrillSegment::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      if (e->type() == ACCIDENTAL)
            return true;
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* TrillSegment::drop(const DropData& data)
      {
      Element* e = data.element;
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
      _subtype = TRILL_LINE;
      _accidental = 0;
      setLen(spatium() * 7);   // for use in palettes
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
      qreal _spatium = spatium();
      setPos(0.0, yoff() * _spatium);
      SLine::layout();

      //
      // special case:
      // if end segment is first chord/rest segment in measure,
      // shorten trill line so it ends at end of previous measure
      //
      Segment* seg1  = static_cast<Segment*>(startElement());
      Segment* seg2  = static_cast<Segment*>(endElement());
      if (seg2
         && (seg1->system() == seg2->system())
         && (spannerSegments().size() == 1)
         && (seg2->tick() == seg2->measure()->tick())
         ) {
            qreal x1   = seg2->pagePos().x();
            Measure* m = seg2->measure()->prevMeasure();
            if (m) {
                  Segment* s2 = m->last();
                  qreal x2 = s2->pagePos().x();
                  qreal dx = x1 - x2 + _spatium * .3;
                  TrillSegment* ls = static_cast<TrillSegment*>(frontSegment());
                  ls->setPos2(ls->pos2() + QPointF(-dx, 0.0));
                  ls->layout();
                  }
            }

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
      xml.tag("subtype", subtypeName());
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Trill::read
//---------------------------------------------------------

void Trill::read(const QDomElement& de)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();
      setId(de.attribute("id", "-1").toInt());
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            if (tag == "subtype")
                  setSubtype(e.text());
            else if (tag == "Accidental") {
                  _accidental = new Accidental(score());
                  _accidental->read(e);
                  }
            else if (!SLine::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Trill::setSubtype(const QString& s)
      {
      if (s == "trill" || s == "0")
            _subtype = TRILL_LINE;
      else if (s == "upprall")
            _subtype = UPPRALL_LINE;
      else if (s == "downprall")
            _subtype = DOWNPRALL_LINE;
      else if (s == "prallprall")
            _subtype = PRALLPRALL_LINE;
      else
            qDebug("Trill::setSubtype: unknown <%s>", qPrintable(s));
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

QString Trill::subtypeName() const
      {
      switch(subtype()) {
            case TRILL_LINE:
                  return "trill";
            case UPPRALL_LINE:
                  return "upprall";
            case DOWNPRALL_LINE:
                  return "downprall";
            case PRALLPRALL_LINE:
                  return "prallprall";
            default:
                  qDebug("unknown Trill subtype %d", subtype());
                  return "?";
            }
      }

