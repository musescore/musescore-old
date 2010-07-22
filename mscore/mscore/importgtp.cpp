//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer
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

#include "importgtp.h"
#include "score.h"
#include "measurebase.h"
#include "text.h"
#include "box.h"
#include "staff.h"
#include "part.h"
#include "measure.h"
#include "timesig.h"
#include "rest.h"
#include "chord.h"
#include "note.h"
#include "tablature.h"
#include "clef.h"
#include "lyrics.h"
#include "tempotext.h"
#include "slur.h"
#include "tuplet.h"

//---------------------------------------------------------
//   errmsg
//---------------------------------------------------------

const char* GuitarPro::errmsg[] = {
      "no error",
      "unknown file format",
      "unexpected end of file",
      "bad number of strings",
      };

//---------------------------------------------------------
//   GuitarPro
//---------------------------------------------------------

GuitarPro::GuitarPro(Score* s)
      {
      score = s;
      }

GuitarPro::~GuitarPro()
      {
      }

//---------------------------------------------------------
//   skip
//---------------------------------------------------------

void GuitarPro::skip(qint64 len)
      {
      char c;
      while (len--)
            read(&c, 1);
      }

//---------------------------------------------------------
//    read
//---------------------------------------------------------

void GuitarPro::read(void* p, qint64 len)
      {
      if (len == 0)
            return;
      qint64 rv = f->read((char*)p, len);
      if (rv != len)
            throw GP_EOF;
      curPos += len;
      }

//---------------------------------------------------------
//   readChar
//---------------------------------------------------------

int GuitarPro::readChar()
      {
      char c;
      read(&c, 1);
      return c;
      }

//---------------------------------------------------------
//   readUChar
//---------------------------------------------------------

int GuitarPro::readUChar()
      {
      uchar c;
      read(&c, 1);
      return c;
      }

//---------------------------------------------------------
//   readPascalString
//---------------------------------------------------------

QString GuitarPro::readPascalString(int n)
      {
      uchar l = readUChar();
      char s[l + 1];
      read(s, l);
      s[l] = 0;
      skip(n - l);
      return QString(s);
      }

//---------------------------------------------------------
//   readWordPascalString
//---------------------------------------------------------

QString GuitarPro::readWordPascalString()
      {
      int l = readInt();
      char c[l+1];
      read(c, l);
      c[l] = 0;
      return QString::fromLocal8Bit(c);
      }

//---------------------------------------------------------
//   readInt
//---------------------------------------------------------

int GuitarPro::readInt()
      {
      uchar x;
      read(&x, 1);
      int r = x;
      read(&x, 1);
      r += x << 8;
      read(&x, 1);
      r += x << 16;
      read(&x, 1);
      r += x << 24;
      return r;
      }

//---------------------------------------------------------
//   readDelphiString
//---------------------------------------------------------

QString GuitarPro::readDelphiString()
      {
      int maxl = readInt();
      uchar l = readUChar();
      if (maxl != l + 1)
            printf("readDelphiString: first word doesn't match second byte");
      char c[l + 1];
      read(c, l);
      c[l] = 0;
      return QString::fromLocal8Bit(c);
      }

//---------------------------------------------------------
//   readChromaticGraph
//    bend graph
//---------------------------------------------------------

void GuitarPro::readChromaticGraph()
      {
printf("readChromaticGraph()\n");

      readUChar();                        // icon
      readInt();                          // shown aplitude
      int n = readInt();
      for (int i = 0; i < n; ++i) {
            readInt();                    // time
            readInt();                    // pitch
            readUChar();                  // vibrato
            }
      }

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

