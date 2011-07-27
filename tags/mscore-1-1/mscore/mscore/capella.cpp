//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2009 Werner Schweer and others
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

//
//    Capella 2000 import filter
//
#include <assert.h>
#include "score.h"
#include "capella.h"
#include "part.h"
#include "staff.h"
#include "rest.h"
#include "chord.h"
#include "note.h"
#include "utils.h"
#include "lyrics.h"
#include "timesig.h"
#include "clef.h"
#include "pitchspelling.h"
#include "keysig.h"
#include "slur.h"
#include "box.h"
#include "measure.h"
#include "al/sig.h"

//---------------------------------------------------------
//   errmsg
//---------------------------------------------------------

const char* Capella::errmsg[] = {
      "no error",
      "bad file signature, no capella file?",
      "unexpected end of file",
      "bad voice signature",
      "bad staff signature",
      "bad system signature",
      };

//---------------------------------------------------------
//   Capella
//---------------------------------------------------------

Capella::Capella()
      {
      author   = 0;
      keywords = 0;
      comment  = 0;
      }

Capella::~Capella()
      {
      delete[] author;
      delete[] keywords;
      delete[] comment;
      }

//---------------------------------------------------------
//   SlurObj::read
//---------------------------------------------------------

void SlurObj::read()
      {
      BasicDrawObj::read();
      for (int i = 0; i < 4; ++i) {
            bezierPoint[i].setX(cap->readInt());
            bezierPoint[i].setY(cap->readInt());
            }
      color     = cap->readColor();
      nEnd      = cap->readByte();
      nMid      = cap->readByte();
      nDotDist  = cap->readByte();
      nDotWidth = cap->readByte();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextObj::read()
      {
      BasicRectObj::read();
      unsigned size = cap->readUnsigned();
      char txt[size+1];
      cap->read(txt, size);
      txt[size] = 0;
      text = QString(txt);
printf("read textObj len %d <%s>\n", size, txt);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SimpleTextObj::read()
      {
      BasicDrawObj::read();
      relPos = cap->readPoint();
      align  = cap->readByte();
      _font  = cap->readFont();
      _text  = cap->readString();
printf("read SimpletextObj(%d,%d) len %d <%s> char0: %02x\n",
      relPos.x(), relPos.y(), strlen(_text), _text, _text[0]);
      }

//---------------------------------------------------------
//   LineObj::read
//---------------------------------------------------------

void LineObj::read()
      {
      BasicDrawObj::read();
      pt1       = cap->readPoint();
      pt2       = cap->readPoint();
      color     = cap->readColor();
      lineWidth = cap->readByte();
printf("LineObj: %d:%d  %d:%d  width %d\n", pt1.x(), pt1.y(), pt2.x(), pt2.y(), lineWidth);
      }

//---------------------------------------------------------
//   BracketObj::read
//---------------------------------------------------------

void BracketObj::read()
      {
      LineObj::read();
      orientation = cap->readByte();
      number      = cap->readByte();
      }

//---------------------------------------------------------
//   GroupObj::read
//---------------------------------------------------------

void GroupObj::read()
      {
      BasicDrawObj::read();
      relPos = cap->readPoint();
      objects = cap->readDrawObjectArray();
      }

//---------------------------------------------------------
//   TransposableObj::read
//---------------------------------------------------------

void TransposableObj::read()
      {
      BasicDrawObj::read();
      relPos = cap->readPoint();
      b = cap->readByte();
      if (b != 12 && b != 21)
            printf("TransposableObj::read: warning: unknown drawObjectArray size of %d\n", b);
      variants = cap->readDrawObjectArray();
      if (variants.size() != b)
            printf("variants.size %d, expected %d\n", variants.size(), b);
      assert(variants.size() == b);
      /*int nRefNote =*/ cap->readInt();
      }

//---------------------------------------------------------
//   MetafileObj::read
//---------------------------------------------------------

void MetafileObj::read()
      {
      BasicDrawObj::read();
      unsigned size = cap->readUnsigned();
      char enhMetaFileBits[size];
      cap->read(enhMetaFileBits, size);
      }

//---------------------------------------------------------
//   RectEllipseObj::read
//---------------------------------------------------------

void RectEllipseObj::read()
      {
      LineObj::read();
      radius = cap->readInt();
      bFilled = cap->readByte();
      clrFill = cap->readColor();
      }

//---------------------------------------------------------
//   PolygonObj::read
//---------------------------------------------------------

void PolygonObj::read()
      {
      BasicDrawObj::read();

      unsigned nPoints = cap->readUnsigned();
      for (unsigned i = 0; i < nPoints; ++i)
          cap->readPoint();

      bFilled = cap->readByte();
      lineWidth = cap->readByte();
      clrFill = cap->readColor();
      clrLine = cap->readColor();
      }

//---------------------------------------------------------
//   WavyLineObj::read
//---------------------------------------------------------

void WavyLineObj::read()
      {
      LineObj::read();
      waveLen = cap->readByte();
      adapt = cap->readByte();
      }

//---------------------------------------------------------
//   NotelinesObj::read
//---------------------------------------------------------

void NotelinesObj::read()
      {
      BasicDrawObj::read();

      x0 = cap->readInt();
      x1 = cap->readInt();
      y  = cap->readInt();
      color = cap->readColor();

      unsigned char b = cap->readByte();
      switch (b) {
            case 1: break; // Einlinienzeile
            case 2: break; // Standard (5 Linien)
            default: {
                  assert(b == 0);
                  char lines[11];
                  cap->read(lines, 11);
                  break;
                  }
              }
      }

//---------------------------------------------------------
//   VoltaObj::read
//---------------------------------------------------------

void VoltaObj::read()
      {
      BasicDrawObj::read();

      x0 = cap->readInt();
      x1 = cap->readInt();
      y  = cap->readInt();
      color = cap->readColor();

      unsigned char flags = cap->readByte();
      bLeft      = (flags & 1) != 0; // links abgeknickt
      bRight     = (flags & 2) != 0; // rechts abgeknickt
      bDotted    = (flags & 4) != 0;
      allNumbers = (flags & 8) != 0;

      unsigned char numbers = cap->readByte();
      from = numbers & 0x0F;
      to = (numbers >> 4) & 0x0F;
      }

//---------------------------------------------------------
//   GuitarObj::read
//---------------------------------------------------------

void GuitarObj::read()
      {
      BasicDrawObj::read();
      relPos  = cap->readPoint();
      color   = cap->readColor();
      flags   = cap->readWord();
      strings = cap->readDWord();   // 8 Saiten in 8 Halbbytes
      }

//---------------------------------------------------------
//   TrillObj::read
//---------------------------------------------------------

void TrillObj::read()
      {
      BasicDrawObj::read();
      x0 = cap->readInt();
      x1 = cap->readInt();
      y  = cap->readInt();
      color = cap->readColor();
      trillSign = cap->readByte();
      }

//---------------------------------------------------------
//   readDrawObjectArray
//---------------------------------------------------------

QList<BasicDrawObj*> Capella::readDrawObjectArray()
      {
      static int level = 0;

      QList<BasicDrawObj*> ol;
      int n = readUnsigned();       // draw obj array
for (int k = 0; k < level; ++k)
      printf("   ");
printf("readDRawObjectArray %d elements\n", n);
      ++level;
      for (int i = 0; i < n; ++i) {
            unsigned char type = readByte();

            for (int k = 0; k < level; ++k)
                  printf("   ");
printf("readDrawObject %d of %d, type %d\n", i, n, type);
            switch (type) {
                  case  0: {
                        GroupObj* o = new GroupObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  1: {
                        TransposableObj* o = new TransposableObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  2: {
                        MetafileObj* o = new MetafileObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  CAP_SIMPLE_TEXT: {
                        SimpleTextObj* o = new SimpleTextObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  4: {
                        TextObj* o = new TextObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  5: {
                        RectEllipseObj* o = new RectEllipseObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case 6: {
                        LineObj* o = new LineObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  7: {
                        PolygonObj* o = new PolygonObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  8: {
                        WavyLineObj* o = new WavyLineObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case 9: {
                        SlurObj* o = new SlurObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  10: {
                        NotelinesObj* o = new NotelinesObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case 11: {
                        WedgeObj* o = new WedgeObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  12: {
                        VoltaObj* o = new VoltaObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case 13: {
                        BracketObj* o = new BracketObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  14: {
                        GuitarObj* o = new GuitarObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  15: {
                        TrillObj* o = new TrillObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  default:
printf("readDrawObjectArray unsupported type %d\n", type);
                        // abort();
                        break;
                  }
            }
      --level;
      return ol;
      }

//---------------------------------------------------------
//   BasicDrawObj
//---------------------------------------------------------

void BasicDrawObj::read()
      {
      modeX       = cap->readByte();      // anchor mode
      modeY       = cap->readByte();
      distY       = cap->readByte();
      flags       = cap->readByte();
      nRefNote    = cap->readInt();
      short range = cap->readWord();
      nNotes      = range & 0x0fff;
      background  = range & 0x1000;
      pageRange   = (range >> 13) & 0x7;
      }

//---------------------------------------------------------
//   BasicRectObj
//---------------------------------------------------------

void BasicRectObj::read()
      {
      BasicDrawObj::read();
      relPos  = cap->readPoint();
      width   = cap->readInt();
      yxRatio = cap->readInt();
      height  = (width * yxRatio) / 0x10000;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void BasicDurationalObj::read()
      {
      unsigned char b = cap->readByte();
      nDots      = b & 0x03;
      noDuration = b & 0x04;
      postGrace  = b & 0x08;
      bSmall     = b & 0x10;
      invisible  = b & 0x20;
      notBlack   = b & 0x40;
      if (b & 0x80)
            abort();

      color = notBlack ? cap->readColor() : Qt::black;

      unsigned char c = cap->readByte();
      t = TIMESTEP(c & 0x0f);
      horizontalShift = (c & 0x10) ? cap->readInt() : 0;
      count = 0;
      if (c & 0x20) {
            unsigned char tuplet = cap->readByte();
            count        = tuplet & 0x0f;
            tripartite   = tuplet & 0x10;
            isProlonging = tuplet & 0x20;
            if (tuplet & 0xc0)
                  printf("bad tuplet value 0x%02x\n", tuplet);
            }
      if (c & 0x40) {
            objects = cap->readDrawObjectArray();
            }
      if (c & 0x80)
            abort();
      }

//---------------------------------------------------------
//   RestObj
//---------------------------------------------------------

RestObj::RestObj(Capella* c)
   : BasicDurationalObj(c), NoteObj(T_REST)
      {
      cap          = c;
      fullMeasures = 0;
      vertShift    = 0;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void RestObj::read()
      {
      BasicDurationalObj::read();
      unsigned char b        = cap->readByte();
      bool bMultiMeasures    = b & 1;
      bVerticalCentered      = b & 2;
      bool bAddVerticalShift = b & 4;
      if (b & 0xf8) {
            printf("RestObj: res. bits 0x%02x\n", b);
            abort();
            }
      fullMeasures = bMultiMeasures ? cap->readUnsigned() : 0;
      vertShift    = bAddVerticalShift ? cap->readInt() : 0;
      }

//---------------------------------------------------------
//   ChordObj
//---------------------------------------------------------

ChordObj::ChordObj(Capella* c)
   : BasicDurationalObj(c), NoteObj(T_CHORD)
      {
      cap      = c;
      beamMode = AUTO_BEAM;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordObj::read()
      {
      stemDir      = 0;
      dStemLength  = 0;
      nTremoloBars = 0;
      articulation = 0;
      leftTie      = false;
      rightTie     = false;
      beamShift    = 0;
      beamSlope    = 0;

      BasicDurationalObj::read();

      unsigned char flags = cap->readByte();
      beamMode      = (flags & 0x01) ? (BEAM_MODE)(cap->readByte()) : AUTO_BEAM;
      notationStave = (flags & 0x02) ? cap->readChar() : 0;
      assert(notationStave >= -1 && notationStave <= 1);

      if (flags & 0x04) {
            stemDir     = cap->readChar();
            dStemLength = cap->readChar();
            }
      if (flags & 0x08) {
            nTremoloBars = cap->readByte();
            articulation = cap->readByte();
            }
      if (flags & 0x10) {
            unsigned char b = cap->readByte();
            leftTie  = b & 1;
            rightTie = b & 2;
printf("Chord: ties %d %d  ticks %d\n", leftTie, rightTie, ticks());
            }
      if (flags & 0x20) {
            beamShift = cap->readChar();
            beamSlope = cap->readChar();
            }
      if (flags & 0x40) {
            unsigned nVerses = cap->readUnsigned();
            for (unsigned int i = 0; i < nVerses; ++i) {
                  bool bVerse = cap->readByte();
                  if (bVerse) {
                        Verse v;
                        unsigned char b = cap->readByte();
                        v.leftAlign = b & 1;
                        v.extender  = b & 2;
                        v.hyphen    = b & 4;
                        v.num       = i;
                        if (b & 8)
                              v.verseNumber = cap->readString();
                        if (b & 16)
                              v.text = cap->readString();
// printf("         Verse(%d) <%s><%s>\n", i, qPrintable(v.verseNumber), qPrintable(v.text));
                        verse.append(v);
                        }
                  }
            }
      unsigned nNotes = cap->readUnsigned();
// printf("         Chord %s flags 0x%02x notes %d\n", timeNames[t], flags, nNotes);
      for (unsigned int i = 0; i < nNotes; ++i) {
            CNote n;
            n.explAlteration = 0;
            char c = cap->readChar();
            bool bit7 = c & 0x80;
            bool bit6 = c & 0x40;
            n.pitch = c;
            if (bit7 != bit6) {
                  n.explAlteration = 2;
                  n.pitch ^= 0x80;
                  }
            unsigned char b = cap->readByte();
            n.headType = b & 7;
            n.alteration = ((b >> 3) & 7) - 2;  // -2 -- +2
            if (b & 0x40)
                  n.explAlteration = 1;
            n.silent = b & 0x80;
// printf("            Note pitch %d head %d alteration %d\n", n.pitch, n.headType, n.alteration);
            notes.append(n);
            }
      }

//---------------------------------------------------------
//    read
//    return false on error
//---------------------------------------------------------

void Capella::read(void* p, qint64 len)
      {
      if (len == 0)
            return;
      qint64 rv = f->read((char*)p, len);
      if (rv != len)
            throw CAP_EOF;
      curPos += len;
      }

//---------------------------------------------------------
//   readByte
//---------------------------------------------------------

unsigned char Capella::readByte()
      {
      unsigned char c;
      read(&c, 1);
      return c;
      }

//---------------------------------------------------------
//   readChar
//---------------------------------------------------------

char Capella::readChar()
      {
      char c;
      read(&c, 1);
      return c;
      }

//---------------------------------------------------------
//   readWord
//---------------------------------------------------------

short Capella::readWord()
      {
      short c;
      read(&c, 2);
      return c;
      }

//---------------------------------------------------------
//   readDWord
//---------------------------------------------------------

int Capella::readDWord()
      {
      int c;
      read(&c, 4);
      return c;
      }

//---------------------------------------------------------
//   readLong
//---------------------------------------------------------

int Capella::readLong()
      {
      int c;
      read(&c, 4);
      return c;
      }

//---------------------------------------------------------
//   readUnsigned
//---------------------------------------------------------

unsigned Capella::readUnsigned()
      {
      unsigned char c;
      read(&c, 1);
      if (c == 254) {
            unsigned short s;
            read(&s, 2);
            return s;
            }
      else if (c == 255) {
            unsigned s;
            read(&s, 4);
            return s;
            }
      else
            return c;
      }

//---------------------------------------------------------
//   readInt
//---------------------------------------------------------

int Capella::readInt()
      {
      char c;
      read(&c, 1);
      if (c == -128) {
            short s;
            read(&s, 2);
            return s;
            }
      else if (c == 127) {
            int s;
            read(&s, 4);
            return s;
            }
      else
            return c;
      }

//---------------------------------------------------------
//   readString
//---------------------------------------------------------

char* Capella::readString()
      {
      unsigned len = readUnsigned();
      char* buffer = new char[len + 1];
      read(buffer, len);
      buffer[len] = 0;
      return buffer;
      }

//---------------------------------------------------------
//   readColor
//---------------------------------------------------------

QColor Capella::readColor()
      {
      static const int colors[] = {
            0x000000, // schwarz
            0x000080, // dunkelrot
            0x008000, // dunkelgrün
            0x008080, // ocker
            0x800000, // dunkelblau
            0x800080, // purpurrot
            0x808000, // blaugün
            0x808080, // grau
            0xC0C0C0, // hellgrau
            0x0000FF, // rot
            0x00FF00, // grün
            0x00FFFF, // gelb
            0xFF0000, // blau
            0xFF00FF, // lila
            0xFFFF00, // aquamarin
            0xFFFFFF  // weiß
            };

      QColor c;
      unsigned char b = readByte();
      if (b >= 16) {
            assert(b == 255);
            int r = readByte();
            int g = readByte();
            int b = readByte();
            c = QColor(r, g, b);
            }
      else {
            c = QColor(colors[b]);
            }
      return c;
      }

//---------------------------------------------------------
//   readFont
//---------------------------------------------------------

QFont Capella::readFont()
      {
      int index = readUnsigned();
      if (index == 0) {
            int lfHeight           = readLong();
            /*int lfWidth            =*/ readLong();
            /*int lfEscapement       =*/ readLong();
            /*int lfOrientation      =*/ readLong();
            int lfWeight           = readLong();
            uchar lfItalic         = readByte();
            uchar lfUnderline      = readByte();
            uchar lfStrikeOut      = readByte();
            /*uchar lfCharSet        =*/ readByte();
            /*uchar lfOutPrecision   =*/ readByte();
            /*uchar lfClipPrecision  =*/ readByte();
            /*uchar lfQuality        =*/ readByte();
            /*uchar lfPitchAndFamily =*/ readByte();
            QColor color           = readColor();
            char* face             = readString();

// printf("Font <%s> size %d, weight %d\n", face, lfHeight, lfWeight);
            QFont font(face);
            font.setPointSizeF(lfHeight / 1000.0);
            font.setItalic(lfItalic);
            font.setStrikeOut(lfStrikeOut);
            font.setUnderline(lfUnderline);
            font.setWeight(lfWeight / 10);
            fonts.append(font);
            return font;
            }
      index -= 1;
      if (index >= fonts.size()) {
            printf("illegal font index %d (max %d)\n", index, fonts.size()-1);
            }
      return fonts[index];
      }

//---------------------------------------------------------
//   readStaveLayout
//---------------------------------------------------------

void Capella::readStaveLayout(CapStaffLayout* sl, int /*idx*/)
      {
      sl->barlineMode = readByte();
      sl->noteLines   = readByte();
      switch(sl->noteLines) {
            case 1:     break;      // one line
            case 2:     break;      // five lines
            default:
                  {
                  char lines[11];
                  f->read(lines, 11);
                  curPos += 11;
                  }
                  break;
            }
//      printf("StaffLayout %d: noteLines %d\n", idx, sl->noteLines);

      sl->bSmall      = readByte();
      sl->topDist      = readInt();
      sl->btmDist      = readInt();
      sl->groupDist    = readInt();
      sl->barlineFrom = readByte();
      sl->barlineTo   = readByte();

      unsigned char clef = readByte();
      sl->form = clef & 7;
      sl->line = (clef >> 3) & 7;
      sl->oct  = (clef >> 6);
//      printf("   clef %x  form %d, line %d, oct %d\n", clef, sl->form, sl->line, sl->oct);

        // Schlagzeuginformation
      unsigned char b   = readByte();
      sl->bPercussion  = b & 1;    // Schlagzeugkanal verwenden
      sl->bSoundMapIn  = b & 2;
      sl->bSoundMapOut = b & 4;
      if (sl->bSoundMapIn) {      // Umleitungstabelle für Eingabe vom Keyboard
            unsigned char iMin = readByte();
            unsigned char n    = readByte();
            assert (n > 0 and iMin + n <= 128);
            f->read(sl->soundMapIn, n);
            curPos += n;
            }
      if (sl->bSoundMapOut) {     // Umleitungstabelle für das Vorspielen
            unsigned char iMin = readByte();
            unsigned char n    = readByte();
            assert (n > 0 and iMin + n <= 128);
            f->read(sl->soundMapOut, n);
            curPos += n;
            }
      sl->sound = readInt();
      sl->volume = readInt();
      sl->transp = readInt();
//      printf("   sound %d vol %d transp %d\n", sl->sound, sl->volume, sl->transp);

      sl->descr              = readString();
      sl->name               = readString();
      sl->abbrev             = readString();
      sl->intermediateName   = readString();
      sl->intermediateAbbrev = readString();
//      printf("   descr <%s> name <%s>  abbrev <%s> iname <%s> iabrev <%s>\n",
//         sl->descr, sl->name, sl->abbrev, sl->intermediateName, sl->intermediateAbbrev);
      }

//---------------------------------------------------------
//   readLayout
//---------------------------------------------------------

void Capella::readLayout()
      {
      smallLineDist  = readInt();
      normalLineDist = readInt();
      topDist        = readInt();
      interDist      = readInt();

      txtAlign   = readByte();    // Stimmenbezeichnungen 0=links, 1=zentriert, 2=rechts
      adjustVert = readByte();    // 0=nein, 1=au�er letzte Seite, 3=alle Seiten

      unsigned char b          = readByte();
      redundantKeys    = b & 1;
      modernDoubleNote = b & 2;
      assert ((b & 0xFC) == 0); // bits 2...7 reserviert

      bSystemSeparators = readByte();
      nUnnamed           = readInt();

      namesFont = readFont();

      // Musterzeilen
      unsigned nStaveLayouts = readUnsigned();

//      printf("%d staves\n", nStaveLayouts);

      for (unsigned iStave = 0; iStave < nStaveLayouts; iStave++) {
            CapStaffLayout* sl = new CapStaffLayout;
            readStaveLayout(sl, iStave);
            staves.append(sl);
            }

      // system brackets:
      unsigned n = readUnsigned();  // number of brackets
      for (unsigned int i = 0; i < n; i++) {
            CapBracket b;
            b.from   = readInt();
            b.to     = readInt();
            b.curly = readByte();
//            printf("Bracket%d %d-%d curly %d\n", i, b.from, b.to, b.curly);
            brackets.append(b);
            }
      }

//---------------------------------------------------------
//   readExtra
//---------------------------------------------------------

void Capella::readExtra()
      {
      uchar n = readByte();
      if (n) {
            printf("Capella::readExtra(%d)\n", n);
            for (int i = 0; i < n; ++i)
                  readByte();
            }
      }

//---------------------------------------------------------
//   CapClef::read
//---------------------------------------------------------

void CapClef::read()
      {
      unsigned char b = cap->readByte();
      form            = (FORM) (b & 7);
      line            = (LINE) ((b >> 3) & 7);
      oct             = (OCT)  (b >> 6);
      }

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

int CapClef::clef() const
      {
      int idx = form + (line << 3) + (oct << 5);
      switch(idx) {
            case FORM_G + (LINE_2 << 3) + (OCT_NULL << 5):  return CLEF_G;
            case FORM_G + (LINE_2 << 3) + (OCT_ALTA << 5):  return CLEF_G1;
            case FORM_G + (LINE_2 << 3) + (OCT_BASSA << 5): return CLEF_G3;

            case FORM_C + (LINE_1 << 3) + (OCT_NULL << 5):  return CLEF_C1;
            case FORM_C + (LINE_2 << 3) + (OCT_NULL << 5):  return CLEF_C2;
            case FORM_C + (LINE_3 << 3) + (OCT_NULL << 5):  return CLEF_C3;
            case FORM_C + (LINE_4 << 3) + (OCT_NULL << 5):  return CLEF_C4;
            case FORM_C + (LINE_5 << 3) + (OCT_NULL << 5):  return CLEF_C5;

            case FORM_F + (LINE_4 << 3) + (OCT_NULL << 5):  return CLEF_F;
            case FORM_F + (LINE_4 << 3) + (OCT_BASSA << 5): return CLEF_F8;
            case FORM_F + (LINE_3 << 3) + (OCT_NULL << 5):  return CLEF_F_B;
            case FORM_F + (LINE_5 << 3) + (OCT_NULL << 5):  return CLEF_F_C;
            default:
                  printf("unknown clef %d %d %d\n", form, line, oct);
                  break;
            }
      return -1;
      }

//---------------------------------------------------------
//   CapKey::read
//---------------------------------------------------------

void CapKey::read()
      {
      unsigned char b = cap->readByte();
      signature = int(b) - 7;
// printf("         Key %d\n", signature);
      }

//---------------------------------------------------------
//   CapMeter::read
//---------------------------------------------------------

void CapMeter::read()
      {
      numerator = cap->readByte();
      uchar d   = cap->readByte();
      log2Denom = (d & 0x7f) - 1;
      allaBreve = d & 0x80;
      if (log2Denom > 7 || log2Denom < 0) {
            printf("   Meter %d/%d allaBreve %d\n", numerator, log2Denom, allaBreve);
            printf("   illegal fraction\n");
            // abort();
            log2Denom = 2;
            numerator = 4;
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void WedgeObj::read()
      {
      LineObj::read();
      char b = cap->readByte();
      height = b & 0x7f;
      decresc = b & 0x80;
      }

//---------------------------------------------------------
//   CapExplicitBarline::read
//---------------------------------------------------------

void CapExplicitBarline::read()
      {
      enum {BAR_SINGLE, BAR_DOUBLE, BAR_END,
            BAR_REPEND, BAR_REPSTART, BAR_REPENDSTART};

      unsigned char b = cap->readByte();
      type = b & 0x0f;
      barMode = b >> 4;         // 0 = auto, 1 = nur Zeilen, 2 = durchgezogen
      assert (type <= BAR_REPENDSTART);
      assert (barMode <= 2);

// printf("         Expl.Barline type %d mode %d\n", type, barMode);
      }

//---------------------------------------------------------
//   readVoice
//---------------------------------------------------------

void Capella::readVoice(CapStaff* cs, int idx)
      {
// printf("      Voice %d\n", idx);
      if (readChar() != 'C')
            throw CAP_BAD_VOICE_SIG;

      enum {T_REST, T_CHORD, T_CLEF, T_KEY, T_METER,
            T_EXPL_BARLINE, T_IMPL_BARLINE, T_PAGE_BKGR
            };

      CapVoice* v   = new CapVoice;
      v->voiceNo    = idx;
      v->y0Lyrics   = readByte();
      v->dyLyrics   = readByte();
      v->lyricsFont = readFont();
      v->stemDir    = readByte();
      readExtra();

      unsigned nNoteObjs = readUnsigned();          // Notenobjekte
      for (unsigned i = 0; i < nNoteObjs; i++) {
            QColor color       = Qt::black;
            uchar type = readByte();
            readExtra();
            if (type != T_REST && type != T_CHORD && type != T_PAGE_BKGR)
                  color = readColor();
            // Die anderen Objekttypen haben eine komprimierte Farbcodierung
            switch (type) {
                  case T_REST:
                        {
                        RestObj* rest = new RestObj(this);
                        rest->read();
                        v->objects.append(rest);
                        }
                        break;
                  case T_CHORD:
                  case T_PAGE_BKGR:
                        {
                        ChordObj* chord = new ChordObj(this);
                        chord->read();
                        v->objects.append(chord);
                        }
                        break;
                  case T_CLEF:
                        {
                        CapClef* clef = new CapClef(this);
                        clef->read();
                        v->objects.append(clef);
                        }
                        break;
                  case T_KEY:
                        {
                        CapKey* key = new CapKey(this);
                        key->read();
                        v->objects.append(key);
                        }
                        break;
                  case T_METER:
                        {
                        CapMeter* meter = new CapMeter(this);
                        meter->read();
                        v->objects.append(meter);
                        }
                        break;
                  case T_EXPL_BARLINE:
                        {
                        CapExplicitBarline* bl = new CapExplicitBarline(this);
                        bl->read();
                        v->objects.append(bl);
                        }
                        break;
                  default:
                        printf("bad voice type %d\n", type);
                        abort();
                 }
            }
      cs->voices.append(v);
      }

//---------------------------------------------------------
//   readStaff
//---------------------------------------------------------

void Capella::readStaff(CapSystem* system)
      {
      if (readChar() != 'V')
            throw CAP_BAD_STAFF_SIG;

      CapStaff* staff = new CapStaff;
      //Meter
      staff->numerator = readByte();
      unsigned char d = readByte();
      staff->log2Denom = (d & 0x7f) - 1;
      staff->allaBreve = d & 0x80;

      staff->iLayout = readByte();
      staff->topDistX = readInt();
      staff->btmDistX = readInt();
      staff->color = readColor();
      readExtra();

// printf("      Staff iLayout %d\n", staff->iLayout);
      // Stimmen
      unsigned nVoices = readUnsigned();
      for (unsigned i = 0; i < nVoices; i++)
            readVoice(staff, i);
      system->staves.append(staff);
      }

//---------------------------------------------------------
//   readSystem
//---------------------------------------------------------

void Capella::readSystem()
      {
      CapSystem* s = new CapSystem;

      if (readChar() != 'S')
            throw CAP_BAD_SYSTEM_SIG;

      s->nAddBarCount   = readInt();
      s->bBarCountReset = readByte();
      s->explLeftIndent = readByte();

      s->beamMode = readByte();
      s->tempo    = readUnsigned();
      s->color    = readColor();
      readExtra();

      unsigned char b  = readByte();
      s->bJustified    = b & 2;
      s->bPageBreak    = (b & 4) != 0;
      s->instrNotation = (b >> 3) & 7;

      unsigned nStaves = readUnsigned();
      for (unsigned i = 0; i < nStaves; i++)
            readStaff(s);
      systems.append(s);
      }

//---------------------------------------------------------
//   toTicks
//---------------------------------------------------------

int BasicDurationalObj::ticks() const
      {
      if (noDuration)
            return 0;
      int len = 0;
      switch (t) {
            case D1:          len = 4 * AL::division; break;
            case D2:          len = 2 * AL::division; break;
            case D4:          len = AL::division; break;
            case D8:          len = AL::division >> 1; break;
            case D16:         len = AL::division >> 2; break;
            case D32:         len = AL::division >> 3; break;
            case D64:         len = AL::division >> 4; break;
            case D128:        len = AL::division >> 5; break;
            case D256:        len = AL::division >> 6; break;
            case D_BREVE:     len = AL::division * 8; break;
            default:
                  printf("BasicDurationalObj::ticks: illegal duration value %d\n", t);
                  break;
            }
      int slen = len;
      int dots = nDots;
      while (dots--) {
            slen >>= 1;
            len += slen;
            }
      return len;
      }

//---------------------------------------------------------
//   readPoint
//---------------------------------------------------------

QPoint Capella::readPoint()
      {
      int x = readInt();
      int y = readInt();
      return QPoint(x, y);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Capella::read(QFile* fp)
      {
      f      = fp;
      curPos = 0;

      char signature[9];
      read(signature, 8);
      signature[8] = 0;
      if (memcmp(signature, "cap3-v:", 7) != 0)
            throw CAP_BAD_SIG;

      printf("read Capella file signature <%s>\n", signature);

      // TODO: test for signature[7] = a-z

      author   = readString();
      keywords = readString();
      comment  = readString();
//      printf("author <%s> keywords <%s> comment <%s>\n", author, keywords, comment);

      nRel   = readUnsigned();            // 75
      nAbs   = readUnsigned();            // 16
      unsigned char b   = readByte();
      bUseReadSize      = b & 1;
      bAllowCompression = b & 2;
      bPrintLandscape   = b & 16;

//      printf("  nRel %d  nAbs %d useReadSize %d compresseion %d\n", nRel, nAbs, bUseReadSize, bAllowCompression);

      readLayout();

      beamRelMin0 = readByte();        // Grundeinstellungen f�r Balkensteigung
      beamRelMin1 = readByte();
      beamRelMax0 = readByte();
      beamRelMax1 = readByte();

      readExtra();

      readDrawObjectArray();

      unsigned n = readUnsigned();
      if (n) {
            printf("Gallery objects\n");
            }
      for (unsigned int i = 0; i < n; ++i) {
            /*char* s =*/ readString();       // names of galerie objects
//            printf("Galerie: <%s>\n", s);
            }

      backgroundChord = new ChordObj(this);
      backgroundChord->read();              // contains graphic objects on the page background
      bShowBarCount    = readByte();        // Taktnumerierung zeigen
      barNumberFrame   = readByte();        // 0=kein, 1=Rechteck, 2=Ellipse
      nBarDistX        = readByte();
      nBarDistY        = readByte();
      QFont barNumFont = readFont();
      nFirstPage       = readUnsigned();    // Versatz fuer Seitenzaehlung
      leftPageMargins  = readUnsigned();    // Seitenraender
      topPageMargins   = readUnsigned();
      rightPageMargins = readUnsigned();
      btmPageMargins   = readUnsigned();

      unsigned nSystems  = readUnsigned();
      for (unsigned i = 0; i < nSystems; i++) {
            printf("readSystem %d of %d\n", i, nSystems);
            readSystem();
            }
      char esig[4];
      read(esig, 4);
      if (memcmp (esig, "\0\0\0\0", 4) != 0)
            throw CAP_BAD_SIG;
      }

//---------------------------------------------------------
//   importCapella
//---------------------------------------------------------

bool Score::importCapella(const QString& name)
      {
      if (name.isEmpty())
            return false;
      QFile fp(name);
      if (!fp.open(QIODevice::ReadOnly))
            return false;

      Capella cf;
      try {
            cf.read(&fp);
            }
      catch(Capella::CapellaError errNo) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: Import Capella"),
               QString("Load failed: ") + cf.error(errNo),
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            fp.close();
            // avoid another error message box
            return true;
            }
      fp.close();

      _saved = false;
      convertCapella(&cf);
      _created = true;
      return true;
      }

//---------------------------------------------------------
//   readCapVoice
//---------------------------------------------------------

int Score::readCapVoice(CapVoice* cvoice, int staffIdx, int tick)
      {
      int voice = cvoice->voiceNo;
      int track = staffIdx * VOICES + voice;

      //
      // pass I
      //
      int startTick = tick;
// printf("=====readCapVoice at staff %d voice %d tick %d\n", staffIdx, voice, tick);

      foreach(NoteObj* no, cvoice->objects) {
            switch(no->type()) {
                  case T_REST:
                        {
                        Measure* m = getCreateMeasure(tick);
                        RestObj* o = static_cast<RestObj*>(no);
                        int ticks  = o->ticks();
                        int ft     = m->tickLen();
                        if (o->fullMeasures) {
// printf("full measure rests %d, invisible %d len %d %d\n",
//    o->fullMeasures, o->invisible, ticks, ft);
                              ticks = ft * o->fullMeasures;
                              if (!o->invisible) {
                                    for (unsigned i = 0; i < o->fullMeasures; ++i) {
                                          Measure* m = getCreateMeasure(tick + i * ft);
                                          Segment* s = m->getSegment(SegChordRest, tick + i * ft);
                                          Rest* rest = new Rest(this);
                                          rest->setTick(tick + i * ft);
                                          rest->setDuration(Duration(Duration::V_MEASURE));
                                          rest->setTrack(staffIdx * VOICES + voice);
                                          s->add(rest);
                                          }
                                    }
                              }
                        if (!o->invisible) {
                              Segment* s = m->getSegment(SegChordRest, tick);
                              Rest* rest = new Rest(this);
                              rest->setTick(tick);
                              Duration d;
                              if (o->fullMeasures)
                                    d.setType(Duration::V_MEASURE);
                              else
                                    d.setVal(ticks);
                              rest->setDuration(d);
                              rest->setTrack(track);
                              s->add(rest);
                              }
                        tick += ticks;
                        }
                        break;
                  case T_CHORD:
                        {
                        ChordObj* o = static_cast<ChordObj*>(no);
                        int ticks = o->ticks();
                        Measure* m = getCreateMeasure(tick);
                        Segment* s = m->getSegment(SegChordRest, tick);
                        Chord* chord = new Chord(this);
                        chord->setTick(tick);
                        Duration d;
                        d.setVal(ticks);
                        chord->setDuration(d);
                        chord->setTrack(track);
                        switch (o->stemDir) {
                              case -1:    // down
                                    chord->setStemDirection(DOWN);
                                    break;
                              case 1:     // up
                                    chord->setStemDirection(UP);
                                    break;
                              case 3:     // no stem
                                    chord->setNoStem(true);
                                    break;
                              case 0:     // auto
                              default:
                                    break;
                              }
                        s->add(chord);
                        int clef = staff(staffIdx)->clef(tick);
                        int key  = staff(staffIdx)->key(tick).accidentalType();
                        int off;
                        switch(clef) {
                              case CLEF_F:    off = -14; break;
                              case CLEF_F_B:  off = -14; break;
                              case CLEF_F_C:  off = -14; break;
                              case CLEF_C1:   off = -7; break;
                              case CLEF_C2:   off = -7; break;
                              case CLEF_C3:   off = -7; break;
                              case CLEF_C4:   off = -7; break;
                              case CLEF_C5:   off = -7; break;
                              default:
                                    off = 0;
                              }

                        static int keyOffsets[15] = {
                         //   -7 -6 -5 -4 -3 -2 -1  0  1  2  3  4  5  6  7
                              7,  4, 1, 5, 2, 6, 3, 0, 4, 1, 5, 2, 6, 3, 0
                              };
                        off += keyOffsets[key + 7];

                        foreach(CNote n, o->notes) {
                              Note* note = new Note(this);
                              int l = n.pitch + off + 7 * 6;
                              int octave = 0;
                              while (l < 0) {
                                    l += 7;
                                    octave--;
                                    }
                              octave += l / 7;
                              l       = l % 7;

                              int pitch = pitchKeyAdjust(l, key) + octave * 12;
                              pitch += n.alteration;

                              if (pitch > 127)
                                    pitch = 127;
                              else if (pitch < 0)
                                    pitch = 0;

                              note->setPitch(pitch);
                              note->setTpcFromPitch();
                              /*int alter1 =*/ tpc2alter(note->tpc());
                              int _tpc = pitch2tpc(note->pitch(), 0);
                              /*int alter2 =*/ tpc2alter(_tpc);

// printf("pitch %d (alter %d), %d - %d\n", pitch, n.alteration, alter1, alter2);
                              // note->setTpc(tpc);

                              chord->add(note);
                              if (o->rightTie) {
                                    Tie* tie = new Tie(this);
                                    tie->setStartNote(note);
                                    tie->setTrack(track);
                                    note->setTieFor(tie);
                                    }
                              }
                        foreach(Verse v, o->verse) {
                              Lyrics* l = new Lyrics(this);
                              l->setTrack(track);
                              l->setText(v.text);
                              if (v.hyphen)
                                    l->setSyllabic(Lyrics::BEGIN);
                              l->setNo(v.num);
                              s->add(l);
                              }
                        tick += ticks;
                        }
                        break;
                  case T_CLEF:
                        {
                        CapClef* o = static_cast<CapClef*>(no);
// printf("%d:%d <Clef> %s line %d oct %d\n", tick, staffIdx, o->name(), o->line, o->oct);
                        int nclef = o->clef();
                        if (nclef == -1)
                              break;
                        staff(staffIdx)->changeClef(tick, nclef);
                        }
                        break;
                  case T_KEY:
                        {
                        CapKey* o = static_cast<CapKey*>(no);
                        // printf("%d:%d <Key> %d\n", tick, staffIdx, o->signature);
                        int key = staff(staffIdx)->key(tick).accidentalType();
                        if (key != o->signature) {
                              staff(staffIdx)->setKey(tick, o->signature);
                              KeySig* ks = new KeySig(this);
                              ks->setTrack(staffIdx * VOICES);
                              Measure* m = getCreateMeasure(tick);
                              Segment* s = m->getSegment(SegKeySig, tick);
                              s->add(ks);
                              ks->setSig(key, o->signature);
                              }
                        }
                        break;
                  case T_METER:
                        {
                        CapMeter* o = static_cast<CapMeter*>(no);
                        if (o->log2Denom > 7 || o->log2Denom < 0)
                              break;
                        AL::SigEvent se = sigmap()->timesig(tick);
                        AL::SigEvent ne(Fraction(o->numerator, 1 << o->log2Denom));
                        if (!(se == ne))
                              sigmap()->add(tick, ne);
                        TimeSig* ts = new TimeSig(this);
                        ts->setSig(1 << o->log2Denom, o->numerator);
                        ts->setTick(tick);
                        ts->setTrack(staffIdx * VOICES + voice);
                        Measure* m = getCreateMeasure(tick);
                        Segment* s = m->getSegment(SegTimeSig, tick);
                        s->add(ts);
                        }
                        break;
                  case T_EXPL_BARLINE:
                        printf("<Barline>\n");
                        break;
                  case T_PAGE_BKGR:
                        printf("<PageBreak>\n");
                        break;
                  }
            }

      //
      // pass II
      //
      tick = startTick;
      foreach(NoteObj* no, cvoice->objects) {
            BasicDurationalObj* d = 0;
            if (no->type() == T_REST)
                  d = static_cast<BasicDurationalObj*>(static_cast<RestObj*>(no));
            else if (no->type() == T_CHORD)
                  d = static_cast<BasicDurationalObj*>(static_cast<ChordObj*>(no));
            if (!d)
                  continue;
            foreach(BasicDrawObj* o, d->objects) {
                  switch (o->type) {
                        case CAP_SIMPLE_TEXT:
                              // printf("simple text at %d\n", tick);
                              break;
                        case CAP_WAVY_LINE:
                              break;
                        case CAP_SLUR:
                              {
                              SlurObj* so = static_cast<SlurObj*>(o);
                              // printf("slur tick %d  %d-%d-%d-%d   %d-%d\n", tick, so->nEnd, so->nMid,
                              //   so->nDotDist, so->nDotWidth, so->nRefNote, so->nNotes);
                              Segment* seg = tick2segment(tick);
                              int tick2 = -1;
                              if (seg) {
                                    int n = so->nNotes;
                                    for (seg = seg->next1(); seg; seg = seg->next1()) {
                                          if (seg->subtype() != SegChordRest)
                                                continue;
                                          if (seg->element(track))
                                                --n;
                                          else
                                                printf("  %d empty seg\n", n);
                                          if (n == 0) {
                                                tick2 = seg->tick();
                                                break;
                                                }
                                          }
                                    }
                              else
                                    printf("  segment at %d not found\n", tick);
                              if (tick2 >= 0) {
                                    Slur* slur = new Slur(this);
                                    slur->setTick(tick);
                                    slur->setTrack(track);
                                    slur->setTick2(tick2);
                                    slur->setTrack2(track);
                                    add(slur);
                                    }
                              else
                                    printf("second anchor for slur not found\n");
                              }
                              break;
                        case CAP_TEXT: {
                              extern QString rtf2html(const QString&);

                              TextObj* to = static_cast<TextObj*>(o);
                              Text* s = new Text(this);
                              QString ss = rtf2html(QString(to->text));

                              printf("string %d:%d w %d ratio %d <%s>\n",
                                 to->relPos.x(), to->relPos.y(), to->width, to->yxRatio, qPrintable(ss));
                              s->setHtml(ss);
                              MeasureBase* measure = _measures.first();
                              if (measure->type() != VBOX) {
                                    measure = new VBox(this);
                                    measure->setTick(0);
                                    addMeasure(measure);
                                    }
                              s->setSubtype(TEXT_TITLE);
                              measure->add(s);
                              }
                              break;
                        default:
                              printf("draw obj %d\n", o->type);
                        }
                  }
            int ticks = d->ticks();
            if (no->type() == T_REST) {
                  RestObj* o = static_cast<RestObj*>(no);
                  if (o->fullMeasures) {
                        Measure* m = getCreateMeasure(tick);
                        int ft     = m->tickLen();
                        ticks = ft * o->fullMeasures;
                        }
                  }
            tick += ticks;
            }
      return tick;
      }

//---------------------------------------------------------
//   convertCapella
//---------------------------------------------------------

void Score::convertCapella(Capella* cap)
      {
      if (cap->systems.isEmpty())
            return;

      int staves   = cap->systems[0]->staves.size();
      CapStaff* cs = cap->systems[0]->staves[0];
      if (cs->log2Denom <= 7)
            sigmap()->add(0, Fraction(cs->numerator, 1 << cs->log2Denom));

      Part* part = new Part(this);
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Staff* s = new Staff(this, part, staffIdx);
            part->insertStaff(s);
            _staves.push_back(s);
            }
      foreach(CapBracket cb, cap->brackets) {
            Staff* staff = _staves.value(cb.from);
            if (staff == 0) {
                  printf("bad bracket 'from' value\n");
                  continue;
                  }
            staff->setBracket(0, cb.curly ? 1 : 0);
            staff->setBracketSpan(0, cb.to - cb.from + 1);
            }

      foreach(BasicDrawObj* o, cap->backgroundChord->objects) {
            switch(o->type) {
                  case CAP_SIMPLE_TEXT:
                        {
                        SimpleTextObj* to = static_cast<SimpleTextObj*>(o);
                        Text* s = new Text(this);
                        s->setSubtype(TEXT_TITLE);
                        s->setDefaultFont(to->font());
                        QString ss = to->text();
                        s->setText(ss);
                        MeasureBase* measure = new VBox(this);
                        measure->setTick(0);
                        addMeasure(measure);
                        measure->add(s);
                        }
                        break;
                  default:
                        printf("page background object type %d\n", o->type);
                        break;
                  }
            }


      int mtick = 0;
      foreach(CapSystem* csys, cap->systems) {
            int oldTick  = mtick;
            int staffIdx = 0;
            foreach(CapStaff* cstaff, csys->staves) {
                  int voice = 0;
                  foreach(CapVoice* cvoice, cstaff->voices) {
                        int tick = readCapVoice(cvoice, staffIdx, oldTick);
                        ++voice;
                        if (tick > mtick)
                              mtick = tick;
                        }
                  ++staffIdx;
                  }
            }
      _parts.push_back(part);
      connectTies();
      }

