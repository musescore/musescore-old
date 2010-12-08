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

#include "globals.h"
#include "segment.h"
#include "element.h"
#include "chord.h"
#include "note.h"
#include "score.h"
#include "beam.h"
#include "tuplet.h"
#include "text.h"
#include "measure.h"
#include "barline.h"
#include "part.h"
#include "lyrics.h"
#include "repeat.h"
#include "staff.h"

//---------------------------------------------------------
//   subTypeName
//---------------------------------------------------------

const char* Segment::subTypeName() const
      {
      switch(subtype()) {
            case SegClef:                 return "Clef";
            case SegKeySig:               return "Key Signature";
            case SegTimeSig:              return "Time Signature";
            case SegStartRepeatBarLine:   return "Begin Repeat";
            case SegBarLine:              return "BarLine";
            case SegGrace:                return "Grace";
            case SegChordRest:            return "ChordRest";
            case SegBreath:               return "Breath";
            case SegEndBarLine:           return "EndBarLine";
            case SegTimeSigAnnounce:      return "Time Sig Precaution";
            case SegKeySigAnnounce:       return "Key Sig Precaution";
            }
      return "??";
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void Segment::setElement(int track, Element* el)
      {
      if (el) {
            el->setParent(this);
            _elist[track] = el;
            empty = false;
            }
      else {
            _elist[track] = 0;
            checkEmpty();
            }
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
      empty = true;
      }

Segment::Segment(Measure* m, int t)
   : Element(m->score())
      {
      setParent(m);
      setTick(t);
      init();
      empty = true;
      }

Segment::~Segment()
      {
      foreach(Element* e, _elist)
            delete e;
      foreach(LyricsList ll, _lyrics) {
            foreach(Lyrics* l, ll) {
                  delete l;
                  }
            }
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
      _prev = 0;
      _next = 0;
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
      MeasureBase* m = measure();
      for (;;) {
            m = m->next();
            if (m == 0)
                  return 0;
            if (m->type() == MEASURE)
                  return static_cast<Measure*>(m)->first();
            }
      }


Segment* Segment::next1(SegmentTypes types) const
      {
      for (Segment* s = next1(); s; s = s->next1()) {
            if (s->subtype() & types)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   next
//    got to next segment which has subtype in types
//---------------------------------------------------------

Segment* Segment::next(SegmentTypes types) const
      {
      for (Segment* s = next(); s; s = s->next()) {
            if (s->subtype() & types)
                  return s;
            }
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
      MeasureBase* m = measure();
      for (;;) {
            m = m->prev();
            if (m == 0)
                  return 0;
            if (m->type() == MEASURE)
                  return static_cast<Measure*>(m)->last();
            }
      }

//---------------------------------------------------------
//   nextCR
//    get next ChordRest Segment
//---------------------------------------------------------

Segment* Segment::nextCR(int track) const
      {
      Segment* seg = next1();
      for (; seg; seg = seg->next1()) {
            if (seg->subtype() == SegChordRest) {
                  if (track != -1 && !seg->element(track))
                        continue;
                  return seg;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   nextChordRest
//    get the next ChordRest, start at this segment
//---------------------------------------------------------

ChordRest* Segment::nextChordRest(int track, bool backwards) const
      {
      for (const Segment* seg = this; seg; seg = backwards ? seg->prev1() : seg->next1()) {
            Element* el = seg->element(track);
            if (el && el->isChordRest())
                  return static_cast<ChordRest*>(el);
            }
      return 0;
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Segment::insertStaff(int staff)
      {
      int track = staff * VOICES;
      for (int voice = 0; voice < VOICES; ++voice)
            _elist.insert(track, 0);
      _lyrics.insert(staff, LyricsList());
      fixStaffIdx();
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Segment::removeStaff(int staff)
      {
      int track = staff * VOICES;
      _elist.erase(_elist.begin() + track, _elist.begin() + track + VOICES);
      _lyrics.removeAt(staff);
      fixStaffIdx();
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Segment::add(Element* el)
      {
// printf("segment add(%d, %d, %s)\n", tick(), el->track(), el->name());
      el->setParent(this);

      el->setTick(tick());    //DEBUG
      int track = el->track();
      if (track == -1) {
            printf("element <%s> has invalid track\n", el->name());
            abort();
            }
      int staffIdx = track / VOICES;

      switch(el->type()) {
            case LYRICS:
                  {
                  Lyrics* l = static_cast<Lyrics*>(el);
                  LyricsList* ll = &_lyrics[staffIdx];
                  int size = ll->size();
                  if (l->no() >= size) {
                        for (int i = size-1; i < l->no(); ++i)
                              ll->append(0);
                        }
                  (*ll)[l->no()] = l;
                  }
                  break;

            case REPEAT_MEASURE:
                  measure()->setRepeatFlags(measure()->repeatFlags() | RepeatMeasureFlag);
                  _elist[track] = el;
                  empty = false;
                  break;

            case CHORD:
            case REST:
                  {
                  ChordRest* cr = static_cast<ChordRest*>(el);
                  if (cr->tuplet()) {
                        cr->tuplet()->add(cr);
                        cr->tuplet()->setTrack(track);      // debug
                        }
                  }
                  if (_elist[track])
                        printf("Segment::add(%s) there is already an %s at %d track %d\n",
                           el->name(), _elist[track]->name(), tick(), track);
                  if (track % VOICES)
                        measure()->mstaff(staffIdx)->hasVoices = true;

            default:
                  if(track < _elist.size()) {
                        _elist[track] = el;
                        empty = false;
                        }
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Segment::remove(Element* el)
      {
// printf("Segment::remove %s\n", el->name());

      int track = el->track();

      switch(el->type()) {
            case LYRICS:
                  {
                  int staffIdx = el->staffIdx();
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
                  }
                  printf("Measure::remove: %s %p not found\n", el->name(), el);
                  break;

            case CHORD:
            case REST:
                  {
                  ChordRest* cr = (ChordRest*)el;
                  if (cr->tuplet())
                        cr->tuplet()->remove(cr);
                  _elist[track] = 0;
                  measure()->checkMultiVoices(cr->staffIdx());
                  }
                  break;

            case REPEAT_MEASURE:
                  measure()->setRepeatFlags(measure()->repeatFlags() & ~RepeatMeasureFlag);
                  _elist[track] = 0;
                  break;

            default:
                  _elist[track] = 0;
                  break;
            }
      checkEmpty();
      }

//---------------------------------------------------------
//   segmentType
//    returns segment type suitable for storage of Element
//---------------------------------------------------------

SegmentType Segment::segmentType(int type)
      {
      switch (type) {
            case CHORD:
            case REST:
            case LYRICS:
            case REPEAT_MEASURE:
                  return SegChordRest;
            case CLEF:
                  return SegClef;
            case KEYSIG:
                  return SegKeySig;
            case TIMESIG:
                  return SegTimeSig;
            case BAR_LINE:
                  return SegStartRepeatBarLine;
            default:
                  printf("Segment:segmentType()  bad type!\n");
                  return (SegmentType)-1;
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
      foreach(const LyricsList& ll, _lyrics) {
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
      checkEmpty();
      }

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Segment::sortStaves(QList<int>& dst)
      {
      QList<Element*>   dl;
      QList<LyricsList> ll;

      for (int i = 0; i < dst.size(); ++i) {
            int startTrack = dst[i] * VOICES;
            int endTrack   = startTrack + VOICES;
            for (int k = startTrack; k < endTrack; ++k)
                  dl.append(_elist[k]);
            ll.append(_lyrics[dst[i]]);
            }
      _elist = dl;
      _lyrics = ll;
      fixStaffIdx();
      }

//---------------------------------------------------------
//   fixStaffIdx
//---------------------------------------------------------

void Segment::fixStaffIdx()
      {
      int track = 0;
      foreach(Element* e, _elist) {
            if (e)
                  e->setTrack(track);
            ++track;
            }
      for (int staffIdx = 0; staffIdx < _lyrics.size(); staffIdx++) {
            foreach(Lyrics* l, _lyrics[staffIdx]) {
                  if (l)
                        l->setTrack(staffIdx * VOICES);
                  }
            }
      }

//---------------------------------------------------------
//   checkEmpty
//---------------------------------------------------------

void Segment::checkEmpty() const
      {
      empty = true;
      foreach(const Element* e, _elist) {
            if (e) {
                  empty = false;
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   swapElements
//---------------------------------------------------------

void Segment::swapElements(int i1, int i2)
      {
      _elist.swap(i1, i2);
      if (_elist[i1])
            _elist[i1]->setTrack(i1);
      if (_elist[i2])
            _elist[i2]->setTrack(i2);
      }
