//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: segment.cpp,v 1.28 2006/04/12 14:58:10 wschweer Exp $
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

#include "globals.h"
#include "segment.h"
#include "element.h"
#include "chord.h"
#include "score.h"
#include "beam.h"
#include "tuplet.h"
#include "text.h"
#include "measure.h"
#include "barline.h"
#include "part.h"
#include "lyrics.h"

const char* Segment::segmentTypeNames[] = {
   "Clef", "Key Signature", "Time Signature", "Begin Repeat", "ChordRest",
   "Breath", "Bar Line", "Time Sig Precaution"
   };

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void Segment::setElement(int track, Element* el)
      {
      if (el)
            el->setParent(this);
      _elist[track] = el;
      }

//---------------------------------------------------------
//   setLyrics
//---------------------------------------------------------

void Segment::setLyrics(int staff, Lyrics* l)
      {
      l->setParent(this);
      int idx = l->no();
      int n = _lyrics[staff].size();
      for (int i = n; i <= idx; ++i)
            _lyrics[staff].push_back(0);
      _lyrics[staff][idx] = l;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Segment::removeElement(int track)
      {
      Element* el = element(track);
      if (el->isChordRest()) {
            ChordRest* cr = (ChordRest*)el;
            Beam* beam = cr->beam();
            if (beam)
                  beam->remove(cr);
            Tuplet* tuplet = cr->tuplet();
            if (tuplet)
                  tuplet->remove(cr);
            }
      }

//---------------------------------------------------------
//   Segment
//---------------------------------------------------------

Segment::Segment(Measure* m)
   : Element(m->score())
      {
      setParent(m);
      init();
      }

Segment::Segment(Measure* m, int t)
   : Element(m->score())
      {
      setParent(m);
      setTick(t);
      init();
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Segment::init()
      {
      int staves = score()->nstaves();
      int tracks = staves * VOICES;
      for (int i = 0; i < tracks; ++i)
            _elist.push_back(0);
      for (int i = 0; i < staves; ++i)
            _lyrics.push_back(LyricsList());
      }

//---------------------------------------------------------
//   next1
//---------------------------------------------------------

/**
 return next \a Segment, dont stop searching at end
 of \a Measure
*/

Segment* Segment::next1() const
      {
      if (next())
            return next();
      Measure* m = measure()->next();
      if (m)
            return m->first();
      return 0;
      }

//---------------------------------------------------------
//   prev1
//---------------------------------------------------------

/**
 return previous \a Segment, dont stop searching at
 \a Measure begin
*/

Segment* Segment::prev1() const
      {
      if (prev())
            return prev();
      Measure* m = measure()->prev();
      if (m)
            return m->last();
      return 0;
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Segment::insertStaff(int staff)
      {
      int track = staff * VOICES;
      for (int voice = 0; voice < VOICES; ++voice)
            _elist.insert(track+voice, 0);
      _lyrics.insert(staff, LyricsList());
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Segment::removeStaff(int staff)
      {
      int track = staff * VOICES;
      _elist.erase(_elist.begin() + track, _elist.begin() + track + VOICES);
      _lyrics.removeAt(staff);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Segment::add(Element* el)
      {
      el->setParent(this);
      el->setTick(tick());    //DEBUG
      int staffIdx = el->staffIdx();

      if (staffIdx == -1)
            printf("Segment(%p)(t:%d)(%d)::add %s tracks %d, staff %d, track = %d\n",
               this, subtype(), el->tick(), el->name(), _elist.size(), staffIdx,
               staffIdx * VOICES + el->voice());

      switch(el->type()) {
            case LYRICS:
                  {
                  Lyrics* l = (Lyrics*) el;
                  LyricsList* ll = &_lyrics[staffIdx];
                  int size = ll->size();
                  if (l->no() >= size) {
                        for (int i = size-1; i < l->no(); ++i)
                              ll->append(0);
                        }
                  (*ll)[l->no()] = l;
                  }
                  break;

            case BAR_LINE:
                  if (_elist[staffIdx * VOICES] + el->voice())
                        printf("%06d: segment slot already used\n", el->tick());
                  if (el->subtype() == END_REPEAT)
                        measure()->setEndRepeat(2);
                  else if (el->subtype() == START_REPEAT)
                        measure()->setStartRepeat(true);

                  // fall through

            case CHORD:
            case REST:
            case BREATH:
            default:
                  _elist[staffIdx * VOICES + el->voice()] = el;
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Segment::remove(Element* el)
      {
// printf("Segment::remove %s\n", el->name());
      int staffIdx = el->staffIdx();
      if (el->type() == LYRICS) {
            LyricsList& ll = _lyrics[staffIdx];
            for (int i = 0; i < ll.size(); ++i) {
                  if (ll[i] == el) {
                        ll[i] = 0;
                        //
                        // remove entry if last or rest of list
                        // is empty
                        //
                        int n = 1;
                        ++i;
                        for (; i < ll.size(); ++i) {
                              if (ll[i])
                                    return;
                              ++n;
                              }
                        for (int i = 0; i < n; ++i)
                              ll.removeLast();
                        return;
                        }
                  }
            printf("Measure::remove: %s %p not found\n", el->name(), el);
            return;
            }

      if (el->type() == BAR_LINE) {
            if (el->subtype() == START_REPEAT)
                  measure()->setStartRepeat(false);
            // reset endRepeat , bug fix for remove/change End Barline. by DK. 02.09.07
      if (el->subtype() == END_REPEAT)
                  measure()->setEndRepeat(0);
            }
      _elist[staffIdx * VOICES + el->voice()] = 0;

      if (el->isChordRest()) {
            ChordRest* cr = (ChordRest*)el;
            if (cr->tuplet())
                  cr->tuplet()->remove(cr);
            }
      }

//---------------------------------------------------------
//   segmentType
//    returns segment type suitable for storage of Element
//---------------------------------------------------------

Segment::SegmentType Segment::segmentType(int type)
      {
      switch (type) {
            case CHORD:
            case REST:
            case LYRICS:
                  return Segment::SegChordRest;
            case CLEF:
                  return Segment::SegClef;
            case KEYSIG:
                  return Segment::SegKeySig;
            case TIMESIG:
                  return Segment::SegTimeSig;
            case BAR_LINE:
                  return Segment::SegBarLine;
            default:
                  printf("Segment:segmentType()  bad type!\n");
                  return (Segment::SegmentType)-1;
            }
      }

//---------------------------------------------------------
//   setTime
//---------------------------------------------------------

void Segment::setTime(int stick)
      {
      setTick(stick);
      foreach(Element* e, _elist) {
            if (e)
                  e->setTick(stick);
            }
      foreach(LyricsList ll, _lyrics) {
            foreach(Lyrics* l, ll) {
                  if (l)
                        l->setTick(stick);
                  }
            }
      }

//---------------------------------------------------------
//   removeGeneratedElements
//---------------------------------------------------------

void Segment::removeGeneratedElements()
      {
      for (int i = 0; i < _elist.size(); ++i) {
            if (_elist[i] && _elist[i]->generated()) {
                  _elist[i] = 0;
                  }
            }
      }

//---------------------------------------------------------
//   isEmpty
//---------------------------------------------------------

bool Segment::isEmpty() const
      {
      foreach(const Element* e, _elist) {
            if (e)
                  return false;
            }
      // TODO: check for lyrics?
      return true;
      }
