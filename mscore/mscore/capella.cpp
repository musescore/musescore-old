//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
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
      if (author)
            delete[] author;
      if (keywords)
            delete[] keywords;
      if (comment)
            delete[] comment;
      }

//---------------------------------------------------------
//   readTextObj
//---------------------------------------------------------

void Capella::readTextObj()
      {
      TextObj* txt = new TextObj(this);
      txt->read();
      }

//---------------------------------------------------------
//   readSimpleTextObj
//---------------------------------------------------------

void Capella::readSimpleTextObj()
      {
      SimpleTextObj* txt = new SimpleTextObj(this);
      txt->read();
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
printf("read textObj len %d <%s>\n", size, txt);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SimpleTextObj::read()
      {
      BasicDrawObj::read();
      relPos.setX(cap->readInt());
      relPos.setY(cap->readInt());
      align = cap->readByte();
      font  = cap->readFont();
      text  = cap->readString();
      printf("read SimpletextObj <%s>\n", text);
      }

//---------------------------------------------------------
//   readSlurObj
//---------------------------------------------------------

void Capella::readSlurObj()
      {
      SlurObj* slur = new SlurObj(this);
      slur->read();
      }

//---------------------------------------------------------
//   readDrawObjectArray
//---------------------------------------------------------

void Capella::readDrawObjectArray()
      {
      int n = readUnsigned();       // draw obj array
      for (int i = 0; i < n; ++i) {
            unsigned char type = readByte();
            switch (type) {
                  case  3:
                        readSimpleTextObj();
                        break;
                  case  4:
                        readTextObj();
                        break;
                  case 9:
                        readSlurObj();
                        break;
                  default:
printf("readDrawObjectArray type %d\n", type);
                        abort();
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   BasicDrawObj
//---------------------------------------------------------

void BasicDrawObj::read()
      {
      modeX = cap->readByte();      // anchor mode
      modeY = cap->readByte();
      distY = cap->readByte();
      flags = cap->readByte();
      nRefNote = cap->readInt();
      range = cap->readWord();
      }

//---------------------------------------------------------
//   BasicRectObj
//---------------------------------------------------------

void BasicRectObj::read()
      {
      BasicDrawObj::read();
      relPos.setX(cap->readInt());
      relPos.setY(cap->readInt());
      width   = cap->readInt();
      yxRatio = cap->readInt();
      height  = (width * yxRatio) / 0x10000;
      }

//---------------------------------------------------------
//   BasicDurationalObj
//---------------------------------------------------------

BasicDurationalObj::BasicDurationalObj(Capella* c)
   : CapellaObj(c)
      {
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
                  abort();
            }
      if (c & 0x40) {
            cap->readDrawObjectArray();
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
      if (bMultiMeasures)
            fullMeasures = cap->readUnsigned();
      if (bAddVerticalShift)
            vertShift = cap->readInt();
printf("         Rest %s invisible %d\n", timeNames[t], invisible);
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
            stemDir = cap->readChar();
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
            }
      if (flags & 0x20) {
            beamShift = cap->readChar();
            beamSlope = cap->readChar();
            }
      if (flags & 0x40) {
            unsigned nVerses = cap->readUnsigned();
            for (unsigned int i = 0; i < nVerses; ++i) {
                  Verse v;
                  unsigned char b = cap->readByte();
                  v.leftAlign = b & 1;
                  v.extender  = b & 2;
                  v.hyphen    = b & 4;
                  if (b & 9)
                        v.verseNumber = cap->readString();
                  if (b & 16)
                        v.text = cap->readString();
                  verse.append(v);
                  }
            }
printf("         Chord %s\n", timeNames[t]);
      unsigned nNotes = cap->readUnsigned();
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
printf("            Note pitch %d head %d alteration %d\n", n.pitch, n.headType, n.alteration);
            notes.append(n);
            }
      }

//---------------------------------------------------------
//    read
//    return false on error
//---------------------------------------------------------

void Capella::read(void* p, qint64 len)
      {
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
//   readUnsigned
//---------------------------------------------------------

unsigned Capella::readUnsigned()
      {
      unsigned char c;
      read(&c, 1);
      if (c >= 254) {
            if (c == 254) {
                  unsigned short s;
                  read(&s, 2);
                  return s;
                  }
            else {
                  unsigned s;
                  read(&s, 4);
                  return s;
                  }
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
      curPos += len;
      f->read(buffer, len);
      buffer[len] = 0;
      return buffer;
      }

//---------------------------------------------------------
//   readColor
//---------------------------------------------------------

QColor Capella::readColor()
      {
      QColor c;
      unsigned char b = readByte();
      if (b >= 16) {
            printf("read RGB =====================\n");
            int r = readByte();
            int g = readByte();
            int b = readByte();
            c = QColor(r, g, b);
            }
      else {
            c = Qt::black;
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
            int n = 28;
            char buffer[n];
            curPos += n;
            f->read(buffer, n);
            readColor();
            char* face = readString();
            CapFont* cf = new CapFont;
            cf->face = face;
            fonts.append(cf);
            return QFont(cf->face);
            }
      index -= 1;
      if (index >= fonts.size()) {
            printf("illegal font index %d (max %d)\n", index, fonts.size()-1);
            }
      return QFont(fonts[index]->face);
      }

//---------------------------------------------------------
//   readStaveLayout
//---------------------------------------------------------

void Capella::readStaveLayout(CapStaffLayout* sl, int idx)
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
      printf("StaffLayout %d: noteLines %d\n", idx, sl->noteLines);

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
      printf("   clef %x  form %d, line %d, oct %d\n", clef, sl->form, sl->line, sl->oct);

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
      printf("   sound %d vol %d transp %d\n", sl->sound, sl->volume, sl->transp);

      sl->descr              = readString();
      sl->name               = readString();
      sl->abbrev             = readString();
      sl->intermediateName   = readString();
      sl->intermediateAbbrev = readString();
      printf("   descr <%s> name <%s>  abbrev <%s> iname <%s> iabrev <%s>\n",
         sl->descr, sl->name, sl->abbrev, sl->intermediateName, sl->intermediateAbbrev);
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

      printf("%d staves\n", nStaveLayouts);

      for (unsigned iStave = 0; iStave < nStaveLayouts; iStave++) {
            CapStaffLayout* sl = new CapStaffLayout;
            readStaveLayout(sl, iStave);
            staves.append(sl);
            }

      // Systemklammern:
      unsigned n = readUnsigned();  // number of brackets
      for (unsigned int i = 0; i < n; i++) {
            int from   = readInt();
            int to     = readInt();
            bool curly = readByte();
            printf("Bracket%d %d-%d curly %d\n", i, from, to, curly);
            }
      }

//---------------------------------------------------------
//   readExtra
//---------------------------------------------------------

void Capella::readExtra()
      {
      unsigned char n = readByte();
      for (int i = 0; i < n; ++i)
            readByte();
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
printf("         Clef %s\n", name());
      }

//---------------------------------------------------------
//   CapKey::read
//---------------------------------------------------------

void CapKey::read()
      {
      unsigned char b = cap->readByte();
      signature = int(b) - 7;
printf("         Key %d\n", signature);
      }

//---------------------------------------------------------
//   CapMeter::read
//---------------------------------------------------------

void CapMeter::read()
      {
      numerator       = cap->readByte();
      unsigned char d = cap->readByte();
      log2Denom       = (d & 0x7f) - 1;
      allaBreve       = d & 0x80;
printf("         Meter %d/%d allaBreve %d\n", numerator, log2Denom, allaBreve);
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

printf("         Expl.Barline type %d mode %d\n", type, barMode);
      }

//---------------------------------------------------------
//   readVoice
//---------------------------------------------------------

void Capella::readVoice(CapStaff*, int idx)
      {
printf("      Voice %d\n", idx);
      if (readChar() != 'C')
            throw CAP_BAD_VOICE_SIG;

      enum {T_REST, T_CHORD, T_CLEF, T_KEY, T_METER,
            T_EXPL_BARLINE, T_IMPL_BARLINE, T_PAGE_BKGR
            };

      CapVoice* v   = new CapVoice;
      v->y0Lyrics   = readByte();
      v->dyLyrics   = readByte();
      v->lyricsFont = readFont();
      v->stemDir    = readByte();
      readExtra();

      unsigned nNoteObjs = readUnsigned();          // Notenobjekte
      for (unsigned i = 0; i < nNoteObjs; i++) {
            QColor color       = Qt::black;
            unsigned char type = readByte();
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
printf("      Staff meter %d/%d allaBreve %d\n", staff->numerator, staff->log2Denom, staff->allaBreve);

      staff->iLayout = readByte();
      staff->topDistX = readInt();
      staff->btmDistX = readInt();
      staff->color = readColor();
      readExtra();

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
//   read
//---------------------------------------------------------

void Capella::read(QFile* fp)
      {
      f      = fp;
      curPos = 0;

      char signature[8];
      read(signature, 8);
      if (memcmp(signature, "cap3-v:", 7) != 0)
            throw CAP_BAD_SIG;

      // TODO: test for signature[7] = a-z

      author   = readString();
      keywords = readString();
      comment  = readString();
      printf("author <%s> keywords <%s> comment <%s>\n", author, keywords, comment);

      nRel   = readUnsigned();
      nAbs   = readUnsigned();
      unsigned char b   = readByte();
      bUseReadSize      = b & 1;
      bAllowCompression = b & 2;
      bPrintLandscape   = b & 16;

      readLayout();

      beamRelMin0 = readByte();        // Grundeinstellungen f�r Balkensteigung
      beamRelMin1 = readByte();
      beamRelMax0 = readByte();
      beamRelMax1 = readByte();

      readExtra();

      readDrawObjectArray();

      unsigned n = readUnsigned();
      for (unsigned int i = 0; i < n; ++i) {
            char* s = readString();       // names of galerie objects
            printf("Galerie: <%s>\n", s);
            }

      backgroundChord = new ChordObj(this);
      backgroundChord->read();            // contains graphic objects on the page background

      bShowBarCount    = readByte();        // Taktnumerierung zeigen
      barNumberFrame   = readByte();        // 0=kein, 1=Rechteck, 2=Ellipse
      nBarDistX        = readByte();
      nBarDistY        = readByte();
      QFont barNumFont = readFont();
      nFirstPage       = readUnsigned();         // Versatz fuer Seitenzaehlung
      leftPageMargins  = readUnsigned();    // Seitenraender
      topPageMargins   = readUnsigned();
      rightPageMargins = readUnsigned();
      btmPageMargins   = readUnsigned();

      unsigned nSystems  = readUnsigned();
      for (unsigned i = 0; i < nSystems; i++)
            readSystem();
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
      // convertCapella(&cf);
      _created = true;
      return true;
      }

