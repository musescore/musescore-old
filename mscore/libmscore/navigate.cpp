//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: navigate.cpp 3229 2010-06-27 14:55:28Z wschweer $
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
#include "measure.h"
#include "page.h"

//---------------------------------------------------------
//   nextChordRest
//    return next NOTE or REST, skip grace notes
//    element - current element
//---------------------------------------------------------

ChordRest* nextChordRest(ChordRest* cr)
      {
      if (!cr)
            return 0;
      int track = cr->track();
      for (Segment* seg = cr->segment()->next1(); seg; seg = seg->next1()) {
            if (seg->subtype() != SegChordRest)
                  continue;
            if (seg->measure()->multiMeasure() < 0)
                  continue;
            Element* e = seg->element(track);
            if (e && e->isChordRest())
                  return static_cast<ChordRest*>(e);
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
      int track = cr->track();
      for (Segment* seg = cr->segment()->prev1(); seg; seg = seg->prev1()) {
            if (seg->subtype() != SegChordRest)
                  continue;
            if (seg->measure()->multiMeasure() < 0)
                  continue;
            Element* e = seg->element(track);
            if (e && e->isChordRest())
                  return static_cast<ChordRest*>(e);
            }
      return 0;
      }

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

ChordRest* Score::nextMeasure(ChordRest* /*element*/, bool /*selectBehavior*/)
      {
/*
      if (!element)
            return 0;

      MeasureBase* mb = element->measure()->next();
      while (mb && ((mb->type() != MEASURE) || (mb->type() == MEASURE && static_cast<Measure*>(mb)->multiMeasure() < 0)))
            mb = mb->next();

      Measure* measure = static_cast<Measure*>(mb);

      int endTick = element->measure()->last()->nextChordRest(element->track(), true)->tick();
      bool last = false;

      if (selection().state() == SEL_RANGE) {
            if (element->tick() != endTick && selection().tickEnd() <= endTick) {
                  measure = element->measure();
                  last = true;
                  }
            else if (element->tick() == endTick && selection().isEndActive())
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
 */
      return 0;
      }

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

ChordRest* Score::prevMeasure(ChordRest* element)
      {
      if (!element)
            return 0;
/*
      MeasureBase* mb = element->measure()->prev();
      while (mb && ((mb->type() != MEASURE) || (mb->type() == MEASURE && static_cast<Measure*>(mb)->multiMeasure() < 0)))
            mb = mb->prev();

      Measure* measure = static_cast<Measure*>(mb);

      int startTick = element->measure()->first()->nextChordRest(element->track())->tick();
      bool last = false;

      if ((selection().state() == SEL_RANGE)
         && selection().isEndActive() && selection().startSegment()->tick() <= startTick)
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
 */
      return 0;
      }

