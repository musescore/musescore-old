//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: tuplet.cpp,v 1.12 2006/03/13 21:35:59 wschweer Exp $
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

#include "tuplet.h"
#include "score.h"
#include "chord.h"
#include "note.h"
#include "xml.h"
#include "preferences.h"
#include "style.h"
#include "text.h"
#include "element.h"
#include "layout.h"
#include "utils.h"

//---------------------------------------------------------
//   Tuplet
//---------------------------------------------------------

Tuplet::Tuplet(Score* s)
  : Element(s), bracketL(3), bracketR(3)
      {
      _numberType  = SHOW_NUMBER;
      _bracketType = AUTO_BRACKET;
      _number      = 0;
      _hasBracket  = false;
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
      if (_numberType != NO_TEXT) {
            if (_number == 0) {
                  _number = new Text(score());
                  _number->setSubtype(TEXT_TUPLET);
                  _number->setParent(this);
                  if (_numberType == SHOW_NUMBER)
                        _number->setText(QString("%1").arg(_actualNotes));
                  else
                        _number->setText(QString("%1:%2").arg(_actualNotes).arg(_normalNotes));

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
            if (cr->type() == CHORD) {
                  Chord* c = (Chord*) cr;
                  if (c->stemDirection() != AUTO)
                        up += c->stemDirection() == UP ? 1000 : -1000;
                  else
                        up += c->up() ? 1 : -1;
                  }
            }
      bool isUp = up > 0;

      //
      // set all elements to main direction
      //
      for (iChordRest i = _elements.begin(); i != _elements.end(); ++i)
            i->second->setUp(up);

      const ChordRest* cr1 = _elements.front();
      const ChordRest* cr2 = _elements.back();
      if (cr1->beam())
            _hasBracket = false;

      QPointF p1, p2;
      if (isUp) {
            if (cr1->type() == CHORD) {
                  Chord* chord1 = (Chord*)cr1;
                  Stem* stem = chord1->stem();
                  if (stem)
                        p1 = QPointF(stem->abbox().topLeft());
                  else
                        p1 = QPointF(cr1->abbox().topLeft());

                  }
            else {
                  p1 = QPointF(cr1->abbox().topLeft());
                  }

            if (cr2->type() == CHORD) {
                  Chord* chord2 = (Chord*)cr2;
                  Stem* stem    = chord2->stem();
                  p2            = QPointF(stem->abbox().topLeft());
                  }
            else  {
                  p2 = QPointF(cr2->abbox().topRight());
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

      qreal l1 = _spatium;          // bracket tip height
      qreal l2 = _spatium * .5;     // bracket distance to note

      setPos(0.0, 0.0);
      QPointF mp(parent()->canvasPos());
      p1 -= mp;
      p2 -= mp;

      // center number
      _number->layout(layout);
      qreal x3 = p1.x() + (p2.x() - p1.x()) * .5;

      qreal y3 = p1.y() + (p2.y() - p1.y()) * .5
         - _number->bbox().height() * .5
         - (l1 + l2) * (isUp ? 1.0 : -1.0);

      qreal numberWidth = _number->bbox().width();
      _number->setPos(QPointF(x3 - numberWidth * .5, y3) - ipos());

      if (_hasBracket) {
            qreal slope = (p2.y() - p1.y()) / (p2.x() - p1.x());

            if (isUp) {
                  bracketL[0] = QPointF(p1.x(), p1.y() - l2);
                  bracketL[1] = QPointF(p1.x(), p1.y() - l1 - l2);
                  qreal x     = x3 - numberWidth * .5 - _spatium * .5;
                  qreal y     = p1.y() + (x - p1.x()) * slope;
                  bracketL[2] = QPointF(x,   y - l1 - l2);

                  x           = x3 + numberWidth * .5 + _spatium * .5;
                  y           = p1.y() + (x - p1.x()) * slope;
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

                  x           = x3 + numberWidth * .5 + _spatium * .5;
                  y           = p1.y() + (x - p1.x()) * slope;
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
      // _hasBracket implies _hasNumber
      //
      if (_hasBracket) {
            r = bracketL.boundingRect() | bracketR.boundingRect();
            r |= _number->bbox().translated(_number->pos());
            }
      if (_numberType != NO_TEXT)
            r |= _number->bbox().translated(_number->pos());
      return r;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Tuplet::draw(QPainter& p) const
      {
      if (_number) {
            p.save();
            p.translate(_number->pos());
            _number->draw(p);
            p.restore();
            if (_hasBracket) {
                  QPen pen(p.pen());
                  pen.setWidthF(_spatium * 0.1);
                  p.setPen(pen);
                  p.drawPolyline(bracketL);
                  p.drawPolyline(bracketR);
                  }
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tuplet::write(Xml& xml, int id) const
      {
      xml.stag(QString("Tuplet id=\"%1\"").arg(id));
      xml.tag("numberType", _numberType);
      xml.tag("bracketType", _bracketType);
      xml.tag("baseLen", _baseLen);
      xml.tag("normalNotes", _normalNotes);
      xml.tag("actualNotes", _actualNotes);
      if (_number)
            _number->write(xml, "Number");
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tuplet::read(QDomElement e)
      {
      _id = e.attribute("id", "0").toInt();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            int i = e.text().toInt();
            if (tag == "hasNumber")             // obsolete
                  _numberType = i ? SHOW_NUMBER : NO_TEXT;
            else if (tag == "hasLine") {          // obsolete
                  _hasBracket = i;
                  _bracketType = AUTO_BRACKET;
                  }
            else if (tag == "numberType")
                  _numberType = i;
            else if (tag == "bracketType")
                  _bracketType = i;
            else if (tag == "baseLen")
                  _baseLen = i;
            else if (tag == "normalNotes")
                  _normalNotes = i;
            else if (tag == "actualNotes")
                  _actualNotes = i;
            else if (tag == "Number") {
                  _number = new Text(score());
                  _number->setParent(this);
                  _number->read(e);
                  _number->setSubtype(TEXT_TUPLET);   // override read
                  }
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Tuplet::add(Element* e)
      {
#if 0
      for (iChordRest i = _elements.begin(); i != _elements.end(); ++i) {
            if (i->second == e) {
                  printf("Tuplet::add: already there\n");
                  abort();
                  }
            }
#endif
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
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Show number"));
      a->setData("number");
      a->setCheckable(true);
//      a->setChecked(_hasNumber);
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Tuplet::propertyAction(const QString& s)
      {
/*      if (s == "number") {
            _hasNumber = !_hasNumber;
            }
      else
*/
            Element::propertyAction(s);
      }

//---------------------------------------------------------
//   tupletDialog
//    create tuplet dialog
//---------------------------------------------------------

void Score::tupletDialog()
      {
      Note* note = getSelectedNote();
      if (note == 0) {
            selectNoteMessage();
            return;
            }
      TupletDialog td;
      if (!td.exec())
            return;

      Chord* chord   = note->chord();
      int baseLen    = chord->tickLen() / td.getNormalNotes();
      Tuplet* tuplet = new Tuplet(this);
      tuplet->setBaseLen(baseLen);
      tuplet->setTrack(chord->track());
      td.setupTuplet(tuplet);

      Measure* measure = chord->measure();
      tuplet->setParent(measure);

      cmdCreateTuplet(chord, tuplet);
      }

//---------------------------------------------------------
//   TupletDialog
//---------------------------------------------------------

TupletDialog::TupletDialog(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      }

//---------------------------------------------------------
//   setupTuplet
//---------------------------------------------------------

void TupletDialog::setupTuplet(Tuplet* tuplet)
      {
      tuplet->setNormalNotes(normalNotes->value());
      tuplet->setActualNotes(actualNotes->value());
      }

