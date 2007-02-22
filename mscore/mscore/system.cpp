//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: system.cpp,v 1.41 2006/04/12 14:58:10 wschweer Exp $
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

/**
 \file
 Implementation of classes SysStaff and System.
*/

#include "system.h"
#include "measure.h"
#include "segment.h"
#include "score.h"
#include "sig.h"
#include "key.h"
#include "xml.h"
#include "clef.h"
#include "text.h"
#include "navigate.h"
#include "select.h"
#include "staff.h"
#include "part.h"
#include "page.h"
#include "painter.h"
#include "style.h"
#include "bracket.h"
#include "globals.h"
#include "barline.h"
#include "lyrics.h"

//---------------------------------------------------------
//   SysStaff
//---------------------------------------------------------

SysStaff::SysStaff()
      {
      idx             = 0;
      sstaff          = 0;
      instrumentName  = 0;
      }

//---------------------------------------------------------
//   ~SysStaff
//---------------------------------------------------------

SysStaff::~SysStaff()
      {
      foreach(Bracket* b, brackets)
            delete b;
      if (sstaff)
            delete sstaff;
      if (instrumentName)
            delete instrumentName;
      }

//---------------------------------------------------------
//   System
//---------------------------------------------------------

System::System(Score* s)
   : Element(s)
      {
      barLine = new BarLine(s);
      barLine->setParent(this);
      ml = new MeasureList;
      }

//---------------------------------------------------------
//   ~System
//---------------------------------------------------------

System::~System()
      {
      if (barLine)
            delete barLine;
      delete ml;
      }

//---------------------------------------------------------
//   bboxStaff
//---------------------------------------------------------

