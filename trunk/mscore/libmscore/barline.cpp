//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: barline.cpp 3587 2010-10-15 11:48:59Z wschweer $
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
#include "segment.h"
#include "al/xml.h"
#include "painter.h"

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
      qreal xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
      qreal yp = y();
      if (system)
            yp += system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   getY
//---------------------------------------------------------

void BarLine::getY(qreal* y1, qreal* y2) const
      {
      if (parent() && parent()->type() == SEGMENT) {
            int staffIdx1    = staffIdx();
            int staffIdx2    = staffIdx1 + _span - 1;
            Segment* segment = static_cast<Segment*>(parent());
            Measure* measure = segment->measure();
            System* system   = measure->system();
            StaffLines* l1   = measure->staffLines(staffIdx1);
            StaffLines* l2   = measure->staffLines(staffIdx2);

            qreal yp;
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

void BarLine::draw(Painter* p) const
      {
      qreal lw = point(score()->styleS(ST_barWidth));
      qreal y1, y2;
      getY(&y1, &y2);

      p->setLineCap(CAP_BUTT);
      p->setPenWidth(lw);

      qreal mags = magS();
      qreal ld   = spatium();       // actual line distance

      switch(subtype()) {
            case BROKEN_BAR:
                  p->setLineStyle(Qt::DashLine);

            case NORMAL_BAR:
                  p->drawLine(lw * .5, y1, lw * .5, y2);
                  break;

            case END_BAR:
                  {
                  qreal lw2 = point(score()->styleS(ST_endBarWidth));
                  qreal d   = point(score()->styleS(ST_endBarDistance));

                  p->drawLine(lw * .5, y1, lw * .5, y2);
                  p->setPenWidth(lw2);
                  qreal x = d + lw2 * .5 + lw;
                  p->drawLine(x, y1, x, y2);
                  }
                  break;

            case DOUBLE_BAR:
                  {
                  lw      = point(score()->styleS(ST_doubleBarWidth));
                  qreal d = point(score()->styleS(ST_doubleBarDistance));

                  p->setPenWidth(lw);
                  qreal x = lw * .5;
                  p->drawLine(x, y1, x, y2);
                  x += d + lw;
                  p->drawLine(x, y1, x, y2);
                  }
                  break;

            case START_REPEAT:
                  {
                  qreal lw2  = point(score()->styleS(ST_endBarWidth));
                  qreal d1   = point(score()->styleS(ST_endBarDistance));

                  const Sym& dotsym = symbols[score()->symIdx()][dotSym];

                  qreal x2   =  lw2 * .5;                               // thick line (lw2)
                  qreal x1   =  lw2 + d1 + lw * .5;                     // thin line (lw)
                  qreal x0   =  lw2 + d1 + lw + d1;                     // dot position

                  if (parent() == 0) {    // for use in palette
                        dotsym.draw(p, mags, x0, 1.5 * ld);
                        dotsym.draw(p, mags, x0, 2.5 * ld);
                        }
                  else {
                        qreal doty1, doty2;
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
                        qreal yp = system->staff(staffIdx())->y();
                        for (int i = 0; i < _span; ++i) {
                              StaffLines* l2 = measure->staffLines(staffIdx() + i);
                              qreal yy = l2->y2() - yp;

                              dotsym.draw(p, mags, x0, yy - doty1 * ld);
                              dotsym.draw(p, mags, x0, yy - doty2 * ld);
                              }
                        }

                  p->drawLine(x1, y1, x1, y2);
                  p->setPenWidth(lw2);
                  p->drawLine(x2, y1, x2, y2);

                  if (score()->styleB(ST_repeatBarTips)) {
                        symbols[score()->symIdx()][brackettipsRightUp].draw(p, mags, 0.0, y1);
                        symbols[score()->symIdx()][brackettipsRightDown].draw(p, mags, 0.0, y2);
                        }
                  }
                  break;

            case END_REPEAT:
                  {
                  qreal lw2  = point(score()->styleS(ST_endBarWidth));
                  qreal d1   = point(score()->styleS(ST_endBarDistance));
                  const Sym& dotsym = symbols[score()->symIdx()][dotSym];
                  qreal dotw = dotsym.width(mags);
                  qreal x1   =  dotw + d1 + lw * .5;
                  qreal x2   =  dotw + d1 + lw + d1 + lw2 * .5;

                  if (parent() == 0) {    // for use in palette
                        dotsym.draw(p, mags, 0.0, 1.5 * ld);
                        dotsym.draw(p, mags, 0.0, 2.5 * ld);
                        }
                  else {
                        qreal doty1, doty2;
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
                        qreal yp = system->staff(staffIdx())->y();
                        for (int i = 0; i < _span; ++i) {
                              StaffLines* l2 = measure->staffLines(staffIdx() + i);
                              qreal yy = l2->y2() - yp;
                              dotsym.draw(p, mags, 0.0, yy - doty1 * ld);
                              dotsym.draw(p, mags, 0.0, yy - doty2 * ld);
                              }
                        }

                  p->drawLine(x1, y1, x1, y2);
                  p->setPenWidth(lw2);
                  p->drawLine(x2, y1, x2, y2);
                  if (score()->styleB(ST_repeatBarTips)) {
                        qreal x = x2 + lw2 * .5;
                        symbols[score()->symIdx()][brackettipsLeftUp].draw(p, mags, x, y1);
                        symbols[score()->symIdx()][brackettipsLeftDown].draw(p, mags, x, y2);
                        }
                  }
                  break;

            case END_START_REPEAT:
                  {
                  qreal lw2  = point(score()->styleS(ST_endBarWidth));
                  qreal d1   = point(score()->styleS(ST_endBarDistance));
                  const Sym& dotsym = symbols[score()->symIdx()][dotSym];
                  qreal dotw = dotsym.width(mags);

                  qreal x1   =  dotw + d1 + lw * .5;                                // thin bar
                  qreal x2   =  dotw + d1 + lw + d1 + lw2 * .5;                     // thick bar
                  qreal x3   =  dotw + d1 + lw + d1 + lw2 + d1 + lw * .5;           // thin bar
                  qreal x4   =  dotw + d1 + lw + d1 + lw2 + d1 + lw + dotw * .5;    // dots

                  if (parent() == 0) {    // for use in palette
                        dotsym.draw(p, mags, .0, 1.5 * ld);
                        dotsym.draw(p, mags, .0, 2.5 * ld);
                        dotsym.draw(p, mags, x4, 1.5 * ld);
                        dotsym.draw(p, mags, x4, 2.5 * ld);
                        }
                  else {
                        qreal doty1, doty2;
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
                        qreal yp = system->staff(staffIdx())->y();
                        for (int i = 0; i < _span; ++i) {
                              StaffLines* l2 = measure->staffLines(staffIdx() + i);
                              qreal yy = l2->y2() - yp;

                              dotsym.draw(p, mags, 0.0, yy - doty1 * ld);
                              dotsym.draw(p, mags, 0.0, yy - doty2 * ld);
                              dotsym.draw(p, mags, x4, yy - doty1 * ld);
                              dotsym.draw(p, mags, x4, yy - doty2 * ld);
                              }
                        }

                  p->drawLine(x1, y1, x1, y2);
                  p->setPenWidth(lw2);
                  p->drawLine(x2, y1, x2, y2);
                  p->setPenWidth(lw);
                  p->drawLine(x3, y1, x3, y2);
                  }
                  break;
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void BarLine::read(XmlReader* r)
      {
      QString val;
      while (r->readElement()) {
            if (r->readString("subtype", &val)) {
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
            else if (!Element::readProperties(r))
                  r->unknown();
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
//   layout
//---------------------------------------------------------

void BarLine::layout()
      {
      _span = staff() ? staff()->barLineSpan() : 1;

      qreal y1, y2;
      getY(&y1, &y2);
      Spatium w = score()->styleS(ST_barWidth);
      qreal dw  = 0.0;

      qreal dotwidth = symbols[score()->symIdx()][dotSym].width(magS());
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
                  w  += score()->styleS(ST_endBarWidth) + 3 * score()->styleS(ST_endBarDistance);
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
            // qreal mags = magS();
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