void GuitarPro::readNote(int string, Note* note)
      {
      uchar noteBits = readUChar();

      bool tieNote = false;
      uchar variant = 1;
      if (noteBits & 0x20) {
            variant = readUChar();
            if (variant == 1) {     // normal note
                  }
            else if (variant == 2)
                  tieNote = true;
            else if (variant == 3) {                 // dead notes
                  //printf("DeathNote tick %d pitch %d\n", note->chord()->segment()->tick(), note->pitch());
                  }
            else
                  printf("unknown note variant: %d\n", variant);
            }

      if (noteBits & 0x1) {               // note != beat
            readUChar();                  // length
            readUChar();                  // t
            }
      if (noteBits & 0x2) {               // note is dotted
            }

      if (noteBits & 0x10) {
            int d = readUChar();                  // dynamic
            printf("Dynamic=========%d\n", d);
            }
      int fretNumber = -1;
      if (noteBits & 0x20)
            fretNumber = readUChar();

      if (noteBits & 0x80) {              // fingering
            readUChar();
            readUChar();
            }
      if (noteBits & 0x8) {
            uchar modMask1 = readUChar();
            uchar modMask2;
            if (version >= 400)
                  modMask2 = readUChar();
            if (modMask1 & 0x1)
                  readChromaticGraph();
            if (modMask1 & 0x2) {         // hammer on / pull off
                  }
            if (modMask1 & 0x8) {         // let ring
                  }
            if (modMask1 & 0x10) {
                  readUChar();            // grace fret
                  readUChar();            // grace dynamic
                  readUChar();            // grace transition
                  readUChar();            // grace length
                  }
            if (version >= 400) {
                  if (modMask2 & 0x1) {   // staccato - palm mute
                        }
                  if (modMask2 & 0x2) {   // palm mute - mute the whole column
                        }
                  if (modMask2 & 0x4)     // tremolo picking length
                        readUChar();
                  if (modMask2 & 0x8)
                        readUChar();      // slide kind
                  if (modMask2 & 0x10)
                        readUChar();      // harmonic kind
                  if (modMask2 & 0x20) {
                        readUChar();      // trill fret
                        readUChar();      // trill length
                        }
                  }
            }
      if (fretNumber == -1) {
            printf("Note: no fret number, tie %d\n", tieNote);
            }
      Staff* staff = note->staff();
      if (fretNumber == 255) {
            fretNumber = 0;
            note->setHeadGroup(1);
            note->setGhost(true);
            }
      int pitch = staff->part()->instr()->tablature()->getPitch(string, fretNumber);
// printf("pitch %d  string %d fret %d\n", pitch, string, fretNumber);
      note->setFret(fretNumber);
      note->setString(string);
      note->setPitch(pitch);

      if (tieNote) {
            bool found = false;
            Chord* chord     = note->chord();
            Segment* segment = chord->segment()->prev1(SegChordRest);
            int track        = note->track();
            while (segment) {
                  Element* e = segment->element(track);
                  if (e) {
                        if (e->type() == CHORD) {
                              Chord* chord2 = static_cast<Chord*>(e);
                              foreach(Note* note2, chord2->notes()) {
                                    if (note2->string() == string) {
                                          Tie* tie = new Tie(score);
                                          tie->setEndNote(note);
                                          note2->add(tie);
                                          note->setFret(note2->fret());
                                          note->setPitch(note2->pitch());
                                          found = true;
                                          break;
                                          }
                                    }
                              }
                        if (found)
                              break;
                        }
                  segment = segment->prev1(SegChordRest);
                  }
            if (!found)
                  printf("tied note not found, pitch %d fret %d string %d\n", note->pitch(), note->fret(), note->string());
            }
      }

//---------------------------------------------------------
//   readMixChange
//---------------------------------------------------------

void GuitarPro::readMixChange()
      {
      char patch   = readChar();
      char volume  = readChar();
      char pan     = readChar();
      char chorus  = readChar();
      char reverb  = readChar();
      char phase   = readChar();
      char tremolo = readChar();
      int tempo    = readInt();

      if (volume >= 0)
            readChar();
      if (pan >= 0)
            readChar();
      if (chorus >= 0)
            readChar();
      if (reverb >= 0)
            readChar();
      if (tremolo >= 0)
            readChar();
      if (tempo >= 0)
            readChar();
      if (version >= 400)
            readChar();       // bitmask: what should be applied to all tracks
      }

//---------------------------------------------------------
//   readColumnEffects
//---------------------------------------------------------

