//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
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

#include "bb.h"
#include "score.h"
#include "part.h"
#include "staff.h"
#include "layout.h"
#include "text.h"
#include "box.h"
#include "slur.h"
#include "note.h"
#include "chord.h"
#include "rest.h"
#include "drumset.h"
#include "utils.h"

const char* keys[] = {
      "/",
      "C",   "Db", "D",   "Eb",  "E",   "F",   "Gb", "G",
      "Ab", "A",   "Bb", "B",   "C#",  "D#",  "F#",  "G#", "A#",
      "Cm", "Dbm", "Dm", "Ebm", "Em",  "Fm",  "Gbm", "Gm",
      "Abm", "Am", "Bbm", "Bm", "C#m", "D#m", "F#m", "G#m", "A#m"
      };

//---------------------------------------------------------
//   BBTrack
//---------------------------------------------------------

BBTrack::BBTrack(BBFile* f)
      {
      bb          = f;
      _outChannel = -1;
      _drumTrack  = false;
      }

BBTrack::~BBTrack()
      {
      }

//---------------------------------------------------------
//   MNote
//	special Midi Note
//---------------------------------------------------------

struct MNote {
	MidiChord* mc;
      QList<Tie*> ties;

      MNote(MidiChord* _mc) : mc(_mc) {
            for (int i = 0; i < mc->notes().size(); ++i)
                  ties.append(0);
            }
      };

//---------------------------------------------------------
//   BBFile
//---------------------------------------------------------

BBFile::BBFile()
      {
      for (int i = 0; i < 1024; ++i) {
            _chordExt[i] = 0;
            _chordBase[i] = 0;
            }
      }

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