QRectF System::bboxStaff(int staff) const
      {
      if (staff >= int(_staves.size()))
            abort();
      return _staves[staff]->bbox();
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

SysStaff* System::insertStaff(Staff*, int idx)
      {
      SysStaff* staff = new SysStaff;
      staff->sstaff   = new StaffLines(score());
      staff->sstaff->setParent(this);
      insertSysStaff(staff, idx);
      setInstrumentName(idx);
      return staff;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF System::bbox() const
      {
      double h   = 0;
      int n      = _staves.size();
      double lastDist = 0.0;
      for (int i = 0; i < n; ++i) {
            Staff* staff = score()->staff(i);
            if (!staff->show())
                  continue;
            lastDist = distance(i);
            h += lastDist;
// printf("add distance %f staff %d\n", distance(i), i);
            h    += _spatium * 4;
            }
      h -= lastDist;
      return QRectF(0.0, 0.0, _width, h);
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void System::insertSysStaff(SysStaff* staff, int idx)
      {
      if (idx >= int(_staves.size()))
            _staves.push_back(staff);
      else
            _staves.insert(_staves.begin()+idx, staff);
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

SysStaff* System::removeStaff(int idx)
      {
      SysStaff* staff = _staves[idx];
      _staves.erase(_staves.begin()+idx);
      return staff;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

/**
 Layout the System.
*/

double System::layout(const QPointF& p, double w)
      {
      static const double instrumentNameOffset = 1;
      setPos(p);
      setWidth(w);

      //---------------------------------------------------
      //  find x position of staves
      //    create brackets
      //---------------------------------------------------

      double xoff2 = 0.0;         // x offset for instrument name

      StaffList* sl = score()->staves();
      iStaff is = sl->begin();

      int bracketLevels = sl->front()->bracketLevels();
      double bracketWidth[bracketLevels];
      for (int i = 0; i < bracketLevels; ++i)
            bracketWidth[i] = 0.0;

      for (iSysStaff iss = _staves.begin(); iss != _staves.end(); ++iss, ++is) {
            Staff* s = *is;
            SysStaff* ss = *iss;
            if (bracketLevels < ss->brackets.size()) {
                  for (int i = bracketLevels; i < ss->brackets.size(); ++i) {
                        Bracket* b = ss->brackets.takeLast();
                        if (b)
                              delete b;
                        }
                  }
            else if (bracketLevels > ss->brackets.size()) {
                  for (int i = ss->brackets.size(); i < bracketLevels; ++i)
                        ss->brackets.append(0);
                  }
            for (int i = 0; i < bracketLevels; ++i) {
                  Bracket* b = ss->brackets[i];
                  if (s->bracket(i) == NO_BRACKET) {
                        if (b) {
                              delete b;
                              ss->brackets[i] = 0;
                              }
                        }
                  else {
                        if (b == 0) {
                              ss->brackets[i] = b = new Bracket(score());
                              b->setSubtype(s->bracket(i));
                              b->setParent(this);
                              b->setStaff(s);
                              }
                        b->setSubtype(s->bracket(i));
                        int span = s->bracketSpan(i);
                        b->setSpan(span);
                        b->setLevel(i);
                        double w = b->width();
                        if (w > bracketWidth[i])
                              bracketWidth[i] = w;
                        }
                  }
            if (ss->instrumentName && !ss->instrumentName->isEmpty()) {
                  ss->instrumentName->layout();
                  double w = ss->instrumentName->width() + instrumentNameOffset * _spatium;
                  if (w > xoff2)
                        xoff2 = w;
                  }
            }

      //---------------------------------------------------
      //  layout  SysStaff and StaffLines
      //---------------------------------------------------

      qreal x = xoff2;
      for (int i = 0; i < bracketLevels; ++i)
            x += bracketWidth[i];

      qreal y = 0.0;
      int staffIdx = 0;
      for (iSysStaff is = _staves.begin(); is != _staves.end(); ++is, ++staffIdx) {
            SysStaff* s    = *is;
            Staff* staff = score()->staff(staffIdx);
            if (!staff->show()) {
                  s->setbbox(QRectF());
                  continue;
                  }
            StaffLines* sstaff = s->sstaff;
            sstaff->setPos(x, y);
            sstaff->setWidth(w - x);
            s->setbbox(QRectF(x, y, w, 4 * _spatium));
            y += 4 * _spatium + s->distance();
            }

      //---------------------------------------------------
      //  layout brackets
      //---------------------------------------------------

      is = sl->begin();
      int nstaves  = _staves.size();
      staffIdx = 0;
      for (iSysStaff iss = _staves.begin(); iss != _staves.end(); ++is, ++iss, ++staffIdx) {
            SysStaff* ss = *iss;

            double xo = 0.0;
            for (int i = 0; i < bracketLevels; ++i) {
                  xo += bracketWidth[i];
                  Bracket*   b = ss->brackets[i];
                  if (b == 0)
                        continue;

                  int restStaves = nstaves - staffIdx;
                  if (b->span() > restStaves) {
                        //
                        // this may happen if a system was removed in
                        // instruments dialog
                        //
                        b->setSpan(restStaves);
                        }
                  qreal sy = ss->bbox().top();
                  qreal ey = (*(iss + b->span() - 1))->bbox().bottom();
                  b->setPos(x - xo, sy);
                  b->setHeight(ey - sy);
                  }
            }

      //---------------------------------------------------
      //  layout instrument names
      //---------------------------------------------------

      PartList* pl = score()->parts();
      int idx = 0;
      for (iPart ip = pl->begin(); ip != pl->end(); ++ip) {
            Part* p     = *ip;
            SysStaff* s = staff(idx);
            int nstaves = p->nstaves();
            if (s->instrumentName && !s->instrumentName->isEmpty()) {
                  //
                  // override Text->layout()
                  //
                  double y1 = s->bbox().top();
                  double y2 = staff(idx + nstaves - 1)->bbox().bottom();
                  double y  = y1 + (y2 - y1) * .5 - s->instrumentName->bbox().height() * .5;
                  double d  = instrumentNameOffset * _spatium + s->instrumentName->bbox().width();
                  s->instrumentName->setPos(xoff2 - d, y);
                  }
            idx += nstaves;
            }
      return x;
      }

//---------------------------------------------------------
//   layout2
//    called after measure layout
//    adjusts staff distance
//---------------------------------------------------------

void System::layout2()
      {
      int staves = score()->nstaves();

      for (int staff = 0; staff < staves; ++staff) {
            if (score()->staff(staff)->isTopSplit())
                  setDistance(staff, ::style->accoladeDistance);
            else
                  setDistance(staff, ::style->staffDistance);
            double dist = 0.0;
            for (iMeasure im = ml->begin(); im != ml->end(); ++im) {
                  Measure* m = *im;
                  dist = std::max(dist, m->distance(staff));
                  }
            if (dist > distance(staff))
                 setDistance(staff, dist);
            //
            //  layout lyrics separators
            //
            for (iMeasure im = ml->begin(); im != ml->end(); ++im) {
                  Measure* m = *im;
                  for (Segment* s = m->first(); s; s = s->next()) {
                        LyricsList* ll = s->lyricsList(staff);
                        if (!ll)
                              continue;
                        foreach(Lyrics* l, *ll) {
                              if (!l)
                                   continue;
                              if (l->syllabic() == Lyrics::SINGLE || l->syllabic() == Lyrics::END) {
                                    Line* line = l->separator();
                                    if (line) {
                                          // delete line;  // does not work, undo needs this object
                                          l->setSeparator(0);
                                          }
                                    continue;
                                    }
                              //
                              // we have to layout a separator to the next
                              // Lyric syllable
                              //
                              int verse = l->no();
                              Segment* ns = s;
                              while ((ns = ns->next1())) {
                                    LyricsList* nll = ns->lyricsList(staff);
                                    if (!nll)
                                          continue;
                                    Lyrics* nl = nll->value(verse);
                                    if (!nl)
                                          continue;
                                    Line* line = l->separator();
                                    if (!line) {
                                          line = new Line(l->score(), false);
                                          line->setLineWidth(Spatium(0.1));
                                          }
                                    QRectF b = l->bbox();
                                    qreal w = b.width();
                                    qreal h = b.height();
                                    qreal x = b.x() + _spatium + w;
                                    qreal y = b.y() + h * .5;
                                    line->setPos(QPointF(x, y));
                                    QPointF p1 = l->apos();
                                    QPointF p2 = nl->apos();
                                    qreal len = p2.x() - p1.x() - 2 * _spatium - w;
                                    Spatium sp;
                                    sp.set(len);
                                    line->setLen(sp);
                                    l->setSeparator(line);
                                    break;
                                    }
                              }
                        }
                  }

            }
      if (barLine) {
            barLine->setHeight(bbox().height());
            barLine->setPos(_staves[0]->bbox().x(), 0);
            }
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void SysStaff::move(double x, double y)
      {
      _bbox.translate(x, y);
      sstaff->move(x, y);

      foreach(Bracket* b, brackets)
            b->move(x, y);
      if (instrumentName)
            instrumentName->move(x, y);
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

/**
 Clear layout of System.
*/

void System::clear()
      {
      ml->clear();
      }

//---------------------------------------------------------
//   findSelectableElement
//---------------------------------------------------------

/**
 If found, return selectable Element in this System near Page relative point \a p.
*/

Element* System::findSelectableElement(QPointF p) const
      {
      for (ciMeasure im = ml->begin(); im != ml->end(); ++im) {
            Measure* measure = *im;
            Element* element = measure->findSelectableElement(p - pos());
            if (element)
                  return element;
            }
      p -= pos();
      for (ciSysStaff is = _staves.begin(); is != _staves.end(); ++is) {
            foreach(Bracket* b, (*is)->brackets) {
                  if (b && b->contains(p))
                        return b;
                  }
            if ((*is)->instrumentName && (*is)->instrumentName->contains(p))
                  return (*is)->instrumentName;
            }
      return 0;
      }

//---------------------------------------------------------
//   setInstrumentNames
//---------------------------------------------------------

void System::setInstrumentNames()
      {
      for (int staff = 0; staff < score()->nstaves(); ++staff)
            setInstrumentName(staff);
      }

//---------------------------------------------------------
//   setInstrumentName
//---------------------------------------------------------

void System::setInstrumentName(int idx)
      {
      Score* cs = score();
      Staff* s = cs->staff(idx);
      if (!s->isTop())
            return;
      SysStaff* staff = _staves[idx];
      if (staff->instrumentName == 0)
            staff->instrumentName = new Text(cs);
      if (cs->systems() && !cs->systems()->empty() && cs->systems()->front() == this) {
            staff->instrumentName->setSubtype(TEXT_INSTRUMENT_LONG);
            staff->instrumentName->setDoc(s->longName());
            }
      else {
            staff->instrumentName->setSubtype(TEXT_INSTRUMENT_SHORT);
            staff->instrumentName->setDoc(s->shortName());
            }
      staff->instrumentName->setParent(this);
      staff->instrumentName->setStaff(s);
      }

//---------------------------------------------------------
//   draw
//    p - has page coordinate system
//    f - System relative
//---------------------------------------------------------

void System::draw(Painter& p)
      {
      p.translate(pos());

      if (barLine /*&& barLine->bbox().intersects(f)*/)
            barLine->draw(p);
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            if (!score()->staff(staffIdx)->show())
                  continue;
            SysStaff* sysStaff = _staves[staffIdx];
            sysStaff->sstaff->draw(p);
            foreach(Bracket* b, sysStaff->brackets) {
                  if (b)
                        b->draw(p);
                  }
            if (sysStaff->instrumentName)
                  sysStaff->instrumentName->draw(p);
            }
      for (ciMeasure im = ml->begin(); im != ml->end(); ++im) {
            Measure* m = *im;
            m->draw(p);
            }
      p.translate(-pos());
      }

//---------------------------------------------------------
//   pos2tick
//    fp is Page relative
//---------------------------------------------------------

bool System::pos2tick(const QPointF& fp, int* tick, Staff** staff, int* pitch) const
      {
      *pitch      = 64;  // default
      double dy   = point(Spatium(4) + style->staffDistance);

      Element* el = findSelectableElement(fp);
      if (el) {
            *tick  = el->tick();
            int clef = (*staff)->clef()->clef(*tick);
            *pitch = y2pitch(fp.y() - y(), clef);
            *staff = el->staff();
            return true;
            }
      //
      // es wurde kein Element direkt angeclickt, also suchen wir nach
      // dem angeclickten Segment

      double sy = bbox().y();
      StaffList* sl = score()->staves();
      for (iStaff i = sl->begin(); i != sl->end(); ++i) {
            double ey = sy + _spatium*4;
            double eey = distance(sl->indexOf(*i));
            if (fp.y() < ey + (eey/2)) {
                  *staff = *i;
                  break;
                  }
            sy = ey + eey;
            }

      for (ciMeasure im = ml->begin(); im != ml->end(); ++im) {
            const Measure* measure = *im;
            double x1 = measure->x();
            double x2 = x1 + measure->width();
            if (x2 < fp.x())
                  continue;
            double measurex2 = measure->x() + measure->width();
            for (Segment* segment = measure->first(); segment;) {
                  Segment* ns = segment->next();
                  double x1 = segment->x();
                  double x2 = ns ? ns->x() : measurex2;
                  if (fp.x() >= x1 && fp.x() < x2) {
                        *tick = segment->tick();
                        double y1 = measure->y() + (sl->indexOf(*staff)) * dy;
                        int clef = (*staff)->clef()->clef(*tick);
                        *pitch = y2pitch(fp.y() - y1, clef);
                        return true;
                        }
                  segment = ns;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   y2staff
//---------------------------------------------------------

/**
 Return staff number for canvas relative y position \a y
 or -1 if not found.

 To allow drag and drop above and below the staff, the actual y range
 considered "inside" the staff is increased a bit.
 TODO: replace magic number "0.6" by something more appropriate.
*/

int System::y2staff(qreal y) const
      {
      y -= pos().y();
      int idx = 0;
      for (ciSysStaff i = _staves.begin(); i != _staves.end(); ++i, ++idx) {
            qreal t = (*i)->bbox().top();
            qreal b = (*i)->bbox().bottom();
            qreal y1 = t - 0.6 * (b - t);
            qreal y2 = b + 0.6 * (b - t);
            if (y >= y1 && y < y2) {
                  return idx;
                  }
            }
      return -1;
      }

//---------------------------------------------------------
//   distance
//---------------------------------------------------------

double System::distance(int n) const
      {
      if (n >= _staves.size()) {
            printf("System::distance(): bad stave index %d >= %zd\n", n, _staves.size());
            return 0;
            }
      return _staves[n]->distance();
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void System::add(Element* el)
      {
      SysStaff* ss = _staves[el->staffIdx()];
      if (el->type() == TEXT && (el->subtype() == TEXT_INSTRUMENT_LONG || el->subtype() == TEXT_INSTRUMENT_SHORT))
            ss->instrumentName = (Text*)el;
      else if (el->type() == BRACKET) {
            Bracket* b = (Bracket*)el;
            b->setParent(this);
            int level = b->level();
            ss->brackets[level] = b;
            b->staff()->setBracket(level,   b->subtype());
            b->staff()->setBracketSpan(level, b->span());
            score()->layout();
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void System::remove(Element* el)
      {
// printf("System::remove: %s staff %d\n", el->name(), el->staffIdx());
      SysStaff* staff = _staves[el->staffIdx()];
      if (el->type() == TEXT && (el->subtype() == TEXT_INSTRUMENT_LONG || el->subtype() == TEXT_INSTRUMENT_SHORT))
            staff->instrumentName = 0;
      else if (el->type() == BRACKET) {
            for (int i = 0; i < staff->brackets.size(); ++i) {
                  if (staff->brackets[i] == el) {
                        staff->brackets[i] = 0;
                        el->staff()->setBracket(i, NO_BRACKET);
                        // TODO: remove empty bracket levels

                        score()->layout();
                        return;
                        }
                  }
            printf("internal error: bracket not found\n");
            }
      }

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

int System::snap(int tick, const QPointF p) const
      {
      for (ciMeasure im = ml->begin(); im != ml->end(); ++im) {
            Measure* m = *im;
            if (p.x() < m->x() + m->width())
                  return m->snap(tick, p - m->pos());
            }
      return ml->back()->snap(tick, p-pos());
      }

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

int System::snapNote(int tick, const QPointF p, int staff) const
      {
      for (ciMeasure im = ml->begin(); im != ml->end(); ++im) {
            Measure* m = *im;
            if (p.x() < m->x() + m->width())
                  return m->snapNote(tick, p - m->pos(), staff);
            }
      return ml->back()->snap(tick, p-pos());
      }

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

Measure* System::prevMeasure(Measure* m) const
      {
      for (iMeasure i = ml->begin(); i != ml->end(); ++i) {
            if (*i == m) {
                  if (i == ml->begin())
                        return 0;
                  --i;
                  return *i;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

Measure* System::nextMeasure(Measure* m) const
      {
      for (iMeasure i = ml->begin(); i != ml->end(); ++i) {
            if (*i == m) {
                  ++i;
                  if (i == ml->end())
                        return 0;
                  return *i;
                  }
            }
      return 0;
      }

