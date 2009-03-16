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

//---------------------------------------------------------
//   ARec
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

void Score::collectMeasureEvents(EventMap* events, Measure* m, int staffIdx,
   int tickOffset)
      {
      Part* prt         = part(staffIdx);
      Instrument* instr = prt->instrument();
      int pitchOffset   = styleB(ST_concertPitch) ? 0 : instr->pitchOffset;

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
                        if (e->type() == OTTAVA) {
                              Ottava* ottava = (Ottava*)e;
                              int tick1 = ottava->tick();
                              int tick2 = ottava->tick2();
                              if (tick >= tick1 && tick < tick2) {
                                    ottavaShift = ottava->pitchShift();
                                    }
                              }
                        else if (e->type() == SLUR) {
                              Slur* slur = (Slur*)e;
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
                  Note* note = chord->noteList()->front();
                  if (note->tieBack())
                        continue;
                  bool tiedNote = false;
                  int lastNoteLen = len;
                  if (note->tieFor()) {
                        tiedNote = true;
                        while (note->tieFor()) {
                              if (note->tieFor()->endNote() == 0)
                                    break;
                              note = note->tieFor()->endNote();
                              lastNoteLen = note->chord()->tickLen();
                              len += lastNoteLen;
                              }
                        }

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
                  if (tiedNote)
                        len = len - lastNoteLen + ((lastNoteLen * gateTime) / 100 - 1);
                  else
                        len = (len * gateTime) / 100 - 1;
                  collectChord(events,
                     instr,
                     pitchOffset + ottavaShift,
                     chord, tick + tickOffset,
                     len
                     );
                  lv.clear();
                  sv.clear();
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
//   toEList
//    export score to event list
//---------------------------------------------------------

void Score::toEList(EventMap* events, int offset)
      {
      bool expandRepeats = getAction("repeat")->isChecked();
      _foundPlayPosAfterRepeats = false;
      updateChannel();
      int staffIdx   = 0;
      foreach(Part* part, _parts) {
            for (int i = 0; i < part->staves()->size(); ++i) {
                  toEList(events, expandRepeats, offset, staffIdx++);
                  }
            }
      }

//---------------------------------------------------------
//   RepeatLoop
//---------------------------------------------------------

struct RepeatLoop {
      enum LoopType { LOOP_REPEAT, LOOP_JUMP };

      LoopType type;
      MeasureBase* m;   // start measure of LOOP_REPEAT
      int count;
      QString stop, cont;

      RepeatLoop() {}
      RepeatLoop(MeasureBase* _m)  {
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

MeasureBase* Score::searchLabel(const QString& s, MeasureBase* start)
      {
      if (s == "start")
            return layout()->first();
//      else if (s == "end")
//            ;
      if (start == 0)
            start = layout()->first();
      for (MeasureBase* m = start; m; m = m->next()) {
            foreach(const Element* e, *m->el()) {
                  if (e->type() == MARKER) {
                        Marker* marker = (Marker*)e;
                        if (marker->label() == s)
                              return m;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   toEList
//    implements:
//          - repeats
//          - volta
//          - d.c. al fine
//          - d.s. al fine
//          - d.s. al coda
//---------------------------------------------------------

void Score::toEList(EventMap* events, bool expandRepeats, int offset, int staffIdx)
      {
      if (!expandRepeats) {
            for (MeasureBase* mb = layout()->first(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  collectMeasureEvents(events, (Measure*)mb, staffIdx, offset);
                  }
            return;
            }
      QStack<RepeatLoop> rstack;
      int tickOffset = 0;
      int overallRepeatCount = 0;
      int repeatEnd = -1;

      MeasureBase* fm = layout()->first();
      for (MeasureBase* mb = fm; mb;) {
            if (mb->type() != MEASURE) {
                  mb = mb->next();
                  continue;
                  }
            Measure* m = static_cast<Measure*>(mb);
            if (
                  (m->repeatFlags() & RepeatStart)
               && (rstack.isEmpty() || (rstack.top().m != m))
               && (rstack.isEmpty() || (rstack.top().type != RepeatLoop::LOOP_JUMP))
               && (repeatEnd == -1)
               )
                  rstack.push(RepeatLoop(m));

            if (!rstack.isEmpty() && !isVolta(m->tick(), rstack.top().count + 1))
                  tickOffset -= m->tickLen();   // skip this measure
            else
                  collectMeasureEvents(events, m, staffIdx, tickOffset + offset);

            if (rstack.isEmpty()) {
                  // Jumps are only accepted outside of other repeats
                  if (m->repeatFlags() & RepeatJump) {
                        Jump* s = 0;
                        foreach(Element* e, *m->el()) {
                              if (e->type() == JUMP) {
                                    s = (Jump*)e;
                                    break;
                                    }
                              }
                        if (s) {
                              const QString& jumpTo = s->jumpTo();
                              MeasureBase* nmb = searchLabel(jumpTo);
                              if (nmb) {
                                    rstack.push(RepeatLoop(s->playUntil(), s->continueAt()));
                                    tickOffset += m->tick() + m->tickLen() - nmb->tick();
                                    mb = nmb;
                                    continue;
                                    }
                              else
                                    printf("JUMP: label <%s> not found\n",
                                       qPrintable(jumpTo));
                              }
                        else
                              printf("Jump not found\n");
                        }
                  else if (m->repeatFlags() & RepeatEnd) {
                        // this is a end repeat without start repeat:
                        //    repeat from beginning
                        //
                        // dont repeat inside a repeat

                        if (repeatEnd < 0 || repeatEnd == m->tick()) {
                              ++overallRepeatCount;
                              if (overallRepeatCount < m->repeatCount()) {
                                    repeatEnd = m->tick();
                                    mb = layout()->first();
                                    tickOffset += m->tick() + m->tickLen() - mb->tick();
                                    continue;
                                    }
                              else {
                                    overallRepeatCount = 0;
                                    repeatEnd = -1;
                                    }
                              }

                        }
                  }
            else if (rstack.top().type == RepeatLoop::LOOP_REPEAT) {
                  if (m->repeatFlags() & RepeatEnd) {
                        //
                        // increment repeat count
                        //
                        // CHECK for nested repeats:
                        //    repeat a repeat inside a repeat only on first
                        //    pass of outer reapeat (?!?)
                        //
                        bool nestedRepeat = false;
                        int n = rstack.size();
                        if (n > 1 && rstack[n-2].type == RepeatLoop::LOOP_REPEAT) {
                              if (rstack[n-2].count)
                                    nestedRepeat = true;
                              }
                        if (!nestedRepeat && (++rstack.top().count < m->repeatCount())) {
                              //
                              // goto start of loop, fix tickOffset
                              //
                              mb = rstack.top().m;
                              tickOffset += m->tick() + m->tickLen() - mb->tick();
                              continue;
                              }
                        rstack.pop();     // end this loop
                        }
                  }
            else if (rstack.top().type == RepeatLoop::LOOP_JUMP) {
                  MeasureBase* m = searchLabel(rstack.top().stop);
                  if (m == 0)
                        printf("LOOP_JUMP: label not found\n");
                  if (m == mb) {
                        if (m->next() == 0)
                              break;
                        MeasureBase* nmb = searchLabel(rstack.top().cont, m->next());
                        if (nmb)
                              tickOffset += m->tick() + m->tickLen() - nmb->tick();
                        else if (!rstack.top().cont.isEmpty())
                              printf("Cont label not found: <%s>\n", qPrintable(rstack.top().cont));

                        mb = nmb;
                        rstack.pop();     // end this loop
                        continue;
                        }
                  }
            mb = mb->next();
            }
      if (!rstack.isEmpty()) {
            if (rstack.top().type == RepeatLoop::LOOP_JUMP
               && rstack.top().stop == "end")
                  ;
            else
                  printf("repeat stack not empty!\n");
            }
      }


