//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: bracket.cpp,v 1.10 2006/04/12 14:58:10 wschweer Exp $
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
#include "layout.h"
#include "viewer.h"

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

Bracket::Bracket(Score* s)
   : Element(s)
      {
      h2       = 0.0;
      _span    = 1;
      _level   = 0;
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
            w = point(score()->style()->akkoladeWidth);
      else
            w = point(score()->style()->bracketWidth + score()->style()->bracketDistance);
      return w;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Bracket::layout(ScoreLayout* layout)
      {
      double _spatium = layout->spatium();
      path = QPainterPath();
      if (h2 == 0.0)
            return;

      qreal h = h2 + yoff * .5;

      if (subtype() == BRACKET_AKKOLADE) {
            qreal w         = point(score()->style()->akkoladeWidth);
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
            qreal w = point(score()->style()->bracketWidth);

            TextStyle* s = &textStyles[TEXT_STYLE_SYMBOL1];
            QChar up(0xe19c);
            QChar down(0xe19d);
            QFont f;
            f.setFamily(s->family);

            //?!?:
            extern int appDpiX;
            double mmag = DPI / double(appDpiX);
            double mag  = mmag * _spatium / (spatiumBase20 * DPI);
            f.setPointSizeF(20.0 * mag);

            qreal o   = _spatium * .27;
            qreal slw = point(score()->style()->staffLineWidth);

            path.addText(QPointF(0.0, -o), f, QString(up));
            path.addText(QPointF(0.0, h * 2.0 + o), f, QString(down));
            path.addRect(0.0, -slw * .5, w, h * 2.0 + slw);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Bracket::draw(QPainter& p)
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
      if (_level)
            xml.tag("level", _level);
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
                  _level = val.toInt();
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool Bracket::startEdit(const QPointF&)
      {
      yoff = 0.0;
      return true;
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

QPointF Bracket::gripAnchor(int)
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

void Bracket::editDrag(int, const QPointF&, const QPointF& delta)
      {
      qreal dy = delta.y();
      yoff += dy;

      qreal h = h2 + yoff * .5;

      path = QPainterPath();
      if (subtype() == BRACKET_AKKOLADE) {
            qreal w         = point(score()->style()->akkoladeWidth);
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
            qreal w = point(score()->style()->bracketWidth);

            TextStyle* s = &textStyles[TEXT_STYLE_SYMBOL1];
            QChar up(0xe19c);
            QChar down(0xe19d);
            QFont f;
            f.setFamily(s->family);

            //?!?:
            extern int appDpiX;
            double mmag   = DPI / double(appDpiX);
            double mag = mmag * _spatium / (spatiumBase20 * DPI);
            f.setPointSizeF(20.0 * mag);

            qreal o = _spatium * .27;
            qreal slw = point(score()->style()->staffLineWidth);

            path.addText(QPointF(0.0, -o), f, QString(up));
            path.addText(QPointF(0.0, h * 2.0 + o), f, QString(down));
            path.addRect(0.0, -slw * .5, w, h * 2.0 + slw);
            }
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

            qreal y = s->staff(staffIdx1)->bbox().y() + ay;
            for (staffIdx2 = staffIdx1 + 1; staffIdx2 < n; ++staffIdx2) {
                  qreal h = s->staff(staffIdx2)->bbox().y() + ay - y;
                  if (ay2 < (y + h * .5))
                        break;
                  y += h;
                  }
            staffIdx2 -= 1;
            }

      qreal sy = system()->staff(staffIdx1)->bbox().top();
      qreal ey = system()->staff(staffIdx2)->bbox().bottom();
      h2 = (ey - sy) * .5;

      int span = staffIdx2 - staffIdx1 + 1;
      score()->staff(staffIdx1)->setBracketSpan(_level, span);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Bracket::acceptDrop(Viewer* viewer, const QPointF&, int type, const QDomElement&) const
      {
      if (type == BRACKET) {
            viewer->setDropTarget(this);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Bracket::drop(const QPointF&, const QPointF&, int type, const QDomElement& e)
      {
      if (ElementType(type) == BRACKET) {
            Bracket* b = new Bracket(score());
            b->read(e);
            b->setSelected(false);
            b->setParent(parent());
            b->setStaff(staff());
            b->setSpan(span());
            b->setLevel(level());
            score()->cmdRemove(this);
            score()->cmdAdd(b);
            return b;
            }
      return 0;
      }

