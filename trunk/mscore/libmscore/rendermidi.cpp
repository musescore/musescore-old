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
#include "dynamic.h"
#include "navigate.h"
#include "pedal.h"
#include "staff.h"
#include "hairpin.h"
#include "bend.h"
#include "tremolo.h"
#include "noteevent.h"
#include "segment.h"

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
      for (Segment* s = fm->first(SegChordRest); s; s = s->next1(SegChordRest)) {
            foreach(const Element* e, s->annotations()) {
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
                              staff->channelList(voice)->insert(s->tick(), a);
                        }
                  }
            }

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
//   playNote
//---------------------------------------------------------

static void playNote(EventMap* events, const Note* note, int channel, int pitch,
   int velo, int onTime, int offTime)
      {
      velo = note->customizeVelocity(velo);
      Event ev(ME_NOTEON);
      ev.setChannel(channel);
      ev.setPitch(pitch);
      ev.setVelo(velo);
      ev.setTuning(note->tuning());
      ev.setNote(note);
      events->insertMulti(onTime, ev);
      ev.setVelo(0);
      events->insertMulti(offTime, ev);
      }

//---------------------------------------------------------
//   collectNote
//---------------------------------------------------------

static void collectNote(EventMap* events, int channel, const Note* note, int velo, int tickOffset)
      {
      if (note->hidden() || note->tieBack())       // do not play overlapping notes
            return;

      int pitch   = note->ppitch();
      int tick    = note->chord()->tick() + tickOffset;
      int onTime  = tick + note->onTimeOffset() + note->onTimeUserOffset();
      int offTime = tick + note->playTicks() + note->offTimeOffset() + note->offTimeUserOffset() - 1;

      if (!note->playEvents().isEmpty()) {
            int ticks = note->playTicks();
            foreach(NoteEvent* e, note->playEvents()) {
                  int p = pitch + e->pitch();
                  if (p < 0)
                        p = 0;
                  else if (p > 127)
                        p = 127;
                  int on  = tick + (ticks * e->ontime())/1000;
                  int off = on + (ticks * e->len())/1000;
                  playNote(events, note, channel, p, velo, on, off);
                  }
            }
      else
            playNote(events, note, channel, pitch, velo, onTime, offTime);
#if 0
      if (note->bend()) {
            Bend* bend = note->bend();
            int ticks = note->playTicks();
            const QList<PitchValue>& points = bend->points();

            // transform into midi values
            //    pitch is in 1/100 semitones
            //    midi pitch is 12/16384 semitones
            //
            //    time is in noteDuration/60

            int n = points.size();
            int tick1 = 0;
            for (int pt = 0; pt < n; ++pt) {
                  int pitch = points[pt].pitch;

                  if ((pt == 0) && (pitch == points[pt+1].pitch)) {
                        Event ev(ME_CONTROLLER);
                        ev.setChannel(channel);
                        ev.setController(CTRL_PITCH);
                        int midiPitch = (pitch * 16384) / 300;
                        ev.setValue(midiPitch);
                        events->insertMulti(tick, ev);
                        }
                  if (pitch != points[pt+1].pitch) {
                        int pitchDelta = points[pt+1].pitch - pitch;
                        int tick2      = (points[pt+1].time * ticks) / 60;
                        int dt = points[pt+1].time - points[pt].time;
                        for (int tick3 = tick1; tick3 < tick2; tick3 += 16) {
                              Event ev(ME_CONTROLLER);
                              ev.setChannel(channel);
                              ev.setController(CTRL_PITCH);

                              int dx = ((tick3-tick1) * 60) / ticks;
                              int p  = pitch + dx * pitchDelta / dt;

                              int midiPitch = (p * 16384) / 1200;
                              ev.setValue(midiPitch);
                              events->insertMulti(tick + tick3, ev);
                              }
                        tick1 = tick2;
                        }
                  if (pt == (n-2))
                        break;
                  }
            Event ev(ME_CONTROLLER);
            ev.setChannel(channel);
            ev.setController(CTRL_PITCH);
            ev.setValue(0);
            events->insertMulti(tick + ticks, ev);
            }
#endif
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

static void collectMeasureEvents(EventMap* events, Measure* m, Part* part, int tickOffset)
      {
      int firstStaffIdx = m->score()->staffIdx(part);
      int nextStaffIdx  = firstStaffIdx + part->nstaves();

      SegmentTypes st = SegGrace | SegChordRest;
      int strack      = firstStaffIdx * VOICES;
      int etrack      = nextStaffIdx * VOICES;

      for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
            int tick = seg->tick();
            for (int track = strack; track < etrack; ++track) {
                  // skip linked staves, except primary
                  if (!m->score()->staff(track / VOICES)->primaryStaff()) {
                        track += VOICES-1;
                        continue;
                        }
                  Element* cr = seg->element(track);
                  if (cr && cr->type() == CHORD) {
                        Chord* chord = static_cast<Chord*>(cr);
                        Staff* staff = chord->staff();
                        int velocity = staff->velocities().velo(seg->tick());
                        Instrument* instr = chord->staff()->part()->instr(tick);
                        //
                        // adjust velocity for instrument, channel and
                        // depending on articulation marks
                        //
                        int channel = 0;  // note->subchannel();
                        instr->updateVelocity(&velocity, channel, "");
                        foreach (Articulation* a, *chord->getArticulations())
                              instr->updateVelocity(&velocity, channel, a->subtypeName());

                        Tremolo* tremolo = chord->tremolo();
                        if (tremolo) {
                              Fraction tl = tremolo->tremoloLen();
                              Fraction cl = chord->durationType().fraction();
                              Fraction r = cl / tl;
                              int repeats = r.numerator() / r.denominator();

                              if (chord->tremoloChordType() == TremoloFirstNote) {
                                    repeats /= 2;
                                    Segment* seg2 = seg->next(st);
                                    while (seg2 && !seg2->element(track))
                                          seg2 = seg2->next(st);
                                    ChordRest* cr = static_cast<ChordRest*>(seg2->element(track));
                                    if (cr && cr->type() == CHORD) {
                                          Chord* c2 = static_cast<Chord*>(cr);
                                          int tick = chord->tick() + tickOffset;
                                          for (int i = 0; i < repeats; ++i) {
                                                foreach (const Note* note, chord->notes()) {
                                                      int channel = instr->channel(note->subchannel()).channel;
                                                      playNote(events, note, channel, note->ppitch(), velocity, tick, tick + tl.ticks() - 1);
                                                      }
                                                tick += tl.ticks();
                                                foreach (const Note* note, c2->notes()) {
                                                      int channel = instr->channel(note->subchannel()).channel;
                                                      playNote(events, note, channel, note->ppitch(), velocity, tick, tick + tl.ticks() - 1);
                                                      }
                                                tick += tl.ticks();
                                                }
                                          }
                                    else
                                          printf("Tremolo: cannot find 2. chord\n");
                                    }
                              else if (chord->tremoloChordType() == TremoloSingle) {
                                    for (int i = 0; i < repeats; ++i) {
                                          int tick = chord->tick() + tickOffset + i * tl.ticks();
                                          foreach (const Note* note, chord->notes()) {
                                                int channel = instr->channel(note->subchannel()).channel;
                                                playNote(events, note, channel, note->ppitch(), velocity, tick, tick + tl.ticks() - 1);
                                                }
                                          }
                                    }
                              // else if (chord->tremoloChordType() == TremoloSecondNote)
                              //    ignore second note of two note tremolo
                              }
                        else {
                              foreach(const Note* note, chord->notes()) {
                                    int channel = instr->channel(note->subchannel()).channel;
                                    collectNote(events, channel, note, velocity, tickOffset);
                                    }
                              }
                        }
                  }
            }

      //
      // collect program changes and controller
      //
      for (Segment* s = m->first(SegChordRest); s; s = s->next(SegChordRest)) {
            // int tick = s->tick();
            foreach(Element* e, s->annotations()) {
                  if (e->type() != STAFF_TEXT
                     || e->staffIdx() < firstStaffIdx
                     || e->staffIdx() >= nextStaffIdx)
                        continue;
                  const StaffText* st = static_cast<const StaffText*>(e);
                  int tick = s->tick() + tickOffset;

                  Instrument* instr = e->staff()->part()->instr(tick);
                  foreach (const ChannelActions& ca, *st->channelActions()) {
                        int channel = ca.channel;
                        foreach(const QString& ma, ca.midiActionNames) {
                              NamedEventList* nel = instr->midiAction(ma, channel);
                              if (!nel)
                                    continue;
                              int n = nel->events.size();
                              for (int i = n-1; i >= 0; --i) {
                                    Event event(nel->events[i]);
                                    event.setOntime(tick);
                                    event.setChannel(channel);
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
                                          Event event;
                                          event.setType(ME_CONTROLLER);
                                          event.setController(98);
                                          event.setValue(k);
                                          event.setOntime(tick);
                                          event.setChannel(channel);
                                          events->insertMulti(tick, event);
                                          }
                                    }
                              Event event(ME_CONTROLLER);
                              event.setController(98);
                              event.setValue(96 + i);
                              event.setOntime(tick);
                              event.setChannel(channel);
                              events->insertMulti(tick, event);

                              event.setValue(64 + i);
                              events->insertMulti(tick, event);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   renderPart
//---------------------------------------------------------

void Score::renderPart(EventMap* events, Part* part)
      {
      foreach (const RepeatSegment* rs, *repeatList()) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            int tickOffset = rs->utick - rs->tick;
            for (Measure* m = tick2measure(startTick); m; m = m->nextMeasure()) {
                  collectMeasureEvents(events, m, part, tickOffset);
                  if (m->tick() + m->ticks() >= endTick)
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   updateRepeatList
//---------------------------------------------------------

void Score::updateRepeatList(bool expandRepeats)
      {
      if (!expandRepeats) {
            foreach(RepeatSegment* s, *repeatList())
                  delete s;
            repeatList()->clear();
            Measure* m = lastMeasure();
            if (m == 0)
                  return;
            RepeatSegment* s = new RepeatSegment;
            s->tick  = 0;
            s->len   = m->tick() + m->ticks();
            s->utick = 0;
            s->utime = 0.0;
            s->timeOffset = 0.0;
            repeatList()->append(s);
            }
      else
            repeatList()->unwind();
      if (debugMode)
            repeatList()->dump();
      }

//---------------------------------------------------------
//   toEList
//    export score to event list
//---------------------------------------------------------

void Score::toEList(EventMap* events)
      {
      // TODO-LIB:   bool repeat = getAction("repeat")->isChecked();
      bool repeat = true;
      updateRepeatList(repeat);
      _foundPlayPosAfterRepeats = false;
      updateChannel();
      foreach (Part* part, _parts)
            renderPart(events, part);
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
      if (!es)
            return;
      int tick2 = es->tick() - 1;

      //
      // If velocity increase/decrease is zero, then assume
      // the end velocity is taken from the next velocity
      // event (the next dynamics symbol after the hairpin).
      //

      int endVelo = velo;
      if (incr == 0)
            endVelo = st->velocities().nextVelo(tick2+1);
      else
            endVelo += incr;

      if (endVelo > 127)
            endVelo = 127;
      else if (endVelo < 1)
            endVelo = 1;

      switch(h->dynType()) {
            case DYNAMIC_STAFF:
                  st->velocities().setVelo(tick,  VeloEvent(VELO_RAMP, velo));
                  st->velocities().setVelo(tick2, VeloEvent(VELO_FIX, endVelo));
                  break;
            case DYNAMIC_PART:
                  foreach(Staff* s, *st->part()->staves()) {
                        s->velocities().setVelo(tick,  VeloEvent(VELO_RAMP, velo));
                        s->velocities().setVelo(tick2, VeloEvent(VELO_FIX, endVelo));
                        }
                  break;
            case DYNAMIC_SYSTEM:
                  foreach(Staff* s, _staves) {
                        s->velocities().setVelo(tick,  VeloEvent(VELO_RAMP, velo));
                        s->velocities().setVelo(tick2, VeloEvent(VELO_FIX, endVelo));
                        }
                  break;
            }
      }

//---------------------------------------------------------
//   removeHairpin
//---------------------------------------------------------

void Score::removeHairpin(Hairpin* h)
      {
      Staff* st = h->staff();
      int tick  = h->segment()->tick();
      Segment* es = static_cast<Segment*>(h->endElement());
      if (!es)
            return;
      int tick2 = es->tick() - 1;

      switch(h->dynType()) {
            case DYNAMIC_STAFF:
                  st->velocities().remove(tick);
                  st->velocities().remove(tick2);
                  break;
            case DYNAMIC_PART:
                  foreach(Staff* s, *st->part()->staves()) {
                        s->velocities().remove(tick);
                        s->velocities().remove(tick2);
                        }
                  break;
            case DYNAMIC_SYSTEM:
                  foreach(Staff* s, _staves) {
                        s->velocities().remove(tick);
                        s->velocities().remove(tick2);
                        }
                  break;
            }
      }

//---------------------------------------------------------
//   updateVelo
//    calculate velocity for all notes
//---------------------------------------------------------

void Score::updateVelo()
      {
      //
      //    collect Dynamics & Ottava & Hairpins
      //
      if (!firstMeasure())
            return;

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
                  foreach(Element* e, s->spannerFor()) {
                        if (e->staffIdx() != staffIdx)
                              continue;
                        if (e->type() == HAIRPIN) {
                              Hairpin* h = static_cast<Hairpin*>(e);
                              updateHairpin(h);
                              }
                        }
                  }
            }
      }
