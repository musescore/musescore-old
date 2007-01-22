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

//---------------------------------------------------------
//   SysStaff
//---------------------------------------------------------

SysStaff::SysStaff()
      {
      idx             = 0;
      bracket           = 0;
      sstaff          = 0;
      _distance       = ::style->staffDistance;
      instrumentName  = 0;
      }

//---------------------------------------------------------
//   ~SysStaff
//---------------------------------------------------------

SysStaff::~SysStaff()
      {
      if (bracket)
            delete bracket;
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

const QRectF& System::bboxStaff(int staff) const
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
      staff->sstaff   = new SStaff(score());
      staff->sstaff->setParent(this);
#if 0
      if (s->bracket() != NO_BRACKET) {
            staff->bracket = new Bracket(score(), s->bracket());
            staff->bracket->setParent(this);
            staff->bracket->setStaff(s);
            staff->setDistance(::style->akkoladeDistance);
            }
#endif
      insertSysStaff(staff, idx);
      setInstrumentName(idx);

      // calculate new height of bounding box
      double h = _spatium * 4;
      int n    = _staves.size();
      for (int i = 1; i < n; ++i) {
            h += distance(i);
            h += _spatium * 4;
            }
      setHeight(h);
      return staff;
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

      double xoff1 = 0.0;         // x offset for bracket
      double xoff2 = 0.0;         // x offset for instrument name

      StaffList* sl = score()->staves();
      iStaff is = sl->begin();
      for (iSysStaff iss = _staves.begin(); iss != _staves.end(); ++iss, ++is) {
            SysStaff* ss = *iss;
            Staff* s     = *is;
            Bracket*   b = ss->bracket;
            if (s->bracket() == NO_BRACKET) {
                  if (b) {
                        delete b;
                        ss->bracket = 0;
                        b = 0;
                        }
                  }
            else {
                  if (b == 0) {
                        ss->bracket = b = new Bracket(score());
                        b->setSubtype(s->bracket());
                        b->setParent(this);
                        b->setStaff(s);
                        }
                  b->setSubtype(s->bracket());
                  int span = s->bracketSpan();
                  b->setSpan(span);
                  double w = b->width();
                  if (w > xoff1)
                        xoff1 = w;
                  }
            if (ss->instrumentName && !ss->instrumentName->isEmpty()) {
                  double w = ss->instrumentName->width() + instrumentNameOffset * _spatium;
                  if (w > xoff2)
                        xoff2 = w;
                  }
            }

      //---------------------------------------------------
      //  layout  SysStaff and SStaff
      //---------------------------------------------------

      qreal x = xoff1 + xoff2;
      qreal y = 0.0;
      for (iSysStaff is = _staves.begin(); is != _staves.end(); ++is) {
            SysStaff* s    = *is;
            SStaff* sstaff = s->sstaff;
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
      int staffIdx = 0;
      for (iSysStaff iss = _staves.begin(); iss != _staves.end(); ++is, ++iss, ++staffIdx) {
            SysStaff* ss = *iss;
            Bracket*   b = ss->bracket;
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
            b->setPos(x - b->width(), sy);
            b->setHeight(ey - sy);
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
            if (s->instrumentName) {
                  double y1 = s->bbox().top();
                  double y2 = staff(idx + nstaves - 1)->bbox().bottom();
                  double y  = y1 + (y2 - y1) * .5 - s->instrumentName->bbox().height() * .5;
                  double d  = instrumentNameOffset * _spatium + s->instrumentName->bbox().width();
                  s->instrumentName->setPos(xoff2 - d, y);
                  }
            idx += nstaves;
            }
      qreal h;
      if (_staves.empty())
            h = 50.0;		// TODO: HACK
      else
      	h = _staves.back()->bbox().bottom();
      if (barLine) {
            barLine->setHeight(h);
            barLine->setPos(x, 0);
            }
      setHeight(h);
      return x;
      }

//---------------------------------------------------------
//   layout2
//    called after measure layout
//---------------------------------------------------------

void System::layout2()
      {
#if 0
      int staves = score()->nstaves();

      for (int staff = 0; staff < staves; ++staff) {
            double dist = 0.0;
            for (iMeasure im = ml->begin(); im != ml->end(); ++im) {
                  Measure* m = *im;
                  dist = std::max(dist, m->distance(staff));
                  }
            if (dist > distance(staff))
                 setDistance(staff, dist);
            }
#endif
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void SysStaff::move(double x, double y)
      {
      _bbox.translate(x, y);
      sstaff->move(x, y);

      if (bracket)
            bracket->move(x, y);
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
            Element* el = (*is)->bracket;
            if (el && el->contains(p))
                  return el;
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
      Staff* s = score()->staff(idx);
      if (!s->isTop())
            return;
      SysStaff* staff = _staves[idx];
      Score* cs = score();
      if (cs->systems() && !cs->systems()->empty() && cs->systems()->front() == this) {
            if (!s->longName().isEmpty()) {
                  if (staff->instrumentName == 0) {
                        staff->instrumentName = new InstrumentName1(score());
                        staff->instrumentName->setParent(this);
                        staff->instrumentName->setStaff(s);
                        }
                  staff->instrumentName->setDoc(s->longName());
                  }
            else if (staff->instrumentName) {
                  staff->instrumentName->setText("");
                  // delete staff->instrumentName;
                  // staff->instrumentName = 0;
                  }
            }
      else {
            if (!s->shortName().isEmpty()) {
                  if (staff->instrumentName == 0) {
                        staff->instrumentName = new InstrumentName2(score());
                        staff->instrumentName->setParent(this);
                        staff->instrumentName->setStaff(s);
                        }
                  staff->instrumentName->setDoc(s->shortName());
                  }
            else if (staff->instrumentName) {
                  // delete staff->instrumentName;
                  // staff->instrumentName = 0;
                  staff->instrumentName->setText("");
                  }
            }
      if (staff->instrumentName) {
            staff->instrumentName->setParent(this);
            staff->instrumentName->setStaff(s);
            }
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
      for (ciSysStaff is = _staves.begin(); is != _staves.end(); ++is) {
            (*is)->sstaff->draw(p);
            if ((*is)->bracket)
                  (*is)->bracket->draw(p);
            if ((*is)->instrumentName)
                  (*is)->instrumentName->draw(p);
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
            double eey = distance(sl->idx(*i));
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
                        double y1 = measure->y() + (sl->idx(*staff)) * dy;
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
      if ((unsigned)n >= _staves.size()) {
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
// printf("System::add: %s staff %d\n", el->name(), el->staffIdx());
      SysStaff* staff = _staves[el->staffIdx()];
      if (el->type() == INSTRUMENT_NAME1 || el->type() == INSTRUMENT_NAME2)
            staff->instrumentName = (Text*)el;
      else if (el->type() == BRACKET) {
            staff->bracket = (Bracket*)el;
            staff->bracket->setParent(this);
            el->staff()->setBracket(staff->bracket->subtype());
            el->staff()->setBracketSpan(staff->bracket->span());
// printf("   bracket %d %d\n", el->staff()->bracket(), el->staff()->bracketSpan());
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
      if (el->type() == INSTRUMENT_NAME1 || el->type() == INSTRUMENT_NAME2)
            staff->instrumentName = 0;
      else if (el->type() == BRACKET) {
            staff->bracket = 0;
            el->staff()->setBracket(NO_BRACKET);
            score()->layout();
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

