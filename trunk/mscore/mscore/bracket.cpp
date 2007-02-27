//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: bracket.cpp,v 1.10 2006/04/12 14:58:10 wschweer Exp $
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

#include "bracket.h"
#include "xml.h"
#include "style.h"
#include "preferences.h"
#include "utils.h"
#include "staff.h"
#include "score.h"

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

Bracket::Bracket(Score* s)
   : Element(s)
      {
      h2       = 0.0;
      editMode = false;
      _span    = 1;
      _level   = 0;
      }

//---------------------------------------------------------
//   setHeight
//---------------------------------------------------------

void Bracket::setHeight(qreal h)
      {
      h2 = h * .5;
      layout();
      }

//---------------------------------------------------------
//   width
//---------------------------------------------------------

double Bracket::width() const
      {
      double w;
      if (subtype() == BRACKET_AKKOLADE)
            w = point(style->akkoladeWidth);
      else
            w = point(style->bracketWidth + style->bracketDistance);
      return w;
      }

//---------------------------------------------------------
//   updateValues
//---------------------------------------------------------

void Bracket::layout()
      {
      path = QPainterPath();
      if (h2 == 0.0)
            return;

      qreal h = h2;
      if (editMode)
            h = h2 + yoff * .5;

      if (subtype() == BRACKET_AKKOLADE) {
            qreal w         = point(style->akkoladeWidth);
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
            qreal w = point(style->bracketWidth);

            TextStyle* s = &textStyles[TEXT_STYLE_SYMBOL1];
            QChar up(0xe19c);
            QChar down(0xe19d);
            QFont f;
            f.setFamily(s->family);
            double mag = score()->spatium() / (spatiumBase20 * DPI);
            f.setPointSizeF(20.0 * mag);

            qreal o = _spatium * .27;
            qreal slw = point(style->staffLineWidth);

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

      if (selected() && editMode) {
            qreal lw = 2.0/p.matrix().m11();
            QPen pen(Qt::blue);
            pen.setWidthF(lw);
            p.setPen(pen);
            p.setBrush(Qt::blue);
            p.drawRect(grip);
            }
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
      xml.etag("Bracket");
      }

//---------------------------------------------------------
//   Bracket::read
//---------------------------------------------------------

void Bracket::read(QDomNode node)
      {
      QDomElement e = node.toElement();
      QString t(e.attribute("type", "Normal"));

      if (t == "Normal")
            setSubtype(BRACKET_NORMAL);
      else if (t == "Akkolade")
            setSubtype(BRACKET_AKKOLADE);
      else
            fprintf(stderr, "unknown brace type <%s>\n", t.toLatin1().data());

      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "level")
                  _level = val.toInt();
            else if (Element::readProperties(node))
                  ;
            else
                  domError(node);
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool Bracket::startEdit(QMatrix& matrix, const QPointF&)
      {
      yoff = 0.0;
      editMode = true;
      updateGrips(matrix);
      return true;
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Bracket::updateGrips(QMatrix& matrix)
      {
      qreal w = 8.0 / matrix.m11();
      qreal h = 8.0 / matrix.m22();
      QRectF r(-w*.5, -h*.5, w, h);

      grip = r.translated(QPointF(0.0, h2 * 2));
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Bracket::endEdit()
      {
      editMode = false;
      }

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

bool Bracket::startEditDrag(const QPointF& p)
      {
      if (grip.contains(p))
            return true;
      return false;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Bracket::bbox() const
      {
      QRectF b = path.boundingRect();
      if (editMode)
            b |= grip;
      return b;
      }

//---------------------------------------------------------
//   dragOff
//---------------------------------------------------------

QPointF Bracket::dragOff() const
      {
//      if (mode)
//            return -ups[mode - 1].off;
//      else
            return QPointF(0,0);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

bool Bracket::editDrag(QMatrix&, QPointF*, const QPointF& delta)
      {
      if (!editMode)
            return false;
      qreal dy = delta.y();

      grip.translate(QPointF(0.0, dy));
      yoff += dy;
      layout();
      return true;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Bracket::edit(QKeyEvent* ev)
      {
      if (editMode == 0)
            return false;
      QPointF ppos(apos());

      qreal dy = 0.0;
      qreal val = 1.0;
      if (ev->modifiers() & Qt::ControlModifier)
            val = 0.1;
      switch (ev->key()) {
            case Qt::Key_Up:
                  dy = -val;
                  break;
            case Qt::Key_Down:
                  dy = val;
                  break;
            }

      grip.translate(QPointF(0.0, dy));
      yoff += dy;
      layout();
      return false;
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

bool Bracket::endEditDrag()
      {
      h2 += yoff * .5;

      qreal ay1 = apos().y();
      qreal ay2 = ay1 + h2 * 2;

      SysStaffList* sl = ((System*)parent())->staves();
      int idx1 = 0;
      int idx2 = -1;
      int i    = 0;

      qreal ay = parent()->apos().y();
      for (iSysStaff iss = sl->begin(); iss != sl->end(); ++iss, ++i) {
            SysStaff* ss = *iss;
            qreal y1 = ss->bbox().y() + ay;
            qreal y2 = y1 + ss->bbox().height();
            if (ay1 >= y1 && ay1 < y2)
                  idx1 = i;
            idx2 = i;
            if (y2 > ay2)
                  break;
            }
      if (idx2 == -1)
            idx2 = i - 1;

      qreal sy = (*(sl->begin() + idx1))->bbox().top();
      qreal ey = (*(sl->begin() + idx2))->bbox().bottom();
      h2 = (ey - sy) / 2.0;

      yoff = 0.0;
      layout();
      score()->staff(idx1)->setBracketSpan(_level, idx2 - idx1 + 1);
      grip.moveTo(0.0, h2 *2);
      return true;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Bracket::acceptDrop(const QPointF&, int type, const QDomNode&) const
      {
      switch(ElementType(type)) {
            case BRACKET:
                  return true;
            default:
                  return false;
            }
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

void Bracket::drop(const QPointF&, int type, const QDomNode& node)
      {
      if (ElementType(type) == BRACKET) {
            Bracket* b = new Bracket(score());
            b->read(node);
            b->setSelected(false);
            b->setParent(parent());
            b->setStaff(staff());
            b->setSpan(span());
            b->setLevel(level());
            score()->cmdRemove(this);
            score()->cmdAdd(b);
            score()->layout();
            }
      }

