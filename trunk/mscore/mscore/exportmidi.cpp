//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include "score.h"
#include "part.h"
#include "staff.h"
#include "al/tempo.h"
#include "midifile.h"
#include "event.h"
#include "al/sig.h"
#include "key.h"
#include "preferences.h"
#include "text.h"
#include "measure.h"
#include "repeatlist.h"

//---------------------------------------------------------
//   exportMidi
//---------------------------------------------------------

class ExportMidi {
      QFile f;
      Score* cs;

      void writeHeader();

   public:
      MidiFile mf;

      ExportMidi(Score* s) { cs = s; }
      bool write(const QString& name);
      };

//---------------------------------------------------------
//   exportMidi
//    return false on error
//---------------------------------------------------------

bool Score::saveMidi(const QString& name)
      {
      ExportMidi em(this);
      return em.write(name);
      }

//---------------------------------------------------------
//   writeHeader
//---------------------------------------------------------

void ExportMidi::writeHeader()
      {
      MidiTrack* track  = mf.tracks()->front();
      MeasureBase* measure  = cs->first();

      foreach (const Element* e, *measure->el()) {
            if (e->type() == TEXT) {
                  const Text* text = (const Text*)(e);
                  QString str = text->getText();
                  int len     = str.length() + 1;
                  unsigned char* data = new unsigned char[len];
                  strcpy((char*)(data), str.toLatin1().data());
                  Event* ev = new Event(ME_META);
                  ev->setOntime(0);
                  ev->setData(data);
                  ev->setLen(len);

                  switch (text->subtype()) {
                        case TEXT_TITLE:
                              ev->setMetaType(META_TITLE);
                              track->insert(ev);
                              break;
                        case TEXT_SUBTITLE:
                              ev->setMetaType(META_SUBTITLE);
                              track->insert(ev);
                              break;
                        case TEXT_COMPOSER:
                              ev->setMetaType(META_COMPOSER);
                              track->insert(ev);
                              break;
                        case TEXT_TRANSLATOR:
                              ev->setMetaType(META_TRANSLATOR);
                              track->insert(ev);
                              break;
                        case TEXT_POET:
                              ev->setMetaType(META_POET);
                              track->insert(ev);
                              break;
                        default:
                              delete ev;
                              break;
                        }
                  }
            }

      //--------------------------------------------
      //    write time signature
      //--------------------------------------------

#if 0 // TODO
      AL::TimeSigMap* sigmap = cs->sigmap();
      foreach(const RepeatSegment* rs, *cs->repeatList()) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            int tickOffset = rs->utick - rs->tick;

            AL::iSigEvent bs = sigmap->lower_bound(startTick);
            AL::iSigEvent es = sigmap->lower_bound(endTick);

            for (AL::iSigEvent is = bs; is != es; ++is) {
                  AL::SigEvent se   = is->second;
                  unsigned char* data = new unsigned char[4];
                  data[0] = se.fraction().numerator();
                  int n;
                  switch (se.fraction().denominator()) {
                        case 1:  n = 0; break;
                        case 2:  n = 1; break;
                        case 4:  n = 2; break;
                        case 8:  n = 3; break;
                        case 16: n = 4; break;
                        case 32: n = 5; break;
                        default:
                              n = 2;
                              printf("ExportMidi: unknown time signature %s\n",
                                 qPrintable(se.fraction().print()));
                              break;
                        }
                  data[1] = n;
                  data[2] = 24;
                  data[3] = 8;

                  Event* ev = new Event(ME_META);
                  ev->setMetaType(META_TIME_SIGNATURE);
                  ev->setData(data);
                  ev->setLen(4);
                  ev->setOntime(is->first + tickOffset);
                  track->insert(ev);
                  }
            }
