//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: tuplet.cpp,v 1.12 2006/03/13 21:35:59 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include "tuplet.h"
#include "score.h"
#include "chord.h"
#include "painter.h"
#include "xml.h"
#include "preferences.h"
#include "style.h"
#include "text.h"
#include "element.h"

//---------------------------------------------------------
//   Tuplet
//---------------------------------------------------------

Tuplet::Tuplet(Score* s)
  : Element(s), bracketL(3), bracketR(3)
      {
      _hasNumber = true;
      _hasLine   = true;
      _number    = 0;
      }

//---------------------------------------------------------
//   Tuplet
//---------------------------------------------------------

Tuplet::~Tuplet()
      {
      //
      // delete all references
      //
      for (iChordRest i = _elements.begin(); i != _elements.end(); ++i) {
            ChordRest* cr = i->second;
            cr->setTuplet(0);
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Tuplet::remove(ChordRest* a)
      {
      for (iChordRest i = _elements.begin(); i != _elements.end(); ++i) {
            if (i->second == a) {
                  _elements.erase(i);
                  return;
                  }
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Tuplet::layout()
      {
      if (_hasNumber) {
            if (_number == 0) {
                  _number = new Text(score());
                  _number->setSubtype(TEXT_FINGERING);
                  _number->setParent(this);
                  _number->setText(QString("%1").arg(_actualNotes));
                  }
            }
      else {
            if (_number) {
                  delete _number;
                  _number = 0;
                  }
            return;
            }

      qreal l1 = _spatium;          // bracket tip height
      qreal l2 = _spatium * .5;     // bracket distance to note

      if (_elements.empty()) {
            printf("layout: not tuplet members\n");
            return;
            }

      //
      // find out main direction
      //
      int up = 1;
      for (iChordRest i = _elements.begin(); i != _elements.end(); ++i) {
            ChordRest* cr = i->second;
            if (cr->type() == CHORD)
                  up += (cr->isUp() ? +1 : -1);
            }
      bool isUp = up > 0;

      //
      // set all elements to main direction
      //
      for (iChordRest i = _elements.begin(); i != _elements.end(); ++i)
            i->second->setUp(up);

      const ChordRest* cr1 = _elements.front();
      const ChordRest* cr2 = _elements.back();
      Measure* measure = cr1->measure();
      if (cr1->beam())
            _hasLine = false;

      QPointF mp(measure->apos());
      qreal p1x, p1y, p2x, p2y;
      if (isUp) {
            if (cr1->type() == CHORD) {
                  Chord* chord1 = (Chord*)cr1;
                  Stem* stem = chord1->stem();
                  QPointF p(stem->abbox().topLeft());
                  p1x = p.x();
                  p1y = p.y();
                  }
            else {
                  QPointF p(cr1->abbox().topLeft());
                  p1x = p.x();
                  p1y = p.y();
                  }

            if (cr2->type() == CHORD) {
                  Chord* chord2 = (Chord*)cr2;
                  Stem* stem = chord2->stem();
                  QPointF p(stem->abbox().topLeft());
                  p2x = p.x();
                  p2y = p.y();
                  }
            else  {
                  QPointF p(cr2->abbox().topRight());
                  p2x = p.x();
                  p2y = p.y();
                  if (p1y < p2y)
                        p2y = p1y;
                  else
                        p1y = p2y;
                  }
            }
      else {
            if (cr1->type() == CHORD) {
                  Chord* chord1 = (Chord*)cr1;
                  Stem* stem = chord1->stem();
                  QPointF p(stem->abbox().bottomLeft());
                  p1x = p.x();
                  p1y = p.y();
                  }
            else {
                  QPointF p(cr1->abbox().bottomLeft());
                  p1x = p.x();
                  p1y = p.y();
                  }

            if (cr2->type() == CHORD) {
                  Chord* chord2 = (Chord*)cr2;
                  Stem* stem = chord2->stem();
                  QPointF p(stem->abbox().bottomLeft());
                  p2x = p.x();
                  p2y = p.y();
                  }
            else  {
                  QPointF p(cr2->abbox().bottomRight());
                  p2x = p.x();
                  p2y = p.y();
                  if (p1y > p2y)
                        p2y = p1y;
                  else
                        p1y = p2y;
                  }
            }

      p1x -= mp.x();
      p1y -= mp.y();
      p2x -= mp.x();
      p2y -= mp.y();

      // center number
      qreal x3 = p1x + (p2x - p1x) * .5;
      qreal y3 = p1y + (p2y - p1y) * .5 - _number->bbox().height(); // - (l1 + l2) * (isUp ? 1.0 : -1.0);
      qreal numberWidth = _number->bbox().width();
      _number->setPos(QPointF(x3 - numberWidth * .5, y3));

      if (_hasLine) {
            qreal slope = (p2y - p1y) / (p2x - p1x);

            if (isUp) {
                  bracketL[0] = QPointF(p1x, p1y - l2);
                  bracketL[1] = QPointF(p1x, p1y - l1 - l2);
                  qreal x = x3 - numberWidth * .5 - _spatium * .5;
                  qreal y = p1y + (x - p1x) * slope;
                  bracketL[2] = QPointF(x,   y - l1 - l2);

                  x = x3 + numberWidth * .5 + _spatium * .5;
                  y = p1y + (x - p1x) * slope;
                  bracketR[0] = QPointF(x,   y - l1 - l2);
                  bracketR[1] = QPointF(p2x, p2y - l1 - l2);
                  bracketR[2] = QPointF(p2x, p2y - l2);
                  }
            else {
                  bracketL[0] = QPointF(p1x, p1y + l2);
                  bracketL[1] = QPointF(p1x, p1y + l1 + l2);
                  qreal x     = x3 - numberWidth * .5 - _spatium * .5;
                  qreal y     = p1y + (x - p1x) * slope;
                  bracketL[2] = QPointF(x,   y + l1 + l2);

                  x = x3 + numberWidth * .5 + _spatium * .5;
                  y = p1y + (x - p1x) * slope;
                  bracketR[0] = QPointF(x,   y + l1 + l2);
                  bracketR[1] = QPointF(p2x, p2y + l1 + l2);
                  bracketR[2] = QPointF(p2x, p2y + l2);
                  }

            setbbox(bracketL.boundingRect() | bracketR.boundingRect() | _number->bbox().translated(_number->pos()));
            }
      else
            setbbox(_number->bbox().translated(_number->pos()));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Tuplet::draw1(Painter& p)
      {
      if (_number) {
            // ? p.setPen(QPen(Qt::NoPen));
            p.setBrush(_number->selected() ? preferences.selectColor[0] : Qt::black);
            _number->draw(p);
            if (_hasLine) {
                  p.setPen(QPen(Qt::black, 5));
                  p.drawPolyline(bracketL);
                  p.drawPolyline(bracketR);
                  }
            }
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void Tuplet::move(double x, double y)
      {
      Element::move(x, y);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tuplet::write(Xml& xml, int id) const
      {
      xml.stag(QString("Tuplet id=\"%1\"").arg(id));
      xml.tag("hasNumber", _hasNumber);
      xml.tag("hasLine", _hasLine);
      xml.tag("baseLen", _baseLen);
      xml.tag("normalNotes", _normalNotes);
      xml.tag("actualNotes", _actualNotes);
      if (_number)
            _number->write(xml, "Number");
      xml.etag("Tuplet");
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tuplet::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            int i = e.text().toInt();
            if (tag == "hasNumber")
                  _hasNumber = i;
            else if (tag == "hasLine")
                  _hasLine = i;
            else if (tag == "baseLen")
                  _baseLen = i;
            else if (tag == "normalNotes")
                  _normalNotes = i;
            else if (tag == "actualNotes")
                  _actualNotes = i;
            else if (tag == "Number") {
                  _number = new Text(score());
                  _number->setParent(this);
                  _number->read(node);
                  }
            else if (Element::readProperties(node))
                  ;
            else
                  domError(node);
            }
      }

void Tuplet::add(Element* e)
      {
      if (e->type() != TEXT)
            return;
      _number = (Text*)e;
      }

void Tuplet::remove(Element* e)
      {
      if (e == _number)
            _number = 0;
      }