void GuitarPro::readColumnEffects()
      {
      uchar fxBits1 = readUChar();
      uchar fxBits2 = 0;
      if (version >= 400)
            fxBits2 = readUChar();
      if (fxBits1 & 0x20) {
            uchar num = readUChar();
            switch(num) {
                  case 0:
                        if (version < 400)
                              readInt();
                        break;
                  case 1:
                        if (version < 400)
                              readInt();
                        break;
                  case 2:
                        if (version < 400)
                              readInt();
                        break;
                  case 3:
                        if (version < 400)
                              readInt();
                        break;
                  default:
                        break;
                  }
            }
      if (fxBits2 & 0x04)
            readChromaticGraph();
      if (fxBits1 & 0x40) {
            readUChar();            // down stroke length
            readUChar();            // up stroke length
            }
      if (fxBits1 & 0x02)
            readUChar();            // stroke pick direction
      if (fxBits1 & 0x01) {         // GP3 column-wide vibrato
            }
      if (fxBits1 & 0x2) {          // GP3 column-wide wide vibrato (="tremolo" in GP3)
            }
      }


//---------------------------------------------------------
//   readChordDiagram
//---------------------------------------------------------

void GuitarPro::readChordDiagram(Segment*)
      {
      int header = readUChar();

//      printf("read chord diagram %x\n", header);

      if ((header & 1) == 0) {
            printf("no version4 chord diagram\n");
            abort();
            }
      if (version >= 400) {
            readChar(); // sharp or flat
            skip(3);
            readChar();             // root -1 - custom, 0 - C, 1 - C#...
            readChar();             // chord type
            readChar();             // chord goes until ninth, eleventh, or thirteenth
            readInt();              // lowest note of chord. It gives the chord inversions.
            readInt();              // tonality linked with 9/11/13:  0:perfect, 1:augmented, 2:diminished
            readChar();             // allows to determine if a 'add' (added note) is present in the chord
            readPascalString(20);   // chord name
            skip(2);
            readChar();
            readChar();
            readChar();
            readInt();              // first fret
            for (int i = 0; i < 7; ++i)
                  readInt();
            readChar();       // number of barres
            for (int i = 0; i < 5; ++i)
                  readChar();
            for (int i = 0; i < 5; ++i)
                  readChar();
            for (int i = 0; i < 5; ++i)
                  readChar();
            readChar();
            readChar();
            readChar();
            readChar();
            readChar();
            readChar();
            readChar();
            skip(1);
            for (int i = 0; i < 7; ++i)
                  readChar();
            readChar();
            }
      else {
            skip(25);
            readPascalString(34);
            int firstFret = readInt();
            for (int i = 0; i < 6; ++i) {
                  int fret = readInt();
                  }
            skip(36);
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void GuitarPro::read(QFile* fp)
      {
      f      = fp;
      curPos = 0;
      QString s = readPascalString(30);
      printf("read <%s>\n", qPrintable(s));

      if (!s.startsWith("FICHIER GUITAR PRO "))
            throw GP_UNKNOWN_FORMAT;
      s = s.mid(20);

      int a = s.left(1).toInt();
      int b = s.mid(1).toInt();
      version = a * 100 + b;
      if (version != 101 && version != 102 && version != 103 && version != 104
         && version != 220 && version != 221 && version != 300
         && version != 400 && version != 406)
            throw GP_UNKNOWN_FORMAT;
printf("found GTP format version %d\n", version);

      title        = readDelphiString();
      subtitle     = readDelphiString();
      artist       = readDelphiString();
      album        = readDelphiString();
      composer     = readDelphiString();
      QString copyright = readDelphiString();
      score->setCopyright(QString("Copyright %1\nAll Rights Reserved - International Copyright Secured").arg(copyright));

      transcriber  = readDelphiString();
      instructions = readDelphiString();
      int n = readInt();
      for (int i = 0; i < n; ++i)
            comments.append(readDelphiString());
      uchar num;
      read(&num, 1);    // Shuffle rhythm feel
      if (version >= 400) {
            readInt();
            for (int i = 0; i < GP_MAX_LYRIC_LINES; ++i) {
                  readInt();
                  readWordPascalString();
                  }
            }
      int tempo = readInt();
      int key = 0;
      int octave = 0;
      if (version >= 400) {
            key = readChar();
            octave = readInt();    // octave
            }
      else {
            key = readInt();    // key
            }

printf("key %d octave %d\n", key, octave);

      for (int i = 0; i < GP_MAX_TRACK_NUMBER * 2; ++i) {
            channelDefaults[i].patch   = readInt();
            channelDefaults[i].volume  = readUChar() * 8 - 1;
            channelDefaults[i].pan     = readUChar() * 8 - 1;
            channelDefaults[i].chorus  = readUChar() * 8 - 1;
            channelDefaults[i].reverb  = readUChar() * 8 - 1;
            channelDefaults[i].phase   = readUChar() * 8 - 1;
            channelDefaults[i].tremolo = readUChar() * 8 - 1;
            readUChar();      // padding
            readUChar();
//            printf("Channel %d patch vol %d pan %d\n", i,
//               channelDefaults[i].patch,
//               channelDefaults[i].volume,
//               channelDefaults[i].pan);
            }
      numBars   = readInt();
      numTracks = readInt();

      int tnumerator   = 4;
      int tdenominator = 4;
      for (int i = 0; i < numBars; ++i) {
            GpBar bar;
            uchar barBits = readUChar();
            if (barBits & 0x1)
                  tnumerator = readUChar();
            if (barBits & 0x2)
                  tdenominator = readUChar();
            if (barBits & 0x4) {                // begin reapeat
printf("BeginRepeat=============================================\n");
                  }
            if (barBits & 0x8)                  // number of repeats
                  uchar c = readUChar();
            if (barBits & 0x10)                 // alternative ending to
                  uchar c = readUChar();
            if (barBits & 0x20) {
                  bar.marker = readDelphiString();     // new section?
                  int color = readInt();    // color?
                  }
            if (barBits & 0x40) {
                  bar.keysig = readUChar();
                  uchar c    = readUChar();        // minor
                  }
            if (barBits & 0x80) {
printf("doubleBar=============================================\n");
                  // double bar
                  }
            bar.timesig = Fraction(tnumerator, tdenominator);
            bars.append(bar);
            }

      //
      // create a part for every staff
      //
      for (int staffIdx = 0; staffIdx < numTracks; ++staffIdx) {
            Part* part = new Part(score);
            Staff* s = new Staff(score, part, staffIdx);
            part->insertStaff(s);
            score->staves().push_back(s);
            score->appendPart(part);
            }

      int tick = 0;
      Fraction ts;
      for (int i = 0; i < numBars; ++i) {
            Fraction nts = bars[i].timesig;
            Measure* m = new Measure(score);
            m->setTick(tick);
            m->setTimesig(nts);
            m->setLen(nts);

            if (i == 0 || ts != nts) {
                  for (int staffIdx = 0; staffIdx < numTracks; ++staffIdx) {
                        TimeSig* t = new TimeSig(score, nts);
                        t->setTrack(staffIdx * VOICES);
                        Segment* s = m->getSegment(SegTimeSig, tick);
                        s->add(t);
                        }
                  }

            score->add(m);
            tick += nts.ticks();
            ts = nts;
            }

      for (int i = 0; i < numTracks; ++i) {
            int tuning[GP_MAX_STRING_NUMBER];

            uchar c      = readUChar();   // simulations bitmask
            if (c & 0x2) {                // 12 stringed guitar
                  }
            if (c & 0x4) {                // banjo track
                  }
            QString name = readPascalString(40);
            int strings  = readInt();
            if (strings <= 0 || strings > GP_MAX_STRING_NUMBER)
                  throw GP_BAD_NUMBER_OF_STRINGS ;
            for (int j = 0; j < strings; ++j)
                  tuning[j] = readInt();
            for (int j = strings; j < GP_MAX_STRING_NUMBER; ++j)
                  readInt();
            int midiPort     = readInt() - 1;
            int midiChannel  = readInt() - 1;
            int midiChannel2 = readInt() - 1;
            int frets        = readInt();
            int capo         = readInt();
            int color        = readInt();

            int tuning2[strings];
            for (int k = 0; k < strings; ++k)
                  tuning2[strings-k-1] = tuning[k];
            Tablature* tab = new Tablature(frets, strings, tuning2);
            Part* part = score->part(i);
            Instrument* instr = part->instr();
            instr->setTablature(tab);
            instr->setTrackName(name);

            //
            // determine clef
            //
            Staff* staff = score->staff(i);
            int patch = channelDefaults[midiChannel].patch;
            int clefId = CLEF_G;
            if (c & 0x1) {
                  clefId = CLEF_PERC;
                  instr->setUseDrumset(true);
                  }
            else if (patch >= 24 && patch < 32)
                  clefId = CLEF_G3;
            else if (patch >= 32 && patch < 40)
                  clefId = CLEF_F8;
            Measure* measure = score->firstMeasure();
            Clef* clef = new Clef(score, clefId);
            clef->setTrack(i * VOICES);
            Segment* segment = measure->getSegment(SegClef, 0);
            segment->add(clef);


            Channel& ch = instr->channel(0);
            if (c & 1) {
                  ch.program = 0;
                  ch.bank    = 128;
                  }
            else {
                  ch.program = patch;
                  ch.bank    = 0;
                  }
            ch.volume  = channelDefaults[midiChannel].volume;
            ch.pan     = channelDefaults[midiChannel].pan;
            ch.chorus  = channelDefaults[midiChannel].chorus;
            ch.reverb  = channelDefaults[midiChannel].reverb;
            // missing: phase, tremolo
            ch.updateInitList();
            }

      Measure* measure = score->firstMeasure();
      for (int bar = 0; bar < numBars; ++bar, measure = measure->nextMeasure()) {
            const GpBar& gpbar = bars[bar];

            if (!gpbar.marker.isEmpty()) {
                  Text* s = new Text(score);
                  s->setText(gpbar.marker.trimmed());
                  s->setTrack(0);
                  s->setSubtype(TEXT_REHEARSAL_MARK);
                  s->setTextStyle(TEXT_STYLE_REHEARSAL_MARK);
                  Segment* segment = measure->getSegment(SegChordRest, measure->tick());
                  segment->add(s);
                  }

            Tuplet* tuplets[numTracks];
            for (int staffIdx = 0; staffIdx < numTracks; ++staffIdx)
                  tuplets[staffIdx] = 0;

            for (int staffIdx = 0; staffIdx < numTracks; ++staffIdx) {
                  int tick  = measure->tick();
                  int beats = readInt();
// printf("bar %d beats %d\n", bar, beats);
                  for (int beat = 0; beat < beats; ++beat) {
                        int pause = 0;
                        uchar beatBits = readUChar();
// printf("beat bits %02x\n", beatBits);
                        bool dotted = beatBits & 0x1;
                        if (beatBits & 0x40)
                              pause = readUChar();
                        int len = readChar();
                        int tuple = 0;
                        if (beatBits & 0x20)
                              tuple = readInt();
                        Segment* segment = measure->getSegment(SegChordRest, tick);
                        if (beatBits & 0x2)
                              readChordDiagram(segment);
                        if (beatBits & 0x4) {
                              QString txt = readDelphiString();
                              Lyrics* l = new Lyrics(score);
                              l->setText(txt);
                              l->setTrack(staffIdx * VOICES);
                              segment->add(l);
                              }
                        if (beatBits & 0x8)
                              readColumnEffects();
                        if (beatBits & 0x10)
                              readMixChange();
                        int strings = readUChar();   // used strings mask

                        Fraction l;
                        switch(len) {
                              case -2: l.set(1, 1);    break;
                              case -1: l.set(1, 2);    break;
                              case  0: l.set(1, 4);    break;
                              case  1: l.set(1, 8);    break;
                              case  2: l.set(1, 16);   break;
                              case  3: l.set(1, 32);   break;
                              case  4: l.set(1, 64);   break;
                              case  5: l.set(1, 128);  break;
                              //case  6: l.set(1, 512);  break;
                              //case  7: l.set(1, 1024);  break;
                              //case  8: l.set(1, 2048);  break;
                              default:
                                    printf("unknown beat len: %d\n", len);
                                    abort();
                              }

                        ChordRest* cr;
                        if (pause || (strings == 0))
                              cr = new Rest(score);
                        else
                              cr = new Chord(score);
                        cr->setTrack(staffIdx * VOICES);
                        if (tuple) {
// printf("%d: Tuplet note beat %d  tuplet %d  len %s\n", tick, beat, tuple, qPrintable(l.print()));

                              Tuplet* tuplet = tuplets[staffIdx];
                              if ((tuplet == 0) || (tuplet->elements().size() == tuple)) {
                                    tuplet = new Tuplet(score);
                                    tuplets[staffIdx] = tuplet;
                                    measure->add(tuplet);
                                    }
                              tuplet->setTrack(staffIdx * VOICES);
                              switch(tuple) {
                                    case 3:
                                          tuplet->setRatio(Fraction(3,2));
                                          tuplet->setBaseLen(l);
                                          break;
                                    default:
                                          printf("unsupported tuplet %d\n", tuple);
                                          abort();
                                    }
                              cr->setTuplet(tuplet);
                              }

                        Duration d(l);
                        d.setDots(dotted ? 1 : 0);

                        if (dotted)
                              l = l + (l/2);
                        cr->setDuration(l);
                        cr->setDurationType(d);
                        segment->add(cr);
// printf("%d: add cr %p <%s>\n", tick, cr, qPrintable(l.print()));

                        Staff* staff = cr->staff();
                        int numStrings = staff->part()->instr()->tablature()->strings();
                        for (int i = 6; i >= 0; --i) {
                              if (strings & (1 << i) && ((6-i) < numStrings)) {
                                    Note* note = new Note(score);
                                    static_cast<Chord*>(cr)->add(note);

                                    readNote(6-i, note);
                                    note->setTpcFromPitch();
                                    }
                              }
                        tick += cr->ticks();
                        }
                  }
            }
      TempoText* tt = new TempoText(score);
      tt->setTempo(double(tempo)/60.0);
      tt->setText(QString("%1%2 = %3").arg(QChar(QChar::highSurrogate(0x1d15f))).arg(QChar(QChar::lowSurrogate(0x1d15f))).arg(tempo));
      tt->setTrack(0);
      measure = score->firstMeasure();
      Segment* segment = measure->getSegment(SegChordRest, 0);
      segment->add(tt);
      }

//---------------------------------------------------------
//   importGTP
//---------------------------------------------------------

bool Score::importGTP(const QString& name)
      {
      if (name.isEmpty())
            return false;
      QFile fp(name);
      if (!fp.open(QIODevice::ReadOnly))
            return false;

      GuitarPro gp(this);
      try {
            gp.read(&fp);
            }
      catch(GuitarPro::GuitarProError errNo) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: Import GuitarPro"),
               QString("Load failed: ") + gp.error(errNo),
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            fp.close();
            // avoid another error message box
            return true;
            }
      fp.close();

      MeasureBase* m;
      if (!_measures.first()) {
            m = new VBox(this);
            m->setTick(0);
            addMeasure(m);
            }
      else  {
            m = _measures.first();
            if (m->type() != VBOX) {
                  m = new VBox(this);
                  m->setTick(0);
                  addMeasure(m);
                  }
            }
      if (!gp.title.isEmpty()) {
            Text* s = new Text(this);
            s->setSubtype(TEXT_TITLE);
            s->setTextStyle(TEXT_STYLE_TITLE);
            s->setText(gp.title);
            m->add(s);
            }
      if (!gp.subtitle.isEmpty()) {
            Text* s = new Text(this);
            s->setSubtype(TEXT_SUBTITLE);
            s->setTextStyle(TEXT_STYLE_SUBTITLE);
            s->setText(gp.subtitle);
            m->add(s);
            }
      if (!gp.composer.isEmpty()) {
            Text* s = new Text(this);
            s->setSubtype(TEXT_COMPOSER);
            s->setTextStyle(TEXT_STYLE_COMPOSER);
            s->setText(gp.composer);
            m->add(s);
            }
      if (!gp.artist.isEmpty()) {
            Text* s = new Text(this);
            s->setSubtype(TEXT_POET);
            s->setTextStyle(TEXT_STYLE_POET);
            s->setText(gp.artist);
            m->add(s);
            }

//      album
//      copyright

//      connectTies();


      _saved = false;
      _created = true;
      return true;
      }