#endif

      //---------------------------------------------------
      //    write key signatures
      //    assume every staff corresponds to a midi track
      //---------------------------------------------------

      QList<MidiTrack*>* tl = mf.tracks();
      for (int i = 0; i < tl->size(); ++i) {
            MidiTrack* track  = tl->at(i);

            KeyList* keymap = cs->staff(i)->keymap();

            foreach(const RepeatSegment* rs, *cs->repeatList()) {
                  int startTick  = rs->tick;
                  int endTick    = startTick + rs->len;
                  int tickOffset = rs->utick - rs->tick;

                  iKeyList sk = keymap->lower_bound(startTick);
                  iKeyList ek = keymap->lower_bound(endTick);

                  for (iKeyList ik = sk; ik != ek; ++ik) {
                        Event* ev  = new Event(ME_META);
                        ev->setOntime(ik->first + tickOffset);
                        int key       = ik->second.accidentalType;   // -7 -- +7
                        ev->setMetaType(META_KEY_SIGNATURE);
                        ev->setLen(2);
                        unsigned char* data = new unsigned char[2];
                        data[0]   = key;
                        data[1]   = 0;  // major
                        ev->setData(data);
                        track->insert(ev);
                        }
                  }
            }

      //--------------------------------------------
      //    write tempo changes
      //--------------------------------------------

      AL::TempoMap* tempomap = cs->tempomap();
      foreach(const RepeatSegment* rs, *cs->repeatList()) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            int tickOffset = rs->utick - rs->tick;

            AL::iTEvent se = tempomap->lower_bound(startTick);
            AL::iTEvent ee = tempomap->lower_bound(endTick);
            for (AL::iTEvent it = se; it != ee; ++it) {
                  Event* ev = new Event(ME_META);
                  ev->setOntime(it->first + tickOffset);
                  //
                  // compute midi tempo: microseconds / quarter note
                  //
                  int tempo = lrint((1.0 / it->second.tempo) * 1000000.0);

                  ev->setMetaType(META_TEMPO);
                  ev->setLen(3);
                  unsigned char* data = new unsigned char[3];
                  data[0]   = tempo >> 16;
                  data[1]   = tempo >> 8;
                  data[2]   = tempo;
                  ev->setData(data);
                  track->insert(ev);
                  }
            }
      }

//---------------------------------------------------------
//  write
//    export midi file
//    return false on error
//---------------------------------------------------------

bool ExportMidi::write(const QString& name)
      {
      f.setFileName(name);
      if (!f.open(QIODevice::WriteOnly))
            return false;

      mf.setDivision(AL::division);
      mf.setFormat(1);
      QList<MidiTrack*>* tracks = mf.tracks();
      int nstaves = cs->nstaves();

      for (int i = 0; i < nstaves; ++i)
            tracks->append(new MidiTrack(&mf));

      cs->updateRepeatList(preferences.midiExpandRepeats);
      writeHeader();

      foreach (Staff* staff, cs->staves()) {
            Part* part       = staff->part();
            int channel      = part->midiChannel();
            int staffIdx     = staff->idx();
            MidiTrack* track = tracks->at(staffIdx);
            track->setOutPort(0);
            track->setOutChannel(channel);

            if (staff->isTop()) {
                  // set pitch bend sensitivity to 12 semitones:
                  track->addCtrl(0, channel, CTRL_LRPN, 0);
                  track->addCtrl(0, channel, CTRL_HRPN, 0);
                  track->addCtrl(0, channel, CTRL_HDATA, 12);

                  // reset fine tuning
                  track->addCtrl(0, channel, CTRL_LRPN, 1);
                  track->addCtrl(0, channel, CTRL_HRPN, 0);
                  track->addCtrl(0, channel, CTRL_HDATA, 64);

                  // deactivate rpn
                  track->addCtrl(0, channel, CTRL_LRPN, 127);
                  track->addCtrl(0, channel, CTRL_HRPN, 127);

                  if (part->midiProgram() != -1)
                        track->addCtrl(0, channel, CTRL_PROGRAM, part->midiProgram());
                  track->addCtrl(0, channel, CTRL_VOLUME, part->volume());
                  track->addCtrl(0, channel, CTRL_PANPOT, part->pan());
                  track->addCtrl(0, channel, CTRL_REVERB_SEND, part->reverb());
                  track->addCtrl(0, channel, CTRL_CHORUS_SEND, part->chorus());
                  }
            EventMap events;
            cs->toEList(&events, staffIdx, staffIdx+1);
            for (EventMap::const_iterator i = events.constBegin(); i != events.constEnd(); ++i) {
                  if (i.value()->type() == ME_NOTEON) {
                        Event* n = i.value();
                        Event* ne = new Event(ME_NOTEON);
                        ne->setOntime(i.key());
                        ne->setChannel(n->channel());
                        ne->setPitch(n->pitch());
                        ne->setVelo(n->velo());
                        track->insert(ne);
                        }
                  else if (i.value()->type() == ME_CONTROLLER) {
                        Event* n = i.value();
                        track->addCtrl(i.key(), n->channel(), n->controller(), n->value());
                        }
                  else {
                        printf("writeMidi: unknown midi event 0x%02x\n", i.value()->type());
                        }
                  }
            }
      return !mf.write(&f);
      }


