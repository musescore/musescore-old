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

#include "barline.h"
#include "score.h"
#include "sym.h"
#include "staff.h"
#include "system.h"
#include "measure.h"

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

BarLine::BarLine(Score* s)
   : Element(s)
      {
      setSubtype(NORMAL_BAR);
      _span = 1;
      yoff  = 0.0;
      setHeight(4.0 * spatium()); // for use in palettes
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF BarLine::canvasPos() const
      {
      if (parent() == 0)
            return pos();
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
      double yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   getY
//---------------------------------------------------------

void BarLine::getY(double* y1, double* y2) const
      {
      if (parent() && parent()->type() == SEGMENT) {
            int staffIdx1    = staffIdx();
            int staffIdx2    = staffIdx1 + _span - 1;
            Segment* segment = (Segment*)parent();
            Measure* measure = segment->measure();
            System* system   = measure->system();
            StaffLines* l1   = measure->staffLines(staffIdx1);
            StaffLines* l2   = measure->staffLines(staffIdx2);

            double yp;
            if (system)
                  yp = system->staff(staffIdx())->y();
            else
                  yp = 0.0;

            *y1 = l1->y1() - yp;
            *y2 = l2->y2() - yp;
            }
      else {
            *y2 = 4.0 * spatium();    // for use in palette
            *y1 = 0.0;
            }

      *y2 += yoff;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void BarLine::draw(QPainter& p) const
      {
      double lw = point(score()->styleS(ST_barWidth));
      double y1, y2;
      getY(&y1, &y2);

      QPen pen(p.pen());
      pen.setWidthF(lw);
      pen.setCapStyle(Qt::FlatCap);
      p.setPen(pen);

      double mags = magS();
      double ld   = spatium();       // actual line distance

      switch(subtype()) {
            case BROKEN_BAR:
                  {
                  QPen pen(p.pen());
                  pen.setStyle(Qt::DashLine);
                  p.setPen(pen);
                  }

            case NORMAL_BAR:
                  p.drawLine(QLineF(lw * .5, y1, lw * .5, y2));
                  break;

            case END_BAR:
                  {
                  double lw2 = point(score()->styleS(ST_endBarWidth));
                  double d   = point(score()->styleS(ST_endBarDistance));

                  p.drawLine(QLineF(lw * .5, y1, lw * .5, y2));
                  pen.setWidthF(lw2);
                  p.setPen(pen);
                  double x = d + lw2 * .5 + lw;
                  p.drawLine(QLineF(x, y1, x, y2));
                  }
                  break;

            case DOUBLE_BAR:
                  {
                  lw      = point(score()->styleS(ST_doubleBarWidth));
                  double d = point(score()->styleS(ST_doubleBarDistance));

                  pen.setWidthF(lw);
                  p.setPen(pen);
                  double x = lw * .5;
                  p.drawLine(QLineF(x, y1, x, y2));
                  x += d + lw;
                  p.drawLine(QLineF(x, y1, x, y2));
                  }
                  break;

            case START_REPEAT:
                  {
                  double lw2  = point(score()->styleS(ST_endBarWidth));
                  double d1   = point(score()->styleS(ST_endBarDistance));

                  double x2   =  lw2 * .5;                               // thick line (lw2)
                  double x1   =  lw2 + d1 + lw * .5;                     // thin line (lw)
                  double x0   =  lw2 + d1 + lw + d1;                     // dot position

                  if (parent() == 0) {    // for use in palette
                        symbols[dotSym].draw(p, mags, x0, 1.5 * ld);
                        symbols[dotSym].draw(p, mags, x0, 2.5 * ld);
                        }
                  else {
                        Segment* segment = (Segment*)parent();
                        Measure* measure = segment->measure();
                        System* system   = measure->system();
                        double yp = system->staff(staffIdx())->y();
                        for (int i = 0; i < _span; ++i) {
                              StaffLines* l2 = measure->staffLines(staffIdx() + i);
                              double yy = l2->y2() - yp;

                              symbols[dotSym].draw(p, mags, x0, yy - 1.5 * ld);
                              symbols[dotSym].draw(p, mags, x0, yy - 2.5 * ld);
                              }
                        }

                  p.drawLine(QLineF(x1, y1, x1, y2));

                  pen.setWidthF(lw2);
                  p.setPen(pen);
                  p.drawLine(QLineF(x2, y1, x2, y2));

                  if (score()->styleB(ST_repeatBarTips)) {
                        symbols[brackettipsRightUp].draw(p, mags, 0.0, y1);
                        symbols[brackettipsRightDown].draw(p, mags, 0.0, y2);
                        }
                  }
                  break;

            case END_REPEAT:
                  {
                  double lw2  = point(score()->styleS(ST_endBarWidth));
                  double d1   = point(score()->styleS(ST_endBarDistance));
                  double dotw = symbols[dotSym].width(mags);

                  double x1   =  dotw + d1 + lw * .5;
                  double x2   =  dotw + d1 + lw + d1 + lw2 * .5;

                  if (parent() == 0) {    // for use in palette
                        symbols[dotSym].draw(p, mags, 0.0, 1.5 * ld);
                        symbols[dotSym].draw(p, mags, 0.0, 2.5 * ld);
                        }
                  else {
                        Segment* segment = (Segment*)parent();
                        Measure* measure = segment->measure();
                        System* system   = measure->system();
                        double yp = system->staff(staffIdx())->y();
                        for (int i = 0; i < _span; ++i) {
                              StaffLines* l2 = measure->staffLines(staffIdx() + i);
                              double yy = l2->y2() - yp;

                              symbols[dotSym].draw(p, mags, 0.0, yy - 1.5 * ld);
                              symbols[dotSym].draw(p, mags, 0.0, yy - 2.5 * ld);
                              }
                        }

                  p.drawLine(QLineF(x1, y1, x1, y2));
                  pen.setWidthF(lw2);
                  p.setPen(pen);
                  p.drawLine(QLineF(x2, y1, x2, y2));
                  if (score()->styleB(ST_repeatBarTips)) {
                        double x = x2 + lw2 * .5;
                        symbols[brackettipsLeftUp].draw(p, mags, x, y1);
                        symbols[brackettipsLeftDown].draw(p, mags, x, y2);
                        }
                  }
                  break;

            case END_START_REPEAT:
                  {
                  double lw2  = point(score()->styleS(ST_endBarWidth));
                  double d1   = point(score()->styleS(ST_endBarDistance));
                  double dotw = symbols[dotSym].width(mags);

                  double x1   =  dotw + d1 + lw * .5;                                // thin bar
                  double x2   =  dotw + d1 + lw + d1 + lw2 * .5;                     // thick bar
                  double x3   =  dotw + d1 + lw + d1 + lw2 + d1 + lw * .5;           // thin bar
                  double x4   =  dotw + d1 + lw + d1 + lw2 + d1 + lw + dotw * .5;    // dots

                  if (parent() == 0) {    // for use in palette
                        symbols[dotSym].draw(p, mags, .0, 1.5 * ld);
                        symbols[dotSym].draw(p, mags, .0, 2.5 * ld);
                        symbols[dotSym].draw(p, mags, x4, 1.5 * ld);
                        symbols[dotSym].draw(p, mags, x4, 2.5 * ld);
                        }
                  else {
                        Segment* segment = (Segment*)parent();
                        Measure* measure = segment->measure();
                        System* system   = measure->system();
                        double yp = system->staff(staffIdx())->y();
                        for (int i = 0; i < _span; ++i) {
                              StaffLines* l2 = measure->staffLines(staffIdx() + i);
                              double yy = l2->y2() - yp;

                              symbols[dotSym].draw(p, mags, 0.0, yy - 1.5 * ld);
                              symbols[dotSym].draw(p, mags, 0.0, yy - 2.5 * ld);
                              symbols[dotSym].draw(p, mags, x4, yy - 1.5 * ld);
                              symbols[dotSym].draw(p, mags, x4, yy - 2.5 * ld);
                              }
                        }

                  p.drawLine(QLineF(x1, y1, x1, y2));

                  pen.setWidthF(lw2);
                  p.setPen(pen);
                  p.drawLine(QLineF(x2, y1, x2, y2));

                  pen.setWidthF(lw);
                  p.setPen(pen);
                  p.drawLine(QLineF(x3, y1, x3, y2));
                  }
                  break;
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void BarLine::write(Xml& xml) const
      {
      xml.stag("BarLine");
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void BarLine::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!Element::readProperties(e))
                  AL::domError(e);
            }
      setSubtype(subtype());
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space BarLine::space() const
      {
      return Space(0.0, width());
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool BarLine::acceptDrop(ScoreView*, const QPointF&, int type, int) const
      {
      return type == BAR_LINE;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* BarLine::drop(ScoreView* view, const QPointF& p1, const QPointF& p2, Element* e)
      {
      int type = e->type();
      int st   = e->subtype();
      if (type != BAR_LINE || st == subtype()) {
            delete e;
            return 0;
            }
      Measure* m = segment()->measure();
      if (st == START_REPEAT) {
            m = m->nextMeasure();
            if (m == 0) {
                  delete e;
                  return 0;
                  }
            }
      m->drop(view, p1, p2, e);
      return 0;
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void BarLine::updateGrips(int* grips, QRectF* grip) const
      {
      *grips   = 1;
      double lw = point(score()->styleS(ST_barWidth));
      double y1, y2;
      getY(&y1, &y2);
      grip[0].translate(QPointF(lw * .5, y2) + canvasPos());
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void BarLine::endEdit()
      {
      if (staff()->barLineSpan() == _span)
            return;

      int idx1 = staffIdx();

      if (_span > staff()->barLineSpan()) {
            int idx2 = idx1 + _span;
            for (int idx = idx1 + 1; idx < idx2; ++idx)
                  score()->undoChangeBarLineSpan(score()->staff(idx), 0);
            }
      else {
            int idx1 = staffIdx() + _span;
            int idx2 = staffIdx() + staff()->barLineSpan();
            for (int idx = idx1; idx < idx2; ++idx)
                  score()->undoChangeBarLineSpan(score()->staff(idx), 1);
            }
      score()->undoChangeBarLineSpan(staff(), _span);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void BarLine::editDrag(int, const QPointF& delta)
      {
      double dy = delta.y();
      yoff += dy;
      }

//---------------------------------------------------------
//   endEditDrag
//    snap to nearest staff
//---------------------------------------------------------

void BarLine::endEditDrag()
      {
      double y1, h2;
      getY(&y1, &h2);
      yoff      = 0.0;
      double ay1 = canvasPos().y();
      double ay2 = ay1 + h2;

      int staffIdx1 = staffIdx();
      int staffIdx2;
      Segment* segment = (Segment*)parent();
      Measure* measure = segment->measure();
      System* s = measure->system();
      int n = s->staves()->size();
      if (staffIdx1 + 1 >= n)
            staffIdx2 = staffIdx1;
      else {
            double ay = s->canvasPos().y();
            double y  = s->staff(staffIdx1)->y() + ay;
            double h1 = staff()->lines() * spatium();

            for (staffIdx2 = staffIdx1 + 1; staffIdx2 < n; ++staffIdx2) {
                  double h = s->staff(staffIdx2)->y() + ay - y;
                  if (ay2 < (y + (h + h1) * .5))
                        break;
                  y += h;
                  }
            staffIdx2 -= 1;
            }
      int newSpan = staffIdx2 - staffIdx1 + 1;
      if (newSpan != _span) {
            if (newSpan > _span) {
                  int diff = newSpan - _span;
                  staffIdx1 += _span;
                  staffIdx2 = staffIdx1 + diff;
                  Segment* s = score()->firstMeasure()->first(SegEndBarLine);
                  for (; s; s = s->next1(SegEndBarLine)) {
                        for (int staffIdx = staffIdx1; staffIdx < staffIdx2; ++staffIdx) {
                              Element* e = s->element(staffIdx * VOICES);
                              if (e) {
                                    score()->undoRemoveElement(e);
                                    }
                              }
                        }
                  }
            _span = newSpan;
            score()->undoChangeBarLineSpan(staff(), _span);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void BarLine::layout()
      {
      _span = staff() ? staff()->barLineSpan() : 1;

      double y1, y2;
      getY(&y1, &y2);
      Spatium w = score()->styleS(ST_barWidth);
      double dw  = 0.0;

      switch(subtype()) {
            case DOUBLE_BAR:
                  w  = score()->styleS(ST_doubleBarWidth) * 2 + score()->styleS(ST_doubleBarDistance);
                  dw = point(w);
                  break;
            case START_REPEAT:
                  w  += score()->styleS(ST_endBarWidth) + 2.0 * score()->styleS(ST_endBarDistance);
                  dw = point(w) + symbols[dotSym].width(magS());
                  break;
            case END_REPEAT:
                  w  += score()->styleS(ST_endBarWidth) + 2.0 * score()->styleS(ST_endBarDistance);
                  dw = point(w) + symbols[dotSym].width(magS());
                  break;
            case END_BAR:
                  w += score()->styleS(ST_endBarWidth) + score()->styleS(ST_endBarDistance);
                  dw = point(w);
                  break;
            case  END_START_REPEAT:
                  w  += score()->styleS(ST_endBarWidth) + 3 * score()->styleS(ST_endBarDistance);
                  dw = point(w) + 2 * symbols[dotSym].width(magS());
                  break;
            case BROKEN_BAR:
            case NORMAL_BAR:
                  dw = point(w);
                  break;
            default:
                  printf("illegal bar line type\n");
                  break;
            }
      QRectF r(0.0, y1, dw, y2);

      if (score()->styleB(ST_repeatBarTips)) {
            double mags = magS();
            switch(subtype()) {
                  case START_REPEAT:
                        //r |= symbols[brackettipsRightUp].bbox(mags).translated(0, y1);
                        //r |= symbols[brackettipsRightDown].bbox(mags).translated(0, y2);
                        break;
                  case END_REPEAT:
                        //r |= symbols[brackettipsLeftUp].bbox(mags).translated(0, y1);
                        //r |= symbols[brackettipsLeftDown].bbox(mags).translated(0, y2);
                        break;
                  }
            }
      setbbox(r);
      }

