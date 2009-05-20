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

#include "element.h"
#include "canvas.h"
#include "clef.h"
#include "score.h"
#include "note.h"
#include "rest.h"
#include "chord.h"
#include "system.h"
#include "segment.h"
#include "lyrics.h"
#include "harmony.h"
#include "utils.h"
#include "input.h"
#include "measure.h"
#include "page.h"

//---------------------------------------------------------
//   nextChordRest
//    return next NOTE or REST
//    element - current element
//---------------------------------------------------------

ChordRest* nextChordRest(ChordRest* cr)
      {
      if (!cr)
            return 0;
      int tick = cr->tick() + cr->ticks();
      Segment* seg = cr->segment();
      int track = cr->track();

      for (;;) {
            seg = seg->next1();
            if (!seg)
                  break;
            if (seg->measure()->multiMeasure() < 0)
                  continue;
            Element* e = seg->element(track);
            if (e && e->isChordRest() && e->tick()) {
                  if (e->tick() == tick)
                        return static_cast<ChordRest*>(e);
                  else if (e->tick() > tick)
                        return 0;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   prevChordRest
//    return previous Chord or Rest
//---------------------------------------------------------

ChordRest* prevChordRest(ChordRest* cr)
      {
      if (!cr)
            return 0;
      Segment* seg = cr->segment();
      int track = cr->staffIdx() * VOICES + cr->voice();

      for (;;) {
            seg = seg->prev1();
            if (!seg)
                  break;
            if (seg->measure()->multiMeasure() < 0)
                  continue;
            Element* e = seg->element(track);
            if (e && e->isChordRest())
                  return static_cast<ChordRest*>(e);
            }
      return 0;
      }

//---------------------------------------------------------
//   upAlt
//    select next higher pitched note in chord
//---------------------------------------------------------

Note* Score::upAlt(Element* element)
      {
      Element* re = 0;
      if (element->type() == REST) {
            if (_is.track <= 0)
                  return 0;
            --(_is.track);
            re = searchNote(element->tick(), _is.track);
            }
      else if (element->type() == NOTE) {
            // find segment
            Chord* chord     = static_cast<Note*>(element)->chord();
            Segment* segment = chord->segment();

            // collect all notes for this segment in noteList:
            NoteList rnl;
            iNote inote = rnl.end();
            int tracks = nstaves() * VOICES;
            for (int track = 0; track < tracks; ++track) {
                  Element* el = segment->element(track);
                  if (!el || el->type() != CHORD)
                        continue;
                  NoteList* nl = static_cast<Chord*>(el)->noteList();
                  for (riNote in   = nl->rbegin(); in != nl->rend(); ++in) {
                        Note* note = in->second;
                        iNote ii = rnl.add(note);
                        if (note == element) {
                              inote = ii;
                              }
                        }
                  }
            if (inote != rnl.end()) {
                  ++inote;
                  if (inote != rnl.end())
                        re = inote->second;
                  }
            }
      if (re == 0)
            return 0;
      if (re->type() == CHORD)
            re = ((Chord*)re)->noteList()->front();
      return (Note*)re;
      }

//---------------------------------------------------------
//   upAltCtrl
//    select top note in chord
//---------------------------------------------------------

Note* Score::upAltCtrl(Note* note) const
      {
      Chord* chord = note->chord();
      NoteList* nl  = chord->noteList();
      return nl->rbegin()->second;
      }

//---------------------------------------------------------
//   downAlt
//    goto next note with lower pitch in chord or to
//    top note in next staff
//---------------------------------------------------------

Note* Score::downAlt(Element* element)
      {
      Element* re = 0;
      int staves = nstaves();
      if (element->type() == REST) {
            if ((_is.track + 1) >= staves * VOICES)
                  return 0;
            ++(_is.track);
            re = searchNote(element->tick(), _is.track);
            }
      else if (element->type() == NOTE) {
            // find segment
            Chord* chord     = static_cast<Note*>(element)->chord();
            Segment* segment = chord->segment();

            // collect all notes for this segment in noteList:
            NoteList rnl;
            iNote inote = rnl.end();
            int tracks = staves * VOICES;
            for (int track = 0; track < tracks; ++track) {
                  Element* el = segment->element(track);
                  if (!el || el->type() != CHORD)
                        continue;
                  NoteList* nl = static_cast<Chord*>(el)->noteList();
                  for (riNote in   = nl->rbegin(); in != nl->rend(); ++in) {
                        Note* note = in->second;
                        iNote ii = rnl.add(note);
                        if (note == element) {
                              inote = ii;
                              }
                        }
                  }
            if (inote != rnl.end()) {
                  if (inote != rnl.begin()) {
                        inote--;
                        re = inote->second;
                        }
                  }
            }

      if (re == 0)
            return 0;
      if (re->type() == CHORD)
            re = static_cast<Chord*>(re)->noteList()->back();
      return (Note*)re;
      }

//---------------------------------------------------------
//   downAltCtrl
//    niedrigste Note in Chord selektieren
//---------------------------------------------------------

Note* Score::downAltCtrl(Note* note) const
      {
      Chord* chord = note->chord();
      NoteList* nl = chord->noteList();
      return nl->begin()->second;
      }

//---------------------------------------------------------
//   upStaff
//---------------------------------------------------------

ChordRest* Score::upStaff(ChordRest* cr)
      {
      Segment* segment = cr->segment();

      if (cr->staffIdx() == 0)
            return cr;

      for (int track = (cr->staffIdx() - 1) * VOICES; track >= 0; --track) {
            Element* el = segment->element(track);
            if (!el)
                  continue;
            if (el->type() == NOTE)
                  el = static_cast<Note*>(el)->chord();
            if (el->isChordRest())
                  return static_cast<ChordRest*>(el);
            }
      return 0;
      }

//---------------------------------------------------------
//   downStaff
//---------------------------------------------------------

ChordRest* Score::downStaff(ChordRest* cr)
      {
      Segment* segment = cr->segment();
      int tracks = nstaves() * VOICES;

      if (cr->staffIdx() == nstaves() - 1)
            return cr;

      for (int track = (cr->staffIdx() + 1) * VOICES; track < tracks; --track) {
            Element* el = segment->element(track);
            if (!el)
                  continue;
            if (el->type() == NOTE)
                  el = static_cast<Note*>(el)->chord();
            if (el->isChordRest())
                  return static_cast<ChordRest*>(el);
            }
      return 0;
      }

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

ChordRest* Score::nextMeasure(ChordRest* element, bool selectBehavior)
      {
      if (!element)
            return 0;

      MeasureBase* mb = element->measure()->next();
      while (mb && ((mb->type() != MEASURE) || (mb->type() == MEASURE && static_cast<Measure*>(mb)->multiMeasure() < 0)))
            mb = mb->next();

      Measure* measure = static_cast<Measure*>(mb);

      int endTick = element->measure()->last()->nextChordRest(element->track(), true)->tick();
      bool last = false;

      bool range = sel->state() == SEL_STAFF;
      if (range) {
            if (element->tick() != endTick && sel->endSegment()->tick() <= endTick) {
                  measure = element->measure();
                  last = true;
                  }
            else if (element->tick() == endTick && sel->isEndActive())
                  last = true;
            }
      else if (element->tick() != endTick && selectBehavior) {
            measure = element->measure();
            last = true;
            }
      if (!measure) {
            measure = element->measure();
            last = true;
            }
      int staff = element->staffIdx();

      Segment* startSeg = last ? measure->last() : measure->first();
      for (Segment* seg = startSeg; seg; seg = last ? seg->prev() : seg->next()) {
            int etrack = (staff+1) * VOICES;
            for (int track = staff * VOICES; track < etrack; ++track) {
                  Element* pel = seg->element(track);

                  if (pel && pel->isChordRest())
                        return static_cast<ChordRest*>(pel);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

ChordRest* Score::prevMeasure(ChordRest* element)
      {
      if (!element)
            return 0;

      MeasureBase* mb = element->measure()->prev();
      while (mb && ((mb->type() != MEASURE) || (mb->type() == MEASURE && static_cast<Measure*>(mb)->multiMeasure() < 0)))
            mb = mb->prev();

      Measure* measure = static_cast<Measure*>(mb);

      int startTick = element->measure()->first()->nextChordRest(element->track())->tick();
      bool last = false;

      bool range = sel->state() == SEL_STAFF;
      if (range && sel->isEndActive() && sel->startSegment()->tick() <= startTick)
            last = true;
      else if (element->tick() != startTick) {
            measure = element->measure();
            }
      if (!measure) {
            measure = element->measure();
            last = false;
            }

      int staff = element->staffIdx();

      Segment* startSeg = last ? measure->last() : measure->first();
      for (Segment* seg = startSeg; seg; seg = last ? seg->prev() : seg->next()) {
            int etrack = (staff+1) * VOICES;
            for (int track = staff * VOICES; track < etrack; ++track) {
                  Element* pel = seg->element(track);

                  if (pel && pel->isChordRest())
                        return static_cast<ChordRest*>(pel);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   adjustCanvasPosition
//---------------------------------------------------------

void Score::adjustCanvasPosition(Element* el, bool playBack)
      {
      Measure* m;
      if (el->type() == NOTE)
            m = static_cast<Note*>(el)->chord()->segment()->measure();
      else if (el->type() == REST)
            m = static_cast<Rest*>(el)->segment()->measure();
      else if (el->type() == CHORD)
            m = static_cast<Chord*>(el)->segment()->measure();
      else if (el->type() == SEGMENT)
            m = static_cast<Segment*>(el)->measure();
      else if (el->type() == LYRICS)
            m = static_cast<Lyrics*>(el)->measure();
      else if (el->type() == HARMONY)
            m = static_cast<Harmony*>(el)->measure();
      else if (el->type() == MEASURE)
            m = static_cast<Measure*>(el);
      else
            return;

      System* sys = m->system();

      QPointF p(el->canvasPos());
      QRectF r(canvas()->vGeometry());
      QRectF mRect(m->abbox());
      QRectF sysRect(sys->abbox());

      const qreal BORDER_X = _spatium * 3;
      const qreal BORDER_Y = _spatium * 3;

      // only try to track measure if not during playback
      if (!playBack)
            sysRect = mRect;
      qreal top = sysRect.top() - BORDER_Y;
      qreal bottom = sysRect.bottom() + BORDER_Y;
      qreal left = mRect.left() - BORDER_X;
      qreal right = mRect.right() + BORDER_X;

      QRectF showRect(left, top, right - left, bottom - top);

      // canvas is not as wide as measure, track note instead
      if (r.width() < showRect.width()) {
            showRect.setX(p.x());
            showRect.setWidth(el->width());
            }

      // canvas is not as tall as system
      if (r.height() < showRect.height()) {
            if (sys->staves()->size() == 1 || !playBack) {
                  // track note if single staff
                  showRect.setY(p.y());
                  showRect.setHeight(el->height());
                  }
            else {
                  // let user control height
//                   showRect.setY(r.y());
//                   showRect.setHeight(1);
                  }
            }

      if (r.contains(showRect))
            return;

//       qDebug() << "showRect" << showRect << "\tcanvas" << r;

      qreal mag = canvas()->xMag() * DPI / PDPI;
      qreal x   = - canvas()->xoffset() / mag;
      qreal y   = - canvas()->yoffset() / mag;

      qreal oldX = x, oldY = y;

      if (showRect.left() < r.left()) {
//             qDebug() << "left < r.left";
            x = showRect.left() - BORDER_X;
            }
      else if (showRect.left() > r.right()) {
//             qDebug() << "left > r.right";
            x = showRect.right() - canvas()->width() / mag + BORDER_X;
            }
      else if (r.width() >= showRect.width() && showRect.right() > r.right()) {
//             qDebug() << "r.width >= width && right > r.right";
            x = showRect.left() - BORDER_X;
            }
      if (showRect.top() < r.top() && showRect.bottom() < r.bottom()) {
//             qDebug() << "top < r.top";
            y = showRect.top() - BORDER_Y;
            }
      else if (showRect.top() > r.bottom()) {
//             qDebug() << "top > r.bottom";
            y = showRect.bottom() - canvas()->height() / mag + BORDER_Y;
            }
      else if (r.height() >= showRect.height() && showRect.bottom() > r.bottom()) {
//             qDebug() << "r.height >= height && bottom > r.bottom";
            y = showRect.top() - BORDER_Y;
            }

      // align to page borders if extends beyond
      Page* page = sys->page();
      if (x < page->x() || r.width() >= page->width())
            x = page->x();
      else if (r.width() < page->width() && r.width() + x > page->width() + page->x())
            x = (page->width() + page->x()) - r.width();
      if (y < page->y() || r.height() >= page->height())
            y = page->y();
      else if (r.height() < page->height() && r.height() + y > page->height())
            y = (page->height() + page->y()) - r.height();

      // hack: don't update if we haven't changed the offset
      if (oldX == x && oldY == y)
            return;
      canvas()->setOffset(-x * mag, -y * mag);
      canvas()->updateNavigator(false);
      updateAll = true;
      }

//---------------------------------------------------------
//   pageNext
//---------------------------------------------------------

void Score::pageNext()
      {
      if (pages().empty())
            return;
      Page* page = pages().back();
      qreal mag  = canvas()->xMag() * DPI / PDPI;
      qreal x    = canvas()->xoffset() - (page->width() + 25.0) * mag;
      qreal lx   = 10.0 - page->canvasPos().x() * mag;
      if (x < lx)
            x = lx;
      canvas()->setOffset(x, 10.0);
      canvas()->updateNavigator(false);
      updateAll = true;
      }

//---------------------------------------------------------
//   pagePrev
//---------------------------------------------------------

void Score::pagePrev()
      {
      if (pages().empty())
            return;
      Page* page = pages().back();
      qreal mag  = canvas()->xMag() * DPI / PDPI;
      qreal x = canvas()->xoffset() +( page->width() + 25.0) * mag;
      if (x > 10.0)
            x = 10.0;
      canvas()->setOffset(x, 10.0);
      canvas()->updateNavigator(false);
      updateAll = true;
      }

//---------------------------------------------------------
//   pageTop
//---------------------------------------------------------

void Score::pageTop()
      {
      canvas()->setOffset(10.0, 10.0);
      canvas()->updateNavigator(false);
      updateAll = true;
      }

//---------------------------------------------------------
//   pageEnd
//---------------------------------------------------------

void Score::pageEnd()
      {
      if (pages().empty())
            return;
      Page* lastPage = pages().back();
      QPointF p(lastPage->canvasPos());
      qreal mag  = canvas()->xMag() * DPI / PDPI;
      canvas()->setOffset(25.0 - p.x() * mag, 25.0);
      canvas()->updateNavigator(false);
      updateAll = true;
      }

