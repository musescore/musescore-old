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
  : DurationElement(s)
      {
      _numberType   = SHOW_NUMBER;
      _bracketType  = AUTO_BRACKET;
      _number       = 0;
      _hasBracket   = false;
      _userModified = false;
      _p1           = QPointF();
      _p2           = QPointF();
      }

//---------------------------------------------------------
//   ~Tuplet
//---------------------------------------------------------

Tuplet::~Tuplet()
      {
      //
      // delete all references
      //
      foreach(DurationElement* e, _elements)
            e->setTuplet(0);
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
                  }
            if (_numberType == SHOW_NUMBER)
                  _number->setText(QString("%1").arg(_actualNotes));
            else
                  _number->setText(QString("%1:%2").arg(_actualNotes).arg(_normalNotes));
            }
      else {
            if (_number) {
                  if (_number->selected())
                        score()->deselect(_number);
                  delete _number;
                  _number = 0;
                  }
            }

      if (_elements.empty()) {
            printf("layout: not tuplet members\n");
            return;
            }

      //
      // find out main direction
      //
      int up = 1;
      foreach(const DurationElement* e, _elements) {
            if (e->type() == CHORD) {
                  const Chord* c = static_cast<const Chord*>(e);
                  if (c->stemDirection() != AUTO)
                        up += c->stemDirection() == UP ? 1000 : -1000;
                  else
                        up += c->up() ? 1 : -1;
                  }
            else if (e->type() == TUPLET) {
                  // TODO
                  }
            }
      bool isUp = up > 0;

      //
      // set all elements to main direction
      //
      foreach(DurationElement* e, _elements) {
            if (e->type() == CHORD || e->type() == REST)
                  static_cast<ChordRest*>(e)->setUp(up);
            }

      const DurationElement* cr1 = _elements.front();
      const DurationElement* cr2 = _elements.back();
      if (cr1->beam()) {
            if (_bracketType == AUTO_BRACKET)
                  _hasBracket = false;
            else
                  _hasBracket = _bracketType == SHOW_BRACKET;
            }
      else {
            _hasBracket = _bracketType != SHOW_NO_BRACKET;
            }

      if (isUp) {
            if (cr1->type() == CHORD) {
                  const Chord* chord1 = static_cast<const Chord*>(cr1);
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
                  const Chord* chord2 = static_cast<const Chord*>(cr2);
                  Stem* stem = chord2->stem();
                  if (stem)
                        p2 = QPointF(stem->abbox().topLeft());
                  else
                        p2 = QPointF(cr2->abbox().topLeft());
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
                  const Chord* chord1 = static_cast<const Chord*>(cr1);
                  Stem* stem = chord1->stem();
                  if (stem)
                        p1 = QPointF(stem->abbox().bottomLeft());
                  else
                        p1 = QPointF();
                  }
            else {
                  QPointF p(cr1->abbox().bottomLeft());
                  p1 = p;
                  }

            if (cr2->type() == CHORD) {
                  const Chord* chord2 = static_cast<const Chord*>(cr2);
                  Stem* stem = chord2->stem();
                  if (stem)
                        p2 = QPointF(stem->abbox().bottomLeft());
                  else
                        p2 = QPointF();
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

      if (_userModified) {
            p1.rx() += _p1.x();
            p1.setY(_p1.y());
            p2.rx() += _p2.x();
            p2.setY(_p2.y());
            }
      _p1.setY(p1.y());
      _p2.setY(p2.y());

      // center number
      qreal x3 = 0.0, y3 = 0.0;
      qreal numberWidth = 0.0;
      if (_number) {
            _number->layout(layout);
            x3 = p1.x() + (p2.x() - p1.x()) * .5;

            y3 = p1.y() + (p2.y() - p1.y()) * .5
               - _number->bbox().height() * .5
               - (l1 + l2) * (isUp ? 1.0 : -1.0);

            numberWidth = _number->bbox().width();
            _number->setPos(QPointF(x3 - numberWidth * .5, y3) - ipos());
            }

      if (_hasBracket) {
            qreal slope = (p2.y() - p1.y()) / (p2.x() - p1.x());

            if (isUp) {
                  if (_number) {
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
                        bracketL[0] = QPointF(p1.x(), p1.y() - l2);
                        bracketL[1] = QPointF(p1.x(), p1.y() - l1 - l2);
                        bracketL[2] = QPointF(p2.x(), p2.y() - l1 - l2);
                        bracketL[3] = QPointF(p2.x(), p2.y() - l2);
                        }
                  }
            else {
                  if (_number) {
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
                  else {
                        bracketL[0] = QPointF(p1.x(), p1.y() + l2);
                        bracketL[1] = QPointF(p1.x(), p1.y() + l1 + l2);
                        bracketL[2] = QPointF(p2.x(), p2.y() + l1 + l2);
                        bracketL[3] = QPointF(p2.x(), p2.y() + l2);
                        }
                  }

            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Tuplet::bbox() const
      {
      QRectF r;
      if (_number) {
            r |= _number->bbox().translated(_number->pos());
            if (_hasBracket) {
                  QRectF b;
                  b.setCoords(bracketL[1].x(), bracketL[1].y(), bracketR[2].x(), bracketR[2].y());
                  r |= b;
                  }
            }
      else if (_hasBracket) {
            QRectF b;
            b.setCoords(bracketL[1].x(), bracketL[1].y(), bracketL[3].x(), bracketL[3].y());
            r |= b;
            }
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
            }
      if (_hasBracket) {
            QPen pen(p.pen());
            pen.setWidthF(_spatium * 0.1);
            p.setPen(pen);
            if (!_number)
                  p.drawPolyline(bracketL, 4);
            else {
                  p.drawPolyline(bracketL, 3);
                  p.drawPolyline(bracketR, 3);
                  }
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tuplet::write(Xml& xml) const
      {
      xml.stag(QString("Tuplet id=\"%1\"").arg(_id));
      xml.tag("numberType", _numberType);
      xml.tag("bracketType", _bracketType);
      xml.tag("baseLen", _baseLen);
      xml.tag("normalNotes", _normalNotes);
      xml.tag("actualNotes", _actualNotes);
      if (_number)
            _number->write(xml, "Number");
      if (_userModified) {
            xml.tag("p1", _p1);
            xml.tag("p2", _p2);
            }
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
            else if (tag == "p1") {
                  _userModified = true;
                  _p1 = readPoint(e);
                  }
            else if (tag == "p2") {
                  _userModified = true;
                  _p2 = readPoint(e);
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
      foreach(DurationElement* el, _elements) {
            if (el == e)
                  printf("Tuplet::add: %s already there\n", el->name());
            }

      switch(e->type()) {
            case TEXT:
                  _number = static_cast<Text*>(e);
                  break;
            case CHORD:
            case REST:
            case TUPLET:
                  {
                  int i;
                  for (i = 0; i < _elements.size(); ++i) {
                        if (_elements[i]->tick() > e->tick()) {
                              _elements.insert(i, static_cast<DurationElement*>(e));
                              break;
                              }
                        }
                  if (i == _elements.size())
                        _elements.append(static_cast<DurationElement*>(e));
                  }

                  // the tick position of a tuplet is the tick position of its
                  // first element:
                  setTick(_elements.front()->tick());
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
            case TUPLET:
                  if (!_elements.removeOne(static_cast<DurationElement*>(e)))
                        printf("Tuplet::remove: cannot find element\n");
                  break;
            default:
                  printf("Tuplet::remove: unknown element\n");
                  break;
            }
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Tuplet::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Tuplet Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Tuplet::propertyAction(const QString& s)
      {
      if (s == "props") {
            TupletProperties vp(this);
            vp.exec();
            }
      else
            Element::propertyAction(s);
      }

//---------------------------------------------------------
//   tupletDialog
//    create tuplet dialog
//---------------------------------------------------------

void Score::tupletDialog()
      {
      ChordRest* cr = getSelectedChordRest();
      if (cr == 0)
            return;
      TupletDialog td;
      if (!td.exec())
            return;

      int len        = cr->tickLen();
      if (cr->type() == REST && len == 0)
            len = cr->measure()->tickLen();
      int baseLen    = len / td.getNormalNotes();
      Tuplet* tuplet = new Tuplet(this);
      tuplet->setBaseLen(baseLen);
      tuplet->setTrack(cr->track());
      tuplet->setTick(cr->tick());
      td.setupTuplet(tuplet);

      Measure* measure = cr->measure();
      tuplet->setParent(measure);

      cmdCreateTuplet(cr, tuplet);
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
      if (number->isChecked())
            tuplet->setNumberType(Tuplet::SHOW_NUMBER);
      else if (relation->isChecked())
            tuplet->setNumberType(Tuplet::SHOW_RELATION);
      else if (noNumber->isChecked())
            tuplet->setNumberType(Tuplet::NO_TEXT);
      if (autoBracket->isChecked())
            tuplet->setBracketType(Tuplet::AUTO_BRACKET);
      else if (bracket->isChecked())
            tuplet->setBracketType(Tuplet::SHOW_BRACKET);
      else if (noBracket->isChecked())
            tuplet->setBracketType(Tuplet::SHOW_NO_BRACKET);
      }

//---------------------------------------------------------
//   TupletProperties
//---------------------------------------------------------

TupletProperties::TupletProperties(Tuplet* t, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      tuplet = t;
      switch(tuplet->numberType()) {
            case Tuplet::SHOW_NUMBER:
                  number->setChecked(true);
                  break;
            case Tuplet::SHOW_RELATION:
                  relation->setChecked(true);
                  break;
            case Tuplet::NO_TEXT:
                  noNumber->setChecked(true);
                  break;
            }
      switch(tuplet->bracketType()) {
            case Tuplet::AUTO_BRACKET:
                  autoBracket->setChecked(true);
                  break;
            case Tuplet::SHOW_BRACKET:
                  bracket->setChecked(true);
                  break;
            case Tuplet::SHOW_NO_BRACKET:
                  noBracket->setChecked(true);
                  break;
            }
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void TupletProperties::accept()
      {
      if (number->isChecked())
            tuplet->setNumberType(Tuplet::SHOW_NUMBER);
      else if (relation->isChecked())
            tuplet->setNumberType(Tuplet::SHOW_RELATION);
      else if (noNumber->isChecked())
            tuplet->setNumberType(Tuplet::NO_TEXT);
      if (autoBracket->isChecked())
            tuplet->setBracketType(Tuplet::AUTO_BRACKET);
      else if (bracket->isChecked())
            tuplet->setBracketType(Tuplet::SHOW_BRACKET);
      else if (noBracket->isChecked())
            tuplet->setBracketType(Tuplet::SHOW_NO_BRACKET);
      QDialog::accept();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool Tuplet::startEdit(Viewer*, const QPointF&)
      {
      return _hasBracket;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Tuplet::editDrag(int grip, const QPointF& d)
      {
      if (grip == 0)
            _p1 += d;
      else
            _p2 += d;
      _userModified = true;
      setGenerated(false);
      score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Tuplet::updateGrips(int* grips, QRectF*grip) const
      {
      *grips = 2;
      grip[0].translate(canvasPos() + p1);
      grip[1].translate(canvasPos() + p2);
      }

//---------------------------------------------------------
//   resetUserOffsets
//---------------------------------------------------------

void Tuplet::resetUserOffsets()
      {
      _userModified = false;
      _p1           = QPointF();
      _p2           = QPointF();
      setGenerated(true);
      }
