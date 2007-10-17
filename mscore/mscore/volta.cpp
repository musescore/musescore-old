//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: element.cpp,v 1.79 2006/04/12 14:58:10 wschweer Exp $
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

#include "volta.h"
#include "style.h"
#include "layout.h"
#include "system.h"
#include "xml.h"
#include "score.h"
#include "voltaproperties.h"

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void VoltaSegment::draw(QPainter& p)
      {
      QPointF _p1;
      QPointF _p2(pos2());

      qreal voltaLineWidth = _spatium * .18;
      qreal h              = _spatium * 1.9;

      QPointF p0(_p1.x(), h);
      QPointF p3(_p2.x(), h);

      QPen pen(p.pen());
      pen.setWidthF(voltaLineWidth);
      p.setPen(pen);
      p.drawLine(QLineF(p0, _p1));
      p.drawLine(QLineF(_p1, _p2));
      if (volta()->subtype() == Volta::VOLTA_CLOSED)
            p.drawLine(QLineF(_p2, p3));

      TextStyle* s = score()->textStyle(TEXT_STYLE_VOLTA);
      QFont f(s->family, s->size);
      f.setItalic(s->italic);
      f.setUnderline(s->underline);
      f.setBold(s->bold);
      p.setFont(f);

      QPointF tp(p0.x() + _spatium * .5, p0.y());
      p.drawText(tp, volta()->text());
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF VoltaSegment::bbox() const
      {
      qreal voltaHeight   = _spatium * 1.8;
      return QRectF(0.0, 0.0, pos2().x(), voltaHeight);
      }

//---------------------------------------------------------
//   pos2anchor
//---------------------------------------------------------

QPointF VoltaSegment::pos2anchor(const QPointF& pos, int* tick) const
      {
      Measure* m = score()->pos2measure3(pos, tick);
      QPointF anchor;
      if (m == m->system()->measures().front())
            m = m->prev();
      if (*tick != m->tick())
            anchor = QPointF(m->abbox().topRight());
      else
            anchor = QPointF(m->abbox().topLeft());
      return anchor;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool VoltaSegment::genPropertyMenu(QMenu* popup) const
      {
      QAction* a = popup->addAction(QT_TR_NOOP("Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void VoltaSegment::propertyAction(const QString& s)
      {
      if (s == "props") {
            VoltaProperties vp;
            vp.setText(volta()->text());
            vp.setEndings(volta()->endings());
            int rv = vp.exec();
            if (rv) {
                  QString txt  = vp.getText();
                  QList<int> l = vp.getEndings();
                  if (txt != volta()->text())
                        score()->undoChangeVoltaText(volta(), txt);
                  if (l != volta()->endings())
                        score()->undoChangeVoltaEnding(volta(), l);
                  }
            }
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool VoltaSegment::edit(int curGrip, QKeyEvent* ev)
      {
      if ((ev->modifiers() & Qt::ShiftModifier)
         && ((_segmentType == SEGMENT_SINGLE)
              || (_segmentType == SEGMENT_BEGIN && curGrip == 0)
              || (_segmentType == SEGMENT_END && curGrip == 1)
         )) {
            int tick1 = line()->tick();
            int tick2 = line()->tick2();

            Measure* m1 = score()->tick2measure(tick1);
            Measure* m2 = score()->tick2measure(tick2);
            if (ev->key() == Qt::Key_Left && m1->prev()) {
                  if (curGrip == 0)
                        tick1 = m1->prev()->tick();
                  else if (curGrip == 1) {
                        int segments = line()->lineSegments().size();
                        tick2 = m2->prev()->tick();
                        if (tick2 <= tick1)
                              return true;
                        line()->setTick2(tick2);
                        line()->layout(score()->mainLayout());
                        if (line()->lineSegments().size() != segments)
                              score()->changeLineSegment(true);
                        }
                  }
            else if (ev->key() == Qt::Key_Right) {
                  if (curGrip == 0) {
                        tick1 = m1->tick() + m1->tickLen();
                        if (tick1 >= tick2)
                              return true;
                        }
                  else if (curGrip == 1) {
                        int segments = line()->lineSegments().size();
                        tick2 = m2->tick() + m2->tickLen();
                        line()->setTick2(tick2);
                        line()->layout(score()->mainLayout());
                        if (line()->lineSegments().size() != segments)
                              score()->changeLineSegment(true);
                        }
                  }
            line()->setTick(tick1);
            line()->setTick2(tick2);
            return true;
            }
      return false;
      }


//---------------------------------------------------------
//   Volta
//---------------------------------------------------------

Volta::Volta(Score* s)
   : SLine(s)
      {
      _text = "1.";
      _endings.append(1);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Volta::layout(ScoreLayout* layout)
      {
      SLine::layout(layout);
      qreal y = -3.0 * layout->spatium();
      setPos(ipos().x(), y);
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Volta::createLineSegment()
      {
      VoltaSegment* seg = new VoltaSegment(score());
      seg->setStaff(staff());
      return seg;
      }

//---------------------------------------------------------
//   tick2pos
//---------------------------------------------------------

QPointF Volta::tick2pos(int grip, int tick, int staffIdx, System** system)
      {
      Measure* m = score()->tick2measure(tick);
      System* s = m->system();
      if ((grip == 1) && (m == s->measures().front())) {
            m = m->prev();
            s = m->system();
            *system = s;
            return QPointF(m->canvasPos().x() + m->width(), s->staff(staffIdx)->bbox().y() + s->canvasPos().y());
            }
      *system = s;
      return QPointF(m->canvasPos().x(), s->staff(staffIdx)->bbox().y() + s->canvasPos().y());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Volta::write(Xml& xml) const
      {
      xml.stag(name());
      SLine::writeProperties(xml);
      xml.tag("text", _text);
      QString s;
      foreach(int i, _endings) {
            if (!s.isEmpty())
                  s += ", ";
            s += QString("%1").arg(i);
            }
      xml.tag("endings", s);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Volta::read(QDomElement e)
      {
      setStaff(score()->staff(0));  // set default staff
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "text")
                  _text = e.text();
            else if (tag == "endings") {
                  QString s = e.text();
                  QStringList sl = s.split(",", QString::SkipEmptyParts);
                  _endings.clear();
                  foreach(QString l, sl) {
                        int i = l.simplified().toInt();
                        _endings.append(i);
                        }
                  }
            else if (!SLine::readProperties(e))
                  domError(e);
            }
      }
