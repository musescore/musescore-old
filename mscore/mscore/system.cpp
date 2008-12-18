//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: system.cpp,v 1.41 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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
#include "system.h"

//---------------------------------------------------------
//   SysStaff
//---------------------------------------------------------

SysStaff::SysStaff()
      {
      idx             = 0;
      instrumentName  = 0;
      _show           = true;
      }

//---------------------------------------------------------
//   ~SysStaff
//---------------------------------------------------------

SysStaff::~SysStaff()
      {
      foreach(Bracket* b, brackets)
            delete b;
      brackets.clear();
      if (instrumentName) {
            delete instrumentName;
            instrumentName = 0;
            }
      }

//---------------------------------------------------------
//   System
//---------------------------------------------------------

System::System(Score* s)
   : Element(s)
      {
      barLine      = 0;
      _leftMargin  = 0.0;
      _pageBreak   = false;
      _firstSystem = false;
      _vbox        = false;
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
      if (staff >= int(_staves.size())) {
            printf("System: bad sysStaff idx %d, size %d, vbox %d\n", staff, _staves.size(), _vbox);
            abort();
            }
      return _staves[staff]->bbox();
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

SysStaff* System::insertStaff(int idx)
      {
      SysStaff* staff = new SysStaff;
      _staves.insert(idx, staff);
      if (!_vbox)
            setInstrumentName(idx);
      return staff;
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

SysStaff* System::removeStaff(int idx)
      {
      return _staves.takeAt(idx);
      }

//---------------------------------------------------------
//   layout
//    If first MeasureBase is a HBOX, then xo1 is the
//    width of this box.
//---------------------------------------------------------

/**
 Layout the System.
*/

void System::layout(ScoreLayout* layout, double xo1)
      {
      if (isVbox())                 // ignore vbox
            return;
      static const Spatium instrumentNameOffset(1.0);

      Style* style = score()->style();

      int nstaves  = _staves.size();

      if (nstaves != score()->nstaves())
            printf("System::layout: nstaves %d != %d\n", nstaves, score()->nstaves());

      //---------------------------------------------------
      //  find x position of staves
      //    create brackets
      //---------------------------------------------------

      double xoff2 = 0.0;         // x offset for instrument name

      int bracketLevels = score()->staff(0)->bracketLevels();
      double bracketWidth[bracketLevels];
      for (int i = 0; i < bracketLevels; ++i)
            bracketWidth[i] = 0.0;

      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            Staff* s     = score()->staff(staffIdx);
            SysStaff* ss = _staves[staffIdx];
            if (!ss->show())
                  continue;

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
                              b->setTrack(staffIdx * VOICES);
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
                  double w = ss->instrumentName->width() + instrumentNameOffset.point();
                  if (w > xoff2)
                        xoff2 = w;
                  }
            }

      //---------------------------------------------------
      //  layout  SysStaff and StaffLines
      //---------------------------------------------------

      // xoff2 += xo1;
      _leftMargin = xoff2;

      for (int i = 0; i < bracketLevels; ++i)
            _leftMargin += bracketWidth[i] + style->bracketDistance.point();

      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            SysStaff* s  = _staves[staffIdx];
            Staff* staff = score()->staff(staffIdx);
            if (!s->show()) {
                  s->setbbox(QRectF());
                  continue;
                  }
            double staffMag = staff->mag();
            s->setbbox(QRectF(_leftMargin + xo1, 0.0, 0.0, 4 * _spatium * staffMag));
            }

      if (nstaves > 1 && barLine == 0) {
            barLine = new Line(score(), true);
            barLine->setLineWidth(style->barWidth);
            barLine->setParent(this);
            }
      else if (nstaves <= 1 && barLine) {
            delete barLine;
            barLine = 0;
            }
      if (barLine) {
            barLine->setPos(_leftMargin + xo1, style->barWidth.point() * .25);
            }

      //---------------------------------------------------
      //  layout brackets
      //---------------------------------------------------

      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            SysStaff* ss = _staves[staffIdx];
            if (!ss->show())
                  continue;

            double xo = 0.0;
            for (int i = 0; i < bracketLevels; ++i) {
                  xo += bracketWidth[i] + style->bracketDistance.point();
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
                  if (!_staves[staffIdx + b->span() - 1]->show()) {
                        //
                        // if the bracket ends on an invisible staff
                        // find last visible staff in bracket
                        //
                        for (int j = staffIdx + b->span() - 2; j >= staffIdx; --j) {
                              if (_staves[j]->show()) {
                                    b->setSpan(j - staffIdx + 1);
                                    break;
                                    }
                              }
                        }
                  qreal ey = _staves[staffIdx + b->span() - 1]->bbox().bottom();
                  b->setPos(_leftMargin - xo + xo1, sy);
                  b->setHeight(ey - sy);
                  }
            }

      //---------------------------------------------------
      //  layout instrument names x position
      //---------------------------------------------------

      int idx = 0;
      foreach (Part* p, *score()->parts()) {
            SysStaff* s = staff(idx);
            int nstaves = p->nstaves();
            if (s->instrumentName && !s->instrumentName->isEmpty()) {
                  double d  = instrumentNameOffset.point() + s->instrumentName->bbox().width();
                  s->instrumentName->setXpos(xoff2 - d + xo1);
                  }
            idx += nstaves;
            }
      }

