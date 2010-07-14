//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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
 render score into event list
*/

#include "mscore.h"
#include "score.h"
#include "volta.h"
#include "note.h"
#include "instrument.h"
#include "part.h"
#include "chord.h"
#include "style.h"
#include "ottava.h"
#include "slur.h"
#include "stafftext.h"
#include "repeat.h"
#include "articulation.h"
#include "arpeggio.h"
#include "durationtype.h"
#include "measure.h"
#include "al/tempo.h"
#include "al/sig.h"
#include "repeatlist.h"
#include "velo.h"
#include "dynamics.h"
#include "navigate.h"
#include "pedal.h"
#include "staff.h"
#include "hairpin.h"

//---------------------------------------------------------
//   updateChannel
//---------------------------------------------------------

void Score::updateChannel()
      {
      foreach(Staff* s, _staves) {
            for (int i = 0; i < VOICES; ++i)
                  s->channelList(i)->clear();
            }
      Measure* fm = firstMeasure();
      if (!fm)
            return;
#if 0 // TODO1
      for (Measure* m = fm; m; m = m->nextMeasure()) {
            foreach(const Element* e, *m->el()) {
                  if (e->type() != STAFF_TEXT)
                        continue;
                  const StaffText* st = static_cast<const StaffText*>(e);
                  for (int voice = 0; voice < VOICES; ++voice) {
                        QString an(st->channelName(voice));
                        if (an.isEmpty())
                              continue;
                        Staff* staff = _staves[st->staffIdx()];
                        int a = staff->part()->instr()->channelIdx(an);
                        if (a != -1)
                              staff->channelList(voice)->insert(st->tick(), a);
                        }
                  }
            }
#endif

      for (Segment* s = fm->first(SegChordRest | SegGrace); s; s = s->next1(SegChordRest | SegGrace)) {
            foreach(Staff* st, _staves) {
                  int strack = st->idx() * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        if (!s->element(track))
                              continue;
                        Element* e = s->element(track);
                        if (e->type() != CHORD)
                              continue;
                        Chord* c = static_cast<Chord*>(e);
                        int channel = st->channel(c->tick(), c->voice());
                        foreach (Note* note, c->notes()) {
                              if (note->hidden())
                                    continue;
                              if (note->tieBack())
                                    continue;
                              note->setSubchannel(channel);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   searchVolta
//    return volta at tick
//---------------------------------------------------------

Volta* Score::searchVolta(int /*tick*/) const
      {
#if 0 // TODO
      foreach(Element* el, *gel()) {
            if (el->type() == VOLTA) {
                  Volta* volta = (Volta*)el;
                  if (tick >= volta->tick() && tick < volta->tick2())
                        return volta;
                  }
            }
#endif
      return 0;
      }

//---------------------------------------------------------
//   playNote
//---------------------------------------------------------

static void playNote(EventMap* events, const Note* note, int channel, int pitch,
   int velo, int onTime, int offTime)
      {
      Event* ev = new Event(ME_NOTEON);
      ev->setChannel(channel);
      ev->setPitch(pitch);
      ev->setVelo(velo);
      ev->setTuning(note->tuning());
      ev->setNote(note);
      events->insertMulti(onTime, ev);

      ev = new Event(ME_NOTEON);
      ev->setChannel(channel);
      ev->setPitch(pitch);
      ev->setVelo(0);
      ev->setNote(note);
      events->insertMulti(offTime, ev);
      }

//---------------------------------------------------------
//   collectNote
//---------------------------------------------------------

static void collectNote(EventMap* events, int channel, const Note* note, int tickOffset)
      {
      if (note->hidden() || note->tieBack())       // do not play overlapping notes
            return;

      int tick    = note->chord()->tick() + tickOffset;
      int onTime  = tick + note->onTimeOffset() + note->onTimeUserOffset();
      int offTime = tick + note->playTicks() + note->offTimeOffset() + note->offTimeUserOffset() - 1;

      int pitch  = note->ppitch();
      int velo = note->velocity();
      if (note->veloType() == OFFSET_VAL) {
            velo = velo + (velo * note->veloOffset()) / 100;
            if (velo < 1)
                  velo = 1;
            else if (velo > 127)
                  velo = 127;
            }
      bool mordent = false;
      foreach(Articulation* a, *note->chord()->getArticulations()) {
            if (a->subtype() == MordentSym) {
                  mordent = true;
                  break;
                  }
            }
      if (mordent) {
            int l = (offTime - onTime) / 8;

            // TODO: downstep depends on scale
            int downstep = 2;

            playNote(events, note, channel, pitch, velo, onTime, onTime + l);
            playNote(events, note, channel, pitch-downstep, velo, onTime+l, onTime + l+l);
            playNote(events, note, channel, pitch, velo, onTime+l+l, offTime);
            }
      else
            playNote(events, note, channel, pitch, velo, onTime, offTime);
      }

//---------------------------------------------------------
//   OttavaShiftSegment
//---------------------------------------------------------

struct OttavaShiftSegment {
      int stick;
      int etick;
      int shift;
      };

//---------------------------------------------------------
//   collectMeasureEvents
//---------------------------------------------------------

static void collectMeasureEvents(EventMap* events, Measure* m, int firstStaffIdx,
   int nextStaffIdx, int tickOffset)
      {
      Instrument* instr = m->score()->part(firstStaffIdx)->instr();

      SegmentTypes st = SegGrace | SegChordRest;
      int strack = firstStaffIdx * VOICES;
      int etrack = nextStaffIdx * VOICES;
      for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
            for (int track = strack; track < etrack; ++track) {
                  Element* cr = seg->element(track);
                  if (cr && cr->type() == CHORD) {
                        Chord* chord = static_cast<Chord*>(cr);
                        foreach(const Note* note, chord->notes()) {
                              int channel = instr->channel(note->subchannel()).channel;
                              collectNote(events, channel, note, tickOffset);
                              }
                        }
                  }
            }

      //
      // collect program changes and controller
      //
#if 0 // TODO1
      for (int staffIdx = firstStaffIdx; staffIdx < nextStaffIdx; ++staffIdx) {
            foreach(const Element* e, *m->el()) {
                  if (e->type() != STAFF_TEXT || e->staffIdx() != staffIdx)
                        continue;
                  const StaffText* st = static_cast<const StaffText*>(e);
                  int tick = st->tick() + tickOffset;
                  foreach (const ChannelActions& ca, *st->channelActions()) {
                        int channel = ca.channel;
                        foreach(const QString& ma, ca.midiActionNames) {
                              NamedEventList* nel = instr->midiAction(ma, channel);
                              if (!nel)
                                    continue;
                              int n = nel->events.size();
                              for (int i = n-1; i >= 0; --i) {
                                    Event* event = new Event(*nel->events[i]);
                                    event->setOntime(tick);
                                    event->setChannel(channel);
                                    events->insertMulti(tick, event);
                                    }
                              }
                        }
                  if (st->setAeolusStops()) {
                        Staff* staff = st->staff();
                        int voice   = 0;
                        int channel = staff->channel(tick, voice);

                        for (int i = 0; i < 4; ++i) {
                              for (int k = 0; k < 16; ++k) {
                                    if (st->getAeolusStop(i, k)) {
                                          Event* event = new Event;
                                          event->setType(ME_CONTROLLER);
                                          event->setController(98);
                                          event->setValue(k);
                                          event->setOntime(tick);
                                          event->setChannel(channel);
                                          events->insertMulti(tick, event);
                                          }
                                    }
                              Event* event = new Event;
                              event->setType(ME_CONTROLLER);
                              event->setController(98);
                              event->setValue(96 + i);
                              event->setOntime(tick);
                              event->setChannel(channel);
                              events->insertMulti(tick, event);

                              event = new Event;
                              event->setType(ME_CONTROLLER);
                              event->setController(98);
                              event->setValue(64 + i);
                              event->setOntime(tick);
                              event->setChannel(channel);
                              events->insertMulti(tick, event);
                              }
                        }
                  }
            }
#endif
      }

//---------------------------------------------------------
//   searchLabel
//---------------------------------------------------------

Measure* Score::searchLabel(const QString& s, Measure* start)
      {
      if (s == "start")
            return firstMeasure();
      if (start == 0)
            start = firstMeasure();
      for (Measure* m = start; m; m = m->nextMeasure()) {
            foreach(const Element* e, *m->el()) {
                  if (e->type() == MARKER) {
                        const Marker* marker = static_cast<const Marker*>(e);
                        if (marker->label() == s)
                              return m;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   toEList
//---------------------------------------------------------

void Score::toEList(EventMap* events, int firstStaffIdx, int nextStaffIdx)
      {
      Instrument* instr = part(firstStaffIdx)->instr();

      foreach(const RepeatSegment* rs, *_repeatList) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            int tickOffset = rs->utick - rs->tick;
            for (Measure* m = tick2measure(startTick); m; m = m->nextMeasure()) {
                  collectMeasureEvents(events, m, firstStaffIdx, nextStaffIdx, tickOffset);
                  if (m->tick() + m->ticks() >= endTick)
                        break;
                  }

            int channel = instr->channel(0).channel;
            // TODO: what if instrument has more than one channel?

            foreach(Element* e, _gel) {
                  int staffIdx = e->staffIdx();
                  if (e->type() == PEDAL && staffIdx >= firstStaffIdx && staffIdx < nextStaffIdx) {
                        Pedal* p = static_cast<Pedal*>(e);
                        if (p->tick() >= startTick && p->tick() < endTick) {
                              Event* ev = new Event(ME_CONTROLLER);
                              ev->setChannel(channel);
                              ev->setController(CTRL_SUSTAIN);
                              ev->setValue(127);
                              events->insertMulti(p->tick() + tickOffset, ev);

                              ev = new Event(ME_CONTROLLER);
                              ev->setChannel(channel);
                              ev->setController(CTRL_SUSTAIN);
                              ev->setValue(0);
                              events->insertMulti(p->tick2() + tickOffset, ev);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   updateRepeatList
//---------------------------------------------------------

void Score::updateRepeatList(bool expandRepeats)
      {
      if (!expandRepeats) {
            foreach(RepeatSegment* s, *_repeatList)
                  delete s;
            _repeatList->clear();
            Measure* m = lastMeasure();
            if (m == 0)
                  return;
            RepeatSegment* s = new RepeatSegment;
            s->tick  = 0;
            s->len   = m->tick() + m->ticks();
            s->utick = 0;
            s->utime = 0.0;
            s->timeOffset = 0.0;
            _repeatList->append(s);
            }
      else
            _repeatList->unwind();
      if (debugMode)
            _repeatList->dump();
      }

//---------------------------------------------------------
//   toEList
//    export score to event list
//---------------------------------------------------------

void Score::toEList(EventMap* events)
      {
      updateRepeatList(getAction("repeat")->isChecked());
      _foundPlayPosAfterRepeats = false;
      updateChannel();
      int staffIdx = 0;
      foreach(Part* part, _parts) {
            int nextStaffIdx = staffIdx + part->staves()->size();
            toEList(events, staffIdx, nextStaffIdx);
            staffIdx = nextStaffIdx;
            }
      }

//---------------------------------------------------------
//   updateHairpin
//---------------------------------------------------------

void Score::updateHairpin(Hairpin* h)
      {
      Staff* st = h->staff();
      int tick  = h->segment()->tick();
      int velo  = st->velocities().velo(tick);
      int incr  = h->veloChange();

      Segment* es = static_cast<Segment*>(h->endElement());
      int tick2 = es->tick();

      //
      // If velocity increase/decrease is zero, then assume
      // the end velocity is taken from the next velocity
      // event (the next dynamics symbol after the hairpin).
      //

      int endVelo = velo;
      if (incr == 0)
            endVelo = st->velocities().nextVelo(tick2);
      else
            endVelo += incr;

      if (endVelo > 127)
            endVelo = 127;
      else if (endVelo < 1)
            endVelo = 1;

      st->velocities().setVelo(tick,    VeloEvent(VELO_INTERPOLATE, velo));
      st->velocities().setVelo(tick2-1, VeloEvent(VELO_FIX, endVelo));

      switch(h->dynType()) {
            case DYNAMIC_STAFF:
                  if (dStaffIdx == staffIdx)
                        velo.setVelo(tick, v);
                  break;
            case DYNAMIC_PART:
                  if (dStaffIdx >= partStaff && dStaffIdx < partStaff+partStaves){
                        for (int i = partStaff; i < partStaff+partStaves; ++i)
                              staff(i)->velocities().setVelo(tick, v);
                        }
                  break;
            case DYNAMIC_SYSTEM:
                  for (int i = 0; i < nstaves(); ++i)
                        staff(i)->velocities().setVelo(tick, v);
                  break;
            }
      }

//---------------------------------------------------------
//   fixPpitch
//    calculate play pitch and velocity for all notes
//---------------------------------------------------------

void Score::fixPpitch()
      {
      //
      //    collect Dynamics & Ottava
      //

      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            Staff* st      = staff(staffIdx);
            VeloList& velo = st->velocities();
            velo.clear();
            velo.setVelo(0, 80);
            Part* prt      = st->part();
            int partStaves = prt->nstaves();
            int partStaff  = Score::staffIdx(prt);

            for (Segment* s = firstMeasure()->first(); s; s = s->next1()) {
                  int tick = s->tick();
                  foreach(const Element* e, s->annotations()) {
                        if (e->staffIdx() != staffIdx)
                              continue;
                        if (e->type() != DYNAMIC)
                              continue;
                        const Dynamic* d = static_cast<const Dynamic*>(e);
                        int v            = d->velocity();
                        if (v < 1)     //  illegal value
                              continue;
                        int dStaffIdx = d->staffIdx();
                        switch(d->dynType()) {
                              case DYNAMIC_STAFF:
                                    if( dStaffIdx == staffIdx)
                                        velo.setVelo(tick, v);
                                    break;
                              case DYNAMIC_PART:
                                    if (dStaffIdx >= partStaff && dStaffIdx < partStaff+partStaves){
                                        for (int i = partStaff; i < partStaff+partStaves; ++i)
                                              staff(i)->velocities().setVelo(tick, v);
                                    }
                                    break;
                              case DYNAMIC_SYSTEM:
                                    for (int i = 0; i < nstaves(); ++i)
                                          staff(i)->velocities().setVelo(tick, v);
                                    break;
                              }
                        }
                  foreach(Element* e, s->spannerFor()) {
                        if (e->staffIdx() != staffIdx)
                              continue;
                        if (e->type() == HAIRPIN) {
                              Hairpin* h = static_cast<Hairpin*>(e);
                              updateHairpin(h);
                              }
                        else if (e->type() == OTTAVA) {
                              Ottava* o = static_cast<Ottava*>(e);
                              Segment* es = static_cast<Segment*>(o->endElement());
                              int tick2 = es->tick();
                              int shift = o->pitchShift();
                              st->pitchOffsets().setPitchOffset(tick, shift);
                              st->pitchOffsets().setPitchOffset(tick2, 0);
                              }
                        }
                  }
            }

      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            int pitchOffset   = styleB(ST_concertPitch) ? 0 : part(staffIdx)->instr()->transpose().chromatic;
            Staff* st         = staff(staffIdx);
            Instrument* instr = st->part()->instr();
            int strack        = staffIdx * VOICES;
            int etrack        = strack + VOICES;

            for (Segment* seg = firstSegment(); seg; seg = seg->next1()) {
                  if (seg->subtype() != SegChordRest && seg->subtype() != SegGrace)
                        continue;
                  int tick        = seg->tick();
                  int ottavaShift = st->pitchOffsets().pitchOffset(tick);
                  for (int track = strack; track < etrack; ++track) {
                        Element* el = seg->element(track);
                        if (!el || el->type() != CHORD)
                              continue;
                        Chord* chord = static_cast<Chord*>(el);

                        //
                        // get velocity depending on dynamic marks as "p" of "sfz"
                        // crescendo and diminuendo
                        //
                        int velocity = staff(staffIdx)->velocities().velo(seg->tick());
                        foreach(Note* note, chord->notes()) {

                              //
                              // adjust velocity for instrument, channel and
                              // depending on articulation marks
                              //
                              int channel = note->subchannel();
                              instr->updateVelocity(&velocity, channel, "");
                              foreach(Articulation* a, *chord->getArticulations())
                                    instr->updateVelocity(&velocity, channel, a->subtypeName());

                              note->setPpitch(note->pitch() + pitchOffset + ottavaShift);
                              switch (note->veloType()) {
                                    case OFFSET_VAL:
                                          velocity += (velocity * note->veloOffset()) / 100;
                                          // fall through

                                    case AUTO_VAL:
                                          if (velocity > 127)
                                                velocity = 127;
                                          else if (velocity < 1)
                                                velocity = 1;
                                          note->setVelocity(velocity);
                                          break;
                                    case USER_VAL:
                                          break;
                                    }
                              }
                        }
                  }
            }
      }


