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
      uchar l;
      read(&l, 1);
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
      int l = readDelphiInteger();
      char c[l+1];
      read(c, l);
      c[l] = 0;
      return QString::fromLocal8Bit(c);
      }

//---------------------------------------------------------
//   readDelphiInteger
//---------------------------------------------------------

int GuitarPro::readDelphiInteger()
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
      int maxl = readDelphiInteger();
      uchar l;
      read(&l, 1);
      if (maxl != l + 1)
            printf("readDelphiString: first word doesn't match second byte");
      char c[l + 5];
      read(c, l);
      c[l] = 0;

      printf("read string <%s>\n", c);
      return QString::fromLocal8Bit(c);
      }

//---------------------------------------------------------
//   readChromaticGraph
//    bend graph
//---------------------------------------------------------

void GuitarPro::readChromaticGraph()
      {
      printf("readChromaticGraph()\n");
      abort();
      }

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

void GuitarPro::readNote(int string, Note* note)
      {
      uchar noteBits = readUChar();
      uchar variant  = readUChar();

      if (noteBits & 0x1) {               // note != beat
            uchar num = readUChar();      // length
            num = readUChar();            // t
            }
      if (noteBits & 0x2) {               // note is dotted
            }

      if (noteBits & 0x10) {
            readUChar();                  // dynamic
            }
      int fretNumber = readUChar();
      if (variant == 2) {                  // link with previous beat
            }
      if (variant == 3) {                 // dead notes
            }
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
      Staff* staff = note->staff();
      int pitch = staff->part()->instr()->tablature()->getPitch(string, fretNumber);
printf("pitch %d  string %d fret %d\n", pitch, string, fretNumber);
      note->setPitch(pitch);
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
      copyright    = readDelphiString();
      transcriber  = readDelphiString();
      instructions = readDelphiString();
      int n = readDelphiInteger();
      for (int i = 0; i < n; ++i)
            comments.append(readDelphiString());
      uchar num;
      read(&num, 1);    // Shuffle rhythm feel
      if (version >= 400) {
            readDelphiInteger();
            for (int i = 0; i < GP_MAX_LYRIC_LINES; ++i) {
                  readDelphiInteger();
                  readWordPascalString();
                  }
            }
      int tempo = readDelphiInteger();
      printf("Tempo: %d\n", tempo);
      if (version >= 400) {
            uchar num;
            read(&num, 1);          // key
            readDelphiInteger();    // octave
            }
      else
            readDelphiInteger();    // key

      for (int i = 0; i < GP_MAX_TRACK_NUMBER * 2; ++i) {
            trackDefaults[i].patch   = readDelphiInteger();
printf("patch==== %d %d\n", i, trackDefaults[i].patch);
            trackDefaults[i].volume  = readUChar();
            trackDefaults[i].pan     = readUChar();
            trackDefaults[i].chorus  = readUChar();
            trackDefaults[i].reverb  = readUChar();
            trackDefaults[i].phase   = readUChar();
            trackDefaults[i].tremolo = readUChar();
            uchar a, b;
            read(&a, 1);
            read(&b, 1);
            if (a != 0 || b != 0) {
                  printf("wrong byte padding\n");
                  }
            }
      numBars   = readDelphiInteger();
      numTracks = readDelphiInteger();

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
                  }
            if (barBits & 0x8)                  // number of repeats
                  uchar c = readUChar();
            if (barBits & 0x10)                 // alternative ending to
                  uchar c = readUChar();
            if (barBits & 0x20) {
                  QString s = readDelphiString();     // new section?
                  int color = readDelphiInteger();    // color?
                  }
            if (barBits & 0x40) {
                  bar.keysig = readUChar();
                  uchar c    = readUChar();        // minor
                  }
            if (barBits & 0x80) {
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
            if (c & 0x1) {                // drum track
printf("Drum Track\n");
                  }
            if (c & 0x2) {                // 12 stringed guitar
                  }
            if (c & 0x4) {                // banjo track
                  }
            QString name = readPascalString(40);
            int strings  = readDelphiInteger();
            if (strings <= 0 || strings > GP_MAX_STRING_NUMBER)
                  throw GP_BAD_NUMBER_OF_STRINGS ;
            for (int j = 0; j < strings; ++j)
                  tuning[j] = readDelphiInteger();
            for (int j = strings; j < GP_MAX_STRING_NUMBER; ++j)
                  readDelphiInteger();
            int midiPort     = readDelphiInteger();
            int midiChannel  = readDelphiInteger();
            int midiChannel2 = readDelphiInteger();
            int frets        = readDelphiInteger();
            int capo         = readDelphiInteger();
            int color        = readDelphiInteger();

            int tuning2[strings];
            for (int k = 0; k < strings; ++k)
                  tuning2[strings-k-1] = tuning[k];
            Tablature* tab = new Tablature(frets, strings, tuning2);
            Part* part = score->part(i);
            Instrument* instr = part->instr();
            instr->setTablature(tab);
            instr->setTrackName(name);
            Staff* staff = score->staff(i);
            int patch = trackDefaults[i * 2].patch;
            int clefId = CLEF_F;
            if (patch >= 24 && patch < 32)
                  clefId = CLEF_G3;
            else if (patch >= 32 && patch < 40)
                  clefId = CLEF_F8;
            Measure* measure = score->firstMeasure();
            Clef* clef = new Clef(score, clefId);
            clef->setTrack(i * VOICES);
            Segment* segment = measure->getSegment(SegClef, 0);
            segment->add(clef);
            }

      Measure* measure = score->firstMeasure();
      for (int bar = 0; bar < numBars; ++bar, measure = measure->nextMeasure()) {
            for (int staffIdx = 0; staffIdx < numTracks; ++staffIdx) {
                  int tick  = measure->tick();
                  int beats = readDelphiInteger();
printf("bar %d beats %d\n", bar, beats);
                  for (int beat = 0; beat < beats; ++beat) {
                        int pause = 0;
                        uchar beatBits = readUChar();
                        bool dotted = beatBits & 0x1;
                        if (beatBits & 0x40)
                              pause = readUChar();
                        int len = readChar();
                        if (beatBits & 0x20) {
                              int tuple = readDelphiInteger();
                              }
                        if (beatBits & 0x2) {
                              // readChordDiagram();
                              abort();
                              }
                        if (beatBits & 0x4) {
                              QString s = readDelphiString();
                              }
                        if (beatBits & 0x8) {
                              // readColumnEffects();
                              abort();
                              }
                        if (beatBits & 0x10) {
                              ;
                              if (version >= 400) {  // what should be applied to all tracks
                                    uchar num = readUChar();
                                    }
                              }
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
                              default:
                                    printf("unknown beat len\n");
                                    abort();
                              }

                        ChordRest* cr;
                        Segment* s = measure->getSegment(SegChordRest, tick);
                        if (pause)
                              cr = new Rest(score);
                        else
                              cr = new Chord(score);
                        cr->setTrack(staffIdx * VOICES);
                        Duration d(l);
                        d.setDots(dotted ? 1 : 0);

                        if (dotted)
                              l = l + (l/2);
                        cr->setDuration(l);
                        cr->setDurationType(d);
                        s->add(cr);
                        int strings = readUChar();   // used strings mask
                        for (int i = 0; i < GP_MAX_STRING_NUMBER; ++i) {
                              if (strings & (1 << i)) {
                                    Note* note = new Note(score);
                                    static_cast<Chord*>(cr)->add(note);

                                    Staff* staff = note->staff();
                                    int numStrings = staff->part()->instr()->tablature()->strings();
                                    int string = i - (GP_MAX_STRING_NUMBER - numStrings);
                                    string = numStrings - string - 1;
                                    readNote(string, note);
                                    note->setTpcFromPitch();
                                    }
                              }
                        tick += l.ticks();
                        }
                  }
            }
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

//      album        = readDelphiString();
//      copyright    = readDelphiString();


      _saved = false;
      _created = true;
      return true;
      }