bool BBFile::read(const QString& name)
      {
      _siglist.clear();
      _siglist.add(0, 4, 4);        // debug

      _path = name;
      QFile f(name);
      if (!f.open(QIODevice::ReadOnly)) {
            return false;
            }
      ba       = f.readAll();
      a        = (const unsigned char*) ba.data();
      size     = ba.size();
      _version = a[0];

      f.close();
      switch(_version) {
            case 0x47:
            case 0x49:
            case 0x44:        // melody not found
                  break;
            default:
                  printf("BB: unknown file version %02x\n", _version);
                  return false;
            }
      int idx = 1;
      int len = a[idx++];
      _title  = new char[len+1];
      for (int i = 0; i < len; ++i)
            _title[i] = a[idx++];
      _title[len] = 0;

      ++idx;
      ++idx;
      _style = a[idx++];
      _key   = a[idx++];
      _bpm   = a[idx] + a[idx+1] << 8;
      idx += 2;

      printf("Title <%s>\n", _title);
      printf("style %d\n",   _style);
      printf("key   %d  %s\n", _key, keys[_key]);
      printf("bpm   %d\n", _bpm);

      int bar = a[idx++];           // starting bar number
      while (bar < 255) {
            int val = a[idx++];
            if (val == 0)
                  bar += a[idx++];
            else
                  bar++;
            }

      printf("read ChordExt table at %x\n", idx);

      int chords = 0;
      for (int i = 0; i < 1020;) {
            int val = a[idx++];
            if (val == 0)
                  i += a[idx++];
            else {
                  _chordExt[i] = val;
                  chords = i;
                  ++i;
                  }
            }

      printf("read ChordBase table at %x\n", idx);

      for (int i = 0; i < 1020;) {
            int val = a[idx++];
            if (val == 0)
                  i += a[idx++];
            else
                  _chordBase[i++] = val;
            }
//      printf("================chords=======================\n");
//      for (int i = 0; i < 1022; ++i) {
//            if (_chordBase[i])
//                  printf("base %d  ext %d\n", _chordBase[i], _chordExt[i]);
//            }
//      printf("================chords=======================\n");

      _measures = (chords+3) / 4;
      printf("Measures %d\n", _measures);

      _startChorus = a[idx++];
      _endChorus   = a[idx++];
      _repeats     = a[idx++];

      printf("start chorus %d  end chorus %d repeats %d, pos now 0x%x\n",
         _startChorus, _endChorus, _repeats, idx);

      if (a[idx] == 1)        // 01 00 01 (vary style in middle chorus on)
            idx += 65;
      else                    // 00 02
            idx += 64;

      printf("read styleName at 0x%x\n", idx);
      len = a[idx++];
      _styleName = new char[len+1];

      for (int i = 0; i < len; ++i)
            _styleName[i] = a[idx++];
      _styleName[len] = 0;

      printf("style name <%s>\n", _styleName);

      printf("flags at 0x%x\n", idx);
      _flags = a[idx++];
      // 0x00 0000      0 - loop enabled, 1 loop disabled

      int i;
      for (i = idx; i < size; ++i) {
            if (a[i] == 0 && a[i+1] == 0xff && a[i+2] == 0 && a[i+3] == 0x0d)
                  break;
            }
      int eventCount;
      if (i == size) {
            printf("note count not found\n");
            eventCount = 999;
            }
      else {
            idx = i + 4;
            eventCount = a[idx] + a[idx+1] * 256;
            idx += 2;
            printf("event count %d\n", eventCount);
            }

      //
      // look for A0 B0 C1    0x90 events
      // look for A0 B0 C0    0x93 events + 0x90 events

      for (i = idx; i < size; ++i) {
//            if (a[i] == 0xa0 && a[i+1] == 0xb0 && ((a[i+2] == 0xc1) || (a[i+2] == 0xc0)))
            if ((a[i] == 0xa0) && (a[i+1] == 0xb0) && (a[i+2] == 0xc1))
                  break;
            }
      if (i == size) {
            printf("melody not found\n");
            return false;
            }
      else {
            idx = i + 3;
            printf("melody found at 0x%x\n", idx);
            int maxNotes = (size - idx) / 12;
            int n = qMin(maxNotes, eventCount-1);  // -1?
            for (int i = 0; i < n; ++i) {
                  if (a[idx + 4] == 0x90) {
                        int channel = a[idx + 7];
                        BBTrack* track = 0;
                        foreach (BBTrack* t, _tracks) {
                              if (t->outChannel() == channel) {
                                    track = t;
                                    break;
                                    }
                              }
                        if (track == 0) {
                              track = new BBTrack(this);
                              track->setOutChannel(channel);
                              _tracks.append(track);
                              }
                        MidiNote* note = new MidiNote();
                        int tick = a[idx] + (a[idx+1]<<8) + (a[idx+2]<<16) + (a[idx+3]<<24);
                        tick -= 4 * 120;
                        note->setOntime((tick * division) / 120);
                        note->setPitch(a[idx + 5]);
                        note->setVelo(a[idx + 6]);
                        note->setChannel(channel);
                        int len = a[idx+8] + (a[idx+9]<<8) + (a[idx+10]<<16) + (a[idx+11]<<24);
                        note->setDuration((len * division) / 120);
                        track->append(note);
                        }
                  else
                        printf("unknown event type 0x%02x\n", a[idx + 4]);
                  idx += 12;
                  }
            }

      return true;
      }

//---------------------------------------------------------
//   importBB
//    return true on success
//---------------------------------------------------------

