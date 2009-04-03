//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: score.cpp,v 1.24 2006/04/12 14:58:10 wschweer Exp $
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
 render score into event list
*/

#include "mscore.h"
#include "score.h"
#include "volta.h"
#include "note.h"
#include "layout.h"
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
#include "tempo.h"
#include "repeatlist.h"

//---------------------------------------------------------
//   ARec
//    articulation record
//---------------------------------------------------------

struct ARec {
      int tick;
      int channel;
      };

//---------------------------------------------------------
//   updateChannel
//---------------------------------------------------------

void Score::updateChannel()
      {
      int staffIdx   = 0;
      QList<ARec> alist;
      foreach(Part* part, _parts) {
            alist.clear();
            for (int i = 0; i < part->staves()->size(); ++i) {
                  for (MeasureBase* mb = layout()->first(); mb; mb = mb->next()) {
                        if (mb->type() != MEASURE)
                              continue;
                        Measure* m = static_cast<Measure*>(mb);
                        foreach(const Element* e, *m->el()) {
                              if (e->type() != STAFF_TEXT || e->staffIdx() != staffIdx)
                                    continue;
                              const StaffText* st = static_cast<const StaffText*>(e);
                              QString an(st->channelName());
                              if (an.isEmpty())
                                    continue;
                              int a = part->instrument()->channelIdx(an);
                              if (a != -1) {
                                    ARec ar;
                                    ar.tick = st->tick();
                                    ar.channel = a;
                                    alist.append(ar);
                                    }
                              else
                                    printf("channel <%s> not found\n", qPrintable(an));
                              }
                        }
                  }

            for (int i = 0; i < part->staves()->size(); ++i) {
                  Segment* s = tick2segment(0);
                  while (s) {
                        for (int track = staffIdx * VOICES; track < staffIdx*VOICES+VOICES; ++track) {
                              if (!s->element(track))
                                    continue;
                              Element* e = s->element(track);
                              if (e->type() != CHORD)
                                    continue;
                              Chord* c = static_cast<Chord*>(e);
                              NoteList* nl = c->noteList();
                              int sc = 0;
                              //
                              // TODO: optimize
                              //
                              foreach(const ARec& ar, alist) {
                                    if (ar.tick > c->tick())
                                          break;
                                    sc = ar.channel;
                                    }
                              QList<ARec> alist;
                              for (iNote in = nl->begin(); in != nl->end(); ++in) {
                                    Note* note = in->second;
                                    if (note->hidden())
                                          continue;
                                    if (note->tieBack())
                                          continue;
                                    note->setSubchannel(sc);
                                    }
                              }
                        s = s->next1();
                        }
                  }
            ++staffIdx;
            }
      }

//---------------------------------------------------------
//   isVolta
//    return true if no volta found
//---------------------------------------------------------

