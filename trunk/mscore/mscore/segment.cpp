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
#include "spanner.h"
#include "line.h"
#include "hairpin.h"
#include "ottava.h"
#include "al/sig.h"
#include "staffstate.h"
#include "instrchange.h"

//---------------------------------------------------------
//   subTypeName
//---------------------------------------------------------

const char* Segment::subTypeName() const
      {
      static char buffer[32];

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
      return buffer;
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

Segment::Segment(Measure* m, SegmentType st, int t)
   : Element(m->score())
      {
      setParent(m);
      setSubtype(st);
      setTick(t);
      init();
      empty = true;
      }

Segment::~Segment()
      {
      foreach(Element* e, _elist)
            delete e;
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
//   prev
//    got to previous segment which has subtype in types
//---------------------------------------------------------

Segment* Segment::prev(SegmentTypes types) const
      {
      for (Segment* s = prev(); s; s = s->prev()) {
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

Segment* Segment::prev1(SegmentTypes types) const
      {
      for (Segment* s = prev1(); s; s = s->prev1()) {
            if (s->subtype() & types)
                  return s;
            }
      return 0;
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
      fixStaffIdx();
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Segment::removeStaff(int staff)
      {
      int track = staff * VOICES;
      _elist.erase(_elist.begin() + track, _elist.begin() + track + VOICES);

/*      foreach(Spanner* sp, _spannerFor) {
            }
      foreach(Spanner* sp, _spannerBack) {
            }
      */
      foreach(Element* e, _annotations) {
            int staffIdx = e->staffIdx();
            if (staffIdx > staff)
                  e->setTrack(e->track() - VOICES);
            }

      fixStaffIdx();
      }

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

void Segment::addSpanner(Spanner* l)
      {
      Element* e = l->endElement();
      if (e)
            static_cast<Segment*>(e)->addSpannerBack(l);
      _spannerFor.append(l);
      }

//---------------------------------------------------------
//   removeSpanner
//---------------------------------------------------------

void Segment::removeSpanner(Spanner* l)
      {
      static_cast<Segment*>(l->endElement())->removeSpannerBack(l);
      if (!_spannerFor.removeOne(l))
            printf("%p cannot remove spanner %p, size %d\n", this, l, _spannerFor.size());
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Segment::add(Element* el)
      {
// printf("%p segment add(%d, %d, %s %p)\n", this, tick(), el->track(), el->name(), el);
      el->setParent(this);

      int track = el->track();
      if (track == -1) {
            printf("element <%s> has invalid track -1\n", el->name());
            abort();
            }
      int staffIdx = track / VOICES;

      switch(el->type()) {
            case REPEAT_MEASURE:
                  measure()->setRepeatFlags(measure()->repeatFlags() | RepeatMeasureFlag);
                  _elist[track] = el;
                  empty = false;
                  break;

            case HAIRPIN:
                  addSpanner(static_cast<Spanner*>(el));
                  score()->updateHairpin(static_cast<Hairpin*>(el));
                  score()->setPlaylistDirty(true);
                  break;

            case OTTAVA:
                  {
                  addSpanner(static_cast<Spanner*>(el));
                  Ottava* o   = static_cast<Ottava*>(el);
                  Segment* es = static_cast<Segment*>(o->endElement());
                  if (es) {
                        int tick2   = es->tick();
                        int shift   = o->pitchShift();
                        Staff* st = o->staff();
                        st->pitchOffsets().setPitchOffset(tick(), shift);
                        st->pitchOffsets().setPitchOffset(tick2, 0);
                        }
                  score()->setPlaylistDirty(true);
                  }
                  break;

            case VOLTA:
            case TRILL:
            case PEDAL:
            case TEXTLINE:
                  addSpanner(static_cast<Spanner*>(el));
                  break;

            case DYNAMIC:
            case HARMONY:
            case SYMBOL:
            case FRET_DIAGRAM:
            case TEMPO_TEXT:
            case STAFF_TEXT:
            case MARKER:
            case JUMP:
            case IMAGE:
            case TEXT:
                  _annotations.append(el);
                  break;
            case STAFF_STATE:
                  if (el->subtype() == STAFF_STATE_INSTRUMENT) {
                        StaffState* ss = static_cast<StaffState*>(el);
                        Part* part = el->staff()->part();
                        part->setInstrument(ss->instrument(), tick());
                        }
                  _annotations.append(el);
                  break;

            case INSTRUMENT_CHANGE: {
                  InstrumentChange* is = static_cast<InstrumentChange*>(el);
                  Part* part = is->staff()->part();
                  part->setInstrument(is->instrument(), tick());
                  _annotations.append(el);
                  break;
                  }

            case CHORD:
            case REST:
                  {
                  ChordRest* cr = static_cast<ChordRest*>(el);
                  if (cr->tuplet()) {
                        cr->tuplet()->add(cr);
                        cr->tuplet()->setTrack(track);      // debug
                        }
                  }
                  if (_elist[track]) {
                        printf("Segment::add(%s) there is already an %s at %d track %d\n",
                           el->name(), _elist[track]->name(), tick(), track);
                        }
                  if (track % VOICES)
                        measure()->mstaff(staffIdx)->hasVoices = true;

            default:
                  _elist[track] = el;
                  empty = false;
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Segment::remove(Element* el)
      {
// printf("%p Segment::remove %s %p\n", this, el->name(), el);

      int track = el->track();

      switch(el->type()) {
            case CHORD:
            case REST:
                  {
                  ChordRest* cr = (ChordRest*)el;
                  if (cr->tuplet())
                        cr->tuplet()->remove(cr);
                  _elist[track] = 0;
                  int staffIdx = cr->staffIdx();
                  measure()->checkMultiVoices(staffIdx);
                  }
                  break;

            case REPEAT_MEASURE:
                  measure()->setRepeatFlags(measure()->repeatFlags() & ~RepeatMeasureFlag);
                  _elist[track] = 0;
                  break;

            case OTTAVA:
                  {
                  Ottava* o   = static_cast<Ottava*>(el);
                  Segment* es = static_cast<Segment*>(o->endElement());
                  int tick2   = es->tick();
                  Staff* st   = o->staff();
                  st->pitchOffsets().remove(tick());
                  st->pitchOffsets().remove(tick2);
                  removeSpanner(static_cast<Spanner*>(el));
                  score()->setPlaylistDirty(true);
                  }
                  break;

            case HAIRPIN:
                  score()->removeHairpin(static_cast<Hairpin*>(el));
                  removeSpanner(static_cast<Spanner*>(el));
                  score()->setPlaylistDirty(true);
                  break;

            case VOLTA:
            case TRILL:
            case PEDAL:
            case TEXTLINE:
                  removeSpanner(static_cast<Spanner*>(el));
                  break;

            case DYNAMIC:
            case HARMONY:
            case SYMBOL:
            case FRET_DIAGRAM:
            case TEMPO_TEXT:
            case STAFF_TEXT:
            case MARKER:
            case JUMP:
            case IMAGE:
            case TEXT:
                  _annotations.removeOne(el);
                  break;
            case STAFF_STATE:
                  if (el->subtype() == STAFF_STATE_INSTRUMENT) {
                        Part* part = el->staff()->part();
                        part->removeInstrument(tick());
                        }
                  _annotations.removeOne(el);
                  break;

            case INSTRUMENT_CHANGE:
                  {
                  InstrumentChange* is = static_cast<InstrumentChange*>(el);
                  Part* part = is->staff()->part();
                  part->removeInstrument(tick());
                  }
                  _annotations.removeOne(el);
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

SegmentType Segment::segmentType(ElementType type)
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

      for (int i = 0; i < dst.size(); ++i) {
            int startTrack = dst[i] * VOICES;
            int endTrack   = startTrack + VOICES;
            for (int k = startTrack; k < endTrack; ++k)
                  dl.append(_elist[k]);
            }
      _elist = dl;
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
//   tick
//---------------------------------------------------------

int Segment::tick() const
      {
      return _tick + measure()->tick();
      }

//---------------------------------------------------------
//   setTick
//---------------------------------------------------------

void Segment::setTick(int t)
      {
      _tick = t - measure()->tick();
      }

//---------------------------------------------------------
//   segLyricsList
//---------------------------------------------------------

const QList<Lyrics*>* Segment::lyricsList(int staffIdx) const
      {
      if (!(subtype() & (SegChordRest | SegGrace)))
            return 0;
      int strack = staffIdx * VOICES;
      int etrack = strack + VOICES;
      for (int track = strack; track < etrack; ++track) {
            ChordRest* cr = static_cast<ChordRest*>(element(track));
            if (cr && !cr->lyricsList().isEmpty())
                  return &cr->lyricsList();
            }
      return 0;
      }