bool Score::importBB(const QString& name)
      {
      BBFile* bb = new BBFile;
      if (!bb->read(name)) {
            printf("cannot open file <%s>\n", qPrintable(name));
            return false;
            }

      QList<BBTrack*>* tracks = bb->tracks();
      int n = tracks->size();
      for (int i = 0; i < n; ++i) {
            Part* part = new Part(this);
            Staff* s   = new Staff(this, part, 0);
            part->insertStaff(s);
            _staves.push_back(s);
            }

      //---------------------------------------------------
      //  create measures
      //---------------------------------------------------

      for (int i = 0; i < bb->measures(); ++i) {
            Measure* measure  = new Measure(this);
            int tick = sigmap->bar2tick(i, 0, 0);
            measure->setTick(tick);
      	_layout->add(measure);
            }

      //---------------------------------------------------
      //  create notes
      //---------------------------------------------------

	foreach (BBTrack* track, *tracks)
            track->cleanup();

	int staffIdx = 0;
	foreach (BBTrack* track, *tracks)
            convertTrack(track, staffIdx++);

      spell();

      //---------------------------------------------------
      //    create title
      //---------------------------------------------------

      Text* text = new Text(this);
      text->setSubtype(TEXT_TITLE);
      text->setText(bb->title());

      ScoreLayout* layout = mainLayout();
      MeasureBase* measure = layout->first();
      if (measure->type() != VBOX) {
            measure = new VBox(this);
            measure->setTick(0);
            measure->setNext(layout->first());
            layout->add(measure);
            }
      measure->add(text);


      _saved   = false;
      _created = true;
      return true;
      }

//---------------------------------------------------------
//   convertTrack
//---------------------------------------------------------