//---------------------------------------------------------
//   layout2
//    called after measure layout
//    adjusts staff distance
//---------------------------------------------------------

void System::layout2(ScoreLayout* layout)
      {
      if (isVbox())                 // ignore vbox
            return;

      int staves = _staves.size();

      qreal y = 0.0;
      int lastStaffIdx = 0;   // last visible staff
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Staff* staff = score()->staff(staffIdx);
            if ((staffIdx + 1) == staves) {
                  setDistance(staffIdx, score()->style()->systemDistance);
                  }
            else if (staff->rstaff() < (staff->part()->staves()->size()-1)) {
                  setDistance(staffIdx, score()->style()->accoladeDistance);
                  }
            else {
                  setDistance(staffIdx, score()->style()->staffDistance);
                  }
            double dist = 0.0;
            foreach(MeasureBase* m, ml) {
                  dist = std::max(dist, m->distance(staffIdx));
                  dist = std::max(dist, m->userDistance(staffIdx).point());
                  }
            if (dist > distance(staffIdx))
                  setDistance(staffIdx, dist);
            SysStaff* s = _staves[staffIdx];
            if (!s->show()) {
                  s->setbbox(QRectF());
                  continue;
                  }
            double sHeight = staff->height();   // (staff->lines() - 1) * _spatium * staffMag;
            s->setbbox(QRectF(_leftMargin, y, width() - _leftMargin, sHeight));
            y += sHeight + s->distance();
            lastStaffIdx = staffIdx;
            }
      qreal systemHeight = staff(lastStaffIdx)->bbox().bottom();
      setHeight(systemHeight);
      foreach(MeasureBase* m, ml) {
            if (m->type() == MEASURE || m->type() == HBOX)
                  m->setHeight(systemHeight);
            }

      double staffY[staves];
      for (int i = 0; i < staves; ++i)
            staffY[i] = staff(i)->bbox().y();

      if (barLine) {
            Spatium lw = score()->style()->barWidth;
            barLine->setLen(Spatium(systemHeight / _spatium) - lw * .5);
            barLine->layout(layout);
            }

      //---------------------------------------------------
      //  layout brackets
      //---------------------------------------------------

      int bracketLevels = score()->staff(0)->bracketLevels();
      double bracketWidth[bracketLevels];
      for (int i = 0; i < bracketLevels; ++i)
            bracketWidth[i] = 0.0;

      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            SysStaff* ss = _staves[staffIdx];
            if (!ss->show())
                  continue;

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

      int staffIdx = 0;
      foreach(Part* p, *score()->parts()) {
            SysStaff* s = staff(staffIdx);
            int nstaves = p->nstaves();
            if (s->instrumentName && !s->instrumentName->isEmpty()) {
                  //
                  // override Text->layout()
                  //
                  double y1 = s->bbox().top();
                  double y2 = staff(staffIdx + nstaves - 1)->bbox().bottom();
                  double y  = y1 + (y2 - y1) * .5 - s->instrumentName->bbox().height() * .5;
                  s->instrumentName->setYpos(y);
                  }
            staffIdx += nstaves;
            }
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void SysStaff::move(double x, double y)
      {
      _bbox.translate(x, y);
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
      _vbox        = false;
      _firstSystem = false;
      _pageBreak   = false;
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

void System::setInstrumentName(int staffIdx)
      {
      if (isVbox())                 // ignore vbox
            return;

      Score* cs = score();
      Staff* s  = cs->staff(staffIdx);
      if (!s->isTop())
            return;
      SysStaff* staff = _staves[staffIdx];
      if (staff->instrumentName == 0)
            staff->instrumentName = new TextC(s->part()->longNameBase(), cs);
      if (_firstSystem) {
            staff->instrumentName->setSubtype(TEXT_INSTRUMENT_LONG);
            staff->instrumentName->setTextBase(s->part()->longNameBase());
            }
      else {
            staff->instrumentName->setSubtype(TEXT_INSTRUMENT_SHORT);
            staff->instrumentName->setTextBase(s->part()->shortNameBase());
            }
      staff->instrumentName->setParent(this);
      staff->instrumentName->setTrack(staffIdx * VOICES);
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
      foreach(SysStaff* s, _staves) {
            qreal t = s->bbox().top();
            qreal b = s->bbox().bottom();
            qreal y1 = t - 0.6 * (b - t);
            qreal y2 = b + 0.6 * (b - t);
            if (y >= y1 && y < y2)
                  return idx;
            ++idx;
            }
      return -1;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void System::add(Element* el)
      {
      el->setParent(this);
      if (el->type() == TEXT && (el->subtype() == TEXT_INSTRUMENT_LONG || el->subtype() == TEXT_INSTRUMENT_SHORT)) {
            _staves[el->staffIdx()]->instrumentName = static_cast<TextC*>(el);
            }
      else if (el->type() == BRACKET) {
            SysStaff* ss = _staves[el->staffIdx()];
            Bracket* b   = (Bracket*)el;
            int level = b->level();
            ss->brackets[level] = b;
            b->staff()->setBracket(level,   b->subtype());
            b->staff()->setBracketSpan(level, b->span());
            }
      else if (el->type() == MEASURE || el->type() == HBOX || el->type() == VBOX)
            score()->addMeasure((MeasureBase*)el);
      else
            printf("System::add(%s) not implemented\n", el->name());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void System::remove(Element* el)
      {
      if (el->type() == TEXT && (el->subtype() == TEXT_INSTRUMENT_LONG || el->subtype() == TEXT_INSTRUMENT_SHORT)) {
            _staves[el->staffIdx()]->instrumentName = 0;
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
      else if (el->type() == MEASURE || el->type() == HBOX || el->type() == VBOX)
            score()->removeMeasure((MeasureBase*)el);
      else
            printf("System::remove(%s) not implemented\n", el->name());
      }

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void System::change(Element* o, Element* n)
      {
      if (o->type() == VBOX || o->type() == HBOX)
            score()->layout()->change((MeasureBase*)o, (MeasureBase*)n);
      else {
            remove(o);
            add(n);
            }
      }

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

int System::snap(int tick, const QPointF p) const
      {
      foreach(const MeasureBase* m, ml) {
            if (p.x() < m->x() + m->width())
                  return ((Measure*)m)->snap(tick, p - m->pos()); //TODO: MeasureBase
            }
      return ((Measure*)ml.back())->snap(tick, p-pos());          //TODO: MeasureBase
      }

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

int System::snapNote(int tick, const QPointF p, int staff) const
      {
      foreach(const MeasureBase* m, ml) {
            if (p.x() < m->x() + m->width())
                  return ((Measure*)m)->snapNote(tick, p - m->pos(), staff);  //TODO: MeasureBase
            }
      return ((Measure*)ml.back())->snap(tick, p-pos());          // TODO: MeasureBase
      }

//---------------------------------------------------------
//   firstMeasure
//---------------------------------------------------------

Measure* System::firstMeasure() const {
      for (MeasureBase* mb = ml.front(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            return static_cast<Measure*>(mb);
            }
      return 0;
      }

//---------------------------------------------------------
//   lastMeasure
//---------------------------------------------------------

Measure* System::lastMeasure() const {
      for (MeasureBase* mb = ml.back(); mb; mb = mb->prev()) {
            if (mb->type() != MEASURE)
                  continue;
            return static_cast<Measure*>(mb);
            }
      return 0;
      }

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

MeasureBase* System::prevMeasure(const MeasureBase* m) const
      {
      if (m == ml.front())
            return 0;
      return m->prev();
      }

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

MeasureBase* System::nextMeasure(const MeasureBase* m) const
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
      if ((l->syllabic() == Lyrics::SINGLE || l->syllabic() == Lyrics::END) && (l->endTick() == 0)) {
            l->clearSeparator();
            return;
            }
      if (l->endTick()) {
            Segment* seg = score()->tick2segment(l->endTick());
            if (seg == 0) {
                  printf("System::layoutLyrics: no segment found for tick %d\n", l->endTick());
                  return;
                  }

            QList<Line*>* sl = l->separatorList();
            QList<System*>* systems = layout->systems();
            System* s1 = this;
            System* s2 = seg->measure()->system();
            int sysIdx1  = systems->indexOf(s1);
            int sysIdx2  = systems->indexOf(s2);

            double lw = l->bbox().right();            // lyrics width
            QPointF p1(lw, l->baseLine());

            int segIdx = 0;
            for (int i = sysIdx1; i <= sysIdx2; ++i, ++segIdx) {
                  System* system = (*systems)[i];
                  Line* line = sl->value(segIdx);
                  if (line == 0) {
                        line = new Line(l->score(), false);
                        line->setLineWidth(Spatium(0.1));
                        l->add(line);
                        }
                  if (sysIdx1 == sysIdx2) {
                        // single segment
                        line->setPos(p1);
                        double len = seg->canvasPos().x() - l->canvasPos().x() - lw + 2 * _spatium;
                        line->setLen(Spatium(len / _spatium));
                        }
                  else if (i == sysIdx1) {
                        // start segment
                        line->setPos(p1);
                        double w   = system->staff(l->staffIdx())->right();
                        double x   = system->canvasPos().x() + w;
                        double len = x - l->canvasPos().x() - lw;
                        line->setLen(Spatium(len / _spatium));
                        }
                  else if (i > 0 && i != sysIdx2) {
                        // middle segment
                        }
                  else if (i == sysIdx2) {
                        // end segment
                        }
                  line->layout(layout);
                  }
            return;
            }
      //
      // we have to layout a separator to the next
      // Lyric syllable
      //
      int verse   = l->no();
      Segment* ns = s;

      // TODO: then next two values should be style parameters
      const double maxl = 0.8 * _spatium;   // lyrics hyphen length
      const Spatium hlw(0.1);               // hyphen line width

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
            QList<Line*>* sl = l->separatorList();
            Line* line;
            if (sl->isEmpty()) {
                  line = new Line(l->score(), false);
                  line->setLineWidth(hlw);
                  l->add(line);
                  }
            else {
                  line = (*sl)[0];
                  }
            QRectF b = l->bbox();
            qreal w  = b.width();
            qreal h  = b.height();
            qreal x  = b.x() + w;
            qreal y  = b.y() + h * .5;
            line->setPos(QPointF(x, y));

            qreal x1 = l->canvasPos().x();
            qreal x2 = nl->canvasPos().x();
            qreal len;
            if (x2 < x1) {
                  System* system = s->measure()->system();
                  x2 = system->canvasPos().x() + system->bbox().width();
                  }

            double gap = x2 - x1 - w;
            len        = gap;
            if (len > maxl)
                  len = maxl;
            double xo = (gap - len) * .5;

            line->setPos(QPointF(x + xo, y));
            Spatium sp;
            sp.set(len);
            line->setLen(sp);
            line->layout(layout);
            return;
            }
      l->clearSeparator();
      }

//---------------------------------------------------------
//   collectElements
//    collect all visible elements
//---------------------------------------------------------

void System::collectElements(QList<const Element*>& el) const
      {
      if (isVbox())
            return;
      if (barLine)
            el.append(barLine);
      foreach(SysStaff* st, _staves) {
            if (!st->show())
                  continue;
            foreach(Bracket* b, st->brackets) {
                  if (b)
                        el.append(b);
                  }
            if (st->instrumentName)
                  el.append(st->instrumentName);
            }
      }


