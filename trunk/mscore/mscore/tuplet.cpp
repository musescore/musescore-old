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
#include "xml.h"
#include "preferences.h"
#include "style.h"
#include "text.h"
#include "element.h"
#include "layout.h"

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
//   setSelected
//---------------------------------------------------------

void Tuplet::setSelected(bool f)
      {
      Element::setSelected(f);
      if (_number)
            _number->setSelected(f);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Tuplet::layout(ScoreLayout* layout)
      {
      double _spatium = layout->spatium();
      if (_hasNumber) {
            if (_number == 0) {
                  _number = new Text(score());
                  _number->setSubtype(TEXT_TUPLET);
                  _number->setParent(this);
                  _number->setText(QString("%1").arg(_actualNotes));
                  }
            }
      else {
            if (_number) {
                  if (_number->selected())
                        score()->deselect(_number);
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

      QPointF mp(measure->canvasPos());
      QPointF p1, p2;
      if (isUp) {
            if (cr1->type() == CHORD) {
                  Chord* chord1 = (Chord*)cr1;
                  Stem* stem = chord1->stem();
                  QPointF p(stem->abbox().topLeft());
                  p1 = p;
                  }
            else {
                  QPointF p(cr1->abbox().topLeft());
                  p1 = p;
                  }

            if (cr2->type() == CHORD) {
                  Chord* chord2 = (Chord*)cr2;
                  Stem* stem = chord2->stem();
                  QPointF p(stem->abbox().topLeft());
                  p2  = p;
                  }
            else  {
                  QPointF p(cr2->abbox().topRight());
                  p2 = p;
                  if (p1.y() < p2.y())
                        p2.setY(p1.y());
                  else
                        p1.setY(p2.y());
                  }
            }
      else {
            if (cr1->type() == CHORD) {
                  Chord* chord1 = (Chord*)cr1;
                  Stem* stem = chord1->stem();
                  QPointF p(stem->abbox().bottomLeft());
                  p1 = p;
                  }
            else {
                  QPointF p(cr1->abbox().bottomLeft());
                  p1 = p;
                  }

            if (cr2->type() == CHORD) {
                  Chord* chord2 = (Chord*)cr2;
                  Stem* stem = chord2->stem();
                  QPointF p(stem->abbox().bottomLeft());
                  p2 = p;
                  }
            else  {
                  QPointF p(cr2->abbox().bottomRight());
                  p2 = p;
                  if (p1.y() > p2.y())
                        p2.setY(p1.y());
                  else
                        p1.setY(p2.y());
                  }
            }
      p1 -= mp;
      p2 -= mp;

      // center number
      _number->layout(layout);
      qreal x3 = p1.x() + (p2.x() - p1.x()) * .5;
      qreal y3 = p1.y() + (p2.y() - p1.y()) * .5
         - _number->bbox().height() * .5
         - (l1 + l2) * (isUp ? 1.0 : -1.0);
      qreal numberWidth = _number->bbox().width();
      _number->setPos(QPointF(x3 - numberWidth * .5, y3));

      if (_hasLine) {
            qreal slope = (p2.y() - p1.y()) / (p2.x() - p1.x());

            if (isUp) {
                  bracketL[0] = QPointF(p1.x(), p1.y() - l2);
                  bracketL[1] = QPointF(p1.x(), p1.y() - l1 - l2);
                  qreal x = x3 - numberWidth * .5 - _spatium * .5;
                  qreal y = p1.y() + (x - p1.x()) * slope;
                  bracketL[2] = QPointF(x,   y - l1 - l2);

                  x = x3 + numberWidth * .5 + _spatium * .5;
                  y = p1.y() + (x - p1.x()) * slope;
                  bracketR[0] = QPointF(x,   y - l1 - l2);
                  bracketR[1] = QPointF(p2.x(), p2.y() - l1 - l2);
                  bracketR[2] = QPointF(p2.x(), p2.y() - l2);
                  }
            else {
                  bracketL[0] = QPointF(p1.x(), p1.y() + l2);
                  bracketL[1] = QPointF(p1.x(), p1.y() + l1 + l2);
                  qreal x     = x3 - numberWidth * .5 - _spatium * .5;
                  qreal y     = p1.y() + (x - p1.x()) * slope;
                  bracketL[2] = QPointF(x,   y + l1 + l2);

                  x = x3 + numberWidth * .5 + _spatium * .5;
                  y = p1.y() + (x - p1.x()) * slope;
                  bracketR[0] = QPointF(x,   y + l1 + l2);
                  bracketR[1] = QPointF(p2.x(), p2.y() + l1 + l2);
                  bracketR[2] = QPointF(p2.x(), p2.y() + l2);
                  }

            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Tuplet::bbox() const
      {
      QRectF r;
      //
      // _hasLine implies _hasNumber
      //
      if (_hasLine) {
            r = bracketL.boundingRect() | bracketR.boundingRect();
            r |= _number->bbox().translated(_number->pos());
            }
      else if (_hasNumber)
            r |= _number->bbox().translated(_number->pos());
      return r;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Tuplet::draw(QPainter& p)
      {
      if (_number) {
            // ? p.setPen(QPen(Qt::NoPen));
            // p.setBrush(_number->selected() ? preferences.selectColor[0] : Qt::black);
            p.save();
            p.translate(_number->pos());
            _number->draw(p);
            p.restore();
            if (_hasLine) {
                  QPen pen(p.pen());
                  pen.setWidthF(_spatium * 0.1);
                  p.setPen(pen);
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
                  _number->setSubtype(TEXT_TUPLET);   // override read
                  }
            else if (Element::readProperties(node))
                  ;
            else
                  domError(node);
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Tuplet::add(Element* e)
      {
      switch(e->type()) {
            case TEXT:
                  _number = (Text*)e;
                  break;
            case CHORD:
            case REST:
                  _elements.add((ChordRest*)e);
                  break;
            default:
                  printf("Tuplet::add() unknown element\n");
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Tuplet::remove(Element* e)
      {
      switch(e->type()) {
            case TEXT:
                  if (e == _number)
                        _number = 0;
                  break;
            case CHORD:
            case REST:
                  for (iChordRest i = _elements.begin(); i != _elements.end(); ++i) {
                        if (i->second == e) {
                              _elements.erase(i);
                              return;
                              }
                        }
                  break;
            default:
                  printf("Tuplet::remove() unknown element\n");
                  break;
            }
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Tuplet::genPropertyMenu(QMenu* popup) const
      {
      QAction* a = popup->addAction(QT_TR_NOOP("Show number"));
      a->setData("number");
      a->setCheckable(true);
      a->setChecked(_hasNumber);
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Tuplet::propertyAction(const QString& s)
      {
      if (s == "number") {
            _hasNumber = !_hasNumber;
            }
//TODO      layout();
      }