void Score::convertTrack(BBTrack* track, int staffIdx)
	{
      int key = 0;      // findKey(midiTrack, sigmap);

      track->findChords();
      int voices = track->separateVoices(2);

      Staff* cstaff = staff(staffIdx);
	const EventList el = track->events();

      Drumset* drumset = cstaff->part()->drumset();
      bool useDrumset  = cstaff->part()->useDrumset();

      for (int voice = 0; voice < voices; ++voice) {
            QList<MNote*> notes;
            int ctick = 0;
            for (ciEvent i = el.begin(); i != el.end();) {
                  MidiEvent* e = *i;
                  if (e->type() != ME_CHORD) {
                        ++i;
                        continue;
                        }
                  if (((MidiChord*)e)->voice() != voice) {
                        ++i;
                        continue;
                        }
                  //
                  // process pending notes
                  //
                  while (!notes.isEmpty()) {
                        int tick = notes[0]->mc->ontime();
                        int len  = (*i)->ontime() - tick;
                        if (len <= 0)
                              break;
                  	foreach (MNote* n, notes) {
                        	if (n->mc->duration() < len)
                                    len = n->mc->duration();
                              }
            		Measure* measure = tick2measure(tick);
                        // split notes on measure boundary
                        if ((tick + len) > measure->tick() + measure->tickLen()) {
                              len = measure->tick() + measure->tickLen() - tick;
                              }
                        Chord* chord = new Chord(this);
                        chord->setTick(tick);
                        chord->setStaffIdx(staffIdx);
                        chord->setTickLen(len);
                        chord->setVoice(voice);
                        Segment* s = measure->getSegment(chord);
                        s->add(chord);

                  	foreach (MNote* n, notes) {
                              QList<MidiNote*>& nl = n->mc->notes();
                              for (int i = 0; i < nl.size(); ++i) {
                                    MidiNote* mn = nl[i];
                        		Note* note = new Note(this);
                                    note->setPitch(mn->pitch());
                                    note->setTpc(mn->tpc());
                        		note->setStaffIdx(staffIdx);
                  	      	chord->add(note);
                                    note->setTick(tick);

                                    if (useDrumset) {
                                          if (!drumset->isValid(mn->pitch())) {
                                                printf("unmapped drum note 0x%02x %d\n", mn->pitch(), mn->pitch());
                                                }
                                          else {
                                                chord->setStemDirection(drumset->stemDirection(mn->pitch()));
                                                }
                                          }

                                    if (n->ties[i]) {
                                          n->ties[i]->setEndNote(note);
                                          n->ties[i]->setStaffIdx(note->staffIdx());
                                          note->setTieBack(n->ties[i]);
                                          }
                                    }
                              if (n->mc->duration() <= len) {
                                    notes.removeAt(notes.indexOf(n));
                                    continue;
                                    }
                              for (int i = 0; i < nl.size(); ++i) {
                                    MidiNote* mn = nl[i];
                                    Note* note = chord->noteList()->find(mn->pitch());
            				n->ties[i] = new Tie(this);
                                    n->ties[i]->setStartNote(note);
      		      		note->setTieFor(n->ties[i]);
                                    }
      	                  n->mc->setOntime(n->mc->ontime() + len);
                              n->mc->setDuration(n->mc->duration() - len);
                              }
                        ctick += len;
                        }
                  //
                  // check for gap and fill with rest
                  //
                  int restLen = (*i)->ontime() - ctick;
                  if (voice == 0) {
                        while (restLen > 0) {
                              int len = restLen;
                  		Measure* measure = tick2measure(ctick);
                              // split rest on measure boundary
                              if ((ctick + len) > measure->tick() + measure->tickLen()) {
                                    len = measure->tick() + measure->tickLen() - ctick;
                                    }
                              Rest* rest = new Rest(this, ctick, len);
                              rest->setStaffIdx(staffIdx);
                              Segment* s = measure->getSegment(rest);
                              s->add(rest);
                              ctick   += len;
                              restLen -= len;
                              }
                        }
                  else
                        ctick += restLen;

                  //
                  // collect all notes on current
                  // tick position
                  //
                  for (;i != el.end(); ++i) {
                  	MidiEvent* e = *i;
                        if (e->type() != ME_CHORD)
                              continue;
                        if ((*i)->ontime() != ctick)
                              break;
                        MidiChord* mc = (MidiChord*)e;
                        if (mc->voice() != voice)
                              continue;
                  	MNote* n = new MNote(mc);
            	      notes.append(n);
                        }
                  }

            //
      	// process pending notes
            //
            if (!notes.isEmpty()) {
                  int tick = notes[0]->mc->ontime();
            	Measure* measure = tick2measure(tick);
                  Chord* chord = new Chord(this);
                  chord->setTick(tick);
                  chord->setStaffIdx(staffIdx);
                  Segment* s = measure->getSegment(chord);
                  s->add(chord);
                  int len = 0x7fffffff;      // MAXINT;
            	foreach (MNote* n, notes) {
                  	if (n->mc->duration() < len)
                              len = n->mc->duration();
                        }
                  chord->setTickLen(len);
            	foreach (MNote* n, notes) {
                        foreach(MidiNote* mn, n->mc->notes()) {
                  		Note* note = new Note(this);
                              note->setPitch(mn->pitch());
            	      	note->setStaffIdx(staffIdx);
                              note->setTick(tick);
                              note->setTpc(mn->tpc());
                              note->setVoice(voice);
            	      	chord->add(note);
                              }
                        n->mc->setDuration(n->mc->duration() - len);
                        if (n->mc->duration() <= 0) {
                              notes.removeAt(notes.indexOf(n));
                              delete n;
                              }
                        else
                              n->mc->setOntime(n->mc->ontime() + len);
                        }
                  ctick += len;

                  //
                  // check for gap and fill with rest
                  //
                  int restLen = measure->tick() + measure->tickLen() - ctick;
                  if (restLen > 0 && voice > 0) {
                        Rest* rest = new Rest(this, ctick, restLen);
            		Measure* measure = tick2measure(ctick);
                        rest->setStaffIdx(staffIdx);
                        Segment* s = measure->getSegment(rest);
                        s->add(rest);
                        }
                  }
            }
#if 0
      bool keyFound = false;

//      Measure* measure = tick2measure(0);
      for (ciEvent i = el.begin(); i != el.end(); ++i) {
            MidiEvent* e = *i;
            if (e->type() == ME_META) {
                  MidiMeta* mm = (MidiMeta*)e;
                  switch(mm->metaType()) {
                        case META_LYRIC:
                              {
      		            Measure* measure = tick2measure(mm->ontime());
                              Segment* seg = measure->findSegment(Segment::SegChordRest, e->ontime());
                              if (seg == 0) {
                                    for (seg = measure->first(); seg;) {
                                          if (seg->subtype() != Segment::SegChordRest) {
                                                seg = seg->next();
                                                continue;
                                                }
                                          Segment* ns;
                                          for (ns = seg->next(); ns && ns->subtype() != Segment::SegChordRest; ns = ns->next())
                                                ;
                                          if (ns == 0 || ns->tick() > e->ontime())
                                                break;
                                          seg = ns;
                                          }
                                    }
                              if (seg == 0) {
                                    printf("no segment found for lyrics<%s> at tick %d\n",
                                       mm->data(), e->ontime());
                                    break;
                                    }
                              Lyrics* l = new Lyrics(this);
                              QString txt((char*)(mm->data()));
                              l->setText(txt);
                              l->setTick(seg->tick());
                              seg->setLyrics(staffIdx, l);
                              }
                              break;
                        case META_TEMPO:
                              break;

                        case META_KEY_SIGNATURE:
                              {
                              unsigned char* data = mm->data();
                              int key = (char)data[0];
                              if (key < -7 || key > 7) {
                                    printf("ImportMidi: illegal key %d\n", key);
                                    break;
                                    }
                              (*cstaff->keymap())[e->ontime()] = key;
                              keyFound = false;
                              }
                              break;
                        case META_COMPOSER:     // mscore extension
                        case META_POET:
                        case META_TRANSLATOR:
                        case META_SUBTITLE:
                        case META_TITLE:
                              {
                              Text* text = new Text(this);
                              switch(mm->metaType()) {
                                    case META_COMPOSER:
                                          text->setSubtype(TEXT_COMPOSER);
                                          break;
                                    case META_TRANSLATOR:
                                          text->setSubtype(TEXT_TRANSLATOR);
                                          break;
                                    case META_POET:
                                          text->setSubtype(TEXT_POET);
                                          break;
                                    case META_SUBTITLE:
                                          text->setSubtype(TEXT_SUBTITLE);
                                          break;
                                    case META_TITLE:
                                          text->setSubtype(TEXT_TITLE);
                                          break;
                                    }

                              text->setText((char*)(mm->data()));

                              ScoreLayout* layout = mainLayout();
                              MeasureBase* measure = layout->first();
                              if (measure->type() != VBOX) {
                                    measure = new VBox(this);
                                    measure->setTick(0);
                                    measure->setNext(layout->first());
                                    layout->add(measure);
                                    }
                              measure->add(text);
                              }
                              break;
                        }
                  }
            }

      if (!keyFound && !midiTrack->isDrumTrack()) {
            (*cstaff->keymap())[0] = key;
            }
#endif
      }

