//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: navigate.cpp,v 1.34 2006/03/02 17:08:37 wschweer Exp $
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

#include "element.h"
#include "canvas.h"
#include "clef.h"
#include "score.h"
#include "note.h"
#include "rest.h"
#include "chord.h"
#include "system.h"
#include "segment.h"
#include "utils.h"
#include "input.h"
#include "measure.h"
#include "page.h"
#include "layout.h"

//---------------------------------------------------------
//   upAlt
//    select next higher pitched note in chord
//---------------------------------------------------------

Note* Score::upAlt(Element* element)
      {
      Element* re = 0;
      if (element->type() == REST) {
            if (cis->staff <= 0)
                  return 0;
            --(cis->staff);
            re = searchNote(((Rest*)element)->tick(), cis->staff);
            }
      else {
            // find segment
            Chord* chord     = ((Note*) element)->chord();
            Segment* segment = chord->segment();
#if 0 //TODOA
            // collect all notes for this segment in noteList:
            NoteList rnl;
            iNote inote = rnl.end();
            int tracks = nstaves() * VOICES;
            for (int track = 0; track < tracks; ++track) {
                  Element* el = segment->element(track);
                  if (!el || el->type() != CHORD)
                        continue;
                  NoteList* nl = ((Chord*)el)->noteList();
                  int n = nl->size() - 1;
                  for (int i = n; n >= 0; --n) {
                        Note* note = nl->at(i);
                        rnl.add(note);
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
#endif
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
      return chord->upNote();
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
            if ((cis->staff+1) >= staves)
                  return 0;
            ++(cis->staff);
            re = searchNote(((Rest*)element)->tick(), cis->staff);
            }
      else {
            // find segment
            Chord* chord     = ((Note*) element)->chord();
            Segment* segment = chord->segment();
#if 0 //TODOA
            // collect all notes for this segment in noteList:
            NoteList rnl;
            iNote inote = rnl.end();
            int tracks = staves * VOICES;
            for (int track = 0; track < tracks; ++track) {
                  Element* el = segment->element(track);
                  if (!el || el->type() != CHORD)
                        continue;
                  NoteList* nl = ((Chord*)el)->noteList();
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
#endif
            }

      if (re == 0)
            return 0;
      if (re->type() == CHORD)
            re = ((Chord*)re)->noteList()->back();
      return (Note*)re;
      }

//---------------------------------------------------------
//   downAltCtrl
//    select lowest note in chord
//---------------------------------------------------------

Note* Score::downAltCtrl(Note* note) const
      {
      Chord* chord = note->chord();
      return chord->downNote();
      }

//---------------------------------------------------------
//   nextChordRest
//    return next NOTE or REST
//    element - current element
//---------------------------------------------------------

ChordRest* Score::nextChordRest(ChordRest* cr)
      {
      if (!cr)
            return 0;
      Segment* seg = cr->segment();
      int track = cr->staffIdx() * VOICES + cr->voice();

      for (;;) {
            seg = seg->next1();
            if (!seg)
                  break;
            Element* e = seg->element(track);
            if (e && e->isChordRest())
                  return (ChordRest*) e;
            }
      return 0;
      }

//---------------------------------------------------------
//   prevChordRest
//    return previous Chord or Rest
//---------------------------------------------------------

ChordRest* Score::prevChordRest(ChordRest* cr)
      {
      if (!cr)
            return 0;
      Segment* seg = cr->segment();
      int track = cr->staffIdx() * VOICES + cr->voice();

      for (;;) {
            seg = seg->prev1();
            if (!seg)
                  break;
            Element* e = seg->element(track);
            if (e && e->isChordRest())
                  return (ChordRest*) e;
            }
      return 0;
      }

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

ChordRest* Score::nextMeasure(ChordRest* element)
      {
      if (!element)
            return 0;

      Measure* measure = element->measure()->next();
      if (!measure)
            return 0;
      int staff = element->staffIdx();

      for (Segment* seg = measure->first(); seg; seg = seg->next()) {
            int etrack = (staff+1) * VOICES;
            for (int track = staff * VOICES; track < etrack; ++track) {
                  Element* pel = seg->element(track);

                  if (pel && pel->isChordRest())
                        return (ChordRest*)pel;
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

      Measure* measure = element->measure()->prev();
      if (!measure)
            return 0;
      int staff = element->staffIdx();

      for (Segment* seg = measure->first(); seg; seg = seg->next()) {
            int etrack = (staff+1) * VOICES;
            for (int track = staff * VOICES; track < etrack; ++track) {
                  Element* pel = seg->element(track);

                  if (pel && pel->isChordRest())
                        return (ChordRest*)pel;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   adjustCanvasPosition
//---------------------------------------------------------

void Score::adjustCanvasPosition(Element* el)
      {
      QPointF p(el->canvasPos());
      QRectF r(canvas()->vGeometry());
      if (r.contains(p))
            return;
      Measure* m;
      if (el->type() == NOTE)
            m = ((Note*)el)->chord()->segment()->measure();
      else if (el->type() == REST)
            m = ((Rest*)el)->segment()->measure();
      else if (el->type() == SEGMENT)
            m = ((Segment*)el)->measure();
      else
            return;

      QPointF pos(m->canvasPos());
      qreal mag = canvas()->xMag();
      qreal x   = canvas()->xoffset() / mag;
      qreal y   = canvas()->yoffset() / mag;

      //TODO handle special cases:
      //    - canvas smaller than measure

      const qreal BORDER = _spatium * 3; // 20.0 / mag;
      if (p.x() < r.x()) {
            // move right
            // move left edge of measure to left edge of canvas
            x = BORDER - pos.x();
            }
      else if (p.x() > (r.x() + r.width())) {
            // move left
            // move right edge of measure to right edge of canvas
            x = - (pos.x() + m->width()) + canvas()->width() / mag - BORDER;
            }
      if (p.y() < r.y())
            y = BORDER - pos.y();
      else if (p.y() > (r.y() + r.height()))
            y = - (pos.y() + m->height()) + canvas()->height() / mag - BORDER;

      canvas()->setOffset(x * mag, y * mag);
      canvas()->updateNavigator(false);
      updateAll = true;
      }

//---------------------------------------------------------
//   pageNext
//---------------------------------------------------------

void Score::pageNext()
      {
      if (_layout->pages()->empty())
            return;
      Page* page = _layout->pages()->back();
      qreal mag  = canvas()->xMag();
      qreal x    = canvas()->xoffset() - page->width() * mag;
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
      if (_layout->pages()->empty())
            return;
      Page* page = _layout->pages()->back();
      qreal x = canvas()->xoffset() + page->width() * canvas()->xMag();
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
      if (_layout->pages()->empty())
            return;
      Page* lastPage = _layout->pages()->back();
      QPointF p(lastPage->canvasPos());
      qreal mag = canvas()->xMag();
      canvas()->setOffset(10.0 - p.x() * mag, 10.0);
      canvas()->updateNavigator(false);
      updateAll = true;
      }

