//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "arpeggio.h"
#include "sym.h"
#include "chord.h"
#include "note.h"
#include "score.h"
#include "staff.h"
#include "part.h"
#include "segment.h"
#include "painter.h"

//---------------------------------------------------------
//   Arpeggio
//---------------------------------------------------------

Arpeggio::Arpeggio(Score* s)
  : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      setHeight(spatium() * 4);      // for use in palettes
      _span = 1;
      }

//---------------------------------------------------------
//   setHeight
//---------------------------------------------------------

void Arpeggio::setHeight(double h)
      {
      _height = h;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Arpeggio::write(Xml& xml) const
      {
      xml.stag("Arpeggio");
      Element::writeProperties(xml);
      if (_userLen1.val() != 0.0)
            xml.sTag("userLen1", _userLen1);
      if (_userLen2.val() != 0.0)
            xml.sTag("userLen2", _userLen2);
      if (_span != 1)
            xml.tag("span", _span);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Arpeggio::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "userLen1")
                  _userLen1 = Spatium(val.toDouble());
            else if (tag == "userLen2")
                  _userLen2 = Spatium(val.toDouble());
            else if (tag == "span")
                  _span = val.toInt();
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Arpeggio::bbox() const
      {
      double _spatium = spatium();
      double y1 = -_userLen1.val() * _spatium;
      double y2 = _height + _userLen2.val() * _spatium;
      switch (subtype()) {
            case ARP_NORMAL:
            case ARP_UP:
            case ARP_DOWN:
            default:
                  return QRectF(0.0, y1, symbols[score()->symIdx()][arpeggioSym].width(magS()), y2-y1);
            case ARP_BRACKET:
                  {
                  double lw = score()->styleS(ST_ArpeggioLineWidth).val() * _spatium;
                  double w = score()->styleS(ST_ArpeggioHookLen).val() * _spatium;
                  return QRectF(-lw * .5, y1 - lw * .5, w + lw, y2 - y1 + lw);
                  }
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Arpeggio::draw(Painter* p) const
      {
      double _spatium = spatium();

      double y1 = _spatium - _userLen1.val() * _spatium;
      double y2 = _height + (_userLen2.val() + .5) * _spatium;
      switch (subtype()) {
            case ARP_NORMAL:
                  for (double y = y1; y < y2; y += _spatium)
                        symbols[score()->symIdx()][arpeggioSym].draw(p, 1.0, 0.0, y);
                  break;
            case ARP_UP:
                  symbols[score()->symIdx()][arpeggioarrowupSym].draw(p, 1.0, 0.0, y1);
                  for (double y = y1 + _spatium; y < y2; y += _spatium)
                        symbols[score()->symIdx()][arpeggioSym].draw(p, 1.0, 0.0, y);
                  break;
            case ARP_DOWN:
                  {
                  double y = y1;
                  for (; y < y2 - _spatium; y += _spatium)
                        symbols[score()->symIdx()][arpeggioSym].draw(p, 1.0, 0.0, y);
                  symbols[score()->symIdx()][arpeggioarrowdownSym].draw(p, 1.0, 0.0, y);
                  }
                  break;
            case ARP_BRACKET:
                  {
                  y1 = - _userLen1.val() * _spatium;
                  y2 = _height + _userLen2.val() * _spatium;
                  p->save();

                  p->setLineWidth(score()->styleS(ST_ArpeggioLineWidth).val() * _spatium);
                  p->setCapStyle(Qt::RoundCap);

                  double w = score()->styleS(ST_ArpeggioHookLen).val() * _spatium;
                  p->drawLine(0.0, y1, 0.0, y2);
                  p->drawLine(0.0, y1, w, y1);
                  p->drawLine(0.0, y2, w, y2);
                  p->restore();
                  }
                  break;
            }
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Arpeggio::updateGrips(int* grips, QRectF* grip) const
      {
      double _spatium = spatium();
      *grips   = 2;
      QPointF p1(0.0, -_userLen1.val() * _spatium);
      QPointF p2(0.0, _height + _userLen2.val() * _spatium);
      grip[0].translate(canvasPos() + p1);
      grip[1].translate(canvasPos() + p2);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Arpeggio::editDrag(const EditData& ed)
      {
      Spatium d(ed.delta.y() / spatium());
      if (ed.curGrip == 0)
            _userLen1 -= d;
      else if (ed.curGrip == 1)
            _userLen2 += d;
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Arpeggio::dragAnchor() const
      {
      Chord* c = chord();
      if (c)
            return QLineF(canvasPos(), c->upNote()->canvasPos());
      return QLineF();
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF Arpeggio::gripAnchor(int n) const
      {
      Chord* c = chord();
      if (c == 0)
            return QPointF();
      if (n == 0)
            return c->upNote()->canvasPos();
      else if (n == 1) {
            Note* dnote = c->downNote();
            int btrack  = track() + (_span - 1) * VOICES;
            ChordRest* bchord = static_cast<ChordRest*>(c->segment()->element(btrack));
            if (bchord && bchord->type() == CHORD)
                  dnote = static_cast<Chord*>(bchord)->downNote();
            return dnote->canvasPos();
            }
      return QPointF();
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Arpeggio::edit(ScoreView*, int curGrip, int key, Qt::KeyboardModifiers modifiers, const QString&)
      {
      if (curGrip != 1 || !(modifiers & Qt::ShiftModifier))
            return false;

      if (key == Qt::Key_Down) {
            Staff* s = staff();
            Part* part = s->part();
            int n = part->nstaves();
            int ridx = part->staves()->indexOf(s);
            if (ridx >= 0) {
                  if (_span + ridx < n)
                        ++_span;
                  }
            }
      else if (key == Qt::Key_Up) {
            if (_span > 1)
                  --_span;
            }
      else
            return false;
      layout();
      Chord* c = chord();
      rxpos() = -(width() + spatium() * .5);
      c->layoutArpeggio2();
      return true;
      }
