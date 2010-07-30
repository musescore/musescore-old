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
#include "barline.h"
#include "excerpt.h"
#include "stafftype.h"
#include "bracket.h"
#include "articulation.h"

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
//   GpBar
//---------------------------------------------------------

GpBar::GpBar()
      {
      barLine = NORMAL_BAR;
      keysig  = 0;
      timesig = Fraction(4,4);
      }

//---------------------------------------------------------
//   GuitarPro
//---------------------------------------------------------

GuitarPro::GuitarPro(Score* s, int v)
      {
      score   = s;
      version = v;
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
      if (rv != len) {
            abort();
            throw GP_EOF;
            }
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
printf("readPascalString <%s>\n", s);
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
printf("readWordPascalString <%s>\n", c);
      return QString::fromLocal8Bit(c);
      }

//---------------------------------------------------------
//   readBytePascalString
//---------------------------------------------------------

QString GuitarPro::readBytePascalString()
      {
      int l = readUChar();
      char c[l+1];
      read(c, l);
      c[l] = 0;
printf("readBytePascalString <%s>\n", c);
      return QString::fromLocal8Bit(c);
      }

//---------------------------------------------------------
//   readDelphiString
//---------------------------------------------------------

QString GuitarPro::readDelphiString()
      {
      int maxl = readInt();
      uchar l = readUChar();
      if (maxl != l + 1) {
            printf("readDelphiString: first word doesn't match second byte");
            abort();
            }
      char c[l + 1];
      read(c, l);
      c[l] = 0;
printf("readDelphiString <%s>\n", c);
      return QString::fromLatin1(c);
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
//   setTuplet
//---------------------------------------------------------

void GuitarPro::setTuplet(Tuplet* tuplet, int tuple)
      {
      switch(tuple) {
            case 3:
                  tuplet->setRatio(Fraction(3,2));
                  break;
            case 5:
                  tuplet->setRatio(Fraction(5,4));
                  break;
            case 6:
                  tuplet->setRatio(Fraction(6,4));
                  break;
            case 7:
                  tuplet->setRatio(Fraction(7,4));
                  break;
            case 9:
                  tuplet->setRatio(Fraction(9,8));
                  break;
            case 10:
                  tuplet->setRatio(Fraction(10,8));
                  break;
            case 11:
                  tuplet->setRatio(Fraction(11,8));
                  break;
            case 12:
                  tuplet->setRatio(Fraction(12,8));
                  break;
            default:
                  printf("unsupported tuplet %d\n", tuple);
                  abort();
            }
      }

//---------------------------------------------------------
//   addDynamic
//---------------------------------------------------------

void GuitarPro::addDynamic(Note* note, int d)
      {
      if (d == 8) {
            Articulation* a = new Articulation(note->score());
            a->setSubtype(SforzatoaccentSym);
            note->chord()->add(a);
            }
      else {
            printf("dynamic %d\n", d);
            }
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
//   readLyrics
//---------------------------------------------------------

void GuitarPro::readLyrics()
      {
      readInt();        // lyric track
      for (int i = 0; i < 5; ++i) {
            readInt();
            readWordPascalString();
            }
      }

//---------------------------------------------------------
//   readChannels
//---------------------------------------------------------

void GuitarPro::readChannels()
      {
      for (int i = 0; i < GP_MAX_TRACK_NUMBER * 2; ++i) {
            channelDefaults[i].patch   = readInt();
            channelDefaults[i].volume  = readUChar() * 8 - 1;
            channelDefaults[i].pan     = readUChar() * 8 - 1;
            channelDefaults[i].chorus  = readUChar() * 8 - 1;
            channelDefaults[i].reverb  = readUChar() * 8 - 1;
            channelDefaults[i].phase   = readUChar() * 8 - 1;
            channelDefaults[i].tremolo = readUChar() * 8 - 1;
            skip(2);
            }
      }

//---------------------------------------------------------
//   len2fraction
//---------------------------------------------------------

Fraction GuitarPro::len2fraction(int len)
      {
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
      return l;
      }

//---------------------------------------------------------
//   readMixChange
//---------------------------------------------------------

void GuitarPro::readMixChange()
      {
printf("readMixChange\n");
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
      if (phase >= 0)
            readChar();
      if (tremolo >= 0)
            readChar();
      if (tempo >= 0)
            readChar();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void GuitarPro1::read(QFile* fp)
      {
      f      = fp;
      curPos = 30;

      title  = readDelphiString();
      artist = readDelphiString();
      readDelphiString();

      int tempo = readInt();
      uchar num = readUChar();      // Shuffle rhythm feel

printf("Tempo %d\n", tempo);

      int octave = 0;
      int key    = 0;
      if (version > 102)
            key = readInt();    // key

printf("key %d octave %d\n", key, octave);

      staves  = version > 102 ? 8 : 1;

      int tnumerator   = 4;
      int tdenominator = 4;

      //
      // create a part for every staff
      //
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Part* part = new Part(score);
            Staff* s = new Staff(score, part, staffIdx);
            part->insertStaff(s);
            score->staves().push_back(s);
            score->appendPart(part);
            }

      int tick = 0;
      Fraction ts;

      for (int i = 0; i < staves; ++i) {
            int tuning[GP_MAX_STRING_NUMBER];

            int strings  = version > 101 ? readInt() : 6;
            for (int j = 0; j < strings; ++j)
                  tuning[j] = readInt();
            int tuning2[strings];
            for (int k = 0; k < strings; ++k)
                  tuning2[strings-k-1] = tuning[k];

            int frets = 32;   // TODO
            Tablature* tab = new Tablature(frets, strings, tuning2);
            Part* part = score->part(i);
            Instrument* instr = part->instr();
            instr->setTablature(tab);
            }

      measures = readInt();

      for (int i = 0; i < measures; ++i) {
            Fraction nts = bars[i].timesig;
            Measure* m = new Measure(score);
            m->setTick(tick);
            m->setTimesig(nts);
            m->setLen(nts);

            if (i == 0 || ts != nts) {
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
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

      Measure* measure = score->firstMeasure();
      for (int bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
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

            Tuplet* tuplets[staves];
            for (int staffIdx = 0; staffIdx < staves; ++staffIdx)
                  tuplets[staffIdx] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  int tick  = measure->tick();
                  int beats = readInt();
                  for (int beat = 0; beat < beats; ++beat) {
                        int pause = 0;
                        uchar beatBits = readUChar();
// printf("bar %d beat %d beat bits %02x\n", bar, beat, beatBits);
                        bool dotted = beatBits & 0x1;
                        if (beatBits & 0x40)
                              pause = readUChar();
                        int len = readChar();
                        int tuple = 0;
                        if (beatBits & 0x20)
                              tuple = readInt();
                        Segment* segment = measure->getSegment(SegChordRest, tick);
                        if (beatBits & 0x2)
                              readChord(segment);
                        if (beatBits & 0x4) {
                              QString txt = readDelphiString();
                              Lyrics* l = new Lyrics(score);
                              l->setText(txt);
                              l->setTrack(staffIdx * VOICES);
                              segment->add(l);
                              }
                        if (beatBits & 0x8)
                              readBeatEffects();
                        if (beatBits & 0x10)
                              readMixChange();
                        int strings = readUChar();   // used strings mask

                        Fraction l = len2fraction(len);
                        ChordRest* cr;
                        if (strings)
                              cr = new Chord(score);
                        else
                              cr = new Rest(score);
                        cr->setTrack(staffIdx * VOICES);
                        if (tuple) {
// printf("%d: Tuplet note beat %d  tuplet %d  len %s\n", tick, beat, tuple, qPrintable(l.print()));

                              Tuplet* tuplet = tuplets[staffIdx];
                              if ((tuplet == 0) || (tuplet->elements().size() == tuple)) {
                                    tuplet = new Tuplet(score);
                                    tuplet->setTrack(cr->track());
                                    tuplets[staffIdx] = tuplet;
                                    measure->add(tuplet);
                                    }
                              tuplet->setTrack(staffIdx * VOICES);
                              tuplet->setBaseLen(l);
                              setTuplet(tuplet, tuple);
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
      setTempo(tempo);
      }

//---------------------------------------------------------
//   setTempo
//---------------------------------------------------------

void GuitarPro::setTempo(int tempo)
      {
      TempoText* tt = new TempoText(score);
      tt->setTempo(double(tempo)/60.0);
      int uc = 0x1d15f;
      QChar h(QChar::highSurrogate(uc));
      QChar l(QChar::lowSurrogate(uc));
      tt->setText(QString("%1%2 = %3 ").arg(h).arg(l).arg(tempo));

      tt->setTrack(0);
      Measure* measure = score->firstMeasure();
      Segment* segment = measure->getSegment(SegChordRest, 0);
      segment->add(tt);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void GuitarPro2::read(QFile* fp)
      {
      f      = fp;
      curPos = 30;

      title        = readDelphiString();
      subtitle     = readDelphiString();
      artist       = readDelphiString();
      album        = readDelphiString();
      composer     = readDelphiString();
      QString copyright = readDelphiString();
      if (!copyright.isEmpty())
            score->setCopyright(QString("Copyright %1\nAll Rights Reserved - International Copyright Secured").arg(copyright));

      transcriber  = readDelphiString();
      instructions = readDelphiString();
      int n = readInt();
      for (int i = 0; i < n; ++i)
            comments.append(readDelphiString());

      uchar num = readUChar();      // Shuffle rhythm feel

      int tempo = readInt();
printf("Tempo %d\n", tempo);

      int octave = 0;
      int key = readInt();    // key

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
      measures   = readInt();
      staves = readInt();

      int tnumerator   = 4;
      int tdenominator = 4;
      for (int i = 0; i < measures; ++i) {
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
            if (barBits & 0x80)
                  bar.barLine = DOUBLE_BAR;
            bar.timesig = Fraction(tnumerator, tdenominator);
            bars.append(bar);
            }

      //
      // create a part for every staff
      //
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Part* part = new Part(score);
            Staff* s = new Staff(score, part, staffIdx);
            part->insertStaff(s);
            score->staves().push_back(s);
            score->appendPart(part);
            }

      int tick = 0;
      Fraction ts;
      for (int i = 0; i < measures; ++i) {
            Fraction nts = bars[i].timesig;
            Measure* m = new Measure(score);
            m->setTick(tick);
            m->setTimesig(nts);
            m->setLen(nts);

            if (i == 0 || ts != nts) {
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
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

      for (int i = 0; i < staves; ++i) {
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
                  staff->setStaffType(score->staffTypes().at(PERCUSSION_STAFF_TYPE));
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
      for (int bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
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

            Tuplet* tuplets[staves];
            for (int staffIdx = 0; staffIdx < staves; ++staffIdx)
                  tuplets[staffIdx] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  int tick  = measure->tick();
                  int beats = readInt();
                  for (int beat = 0; beat < beats; ++beat) {
                        int pause = 0;
                        uchar beatBits = readUChar();
// printf("bar %d beat %d beat bits %02x\n", bar, beat, beatBits);
                        bool dotted = beatBits & 0x1;
                        if (beatBits & 0x40)
                              pause = readUChar();
                        int len = readChar();
                        int tuple = 0;
                        if (beatBits & 0x20)
                              tuple = readInt();
                        Segment* segment = measure->getSegment(SegChordRest, tick);
                        if (beatBits & 0x2)
                              readChord(segment);
                        if (beatBits & 0x4) {
                              QString txt = readDelphiString();
                              Lyrics* l = new Lyrics(score);
                              l->setText(txt);
                              l->setTrack(staffIdx * VOICES);
                              segment->add(l);
                              }
                        if (beatBits & 0x8)
                              readBeatEffects();
                        if (beatBits & 0x10)
                              readMixChange();
                        int strings = readUChar();   // used strings mask

                        Fraction l = len2fraction(len);
                        ChordRest* cr;
                        if (strings)
                              cr = new Chord(score);
                        else
                              cr = new Rest(score);
                        cr->setTrack(staffIdx * VOICES);
                        if (tuple) {
// printf("%d: Tuplet note beat %d  tuplet %d  len %s\n", tick, beat, tuple, qPrintable(l.print()));

                              Tuplet* tuplet = tuplets[staffIdx];
                              if ((tuplet == 0) || (tuplet->elements().size() == tuple)) {
                                    tuplet = new Tuplet(score);
                                    tuplet->setTrack(cr->track());
                                    tuplets[staffIdx] = tuplet;
                                    measure->add(tuplet);
                                    }
                              tuplet->setTrack(staffIdx * VOICES);
                              tuplet->setBaseLen(l);
                              setTuplet(tuplet, tuple);
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
      setTempo(tempo);
      }

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

void GuitarPro1::readNote(int string, Note* note)
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

      //
      // noteBits:
      //    7 - Right hand or left hand fingering;
      //    6 - Accentuated note
      //    5 - Note type (rest, empty note, normal note);
      //    4 - note dynamic;
      //    3 - Presence of effects linked to the note;
      //    2 - Ghost note;
      //    1 - Dotted note;  ?
      //    0 - Time-independent duration

      printf("note bits %d  %02x\n", note->chord()->segment()->tick(), noteBits);

      if (noteBits & 0x1) {               // note != beat
            int a = readUChar();          // length
            int b = readUChar();          // t
            printf("Time independend note len, len %d t %d\n", a, b);
            }
      if (noteBits & 0x2) {               // note is dotted
            }

      if (noteBits & 0x10) {
            int d = readUChar();                  // dynamic
            addDynamic(note, d);
            }
      int fretNumber = -1;
      if (noteBits & 0x20)
            fretNumber = readUChar();

      if (noteBits & 0x80) {              // fingering
            int a = readUChar();
            int b = readUChar();
            printf("Fingering=========%d %d\n", a, b);
            }
      if (noteBits & 0x8) {
            uchar modMask1 = readUChar();
            uchar modMask2 = 0;
            if (version >= 400)
                  modMask2 = readUChar();
            if (modMask1 & 0x1)
                  readChromaticGraph();
            if (modMask1 & 0x2) {         // hammer on / pull off
                  }
            if (modMask1 & 0x8) {         // let ring
                  }
           printf("    0x%02x 0x%02x\n", modMask1, modMask2);
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
//   readBeatEffects
//---------------------------------------------------------

void GuitarPro1::readBeatEffects()
      {
printf("1readBeatEffects\n");
      uchar fxBits1 = readUChar();
      if (fxBits1 & 0x20) {
            uchar num = readUChar();
            switch(num) {
                  case 0:           // tremolo bar
                        readInt();
                        break;
                  default:
                        readInt();
                        break;
                  }
            }
      if (fxBits1 & 0x40) {
            readUChar();            // down stroke length
            readUChar();            // up stroke length
            }
//      if (fxBits1 & 0x02)
//            readUChar();            // stroke pick direction
//      if (fxBits1 & 0x01) {         // GP3 column-wide vibrato
//            }
      }

//---------------------------------------------------------
//   readChord
//---------------------------------------------------------

void GuitarPro1::readChord(Segment*)
      {
      int header = readUChar();

//      printf("read chord diagram %x\n", header);

      if ((header & 1) == 0) {
            readDelphiString();
            int firstFret = readInt();
            if (firstFret) {
                  for (int i = 0; i < 6; ++i)
                        readInt();
                  }
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

void GuitarPro3::read(QFile* fp)
      {
      f      = fp;
      curPos = 30;

      title        = readDelphiString();
      subtitle     = readDelphiString();
      artist       = readDelphiString();
      album        = readDelphiString();
      composer     = readDelphiString();
      QString copyright = readDelphiString();
      if (!copyright.isEmpty())
            score->setCopyright(QString("Copyright %1\nAll Rights Reserved - International Copyright Secured").arg(copyright));

      transcriber  = readDelphiString();
      instructions = readDelphiString();
      int n = readInt();
      for (int i = 0; i < n; ++i)
            comments.append(readDelphiString());

      uchar num = readUChar();      // Shuffle rhythm feel

      int tempo = readInt();
printf("Tempo %d\n", tempo);

      int octave = 0;
      int key = readInt();    // key

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
      measures   = readInt();
      staves = readInt();

      int tnumerator   = 4;
      int tdenominator = 4;
      for (int i = 0; i < measures; ++i) {
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
            if (barBits & 0x80)
                  bar.barLine = DOUBLE_BAR;
            bar.timesig = Fraction(tnumerator, tdenominator);
            bars.append(bar);
            }

      //
      // create a part for every staff
      //
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Part* part = new Part(score);
            Staff* s = new Staff(score, part, staffIdx);
            part->insertStaff(s);
            score->staves().push_back(s);
            score->appendPart(part);
            }

      int tick = 0;
      Fraction ts;
      for (int i = 0; i < measures; ++i) {
            Fraction nts = bars[i].timesig;
            Measure* m = new Measure(score);
            m->setTick(tick);
            m->setTimesig(nts);
            m->setLen(nts);

            if (i == 0 || ts != nts) {
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
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

      for (int i = 0; i < staves; ++i) {
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
            part->setLongName(name);

            //
            // determine clef
            //
            Staff* staff = score->staff(i);
            int patch = channelDefaults[midiChannel].patch;
            int clefId = CLEF_G;
            if (c & 0x1) {
                  clefId = CLEF_PERC;
                  instr->setUseDrumset(true);
                  staff->setStaffType(score->staffTypes().at(PERCUSSION_STAFF_TYPE));
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
      for (int bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
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

            Tuplet* tuplets[staves];
            for (int staffIdx = 0; staffIdx < staves; ++staffIdx)
                  tuplets[staffIdx] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  int tick  = measure->tick();
                  int beats = readInt();
                  for (int beat = 0; beat < beats; ++beat) {
                        int pause = 0;
                        uchar beatBits = readUChar();
printf("bar %d beat %d beat bits %02x\n", bar, beat, beatBits);
                        bool dotted = beatBits & 0x1;
                        if (beatBits & 0x40)
                              pause = readUChar();

                        int len = readChar();
                        int tuple = 0;
                        if (beatBits & 0x20)
                              tuple = readInt();

                        Segment* segment = measure->getSegment(SegChordRest, tick);
                        if (beatBits & 0x2)
                              readChord(segment);
                        if (beatBits & 0x4) {
                              QString txt = readDelphiString();
                              Lyrics* l = new Lyrics(score);
                              l->setText(txt);
                              l->setTrack(staffIdx * VOICES);
                              segment->add(l);
                              }
                        if (beatBits & 0x8)
                              readBeatEffects();
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
                        // if (!pause || strings)
                        if (strings)
                              cr = new Chord(score);
                        else
                              cr = new Rest(score);
                        cr->setTrack(staffIdx * VOICES);
                        if (tuple) {
// printf("%d: Tuplet note beat %d  tuplet %d  len %s\n", tick, beat, tuple, qPrintable(l.print()));

                              Tuplet* tuplet = tuplets[staffIdx];
                              if ((tuplet == 0) || (tuplet->elements().size() == tuple)) {
                                    tuplet = new Tuplet(score);
                                    tuplet->setTrack(cr->track());
                                    tuplets[staffIdx] = tuplet;
                                    measure->add(tuplet);
                                    }
                              tuplet->setTrack(staffIdx * VOICES);
                              tuplet->setBaseLen(l);
                              setTuplet(tuplet, tuple);
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
      setTempo(tempo);
      }

//---------------------------------------------------------
//   readChromaticGraph
//    bend graph
//---------------------------------------------------------

void GuitarPro4::readChromaticGraph()
      {
printf("4:readChromaticGraph()\n");

      skip(5);
      int n = readInt();
      for (int i = 0; i < n; ++i) {
            readInt();                    // time position
            readInt();                    // pitch
            readUChar();                  // vibrato
            }
      }

//---------------------------------------------------------
//   readMixChange
//---------------------------------------------------------

void GuitarPro4::readMixChange()
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
      if (phase >= 0)
            readChar();
      if (tremolo >= 0)
            readChar();
      if (tempo >= 0)
            readChar();
      readChar();       // bitmask: what should be applied to all tracks
printf("readMixChange patch:%d vol:%d pan:%d chorus:%d reverb:%d phase:%d tremolo:%d tempo %d\n",
   patch, volume, pan, chorus, reverb, phase, tremolo, tempo);
      }

//---------------------------------------------------------
//   readBeatEffects
//---------------------------------------------------------

void GuitarPro4::readBeatEffects()
      {
      uchar fxBits1 = readUChar();
      uchar fxBits2 = readUChar();
printf("readBeatEffects 0x%02x 0x%02x\n", fxBits1, fxBits2);
      if (fxBits1 & 0x20) {
            uchar num = readUChar();      // effect 1-tapping, 2-slapping, 3-popping
            }
      if (fxBits2 & 0x04)
            readChromaticGraph();
      if (fxBits1 & 0x40) {
            readUChar();            // down stroke length
            readUChar();            // up stroke length
            }
      if (fxBits2 & 0x02)
            readUChar();            // stroke pick direction
      if (fxBits1 & 0x01) {         // GP3 column-wide vibrato
            }
      if (fxBits1 & 0x2) {          // GP3 column-wide wide vibrato (="tremolo" in GP3)
            }
      }

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

void GuitarPro4::readNote(int string, Note* note)
      {
      uchar noteBits = readUChar();

      //
      // noteBits:
      //    7 - Right hand or left hand fingering;
      //    6 - Accentuated note
      //    5 - Note type (rest, empty note, normal note);
      //    4 - note dynamic;
      //    3 - Presence of effects linked to the note;
      //    2 - Ghost note;
      //    1 - Dotted note;  ?
      //    0 - Time-independent duration

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

      printf("        note bits %02x\n", noteBits);

      if (noteBits & 0x1) {               // note != beat
            int a = readUChar();          // length
            int b = readUChar();          // t
            printf("          Time independend note len, len %d t %d\n", a, b);
            }
      if (noteBits & 0x2) {               // note is dotted
            }

      if (noteBits & 0x10) {
            int d = readUChar();                  // dynamic
            addDynamic(note, d);
            }
      int fretNumber = -1;
      if (noteBits & 0x20)
            fretNumber = readUChar();

      if (noteBits & 0x80) {              // fingering
            int a = readUChar();
            int b = readUChar();
            printf("Fingering=========%d %d\n", a, b);
            }
      if (noteBits & 0x8) {
            uchar modMask1 = readUChar();
            uchar modMask2 = readUChar();
            if (modMask1 & 0x1)
                  readChromaticGraph();
            if (modMask1 & 0x2) {         // hammer on / pull off
                  }
            if (modMask1 & 0x8) {         // let ring
                  }
           printf("          0x%02x 0x%02x\n", modMask1, modMask2);
            if (modMask1 & 0x10) {
                  readUChar();            // grace fret
                  readUChar();            // grace dynamic
                  readUChar();            // grace transition
                  readUChar();            // grace length
                  }
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
printf("          pitch %d  string %d fret %d\n", pitch, string, fretNumber);
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
//   readInfo
//---------------------------------------------------------

void GuitarPro4::readInfo()
      {
      title        = readDelphiString();
      subtitle     = readDelphiString();
      artist       = readDelphiString();
      album        = readDelphiString();
      composer     = readDelphiString();
      QString copyright = readDelphiString();
      if (!copyright.isEmpty())
            score->setCopyright(QString("Copyright %1\nAll Rights Reserved - International Copyright Secured").arg(copyright));

      transcriber  = readDelphiString();
      instructions = readDelphiString();
      int n = readInt();
      for (int i = 0; i < n; ++i)
            comments.append(readDelphiString());
      }

//---------------------------------------------------------
//   readChord
//---------------------------------------------------------

void GuitarPro4::readChord(Segment*)
      {
      int header = readUChar();

//      printf("read chord diagram %x\n", header);

      if ((header & 1) == 0) {
            readDelphiString();
            int firstFret = readInt();
            if (firstFret != 0) {
                  for (int i = 0; i < 6; ++i) {
                        readInt();
                        }
                  }
            }
      else {
            skip(16);
            readPascalString(21);   // chord name
            skip(4);
            int firstFret = readInt();
            for (int i = 0; i < 7; ++i)
                  readInt();
            skip(32);
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void GuitarPro4::read(QFile* fp)
      {
      f      = fp;
      curPos = 30;

      readInfo();
      readUChar();      // triplet feeling
      readLyrics();

      int tempo = readInt();
      int key    = readInt();
      int octave = readUChar();    // octave

printf("tempo %d key %d octave %d\n", tempo, key, octave);

      readChannels();
      measures   = readInt();
      staves = readInt();

printf("bars %d tracks %d\n", measures, staves);

      int tnumerator   = 4;
      int tdenominator = 4;
      for (int i = 0; i < measures; ++i) {
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
            if (barBits & 0x80)
                  bar.barLine = DOUBLE_BAR;
            bar.timesig = Fraction(tnumerator, tdenominator);
            bars.append(bar);
            }

      //
      // create a part for every staff
      //
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Part* part = new Part(score);
            Staff* s = new Staff(score, part, staffIdx);
            part->insertStaff(s);
            score->staves().push_back(s);
            score->appendPart(part);
            }

      int tick = 0;
      Fraction ts;
      for (int i = 0; i < measures; ++i) {
            Fraction nts = bars[i].timesig;
            Measure* m = new Measure(score);
            m->setTick(tick);
            m->setTimesig(nts);
            m->setLen(nts);

            if (i == 0 || ts != nts) {
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
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

      for (int i = 0; i < staves; ++i) {
            int tuning[GP_MAX_STRING_NUMBER];

            uchar c      = readUChar();   // simulations bitmask
            if (c & 0x2) {                // 12 stringed guitar
                  }
            if (c & 0x4) {                // banjo track
                  }
            QString name = readPascalString(40);
            int strings  = readInt();
printf("track %d strings %d <%s>\n", i, strings, qPrintable(name));
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
            part->setLongName(name);

            //
            // determine clef
            //
            Staff* staff = score->staff(i);
            int patch = channelDefaults[midiChannel].patch;
            int clefId = CLEF_G;
            if (c & 0x1) {
                  clefId = CLEF_PERC;
                  instr->setUseDrumset(true);
                  staff->setStaffType(score->staffTypes().at(PERCUSSION_STAFF_TYPE));
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
      for (int bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
printf("  read measure %d(%d)\n", bar, measures);
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

            Tuplet* tuplets[staves];
            for (int staffIdx = 0; staffIdx < staves; ++staffIdx)
                  tuplets[staffIdx] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  int tick  = measure->tick();
                  int beats = readInt();
printf("    readStaff %d(%d)\n", staffIdx, staves);
                  for (int beat = 0; beat < beats; ++beat) {
                        uchar beatBits = readUChar();
printf("      readBeat %d(%d) flags 0x%02x\n", beat, beats, beatBits);
                        bool dotted = beatBits & 0x1;
                        int pause = -1;
                        if (beatBits & 0x40)
                              pause = readUChar();
                        int len = readChar();
                        int tuple = 0;
                        if (beatBits & 0x20)
                              tuple = readInt();
                        Segment* segment = measure->getSegment(SegChordRest, tick);
                        if (beatBits & 0x2)
                              readChord(segment);
                        if (beatBits & 0x4) {
                              QString txt = readDelphiString();
                              Lyrics* l = new Lyrics(score);
                              l->setText(txt);
                              l->setTrack(staffIdx * VOICES);
                              segment->add(l);
                              }
                        if (beatBits & 0x8)
                              readBeatEffects();
                        if (beatBits & 0x10)
                              readMixChange();
                        int strings = readUChar();   // used strings mask
                        Fraction l = len2fraction(len);

printf("      bar %d beat %d(%d) beat bits %02x len %d(%s) tuple %d strings %d\n",
         bar, beat, beats, beatBits, len, qPrintable(l.print()), tuple, strings);

                        ChordRest* cr;
                        if (strings == 0)
                              cr = new Rest(score);
                        else
                              cr = new Chord(score);
                        cr->setTrack(staffIdx * VOICES);
                        if (tuple) {
printf("      Tuplet note beat %d  tuplet %d  len %s\n", beat, tuple, qPrintable(l.print()));
                              Tuplet* tuplet = tuplets[staffIdx];
                              if ((tuplet == 0) || (tuplet->elements().size() == tuple)) {
                                    tuplet = new Tuplet(score);
                                    tuplet->setTrack(cr->track());
                                    tuplets[staffIdx] = tuplet;
                                    measure->add(tuplet);
                                    }
                              tuplet->setTrack(staffIdx * VOICES);
                              tuplet->setBaseLen(l);
                              setTuplet(tuplet, tuple);
                              cr->setTuplet(tuplet);
                              }

                        Duration d(l);
                        d.setDots(dotted ? 1 : 0);

                        if (dotted)
                              l = l + (l/2);
                        cr->setDuration(l);
                        cr->setDurationType(d);
                        segment->add(cr);
printf("      add cr %p <%s>\n", cr, qPrintable(l.print()));

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
      setTempo(tempo);
      }

//---------------------------------------------------------
//   readInfo
//---------------------------------------------------------

void GuitarPro5::readInfo()
      {
      title        = readDelphiString();
      subtitle     = readDelphiString();
      artist       = readDelphiString();
      album        = readDelphiString();
      composer     = readDelphiString();
      readDelphiString();
      QString copyright = readDelphiString();
      if (!copyright.isEmpty())
            score->setCopyright(QString("Copyright %1\nAll Rights Reserved - International Copyright Secured").arg(copyright));

      transcriber  = readDelphiString();
      instructions = readDelphiString();
      int n = readInt();
      for (int i = 0; i < n; ++i)
            comments.append(readDelphiString());
      }

//---------------------------------------------------------
//   readNoteEffects
//---------------------------------------------------------

void GuitarPro5::readNoteEffects()
      {
      uchar modMask1 = readUChar();
      uchar modMask2 = readUChar();
      if (modMask1 & 0x1)
            readChromaticGraph();
      if (modMask1 & 0x2) {         // hammer on / pull off
            }
      if (modMask1 & 0x8) {         // let ring
            }
      printf("    readNoteEffects: 0x%02x 0x%02x\n", modMask1, modMask2);
      if (modMask1 & 0x10) {
            readUChar();            // grace fret
            readUChar();            // grace dynamic
            readUChar();            // grace transition
            readUChar();            // grace length
            int flags = readUChar();
                  // 1  -  dead
                  // 2  - on beat
            }
      if (modMask2 & 0x1) {   // staccato - palm mute
            }
      if (modMask2 & 0x2) {   // palm mute - mute the whole column
            }
      if (modMask2 & 0x4)     // tremolo picking length
            readUChar();
      if (modMask2 & 0x8)
            readUChar();      // slide kind
      if (modMask2 & 0x10)
            readArtificialHarmonic();
      if (modMask2 & 0x20) {
            readUChar();      // trill fret
            int period = readUChar();      // trill length
            switch(period) {
                  case 1:           // 16
                        break;
                  case 2:           // 32
                        break;
                  case 3:           // 64
                        break;
                  default:
                        printf("unknown trill period %d\n", period);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

void GuitarPro5::readNote(int string, Note* note)
      {
      uchar noteBits = readUChar();
      //
      // noteBits:
      //    7 - Right hand or left hand fingering;
      //    6 - Accentuated note
      //    5 - Note type (rest, empty note, normal note);
      //    4 - note dynamic;
      //    3 - Presence of effects linked to the note;
      //    2 - Ghost note;
      //    1 - Dotted note;  ?
      //    0 - Time-independent duration

      printf("   note bits %d  %02x\n", note->chord()->segment()->tick(), noteBits);


      bool tieNote = false;
      if (noteBits & 0x20) {
            uchar noteType = readUChar();
            if (noteType == 1) {     // normal note
                  }
            else if (noteType == 2)
                  tieNote = true;
            else if (noteType == 3) {                 // dead notes
                  //printf("DeathNote tick %d pitch %d\n", note->chord()->segment()->tick(), note->pitch());
                  }
            else
                  printf("unknown note type: %d\n", noteType);
            }

      if (noteBits & 0x10) {          // velocity
            int d = readChar();
            addDynamic(note, d);
            }
      int fretNumber = 0;
      if (noteBits & 0x20)
            fretNumber = readChar();

      if (noteBits & 0x80) {              // fingering
            int a = readUChar();
            int b = readUChar();
            printf("   Fingering=========%d %d\n", a, b);
            }
      if (noteBits & 0x1)
            skip(8);

      int aa = readUChar();
      printf("    note 0x%02x\n", aa);

      if (noteBits & 0x8) {
            readNoteEffects();
            }
      Staff* staff = note->staff();
      if (fretNumber == 255) {
            fretNumber = 0;
            note->setHeadGroup(1);
            note->setGhost(true);
            }
      int pitch = staff->part()->instr()->tablature()->getPitch(string, fretNumber);
printf("   pitch %d  string %d fret %d\n", pitch, string, fretNumber);
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
//   readArtificialHarmonic
//---------------------------------------------------------

void GuitarPro5::readArtificialHarmonic()
      {
      int type = readChar();
      switch(type) {
            case 1:           // natural
                  break;
            case 2:           // artificial
                  skip(3);
                  break;
            case 3:           // tapped
                  skip(1);
                  break;
            case 4:           // pinch
                  break;
            case 5:           // semi
                  break;
            }
      }

//---------------------------------------------------------
//   readChromaticGraph
//---------------------------------------------------------

void GuitarPro5::readChromaticGraph()
      {
      skip(5);
      int n = readInt();
printf("5readChromaticGraph() n=%d\n", n);
      for (int i = 0; i < n; ++i) {
            readInt();                    // time
            readInt();                    // pitch
            readUChar();                  // vibrato
            }
      }

//---------------------------------------------------------
//   readTremoloBar
//---------------------------------------------------------

void GuitarPro5::readTremoloBar()
      {
      skip(5);
      int n = readInt();
printf("5readTremoloBar() n=%d\n", n);
      for (int i = 0; i < n; ++i) {
            readInt();                    // time
            readInt();                    // pitch
            readUChar();
            }
      }

//---------------------------------------------------------
//   readBeatEffects
//---------------------------------------------------------

void GuitarPro5::readBeatEffects()
      {
      uchar fxBits1 = readUChar();
      uchar fxBits2 = readUChar();
printf("5readBeatEffects %02x %02x\n", fxBits1, fxBits2);
      if (fxBits1 & 0x20) {
            uchar num = readUChar();
            // 1 - tapping
            // 2 - slapping
            // 3 - popping
            }
      if (fxBits2 & 0x04)
            readTremoloBar();       // readChromaticGraph();
      if (fxBits1 & 0x40) {
            readChar();            // down stroke length
            readChar();            // up stroke length
            }
      if (fxBits2 & 0x02)
            readChar();            // stroke pick direction
      }

//---------------------------------------------------------
//   readPageSetup
//---------------------------------------------------------

void GuitarPro5::readPageSetup()
      {
      skip(version > 500 ? 49 : 30);
      for (int i = 0; i < 11; ++i) {
            skip(4);
            readBytePascalString();
            }
      }

//---------------------------------------------------------
//   readBeat
//---------------------------------------------------------

int GuitarPro5::readBeat(int tick, int voice, Measure* measure, int staffIdx, Tuplet** tuplets)
      {
      uchar beatBits = readUChar();
      bool dotted    = beatBits & 0x1;

      int pause = -1;
      if (beatBits & 0x40)
            pause = readUChar();

      // readDuration
      int len = readChar();
      int tuple = 0;
      if (beatBits & 0x20)
            tuple = readInt();

      Segment* segment = measure->getSegment(SegChordRest, tick);
      if (beatBits & 0x2)
            readChord(segment);
      if (beatBits & 0x4) {
            QString txt = readDelphiString();
            Lyrics* l = new Lyrics(score);
            l->setText(txt);
            l->setTrack(staffIdx * VOICES);
            segment->add(l);
            }
      if (beatBits & 0x8)
            readBeatEffects();
      if (beatBits & 0x10)
            readMixChange();

      int strings = readUChar();   // used strings mask

      Fraction l    = len2fraction(len);
      ChordRest* cr;
printf("voice %d bits 0x%02x strings %d pause %d len %d, %s\n",
     voice, beatBits, strings, pause, len, qPrintable(l.print()));
      if (voice != 0 && pause == 0 && strings == 0)
            cr = 0;
      else {
            if (strings == 0)
                  cr = new Rest(score);
            else
                  cr = new Chord(score);
            cr->setTrack(staffIdx * VOICES + voice);
            if (tuple) {
                  Tuplet* tuplet = tuplets[staffIdx * 2 + voice];
                  if ((tuplet == 0) || (tuplet->elements().size() == tuple)) {
                        tuplet = new Tuplet(score);
                        int track = staffIdx * 2 + voice;
                        tuplets[staffIdx * 2 + voice] = tuplet;
                        tuplet->setTrack(cr->track());
                        measure->add(tuplet);
                        }
                  tuplet->setTrack(cr->track());
                  tuplet->setBaseLen(l);
                  setTuplet(tuplet, tuple);
                  cr->setTuplet(tuplet);
                  }

            Duration d(l);
            d.setDots(dotted ? 1 : 0);

            if (dotted)
                  l = l + (l/2);
            cr->setDuration(l);
            if (cr->type() == REST && pause == 0)
                  cr->setDurationType(Duration::V_MEASURE);
            else
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
            }
      int rr = readChar();
      if (cr && (cr->type() == CHORD)) {
            Chord* chord = static_cast<Chord*>(cr);
            if (rr == 0x2)
                  chord->setStemDirection(DOWN);
            else if (rr == 0xa)
                  chord->setStemDirection(UP);
            else
                  printf("  1beat read 0x%02x\n", rr);
            }
      int r = readChar();
printf("  2beat read 0x%02x\n", r);
      if (r & 0x8) {
            int rrr = readChar();
printf("  3beat read 0x%02x\n", rrr);
            }
      return cr ? cr->ticks() : measure->ticks();
      }

//---------------------------------------------------------
//   readMeasure
//---------------------------------------------------------

void GuitarPro5::readMeasure(Measure* measure, int staffIdx, Tuplet** tuplets)
      {
      for (int voice = 0; voice < 2; ++voice) {
            int tick = measure->tick();
            int beats = readInt();
            for (int beat = 0; beat < beats; ++beat) {
printf("readBeat staff%d voice %d(%d) beat %d(%d)\n", staffIdx, voice, 2, beat, beats);
                  tick += readBeat(beat, voice, measure, staffIdx, tuplets);
                  }
            }
      }

//---------------------------------------------------------
//   readMixChange
//---------------------------------------------------------

void GuitarPro5::readMixChange()
      {
      char patch   = readChar();
      skip(16);
      char volume  = readChar();
      char pan     = readChar();
      char chorus  = readChar();
      char reverb  = readChar();
      char phase   = readChar();
      char tremolo = readChar();
      readDelphiString();                 // tempo name

      int tempo = readInt();

      if (volume >= 0)
            readChar();
      if (pan >= 0)
            readChar();
      if (chorus >= 0)
            readChar();
      if (reverb >= 0)
            readChar();
      if (phase >= 0)
            readChar();
      if (tremolo >= 0)
            readChar();
      if (tempo >= 0) {
            readChar();
            if (version > 500)
                  readChar();
            }
      readChar();
      skip(1);
      if (version > 500) {
            readDelphiString();
            readDelphiString();
            }
      }

//---------------------------------------------------------
//   readChord
//---------------------------------------------------------

void GuitarPro5::readChord(Segment*)
      {
      skip(17);
      readPascalString(21);
      skip(4);
      int firstFret = readInt();
      for (int i = 0; i < 7; ++i) {
            int fret = readInt();
            }
      skip(32);
      }

//---------------------------------------------------------
//   readTracks
//---------------------------------------------------------

void GuitarPro5::readTracks()
      {
      for (int i = 0; i < staves; ++i) {
            int tuning[GP_MAX_STRING_NUMBER];
            Staff* staff = score->staff(i);
            Part* part = staff->part();

            uchar c = readUChar();   // simulations bitmask
            if (c & 0x2) {                // 12 stringed guitar
                  }
            if (c & 0x4) {                // banjo track
                  }
            if (i == 0 || version == 500)
                  skip(1);
            QString name = readPascalString(40);

            int strings  = readInt();
            if (strings <= 0 || strings > GP_MAX_STRING_NUMBER)
                  throw GP_BAD_NUMBER_OF_STRINGS ;
            for (int j = 0; j < strings; ++j) {
                  tuning[j] = readInt();
printf("tuning %d %d\n", j, tuning[j]);
                  }
            for (int j = strings; j < GP_MAX_STRING_NUMBER; ++j)
                  readInt();
            int midiPort     = readInt() - 1;
            int midiChannel  = readInt() - 1;
            int midiChannel2 = readInt() - 1;

            int frets        = readInt();
            int capo         = readInt();
            int color        = readInt();

printf("midi %d %d %d  frets %d capo %d color %d\n", midiPort, midiChannel,
            midiChannel2, frets, capo, color);

            skip(version > 500 ? 49 : 44);
            if (version > 500) {
                  readDelphiString();
                  readDelphiString();
                  }

            int tuning2[strings];
            for (int k = 0; k < strings; ++k)
                  tuning2[strings-k-1] = tuning[k];
            Tablature* tab = new Tablature(frets, strings, tuning2);
            Instrument* instr = part->instr();
            instr->setTablature(tab);
            instr->setTrackName(name);
            part->setLongName(name);

            //
            // determine clef
            //
            int patch = channelDefaults[midiChannel].patch;
            int clefId = CLEF_G;
            if (c & 0x1) {
                  clefId = CLEF_PERC;
                  instr->setUseDrumset(true);
                  staff->setStaffType(score->staffTypes().at(PERCUSSION_STAFF_TYPE));
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
      skip(version == 500 ? 2 : 1);
      }

//---------------------------------------------------------
//   readMeasures
//---------------------------------------------------------

void GuitarPro5::readMeasures()
      {
      Measure* measure = score->firstMeasure();
      for (int bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
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

            Tuplet* tuplets[staves * 2];     // two voices
            for (int track = 0; track < staves*2; ++track)
                  tuplets[track] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
printf("readMeasure %d(%d), staff %d(%d)\n", bar, measures, staffIdx, staves);
                  readMeasure(measure, staffIdx, tuplets);
                  if (!(((bar == (measures-1)) && (staffIdx == (staves-1))))) {
                        int a = readChar();
                        printf("    ======skip %02x\n", a);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void GuitarPro5::read(QFile* fp)
      {
      f = fp;
      readInfo();
      readLyrics();
      readPageSetup();

      int tempo = readInt();
      if (version > 500)
            skip(1);

      int key    = readInt();
      int octave = readChar();    // octave

      readChannels();
      skip(42);

      measures = readInt();
      staves  = readInt();

printf("bars %d tracks %d\n", measures, staves);

      int tnumerator   = 4;
      int tdenominator = 4;
      for (int i = 0; i < measures; ++i) {
            if (i > 0)
                  skip(1);
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
            if (barBits & 0x20) {
                  bar.marker = readDelphiString();     // new section?
                  int color = readInt();    // color?
                  }
            if (barBits & 0x10)                 // alternative ending to
                  uchar c = readUChar();
            if (barBits & 0x40) {
                  bar.keysig = readUChar();
                  uchar c    = readUChar();        // minor
                  }
            if (barBits & 0x80)
                  bar.barLine = DOUBLE_BAR;
            if (barBits & 0x3)
                  skip(4);
            if ((barBits & 0x10) == 0)
                  skip(1);
            readChar();             // triple feel  (none, 8, 16)
            bar.timesig = Fraction(tnumerator, tdenominator);
            bars.append(bar);
            }

      //
      // create a part for every staff
      //
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Part* part = new Part(score);
            Staff* s = new Staff(score, part, staffIdx);
            part->insertStaff(s);
            score->staves().push_back(s);
            score->appendPart(part);
            }

      int tick = 0;
      Fraction ts;
      for (int i = 0; i < measures; ++i) {
            Fraction nts = bars[i].timesig;
            Measure* m = new Measure(score);
            m->setTick(tick);
            m->setTimesig(nts);
            m->setLen(nts);

            if (i == 0 || ts != nts) {
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
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

      readTracks();
      readMeasures();
      setTempo(tempo);
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
      uchar l;
      fp.read((char*)&l, 1);
      char ss[30];
      fp.read(ss, 30);
      ss[l] = 0;
      QString s(ss);
      if (s.startsWith("FICHIER GUITAR PRO "))
            s = s.mid(20);
      else if (s.startsWith("FICHIER GUITARE PRO "))
            s = s.mid(21);
      else {
            printf("unknown gtp format <%s>\n", ss);
            return false;
            }

      int a = s.left(1).toInt();
      int b = s.mid(2).toInt();
      int version = a * 100 + b;
      GuitarPro* gp;

      if (a == 1)
            gp = new GuitarPro1(this, version);
      if (a == 2)
            gp = new GuitarPro2(this, version);
      if (a == 3)
            gp = new GuitarPro3(this, version);
      else if (a == 4)
            gp = new GuitarPro4(this, version);
      else if (a == 5)
            gp = new GuitarPro5(this, version);
      else {
            printf("unknown gtp format %d\n", version);
            return false;
            }
      try {
            gp->read(&fp);
            }
      catch(GuitarPro::GuitarProError errNo) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: Import GuitarPro"),
               QString("Load failed: ") + gp->error(errNo),
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
      if (!gp->title.isEmpty()) {
            Text* s = new Text(this);
            s->setSubtype(TEXT_TITLE);
            s->setTextStyle(TEXT_STYLE_TITLE);
            s->setText(gp->title);
            m->add(s);
            }
      if (!gp->subtitle.isEmpty() && !gp->artist.isEmpty() && !gp->album.isEmpty()) {
            Text* s = new Text(this);
            s->setSubtype(TEXT_SUBTITLE);
            s->setTextStyle(TEXT_STYLE_SUBTITLE);
            QString str;
            if (!gp->subtitle.isEmpty())
                  str.append(gp->subtitle);
            if (!gp->artist.isEmpty()) {
                  if (!str.isEmpty())
                        str.append("\n");
                  str.append(gp->artist);
                  }
            if (!gp->album.isEmpty()) {
                  if (!str.isEmpty())
                        str.append("\n");
                  str.append(gp->album);
                  }
            s->setText(str);
            m->add(s);
            }
      if (!gp->composer.isEmpty()) {
            Text* s = new Text(this);
            s->setSubtype(TEXT_COMPOSER);
            s->setTextStyle(TEXT_STYLE_COMPOSER);
            s->setText(gp->composer);
            m->add(s);
            }
      int idx = 0;

      for (Measure* m = firstMeasure(); m; m = m->nextMeasure(), ++idx) {
            const GpBar& bar = gp->bars[idx];
            if (bar.barLine != NORMAL_BAR)
                  m->setEndBarLineType(bar.barLine, false);
            }
      lastMeasure()->setEndBarLineType(END_BAR, false);
#if 1
      //
      // create excerpts
      //
      foreach(Part* part, _parts) {
            Score* score = new Score(this);
            score->style().set(ST_createMultiMeasureRests, true);

            QList<int> stavesMap;
            Part* p = new Part(score);
            p->setInstrument(*part->instr());

            Staff* staff = part->staves()->front();

            Staff* s = new Staff(score, p, 0);
            s->setUpdateClefList(true);
            s->setUpdateKeymap(true);
            StaffType* st = staff->staffType();
            if (st->modified())
                  st = new StaffType(*st);
            s->setStaffType(st);
            int idx = score->staffTypes().indexOf(st);
            if (idx == -1)
                  score->staffTypes().append(st);
            s->linkTo(staff);
            p->staves()->append(s);
            score->staves().append(s);
            stavesMap.append(staffIdx(staff));
            if (part->staves()->front()->staffType()->group() == PITCHED_STAFF) {
                  s = new Staff(score, p, 1);
                  s->setUpdateClefList(true);
                  s->setUpdateKeymap(true);
                  StaffType* st = score->staffTypes().at(TAB_STAFF_TYPE);
                  s->setStaffType(st);
                  s->linkTo(staff);
                  p->staves()->append(s);
                  score->staves().append(s);
                  stavesMap.append(staffIdx(staff));
                  p->staves()->front()->addBracket(BracketItem(BRACKET_NORMAL, 2));
                  }
            score->appendPart(p);

            cloneStaves(this, score, stavesMap);

            score->setName(part->instr()->trackName());
            Excerpt* excerpt = new Excerpt(score);
            _excerpts.append(excerpt);
            if (part->staves()->front()->staffType()->group() == PITCHED_STAFF) {
                  Staff* staff2 = score->staff(1);
                  staff2->setStaffType(score->staffTypes().at(TAB_STAFF_TYPE));
                  }

            //
            // create excerpt title
            //
            MeasureBase* measure = score->first();
            if (!measure || (measure->type() != VBOX)) {
                  measure = new VBox(score);
                  measure->setTick(0);
                  score->addMeasure(measure);
                  }
            Text* txt = new Text(score);
            txt->setSubtype(TEXT_INSTRUMENT_EXCERPT);
            txt->setTextStyle(TEXT_STYLE_INSTRUMENT_EXCERPT);
            txt->setText(part->longName()->getText());
            measure->add(txt);

            //
            // layout score
            //
            score->setPlaylistDirty(true);
            score->rebuildMidiMapping();
            score->updateChannel();

            score->setLayoutAll(true);
            score->addLayoutFlag(LAYOUT_FIX_TICKS);
            score->addLayoutFlag(LAYOUT_FIX_PITCH_VELO);
            score->doLayout();
            }
#endif

//      album
//      copyright

      _saved = false;
      _created = true;
      delete gp;
      return true;
      }