//---------------------------------------------------------
//   quantize
//    process one segment (measure)
//---------------------------------------------------------

void BBTrack::quantize(int startTick, int endTick, EventList* dst)
      {
      int mintick = division * 64;
      iEvent i = _events.begin();
      for (; i != _events.end(); ++i) {
            if ((*i)->ontime() >= startTick)
                  break;
            }
      iEvent si = i;
      for (; i != _events.end(); ++i) {
            MidiEvent* e = *i;
            if (e->ontime() >= endTick)
                  break;
            if (e->type() == ME_NOTE && (((MidiNote*)e)->duration() < mintick))
                  mintick = ((MidiNote*)e)->duration();
            }
      if (mintick <= division / 16)        // minimum duration is 1/64
            mintick = division / 16;
      else if (mintick <= division / 8)
            mintick = division / 8;
      else if (mintick <= division / 4)
            mintick = division / 4;
      else if (mintick <= division / 2)
            mintick = division / 2;
      else if (mintick <= division)
            mintick = division;
      else if (mintick <= division * 2)
            mintick = division * 2;
      else if (mintick <= division * 4)
            mintick = division * 4;
      else if (mintick <= division * 8)
            mintick = division * 8;
      int raster;
      if (mintick > division)
            raster = division;
      else
            raster = mintick;

      for (iEvent i = si; i != _events.end(); ++i) {
            MidiEvent* e = *i;
            if (e->ontime() >= endTick)
                  break;
            if (e->type() == ME_NOTE) {
                  MidiNote* note = (MidiNote*)e;
	            int len  = quantizeLen(division, note->duration(), raster);
      	      int tick = (note->ontime() / raster) * raster;
	            note->setOntime(tick);
      	      note->setDuration(len);
                  }
            dst->insert(e);
            }
      }