bool Score::isVolta(int tick, int repeat) const
      {
      foreach(const Element* el, *gel()) {
            if (el->type() == VOLTA) {
                  const Volta* volta = (Volta*)el;
                  if (tick >= volta->tick() && tick < volta->tick2()) {
                        foreach(int ending, volta->endings()) {
                              if (ending == repeat)
                                    return true;
                              }
                        return false;
                        }
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   collectChord
//---------------------------------------------------------

void Score::collectChord(EventMap* events, Instrument* instr,
   int pitchOffset, Chord* chord, int tick, int len)
      {
      NoteList* nl = chord->noteList();
      Arpeggio* arpeggio = chord->arpeggio();

      int arpeggioOffset = 0;
      static const int arpeggioNoteDistance = Duration(Duration::V_32ND).ticks();
      if (arpeggio && chord->noteList()->size() * arpeggioNoteDistance <= unsigned(len))
            arpeggioOffset = arpeggioNoteDistance;

      int i = 0;
      for (iNote in = nl->begin(); in != nl->end(); ++in, ++i) {
            Note* note = in->second;
            if (note->hidden())       // do not play overlapping notes
                  continue;
            if (note->tieBack())
                  continue;
            int idx = instr->channel[note->subchannel()]->channel;
            NoteOn* ev = new NoteOn();
            int pitch = note->pitch() + pitchOffset;
            if (pitch > 127)
                  pitch = 127;
            ev->setPitch(pitch);
            ev->setVelo(60);
            ev->setNote(note);
            ev->setChannel(idx);
            events->insertMulti(tick + i * arpeggioOffset, ev);

            ev = new NoteOn();
            ev->setPitch(pitch);
            ev->setVelo(0);
            ev->setNote(note);
            ev->setChannel(idx);
            events->insertMulti(tick + len, ev);
            }
      }

//---------------------------------------------------------
//   collectMeasureEvents
//---------------------------------------------------------

void Score::collectMeasureEvents(EventMap* events, Measure* m, int staffIdx, int tickOffset)
      {
      Part* prt         = part(staffIdx);
      Instrument* instr = prt->instrument();
      int pitchOffset   = styleB(ST_concertPitch) ? 0 : instr->pitchOffset;

// printf("collect %d-%d\n", m->tick(), m->tick() + m->tickLen());

      QList<Chord*> lv;       // appoggiatura
      QList<Chord*> sv;       // acciaccatura

      // for the purpose of knowing whether to find the playPos after repeats
      bool playExpandRepeats = getAction("repeat")->isChecked();

      for (int voice = 0; voice < VOICES; ++voice) {
            int track = staffIdx * VOICES + voice;
            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                  Element* el = seg->element(track);
                  if (!el || el->type() != CHORD)
                        continue;
                  Chord* chord = (Chord*)el;
                  if (chord->noteType() != NOTE_NORMAL) {
                        if (chord->noteType() == NOTE_ACCIACCATURA)
                              sv.append(chord);
                        else if (chord->noteType() == NOTE_APPOGGIATURA)
                              lv.append(chord);
                        continue;
                        }

                  int gateTime    = _style[ST_gateTime].toInt();  // 100 - legato (100%)
                  int tick        = chord->tick();
                  if (playExpandRepeats && !_foundPlayPosAfterRepeats && tick == playPos()) {
                        setPlayPos(tick + tickOffset);
                        _foundPlayPosAfterRepeats = true;
                        }
                  int ottavaShift = 0;
                  foreach(Element* e, *m->score()->gel()) {
                        if ((e->type() == OTTAVA) && (e->staffIdx() == staffIdx)) {
                              Ottava* ottava = static_cast<Ottava*>(e);
                              int tick1 = ottava->tick();
                              int tick2 = ottava->tick2();
                              if (tick >= tick1 && tick < tick2) {
                                    ottavaShift = ottava->pitchShift();
                                    }
                              }
                        else if (e->type() == SLUR) {
                              Slur* slur = static_cast<Slur*>(e);
                              if (slur->startElement()->staffIdx() != staffIdx)
                                    continue;
                              int tick1 = slur->tick();
                              int tick2 = slur->tick2();
                              if (tick >= tick1 && tick < tick2 && slur->track() == track) {
                                    gateTime = _style[ST_slurGateTime].toInt();
                                    }

                              }
                        }
                  foreach(Articulation* a, *chord->getArticulations()) {
                        switch(a->subtype()) {
                              case TenutoSym:
                                    gateTime = _style[ST_tenutoGateTime].toInt();
                                    break;
                              case StaccatoSym:
                                    gateTime = _style[ST_staccatoGateTime].toInt();
                                    break;
                              default:
                                    break;
                              }
                        }

                  // compute len of chord

                  int len = chord->tickLen();
                  int apl = 0;
                  if (!lv.isEmpty()) {
                        foreach(Chord* c, lv) {
                              apl += c->tickLen();
                              }
                        // treat appogiatura as acciaccatura if it exceeds the note length
                        if (apl >= len)
                              sv = lv;
                        }
                  if (!sv.isEmpty()) {
                        //
                        // move acciaccatura's in front of
                        // main note
                        //
                        int sl  = len / 4;
                        int ssl = sl / sv.size();
                        foreach(Chord* c, sv) {
                              collectChord(events, instr,
                                 pitchOffset + ottavaShift,
                                 c,
                                 tick + tickOffset - sl,
                                 ssl * gateTime / 100 - 1
                                 );
                              sl -= ssl;
                              }
                        }
                  else if (!lv.isEmpty()) {
                        //
                        // appoggiatura's use time from main note
                        //
                        int sl = 0;
                        foreach(Chord* c, lv) {
                              int ssl = c->tickLen();
                              collectChord(events, instr,
                                 pitchOffset + ottavaShift,
                                 c,
                                 tick + tickOffset + sl,
                                 ssl * gateTime / 100 - 1
                                 );
                              sl += ssl;
                              }
                        len -= sl;
                        tick += sl;
                        }
                  {
                  pitchOffset += ottavaShift;
                  tick += tickOffset;
                  NoteList* nl = chord->noteList();
                  Arpeggio* arpeggio = chord->arpeggio();

                  int arpeggioOffset = 0;
                  static const int arpeggioNoteDistance = Duration(Duration::V_32ND).ticks();
                  if (arpeggio && chord->noteList()->size() * arpeggioNoteDistance <= unsigned(len))
                        arpeggioOffset = arpeggioNoteDistance;

                  int i = 0;
                  for (iNote in = nl->begin(); in != nl->end(); ++in, ++i) {
                        Note* note = in->second;
                        if (note->hidden() || note->tieBack())       // do not play overlapping notes
                              continue;
                        int idx = instr->channel[note->subchannel()]->channel;

                        int len = note->chord()->tickLen();
                        bool tiedNote = false;
                        int lastNoteLen = len;
                        if (note->tieFor()) {
                              Note* n = note;
                              tiedNote = true;
                              while (n->tieFor()) {
                                    if (n->tieFor()->endNote() == 0)
                                          break;
                                    n = n->tieFor()->endNote();
                                    lastNoteLen = n->chord()->tickLen();
                                    len += lastNoteLen;
                                    }
                              }
                        if (tiedNote)
                              len = len - lastNoteLen + ((lastNoteLen * gateTime) / 100 - 1);
                        else
                              len = (len * gateTime) / 100 - 1;

                        NoteOn* ev = new NoteOn();
                        int pitch = note->pitch() + pitchOffset;
                        if (pitch > 127)
                              pitch = 127;
                        ev->setPitch(pitch);
                        ev->setVelo(60);
                        ev->setNote(note);
                        ev->setChannel(idx);
                        events->insertMulti(tick + i * arpeggioOffset, ev);

                        ev = new NoteOn();
                        ev->setPitch(pitch);
                        ev->setVelo(0);
                        ev->setNote(note);
                        ev->setChannel(idx);
                        events->insertMulti(tick + len, ev);
                        }
                  }
                  }
            }
      //
      // collect program changes and controller
      //
      foreach(const Element* e, *m->el()) {
            if (e->type() != STAFF_TEXT || e->staffIdx() != staffIdx)
                  continue;
            const StaffText* st = static_cast<const StaffText*>(e);
            QString ma(st->midiActionName());
            if (!ma.isEmpty()) {
                  NamedEventList* nel = prt->instrument()->midiAction(ma);
                  if (nel) {
                        foreach(Event* ev, nel->events) {
                              Event* event = ev->clone();
                              int tick = st->tick() + tickOffset;
                              event->setOntime(tick);
                              events->insertMulti(tick, event);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   RepeatLoop
//---------------------------------------------------------

struct RepeatLoop {
      enum LoopType { LOOP_REPEAT, LOOP_JUMP };

      LoopType type;
      Measure* m;   // start measure of LOOP_REPEAT
      int count;
      QString stop, cont;

      RepeatLoop() {}
      RepeatLoop(Measure* _m)  {
            m     = _m;
            count = 0;
            type  = LOOP_REPEAT;
            }
      RepeatLoop(const QString s, const QString c)
         : stop(s), cont(c)
            {
            type = LOOP_JUMP;
            }
      };

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

void Score::toEList(EventMap* events, int offset, int staffIdx)
      {
      foreach(const RepeatSegment* rs, *_repeatList) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            int tickOffset = rs->utick - rs->tick;
            for (Measure* m = tick2measure(startTick); m; m = m->nextMeasure()) {
                  collectMeasureEvents(events, m, staffIdx, tickOffset + offset);
                  if (m->tick() + m->tickLen() >= endTick)
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
            foreach(RepeatSegment* s, *_repeatList)
                  delete s;
            _repeatList->clear();
            Measure* m = lastMeasure();
            if (m == 0)
                  return;
            RepeatSegment* s = new RepeatSegment;
            s->tick  = 0;
            s->len   = m->tick() + m->tickLen();
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
      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx)
            toEList(events, 0, staffIdx);
      }

