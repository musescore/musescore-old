//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: system.cpp,v 1.41 2006/04/12 14:58:10 wschweer Exp $
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
#include "style.h"
#include "bracket.h"
#include "globals.h"
#include "barline.h"
#include "lyrics.h"
#include "layout.h"

//---------------------------------------------------------
//   SysStaff
//---------------------------------------------------------

SysStaff::SysStaff()
      {
      idx             = 0;
      instrumentName  = 0;
      }

//---------------------------------------------------------
//   ~SysStaff
//---------------------------------------------------------

SysStaff::~SysStaff()
      {
      foreach(Bracket* b, brackets)
            delete b;
      if (instrumentName)
            delete instrumentName;
      }

//---------------------------------------------------------
//   System
//---------------------------------------------------------

System::System(Score* s)
   : Element(s)
      {
      barLine = 0;
      }

//---------------------------------------------------------
//   ~System
//---------------------------------------------------------

System::~System()
      {
      if (barLine)
            delete barLine;
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

SysStaff* System::insertStaff(Staff* /*s*/, int idx)
      {
      SysStaff* staff = new SysStaff;
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

double System::layout(ScoreLayout* layout, const QPointF& p, double w)
      {
      static const double instrumentNameOffset = 1;
      setPos(p);
      setWidth(w);
      int nstaves  = _staves.size();

      //---------------------------------------------------
      //  find x position of staves
      //    create brackets
      //---------------------------------------------------

      double xoff2 = 0.0;         // x offset for instrument name

      const QList<Staff*> sl = score()->staves();
      QList<Staff*>::const_iterator is = sl.begin();

      int bracketLevels = sl.front()->bracketLevels();
      double bracketWidth[bracketLevels];
      for (int i = 0; i < bracketLevels; ++i)
            bracketWidth[i] = 0.0;

      for (iSysStaff iss = _staves.begin(); iss != _staves.end(); ++iss, ++is) {
            Staff* s = *is;
            if (!s->show())
                  continue;
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
                  ss->instrumentName->layout(layout);
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

      int staffIdx = 0;

      for (iSysStaff is = _staves.begin(); is != _staves.end(); ++is, ++staffIdx) {
            SysStaff* s    = *is;
            Staff* staff = score()->staff(staffIdx);
            if (!staff->show()) {
                  s->setbbox(QRectF());
                  continue;
                  }
            double staffMag = staff->small() ? 0.7 : 1.0;
            s->setbbox(QRectF(x, 0.0, w, 4 * _spatium * staffMag));
            }

      if (nstaves > 1 && barLine == 0) {
            barLine = new BarLine(score());
            barLine->setParent(this);
            }
      else if (nstaves <= 1 && barLine) {
            delete barLine;
            barLine = 0;
            }
      if (barLine)
            barLine->setPos(x, 0);

      //---------------------------------------------------
      //  layout brackets
      //---------------------------------------------------

      is = sl.begin();
      for (staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            SysStaff* ss = _staves[staffIdx];

            double xo = 0.0;
            for (int i = 0; i < bracketLevels; ++i) {
                  xo += bracketWidth[i] + _spatium * .25;
                  Bracket*   b = ss->brackets[i];
                  if (b == 0)
                        continue;

                  qreal sy = ss->bbox().top();
                  if (b->span() >= (nstaves - staffIdx)) {
                        //
                        // this may happen if a system was removed in
                        // instruments dialog
                        //
                        b->setSpan(nstaves - staffIdx);
                        }
                  qreal ey = _staves[staffIdx + b->span() - 1]->bbox().bottom();
                  b->setPos(x - xo, sy);
                  b->setHeight(ey - sy);
                  }
            }

      //---------------------------------------------------
      //  layout instrument names
      //---------------------------------------------------

      int idx = 0;
      foreach (Part* p, *score()->parts()) {
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

void System::layout2(ScoreLayout* layout)
      {
// printf("System::layout2() %f\n", _spatium);
      int staves = _staves.size();

      qreal y = 0.0;
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Staff* staff    = score()->staff(staffIdx);
            double staffMag = staff->small() ? 0.7 : 1.0;
            if (staff->isTopSplit())
                  setDistance(staffIdx, score()->style()->accoladeDistance);
            else
                  setDistance(staffIdx, score()->style()->staffDistance);
            double dist = 0.0;
            foreach(Measure* m, ml)
                  dist = std::max(dist, m->distance(staffIdx));
            if (dist > distance(staffIdx))
                  setDistance(staffIdx, dist);
            //
            //  layout lyrics separators
            //
            foreach(Measure* m, ml) {
                  MStaff* ms = m->staffList()->at(staffIdx);
                  if (ms->lines)
                        ms->lines->setPos(QPointF(0.0, 0.0));
                  for (Segment* s = m->first(); s; s = s->next()) {
                        LyricsList* ll = s->lyricsList(staffIdx);
                        if (!ll)
                              continue;
                        foreach(Lyrics* l, *ll) {
                              if (!l)
                                   continue;
                              layoutLyrics(layout, l, s, staffIdx);
                              }
                        }
                  }

            SysStaff* s = _staves[staffIdx];
            if (!staff->show()) {
                  s->setbbox(QRectF());
                  continue;
                  }
            s->setbbox(QRectF(s->bbox().x(), y, s->bbox().width(), 4 * _spatium * staffMag));
            // moveY measures
            if (y != 0.0) {
                  foreach(Measure* m, ml)
                        m->moveY(staffIdx, y);
                  }
            y += 4 * _spatium * staffMag + s->distance();
            }

      //---------------------------------------------------
      //    layout bars
      //---------------------------------------------------

      double staffY[staves];
      for (int i = 0; i < staves; ++i)
            staffY[i] = staff(i)->bbox().y();

      qreal systemHeight = staff(staves-1)->bbox().bottom();
      setHeight(systemHeight);

      foreach(Measure* m, ml) {
            QList<Part*>* pl = _score->parts();
            // double x  = m->width();
            int staffIdx = 0;
            // barLineLen += score()->style()->staffLineWidth;
            foreach(Part* p, *pl) {
                  int track = staffIdx * VOICES;
                  double staffMag = score()->staff(staffIdx)->small() ? 0.7 : 1.0;
                  Spatium barLineLen(4.0 * staffMag);
                  for (Segment* s = m->first(); s; s = s->next()) {
                        if ((s->subtype() != Segment::SegEndBarLine)
                           && (s->subtype() != Segment::SegStartRepeatBarLine))
                              continue;
                        if (s->element(track)) {
                              BarLine* barLine = (BarLine*)(s->element(track));
                              double y1 = staffY[staffIdx];
                              double y2 = staffY[staffIdx + p->nstaves() - 1] + point(barLineLen);
                              barLine->setHeight(y2 - y1);
                              }
                        }
                  staffIdx += p->nstaves();
                  }
            m->setHeight(systemHeight);
            }

      if (barLine)
            barLine->setHeight(systemHeight);

      //---------------------------------------------------
      //  layout brackets
      //---------------------------------------------------

      int bracketLevels = score()->staff(0)->bracketLevels();
      double bracketWidth[bracketLevels];
      for (int i = 0; i < bracketLevels; ++i)
            bracketWidth[i] = 0.0;

      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            SysStaff* ss = _staves[staffIdx];

            double xo = 0.0;
            for (int i = 0; i < bracketLevels; ++i) {
                  xo += bracketWidth[i] + _spatium * .25;
                  Bracket*   b = ss->brackets[i];
                  if (b == 0)
                        continue;

                  int restStaves = staves - staffIdx;
                  if (b->span() >= restStaves) {
                        //
                        // this may happen if a system was removed in
                        // instruments dialog
                        //
                        b->setSpan(restStaves);
                        }
                  qreal sy = ss->bbox().top();
                  qreal ey = _staves[staffIdx + b->span() - 1]->bbox().bottom();
                  b->setPos(b->ipos().x(), sy);
                  b->setHeight(ey - sy);
                  b->layout(layout);
                  }
            }

      //---------------------------------------------------
      //  layout instrument names
      //---------------------------------------------------

      int idx = 0;
      foreach(Part* p, *score()->parts()) {
            SysStaff* s = staff(idx);
            int nstaves = p->nstaves();
            if (s->instrumentName && !s->instrumentName->isEmpty()) {
                  //
                  // override Text->layout()
                  //
                  double y1 = s->bbox().top();
                  double y2 = staff(idx + nstaves - 1)->bbox().bottom();
                  double y  = y1 + (y2 - y1) * .5 - s->instrumentName->bbox().height() * .5;
                  s->instrumentName->setPos(s->instrumentName->ipos().x(), y);
                  }
            idx += nstaves;
            }
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void SysStaff::move(double x, double y)
      {
      _bbox.translate(x, y);
  //    sstaff->move(x, y);

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
      ml.clear();
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
      if (cs->mainLayout()->systems() && !cs->mainLayout()->systems()->empty() && cs->mainLayout()->systems()->front() == this) {
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
//   add
//---------------------------------------------------------

void System::add(Element* el)
      {
      if (el->type() == TEXT && (el->subtype() == TEXT_INSTRUMENT_LONG || el->subtype() == TEXT_INSTRUMENT_SHORT)) {
            SysStaff* ss = _staves[el->staffIdx()];
            ss->instrumentName = (Text*)el;
            }
      else if (el->type() == BRACKET) {
            SysStaff* ss = _staves[el->staffIdx()];
            Bracket* b = (Bracket*)el;
            b->setParent(this);
            int level = b->level();
            ss->brackets[level] = b;
            b->staff()->setBracket(level,   b->subtype());
            b->staff()->setBracketSpan(level, b->span());
            }
      else if (el->type() == MEASURE)
            score()->addMeasure((Measure*)el);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void System::remove(Element* el)
      {
// printf("System::remove: %s staff %d\n", el->name(), el->staffIdx());
      if (el->type() == TEXT && (el->subtype() == TEXT_INSTRUMENT_LONG || el->subtype() == TEXT_INSTRUMENT_SHORT)) {
            SysStaff* staff = _staves[el->staffIdx()];
            staff->instrumentName = 0;
            }
      else if (el->type() == BRACKET) {
            SysStaff* staff = _staves[el->staffIdx()];
            for (int i = 0; i < staff->brackets.size(); ++i) {
                  if (staff->brackets[i] == el) {
                        staff->brackets[i] = 0;
                        el->staff()->setBracket(i, NO_BRACKET);
                        // TODO: remove empty bracket levels
//TODO                        score()->layout();
                        return;
                        }
                  }
            printf("internal error: bracket not found\n");
            }
      else if (el->type() == MEASURE)
            score()->removeMeasure((Measure*)el);
      else
            printf("System::remove(%s) not implemented\n", el->name());
      }

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

int System::snap(int tick, const QPointF p) const
      {
      foreach(const Measure* m, ml) {
            if (p.x() < m->x() + m->width())
                  return m->snap(tick, p - m->pos());
            }
      return ml.back()->snap(tick, p-pos());
      }

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

int System::snapNote(int tick, const QPointF p, int staff) const
      {
      foreach(const Measure* m, ml) {
            if (p.x() < m->x() + m->width())
                  return m->snapNote(tick, p - m->pos(), staff);
            }
      return ml.back()->snap(tick, p-pos());
      }

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

Measure* System::prevMeasure(const Measure* m) const
      {
      if (m == ml.front())
            return 0;
      return m->prev();
      }

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

Measure* System::nextMeasure(const Measure* m) const
      {
      if (m == ml.back())
            return 0;
      return m->next();
      }

//---------------------------------------------------------
//   layoutLyrics
//---------------------------------------------------------

void System::layoutLyrics(ScoreLayout* layout, Lyrics* l, Segment* s, int staffIdx)
      {
      if (l->syllabic() == Lyrics::SINGLE || l->syllabic() == Lyrics::END) {
            Line* line = l->separator();
            if (line) {
                  // delete line;  // does not work, undo needs this object
                  l->setSeparator(0);
                  }
            return;
            }
      //
      // we have to layout a separator to the next
      // Lyric syllable
      //
      int verse   = l->no();
      Segment* ns = s;
      while ((ns = ns->next1())) {
            LyricsList* nll = ns->lyricsList(staffIdx);
            if (!nll)
                  continue;
            Lyrics* nl = nll->value(verse);
            if (!nl) {
                  // ignore last line which connects
                  // to nothing
                  continue;
                  }
            Line* line = l->separator();
            if (!line) {
                  line = new Line(l->score(), false);
                  line->setLineWidth(Spatium(0.1));
                  }
            QRectF b = l->bbox();
            qreal w  = b.width();
            qreal h  = b.height();
            qreal x  = b.x() + _spatium + w;
            qreal y  = b.y() + h * .5;
            line->setPos(QPointF(x, y));

            qreal x1 = l->canvasPos().x();
            qreal x2 = nl->canvasPos().x();
            qreal len;
            if (x2 < x1) {
                  System* system = s->measure()->system();
                  x2 = system->canvasPos().x() + system->bbox().width();
                  }
            len = x2 - x1 - 2 * _spatium - w;
            Spatium sp;
            sp.set(len);
            line->setLen(sp);
            line->layout(layout);
            l->setSeparator(line);
            break;
            }
      }

