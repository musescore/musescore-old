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
#include "painter.h"
#include "preferences.h"
#include "utils.h"
#include "staff.h"
#include "score.h"

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

Bracket::Bracket(Score* s, int t)
   : Element(s)
      {
      setSubtype(t);
      path  = 0;
      h2    = 0.0;
      mode  = 0;
      _span = 1;
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
      if (h2 == 0.0)
            return;
      if (path)
            delete path;
      path = new QPainterPath();

      qreal h = h2;
      if (mode == 2)
            h = h2 + yoff * .5;

      if (subtype() == BRACKET_AKKOLADE) {
            qreal w         = point(style->akkoladeWidth);
            const double X1 =  2.0 * w;
            const double X2 = -0.7096 * w;
            const double X3 = -1.234 * w;
            const double X4 =  1.734 * w;

            path->moveTo(0, h);
            path->cubicTo(X1,  h + h * .3359, X2,  h + h * .5089, w, 2 * h);
            path->cubicTo(X3,  h + h * .5025, X4,  h + h * .2413, 0, h);
            path->cubicTo(X1,  h - h * .3359, X2,  h - h * .5089, w, 0);
            path->cubicTo(X3,  h - h * .5025, X4,  h - h * .2413, 0, h);
            }
      else if (subtype() == BRACKET_NORMAL) {
            qreal w = point(style->bracketWidth);

            TextStyle* s = &textStyles[TEXT_STYLE_SYMBOL1];
            QChar up(0xe19c);
            QChar down(0xe19d);
            QFont f;
            f.setStyleStrategy(QFont::PreferMatch);
            f.setFamily(s->family);
            f.setPixelSize(lrint(20.0 / 5 * _spatium));

            qreal o = _spatium * .27;
            qreal slw = point(style->staffLineWidth);

            path->addText(QPointF(0.0, -o), f, QString(up));
            path->addText(QPointF(0.0, h * 2.0 + o), f, QString(down));
            path->addRect(0.0, -slw * .5, w, h * 2.0 + slw);
            }
      bboxUpdate();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Bracket::draw1(Painter& p)
      {
      if (path == 0)
            return;
      p.setBrush(selected() ? preferences.selectColor[0] : Qt::black);
      p.drawPath(*path);

      if (selected() && mode) {
            qreal lw = 2.0/p.matrix().m11();
            QPen pen(Qt::blue);
            pen.setWidthF(lw);
            p.setPen(pen);
            for (int i = 0; i < 2; ++i) {
                  if (i == (mode-1))
                        p.setBrush(Qt::blue);
                  else
                        p.setBrush(Qt::NoBrush);
                  p.drawRect(grip[i]);
                  }
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
            if (Element::readProperties(node))
                  ;
            else
                  domError(node);
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool Bracket::startEdit(QMatrix& matrix)
      {
      mode = 2;
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
      QRectF r(-w/2, -h/2, w, h);

      grip[0] = r.translated(QPointF(0.0, 0.0));
      grip[1] = r.translated(QPointF(0.0, h2 * 2));
      bboxUpdate();
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Bracket::endEdit()
      {
      mode = 0;
      }

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

bool Bracket::startEditDrag(const QPointF& p)
      {
      yoff = 0.0;
      mode = 0;
      if (grip[0].contains(p))
            mode = 1;
      else if (grip[1].contains(p))
            mode = 2;
      if (mode) {
            bboxUpdate();
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   bboxUpdate
//---------------------------------------------------------

void Bracket::bboxUpdate()
      {
      setbbox(path->boundingRect());
      if (mode) {
            orBbox(grip[0]);
            orBbox(grip[1]);
            }
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
      if (!mode)
            return false;
      int n = mode - 1;
      qreal dy = delta.y();

      grip[n].translate(QPointF(0.0, dy));
      yoff += dy;
      layout();
      return true;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Bracket::edit(QKeyEvent*)
      {
#if 0
      QPointF ppos(apos());

      if ((ev->state() & Qt::ShiftButton)) {
            if (ev->key() == Qt::Key_Left)
                  slur->prevSeg(apos(), mode, ups[mode-1]);
            else if (ev->key() == Qt::Key_Right)
                  slur->nextSeg(apos(), mode, ups[mode-1]);
            return false;
            }

      QPointF delta;
      qreal val = 1.0;
      if (ev->state() & Qt::ControlButton)
            val = 0.1;
      switch (ev->key()) {
            case Qt::Key_Left:
                  delta = QPointF(-val, 0);
                  break;
            case Qt::Key_Right:
                  delta = QPointF(val, 0);
                  break;
            case Qt::Key_Up:
                  delta = QPointF(0, -val);
                  break;
            case Qt::Key_Down:
                  delta = QPointF(0, val);
                  break;
            case Qt::Key_Tab:
                  if (mode < 4)
                        ++mode;
                  else
                        mode = 1;
                  break;
            case Qt::Key_X:
                  slur->setSlurDirection(slur->isUp() ? DOWN : UP);
                  break;
            }
      if (mode == 0)
            return false;

      int idx       = (mode-1) % 4;
      ups[idx].off += delta;
      ups[idx].r   += (delta * _spatium);

      if (mode == 1 || mode == 4) {
            slur->layout2(apos(), mode, ups[idx]);
            if (!showRubberBand || (mode != 1 && mode != 4))
                  return true;
            QPointF ppos(apos());
            QPointF rp1, rp2;
            rp1 = ups[idx].p + ups[idx].off * _spatium + ppos;
            rp2 = ups[idx].p + ppos;
            rb->set(rp1, rp2);
            setbbox(bbox() | QRectF(rp1-ppos, rp2-ppos));
            }
#endif
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
            if (ay2 >= y1 && ay2 < y2)
                  idx2 = i;
            }
      if (idx2 == -1)
            idx2 = i -1;

      qreal sy = (*(sl->begin() + idx1))->bbox().top();
      qreal ey = (*(sl->begin() + idx2))->bbox().bottom();
      h2 = (ey - sy) / 2.0;

      yoff = 0.0;
      layout();
      score()->staff(idx1)->setBracketSpan(idx2 - idx1 + 1);
      grip[1].moveTo(0.0, h2 *2);
      bboxUpdate();
      return true;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Bracket::acceptDrop(int type, int) const
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

void Bracket::drop(const QPointF&, int type, int subtype)
      {
      if (ElementType(type) == BRACKET) {
            Bracket* b = new Bracket(score(), subtype);
            b->setParent(parent());
            b->setStaff(staff());
            b->setSpan(span());
            score()->cmdRemove(this);
            score()->cmdAdd(b);
            b->layout();
            score()->layout();
            }
      }

