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

#include "barline.h"
#include "score.h"
#include "sym.h"
#include "staff.h"
#include "system.h"
#include "measure.h"
#include "segment.h"
#include "painter.h"
#include "articulation.h"

//---------------------------------------------------------
//   barLineNames
//    must be synchronized with enum BarLineType
//---------------------------------------------------------

static const char* barLineNames[] = {
      "normal", "double", "start-repeat", "end-repeat", "dashed", "end",
      "end-start-repeat"
      };

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

BarLine::BarLine(Score* s)
   : Element(s)
      {
      setBarLineType(NORMAL_BAR);
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
      double yp = y();
      if (system)
            yp += system->staff(staffIdx())->y() + system->y();
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
            Segment* segment = static_cast<Segment*>(parent());
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

void BarLine::draw(Painter* painter) const
      {
      double lw = point(score()->styleS(ST_barWidth));
      double y1, y2;
      getY(&y1, &y2);

      painter->setLineWidth(lw);
      painter->setCapStyle(Qt::FlatCap);

      double mags = magS();
      double ld   = spatium();       // actual line distance

      switch(subtype()) {
            case BROKEN_BAR:
                  painter->setLineStyle(Qt::DashLine);

            case NORMAL_BAR:
                  // painter->drawLine(lw * .5, y1, lw * .5, y2);
                  painter->drawLine(0.0, y1, 0.0, y2);
                  break;

            case END_BAR:
                  {
                  double lw2 = point(score()->styleS(ST_endBarWidth));
                  double d   = point(score()->styleS(ST_endBarDistance));

                  painter->drawLine(lw * .5, y1, lw * .5, y2);
                  painter->setLineWidth(lw2);
                  double x = d + lw2 * .5 + lw;
                  painter->drawLine(x, y1, x, y2);
                  }
                  break;

            case DOUBLE_BAR:
                  {
                  lw      = point(score()->styleS(ST_doubleBarWidth));
                  double d = point(score()->styleS(ST_doubleBarDistance));

                  painter->setLineWidth(lw);
                  double x = lw * .5;
                  painter->drawLine(x, y1, x, y2);
                  x += d + lw;
                  painter->drawLine(x, y1, x, y2);
                  }
                  break;

            case START_REPEAT:
                  {
                  double lw2 = point(score()->styleS(ST_endBarWidth));
                  double d1  = point(score()->styleS(ST_endBarDistance));

                  const Sym& dotsym = symbols[score()->symIdx()][dotSym];

                  double x2   =  lw2 * .5;                               // thick line (lw2)
                  double x1   =  lw2 + d1 + lw * .5;                     // thin line (lw)
                  double x0   =  lw2 + d1 + lw + d1;                     // dot position

                  if (parent() == 0) {    // for use in palette
                        dotsym.draw(painter, mags, x0, 1.5 * ld);
                        dotsym.draw(painter, mags, x0, 2.5 * ld);
                        }
                  else {
                        double doty1, doty2;
                        if (staff()->useTablature()) {
                              switch(staff()->lines()) {
                                    case 4:
                                          doty1 = .5 * 1.5;
                                          doty2 = 2.5 * 1.5;
                                          break;
                                    default:
                                    case 6:
                                          doty1 = 1.5 * 1.5;
                                          doty2 = 3.5 * 1.5;
                                          break;
                                    }
                              }
                        else {
                              doty1 = 1.5;
                              doty2 = 2.5;
                              }
                        Segment* segment = (Segment*)parent();
                        Measure* measure = segment->measure();
                        System* system   = measure->system();
                        double yp = system->staff(staffIdx())->y();
                        for (int i = 0; i < _span; ++i) {
                              StaffLines* l2 = measure->staffLines(staffIdx() + i);
                              double yy = l2->y2() - yp;

                              dotsym.draw(painter, mags, x0, yy - doty1 * ld);
                              dotsym.draw(painter, mags, x0, yy - doty2 * ld);
                              }
                        }

                  painter->drawLine(x1, y1, x1, y2);

                  painter->setLineWidth(lw2);
                  painter->drawLine(x2, y1, x2, y2);

                  if (score()->styleB(ST_repeatBarTips)) {
                        symbols[score()->symIdx()][brackettipsRightUp].draw(painter, mags, 0.0, y1);
                        symbols[score()->symIdx()][brackettipsRightDown].draw(painter, mags, 0.0, y2);
                        }
                  }
                  break;

            case END_REPEAT:
                  {
                  double lw2  = point(score()->styleS(ST_endBarWidth));
                  double d1   = point(score()->styleS(ST_endBarDistance));
                  const Sym& dotsym = symbols[score()->symIdx()][dotSym];
                  double dotw = dotsym.width(mags);
                  double x1   =  dotw + d1 + lw * .5;
                  double x2   =  dotw + d1 + lw + d1 + lw2 * .5;

                  if (parent() == 0) {    // for use in palette
                        dotsym.draw(painter, mags, 0.0, 1.5 * ld);
                        dotsym.draw(painter, mags, 0.0, 2.5 * ld);
                        }
                  else {
                        double doty1, doty2;
                        if (staff()->useTablature()) {
                              switch(staff()->lines()) {
                                    case 4:
                                          doty1 = .5 * 1.5;
                                          doty2 = 2.5 * 1.5;
                                          break;
                                    default:
                                    case 6:
                                          doty1 = 1.5 * 1.5;
                                          doty2 = 3.5 * 1.5;
                                          break;
                                    }
                              }
                        else {
                              doty1 = 1.5;
                              doty2 = 2.5;
                              }
                        Segment* segment = (Segment*)parent();
                        Measure* measure = segment->measure();
                        System* system   = measure->system();
                        double yp = system->staff(staffIdx())->y();
                        for (int i = 0; i < _span; ++i) {
                              StaffLines* l2 = measure->staffLines(staffIdx() + i);
                              double yy = l2->y2() - yp;
                              dotsym.draw(painter, mags, 0.0, yy - doty1 * ld);
                              dotsym.draw(painter, mags, 0.0, yy - doty2 * ld);
                              }
                        }

                  painter->drawLine(x1, y1, x1, y2);
                  painter->setLineWidth(lw2);
                  painter->drawLine(x2, y1, x2, y2);
                  if (score()->styleB(ST_repeatBarTips)) {
                        double x = x2 + lw2 * .5;
                        symbols[score()->symIdx()][brackettipsLeftUp].draw(painter, mags, x, y1);
                        symbols[score()->symIdx()][brackettipsLeftDown].draw(painter, mags, x, y2);
                        }
                  }
                  break;

            case END_START_REPEAT:
                  {
                  double lw2  = point(score()->styleS(ST_endBarWidth));
                  double d1   = point(score()->styleS(ST_endBarDistance));
                  const Sym& dotsym = symbols[score()->symIdx()][dotSym];
                  double dotw = dotsym.width(mags);

                  double x1   =  dotw + d1 + lw * .5;                                // thin bar
                  double x2   =  dotw + d1 + lw + d1 + lw2 * .5;                     // thick bar
                  double x3   =  dotw + d1 + lw + d1 + lw2 + d1 + lw * .5;           // thin bar
                  double x4   =  dotw + d1 + lw + d1 + lw2 + d1 + lw + d1;           // dot position

                  if (parent() == 0) {    // for use in palette
                        dotsym.draw(painter, mags, .0, 1.5 * ld);
                        dotsym.draw(painter, mags, .0, 2.5 * ld);
                        dotsym.draw(painter, mags, x4, 1.5 * ld);
                        dotsym.draw(painter, mags, x4, 2.5 * ld);
                        }
                  else {
                        double doty1, doty2;
                        if (staff()->useTablature()) {
                              switch(staff()->lines()) {
                                    case 4:
                                          doty1 = .5 * 1.5;
                                          doty2 = 2.5 * 1.5;
                                          break;
                                    default:
                                    case 6:
                                          doty1 = 1.5 * 1.5;
                                          doty2 = 3.5 * 1.5;
                                          break;
                                    }
                              }
                        else {
                              doty1 = 1.5;
                              doty2 = 2.5;
                              }
                        Segment* segment = (Segment*)parent();
                        Measure* measure = segment->measure();
                        System* system   = measure->system();
                        double yp = system->staff(staffIdx())->y();
                        for (int i = 0; i < _span; ++i) {
                              StaffLines* l2 = measure->staffLines(staffIdx() + i);
                              double yy = l2->y2() - yp;

                              dotsym.draw(painter, mags, 0.0, yy - doty1 * ld);
                              dotsym.draw(painter, mags, 0.0, yy - doty2 * ld);
                              dotsym.draw(painter, mags, x4, yy - doty1 * ld);
                              dotsym.draw(painter, mags, x4, yy - doty2 * ld);
                              }
                        }

                  painter->drawLine(x1, y1, x1, y2);

                  painter->setLineWidth(lw2);
                  painter->drawLine(x2, y1, x2, y2);

                  painter->setLineWidth(lw);
                  painter->drawLine(x3, y1, x3, y2);
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
      xml.tag("span", _span);
      foreach(const Element* e, _el)
            e->write(xml);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void BarLine::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "subtype") {
                  BarLineType ct;
                  bool ok;
                  int i = val.toInt(&ok);
                  if (!ok)
                        ct = BarLine::barLineType(val);
                  else {
                        switch (i) {
                              default:
                              case  0: ct = NORMAL_BAR; break;
                              case  1: ct = DOUBLE_BAR; break;
                              case  2: ct = START_REPEAT; break;
                              case  3: ct = END_REPEAT; break;
                              case  4: ct = BROKEN_BAR; break;
                              case  5: ct = END_BAR; break;
                              case  6: ct = END_START_REPEAT; break;
                              }
                        }
                  setBarLineType(ct);
                  }
            else if (tag == "span")
                  _span = val.toInt();
            else if (tag == "Articulation") {
                  Articulation* a = new Articulation(score());
                  a->read(e);
                  add(a);
                  }
            else if (!Element::readProperties(e))
                  AL::domError(e);
            }
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
      return type == BAR_LINE
         || (type == ARTICULATION && segment() && segment()->subtype() == SegEndBarLine)
         ;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* BarLine::drop(const DropData& data)
      {
      Element* e = data.element;
      int type = e->type();
      int st   = e->subtype();
      if (type == BAR_LINE) {
            if (st == subtype()) {
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
            m->drop(data);
            }
      else if (type == ARTICULATION) {
            Articulation* atr = static_cast<Articulation*>(e);
            atr->setParent(this);
            atr->setTrack(track());
            score()->select(atr, SELECT_SINGLE, 0);
            score()->undoAddElement(atr);
            }
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

void BarLine::editDrag(const EditData& ed)
      {
      yoff += ed.delta.y();
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
//      _span = staff() ? staff()->barLineSpan() : 1;

      double y1, y2;
      getY(&y1, &y2);
      Spatium w = score()->styleS(ST_barWidth);
      double dw  = 0.0;

      double dotwidth = symbols[score()->symIdx()][dotSym].width(magS());
      switch(subtype()) {
            case DOUBLE_BAR:
                  w  = score()->styleS(ST_doubleBarWidth) * 2 + score()->styleS(ST_doubleBarDistance);
                  dw = point(w);
                  break;
            case START_REPEAT:
                  w  += score()->styleS(ST_endBarWidth) + 2.0 * score()->styleS(ST_endBarDistance);
                  dw = point(w) + dotwidth;
                  break;
            case END_REPEAT:
                  w  += score()->styleS(ST_endBarWidth) + 2.0 * score()->styleS(ST_endBarDistance);
                  dw = point(w) + dotwidth;
                  break;
            case END_BAR:
                  w += score()->styleS(ST_endBarWidth) + score()->styleS(ST_endBarDistance);
                  dw = point(w);
                  break;
            case  END_START_REPEAT:
                  // w  += score()->styleS(ST_endBarWidth) + 3 * score()->styleS(ST_endBarDistance);
                  w  += score()->styleS(ST_barWidth) + score()->styleS(ST_endBarWidth) + 4 * score()->styleS(ST_endBarDistance);
                  dw = point(w) + 2 * dotwidth;
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
            // double mags = magS();
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
      qreal _spatium = spatium();
      foreach(Element* e, _el) {
            e->layout();
            if (e->type() == ARTICULATION) {
                  Articulation* a       = static_cast<Articulation*>(e);
                  ArticulationAnchor aa = a->anchor();
                  double distance       = score()->styleS(ST_propertyDistanceStem).val() * _spatium;
                  qreal topY            = y1 - distance;
                  qreal botY            = y2 + distance;
                  qreal x               = 0.0;

                  if (aa == A_TOP_STAFF)
                        a->setPos(x, topY);
                  else if (aa == A_BOTTOM_STAFF)
                        a->setPos(x, botY);
                  }
            }
      setbbox(r);
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int BarLine::tick() const
      {
      return segment() ? segment()->tick() : 0;
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString BarLine::subtypeName() const
      {
      return QString(barLineNames[subtype()]);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void BarLine::setSubtype(const QString& s)
      {
      setBarLineType(barLineType(s));
      }

//---------------------------------------------------------
//   barLineType
//---------------------------------------------------------

BarLineType BarLine::barLineType(const QString& s)
      {
      for (unsigned i = 0; i < sizeof(barLineNames)/sizeof(*barLineNames); ++i) {
            if (barLineNames[i] == s)
                  return BarLineType(i);
            }
      return NORMAL_BAR;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void BarLine::scanElements(void* data, void (*func)(void*, Element*))
      {
      func(data, this);
      foreach(Element* e, _el)
            e->scanElements(data, func);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void BarLine::add(Element* e)
      {
	e->setParent(this);
      switch(e->type()) {
            case ARTICULATION:
                  _el.append(e);
                  setGenerated(false);
                  if (measure())
                        measure()->setEndBarLineGenerated(false);
                  break;
            default:
                  printf("BarLine::add() not impl. %s\n", e->name());
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void BarLine::remove(Element* e)
      {
      switch(e->type()) {
            case ARTICULATION:
                  if (!_el.remove(e))
                        printf("BarLine::remove(): cannot find %s\n", e->name());
                  break;
            default:
                  printf("BarLine::remove() not impl. %s\n", e->name());
                  break;
            }
      }