//---------------------------------------------------------
//   cleanup
//    - quantize
//    - remove overlaps
//---------------------------------------------------------

void BBTrack::cleanup()
	{
      EventList dl;

      //
      //	quantize
      //
      int lastTick = 0;
      foreach (MidiEvent* e, _events) {
            if (e->type() != ME_NOTE)
                  continue;
            int offtime  = ((MidiNote*)e)->offtime();
            if (offtime > lastTick)
                  lastTick = offtime;
            }
      int startTick = 0;
      for (int i = 1;; ++i) {
            int endTick = bb->siglist().bar2tick(i, 0, 0);
            quantize(startTick, endTick, &dl);
            if (endTick > lastTick)
                  break;
            startTick = endTick;
            }

      //
      //
      //
      _events.clear();

      for(iEvent i = dl.begin(); i != dl.end(); ++i) {
            MidiEvent* e = *i;
            if (e->type() == ME_NOTE) {
                  iEvent ii = i;
                  ++ii;
                  for (; ii != dl.end(); ++ii) {
                        MidiEvent* ee = *ii;
                        if (ee->type() != ME_NOTE || ((MidiNote*)ee)->pitch() != ((MidiNote*)e)->pitch())
                              continue;
                        if (ee->ontime() >= e->ontime() + ((MidiNote*)e)->duration())
                              break;
                        ((MidiNote*)e)->setDuration(ee->ontime() - e->ontime());
                        break;
                        }
                  if (((MidiNote*)e)->duration() <= 0)
                        continue;
                  }
		_events.insert(e);
            }
      }

//---------------------------------------------------------
//   findChords
//---------------------------------------------------------

void BBTrack::findChords()
      {
      EventList dl;
      int n = _events.size();

      Drumset* drumset;
      if (_drumTrack)
            drumset = smDrumset;
      else
            drumset = 0;
      int jitter = 3;   // tick tolerance for note on/off

      for (int i = 0; i < n; ++i) {
            MidiEvent* e = _events[i];
            if (e == 0)
                  continue;
            if (e->type() != ME_NOTE) {
                  dl.append(e);
                  continue;
                  }

            MidiNote* note   = (MidiNote*)e;
            int ontime       = note->ontime();
            int offtime      = note->offtime();
            MidiChord* chord = new MidiChord();
            chord->setOntime(ontime);
            chord->setDuration(note->duration());
            chord->notes().append(note);
            int voice = 0;
            chord->setVoice(voice);
            dl.append(chord);
            _events[i] = 0;

            bool useDrumset = false;
            if (drumset) {
                  int pitch = note->pitch();
                  if (drumset->isValid(pitch)) {
                        useDrumset = true;
                        voice = drumset->voice(pitch);
                        chord->setVoice(voice);
                        }
                  }
            for (int k = i + 1; k < n; ++k) {
                  if (_events[k] == 0 || _events[k]->type() != ME_NOTE)
                        continue;
                  MidiNote* nn = (MidiNote*)_events[k];
                  if (nn->ontime() - jitter > ontime)
                        break;
                  if (qAbs(nn->ontime() - ontime) > jitter || qAbs(nn->offtime() - offtime) > jitter)
                        continue;
                  int pitch = nn->pitch();
                  if (useDrumset) {
                        if (drumset->isValid(pitch) && drumset->voice(pitch) == voice) {
                              chord->notes().append(nn);
                              _events[k] = 0;
                              }
                        }
                  else {
                        chord->notes().append(nn);
                        _events[k] = 0;
                        }
                  }
            }
      _events = dl;
      }


//---------------------------------------------------------
//   separateVoices
//---------------------------------------------------------

int BBTrack::separateVoices(int /*maxVoices*/)
      {
      return 1;
      }

