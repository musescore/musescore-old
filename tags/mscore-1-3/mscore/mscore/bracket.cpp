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

#include "bracket.h"
#include "xml.h"
#include "style.h"
#include "preferences.h"
#include "utils.h"
#include "staff.h"
#include "score.h"
// #include "scoreview.h"
#include "system.h"
#include "sym.h"

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

Bracket::Bracket(Score* s)
   : Element(s)
      {
      h2       = 3.5 * spatium();
      _span    = 1;
      _column   = 0;
      yoff     = 0.0;
      }

//---------------------------------------------------------
//   setHeight
//---------------------------------------------------------

void Bracket::setHeight(qreal h)
      {
      h2 = h * .5;
      }

//---------------------------------------------------------
//   width
//---------------------------------------------------------

double Bracket::width() const
      {
      double w;
      if (subtype() == BRACKET_AKKOLADE)
            w = point(score()->styleS(ST_akkoladeWidth));
      else
            w = point(score()->styleS(ST_bracketWidth) + score()->styleS(ST_bracketDistance));
      return w;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Bracket::layout()
      {
      double _spatium = spatium();
      path = QPainterPath();
      if (h2 == 0.0)
            return;

      qreal h = h2 + yoff * .5;

      if (subtype() == BRACKET_AKKOLADE) {
            qreal w         = point(score()->styleS(ST_akkoladeWidth));
            const double X1 =  2.0 * w;
            const double X2 = -0.7096 * w;
            const double X3 = -1.234 * w;
            const double X4 =  1.734 * w;

            path.moveTo(0, h);
            path.cubicTo(X1,  h + h * .3359, X2,  h + h * .5089, w, 2 * h);
            path.cubicTo(X3,  h + h * .5025, X4,  h + h * .2413, 0, h);
            path.cubicTo(X1,  h - h * .3359, X2,  h - h * .5089, w, 0);
            path.cubicTo(X3,  h - h * .5025, X4,  h - h * .2413, 0, h);
            }
      else if (subtype() == BRACKET_NORMAL) {
            qreal w = point(score()->styleS(ST_bracketWidth));

            TextStyle* s = score()->textStyle(TEXT_STYLE_SYMBOL1);
            QChar up = symbols[brackettipsRightUp].code();
            QChar down = symbols[brackettipsRightDown].code();

            QFont f(s->family, lrint(2.0 * _spatium));

            qreal o   = _spatium * .17;
            qreal slw = point(score()->styleS(ST_staffLineWidth));

            path.setFillRule(Qt::WindingFill);

            path.addText(QPointF(0.0, -o), f, QString(up));
            path.addText(QPointF(0.0, h * 2.0 + o), f, QString(down));
            path.addRect(0.0, -slw * .5, w, h * 2.0 + slw);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Bracket::draw(QPainter& p) const
      {
      p.setBrush(p.pen().color());
      p.drawPath(path);
      }

//---------------------------------------------------------
//   Bracket::write
//---------------------------------------------------------

void Bracket::write(Xml& xml) const
      {
      switch(subtype()) {
            case BRACKET_AKKOLADE:
                  xml.stag("Bracket type=\"Akkolade\"");
                  break;
            case BRACKET_NORMAL:
                  xml.stag("Bracket");
                  break;
            }
      if (_column)
            xml.tag("level", _column);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Bracket::read
//---------------------------------------------------------

void Bracket::read(QDomElement e)
      {
      QString t(e.attribute("type", "Normal"));

      if (t == "Normal")
            setSubtype(BRACKET_NORMAL);
      else if (t == "Akkolade")
            setSubtype(BRACKET_AKKOLADE);
      else
            fprintf(stderr, "unknown brace type <%s>\n", t.toLatin1().data());

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "level")
                  _column = val.toInt();
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Bracket::startEdit(ScoreView*, const QPointF&)
      {
      yoff = 0.0;
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Bracket::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 1;
      grip[0].translate(QPointF(0.0, h2 * 2) + QPointF(0.0, yoff) + canvasPos());
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF Bracket::gripAnchor(int) const
      {
      return QPointF();
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Bracket::endEdit()
      {
      endEditDrag();
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Bracket::bbox() const
      {
      return path.boundingRect();
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Bracket::editDrag(int, const QPointF& delta)
      {
      qreal dy = delta.y();
      yoff += dy;

      layout();
      }

//---------------------------------------------------------
//   endEditDrag
//    snap to nearest staff
//---------------------------------------------------------

void Bracket::endEditDrag()
      {
      h2 += yoff * .5;
      yoff = 0.0;

      qreal ay1 = canvasPos().y();
      qreal ay2 = ay1 + h2 * 2;

      int staffIdx1 = staffIdx();
      int staffIdx2;
      int n = system()->staves()->size();
      if (staffIdx1 + 1 >= n)
            staffIdx2 = staffIdx1;
      else {
            qreal ay  = parent()->canvasPos().y();
            System* s = system();
            qreal y   = s->staff(staffIdx1)->y() + ay;
            qreal h1  = staff()->height();

            for (staffIdx2 = staffIdx1 + 1; staffIdx2 < n; ++staffIdx2) {
                  qreal h = s->staff(staffIdx2)->y() + ay - y;
                  if (ay2 < (y + (h + h1) * .5))
                        break;
                  y += h;
                  }
            staffIdx2 -= 1;
            }

      qreal sy = system()->staff(staffIdx1)->y();
      qreal ey = system()->staff(staffIdx2)->y() + score()->staff(staffIdx2)->height();
      h2 = (ey - sy) * .5;

      int span = staffIdx2 - staffIdx1 + 1;
      staff()->setBracketSpan(_column, span);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Bracket::acceptDrop(ScoreView*, const QPointF&, int type, int) const
      {
      return type == BRACKET;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Bracket::drop(ScoreView*, const QPointF&, const QPointF&, Element* e)
      {
      if (e->type() == BRACKET) {
            Bracket* b = (Bracket*)e;
            b->setParent(parent());
            b->setTrack(track());
            b->setSpan(span());
            b->setLevel(level());
            score()->cmdRemove(this);
            score()->cmdAdd(b);
            return b;
            }
      delete e;
      return 0;
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool Bracket::edit(ScoreView*, int, int key, Qt::KeyboardModifiers modifiers, const QString&)
      {
      if (modifiers & Qt::ShiftModifier) {
            if (key == Qt::Key_Left) {
                  int bt = staff()->bracket(_column);
                  // search empty level
                  int oldColumn = _column;
                  staff()->setBracket(_column, NO_BRACKET);
                  for (;;) {
                        ++_column;
                        if (staff()->bracket(_column) == NO_BRACKET)
                              break;
                        }
                  staff()->setBracket(_column, bt);
                  staff()->setBracketSpan(_column, _span);
                  score()->moveBracket(staffIdx(), oldColumn, _column);
                  score()->setLayoutAll(true);
                  return true;
                  }
            else if (key == Qt::Key_Right) {
                  if (_column) {
                        int l = _column - 1;
                        for (; l >= 0; --l) {
                              if (staff()->bracket(l) == NO_BRACKET) {
                                    int bt = staff()->bracket(_column);
                                    staff()->setBracket(_column, -1);
                                    staff()->setBracket(l, bt);
                                    staff()->setBracketSpan(l, _span);
                                    score()->moveBracket(staffIdx(), _column, l);
                                    _column = l;
                                    score()->setLayoutAll(true);
                                    break;
                                    }
                              }
                        }
                  return true;
                  }
            }
      return false;
      }

