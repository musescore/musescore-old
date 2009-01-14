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

#ifndef __CAPELLA_H__
#define __CAPELLA_H__

enum TIMESTEP { D1, D2, D4, D8, D16, D32, D64, D128, D256, D_BREVE };

static const char* timeNames[] = { "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/64",
      "1/128", "1/256", "breve" };

class Capella;

enum {T_REST, T_CHORD, T_CLEF, T_KEY, T_METER, T_EXPL_BARLINE, T_IMPL_BARLINE,
      T_PAGE_BKGR
      };

enum BEAM_MODE { AUTO_BEAM, FORCE_BEAM, SPLIT_BEAM };

//---------------------------------------------------------
//   CapellaObj
//---------------------------------------------------------

class CapellaObj {
   protected:
      Capella* cap;

   public:
      CapellaObj(Capella* c) { cap = c; }
      };

//---------------------------------------------------------
//   NoteObj
//---------------------------------------------------------

class NoteObj {
      int _type;

   public:
      NoteObj(int t) { _type = t; }
      int type() const  { return _type; }
      };

//---------------------------------------------------------
//   CapClef
//---------------------------------------------------------

class CapClef : public NoteObj, public CapellaObj {
      enum FORM { FORM_G, FORM_C, FORM_F, FORM_PERCUSSION,
                  FORM_NULL, CLEF_UNCHANGED
                  };
      enum LINE {LINE_5, LINE_4, LINE_3, LINE_2, LINE_1};
      enum OCT  {OCT_ALTA, OCT_NULL, OCT_BASSA};

      FORM form;
      LINE line;
      OCT  oct;

   public:
      CapClef(Capella* c) : NoteObj(T_CLEF), CapellaObj(c) {}
      void read();
      const char* name() {
            static const char* formName[] = { "G", "C", "F", "=", " ", "*" };
            return formName[form];
            }
      };

//---------------------------------------------------------
//   CapKey
//---------------------------------------------------------

class CapKey : public NoteObj, public CapellaObj {
      int signature;

   public:
      CapKey(Capella* c) : NoteObj(T_KEY), CapellaObj(c) {}
      void read();
      };

//---------------------------------------------------------
//   CapMeter
//---------------------------------------------------------

class CapMeter : public NoteObj, public CapellaObj {
   public:
      unsigned char numerator;
      int log2Denom;
      bool allaBreve;

      CapMeter(Capella* c) : NoteObj(T_METER), CapellaObj(c) {}
      void read();
      };

//---------------------------------------------------------
//   CapExplicitBarline
//---------------------------------------------------------

class CapExplicitBarline : public NoteObj, public CapellaObj {
      enum {BAR_SINGLE, BAR_DOUBLE, BAR_END,
            BAR_REPEND, BAR_REPSTART, BAR_REPENDSTART};

      int type;
      int barMode;      // 0 = auto, 1 = nur Zeilen, 2 = durchgezogen

   public:
      CapExplicitBarline(Capella* c) : NoteObj(T_IMPL_BARLINE), CapellaObj(c) {}
      void read();
      };

//---------------------------------------------------------
//   CapVoice
//---------------------------------------------------------

struct CapVoice {
      unsigned char y0Lyrics;
      unsigned char dyLyrics;
      QFont lyricsFont;
      unsigned char stemDir;
      QList<NoteObj*> objects;
      };

//---------------------------------------------------------
//   CapStaff
//---------------------------------------------------------

struct CapStaff {
      unsigned char numerator;
      int log2Denom;
      bool allaBreve;

      unsigned char iLayout;
      int topDistX;
      int btmDistX;
      QColor color;
      QList<CapVoice*> voices;
      };

//---------------------------------------------------------
//   struct CapStaffLayout
//---------------------------------------------------------

struct CapStaffLayout {
      unsigned char barlineMode;
      unsigned char noteLines;
      bool bSmall;
      int topDist;
      int btmDist;
      int groupDist;
      unsigned char barlineFrom;
      unsigned char barlineTo;

      int form, line, oct;          // clef

      // Schlagzeuginformation
      bool bPercussion;             // use drum channel
      bool bSoundMapIn;
      bool bSoundMapOut;
      char soundMapIn[128];         // Tabelle für MIDI-Töne iMin...iMin+n-1
      char soundMapOut[128];        // Tabelle für MIDI-Töne iMin...iMin+n-1

      int sound, volume, transp;

      char* descr;
      char* name;
      char* abbrev;
      char* intermediateName;
      char* intermediateAbbrev;
      };

//---------------------------------------------------------
//   CapSystem
//---------------------------------------------------------

struct CapSystem {
      int nAddBarCount;
      bool bBarCountReset;
      unsigned char explLeftIndent;      // < 0 --> Einrückung gemäß Stimmenbezeichnungen
                                         // >=  --> explizite Einrückung
      unsigned char beamMode;
      unsigned tempo;
      QColor color;                 // fuer Systemklammern
      bool bJustified;              // Randausgleich (Blocksatz)
      bool bPageBreak;              // nach dem System neue Seite anfangen
      int instrNotation;            // 0 = keine Instrumentenbezeichnung
                                    // 1 = abgekürzt, 2 = vollständig
      QList<CapStaff*> staves;
      };

//---------------------------------------------------------
//   BasicDrawObj
//---------------------------------------------------------

class BasicDrawObj : public CapellaObj {
      unsigned char modeX, modeY, distY, flags;
      int nRefNote;
      short range;

   public:
      BasicDrawObj(Capella* c) : CapellaObj(c) {}
      void read();
      };

//---------------------------------------------------------
//   BasicRectObj
//---------------------------------------------------------

class BasicRectObj : public BasicDrawObj {
      QPoint relPos;
      int width;
      int yxRatio;
      int height;

   public:
      BasicRectObj(Capella* c) : BasicDrawObj(c) {}
      void read();
      };

//---------------------------------------------------------
//   SlurObj
//---------------------------------------------------------

class SlurObj : public BasicDrawObj {
      QPoint bezierPoint[4];
      QColor color;
      unsigned char nEnd, nMid, nDotDist, nDotWidth;

   public:
      SlurObj(Capella* c) : BasicDrawObj(c) {}
      void read();
      };

//---------------------------------------------------------
//   TextObj
//---------------------------------------------------------

class TextObj : public BasicRectObj {
      char* text;

   public:
      TextObj(Capella* c) : BasicRectObj(c) { text = 0;}
      ~TextObj() { if (text) delete text; }
      void read();
      };

//---------------------------------------------------------
//   SimpleTextObj
//---------------------------------------------------------

class SimpleTextObj : public BasicDrawObj {
      char* text;
      QPoint relPos;
      unsigned char align;
      QFont font;

   public:
      SimpleTextObj(Capella* c) : BasicDrawObj(c) { text = 0;}
      ~SimpleTextObj() { if (text) delete text; }
      void read();
      };

//---------------------------------------------------------
//   BasicDurationalObj
//---------------------------------------------------------

class BasicDurationalObj : public CapellaObj {
   protected:
      int nDots;
      bool noDuration;
      bool postGrace;
      bool bSmall;
      bool notBlack;
      QColor color;
      TIMESTEP t;
      int horizontalShift;
      int count;              // tuplet
      bool tripartite;
      bool isProlonging;

   public:
      BasicDurationalObj(Capella* c);
      void read();
      int ticks() const;
      bool invisible;
      };

//---------------------------------------------------------
//   Verse
//---------------------------------------------------------

struct Verse {
      bool leftAlign;
      bool extender;
      bool hyphen;
      int num;
      QString verseNumber;
      QString text;
      };

struct CNote {
      char pitch;
      int explAlteration;     // 1 force, 2 suppress
      int headType;
      int alteration;
      int silent;
      };

//---------------------------------------------------------
//   ChordObj
//---------------------------------------------------------

class ChordObj : public BasicDurationalObj, public NoteObj {
      unsigned char beamMode;
      char notationStave;
      char stemDir;
      char dStemLength;
      unsigned char nTremoloBars;
      unsigned articulation;
      bool leftTie;
      bool rightTie;
      char beamShift;
      char beamSlope;

   public:
      ChordObj(Capella*);
      void read();
      QList<Verse> verse;
      QList<CNote> notes;
      };

//---------------------------------------------------------
//   RestObj
//---------------------------------------------------------

class RestObj : public BasicDurationalObj, public NoteObj {
      bool bVerticalCentered;
      unsigned fullMeasures;  // >0, multi measure rest (counting measures)
      int vertShift;

   public:
      RestObj(Capella*);
      void read();
      };

//---------------------------------------------------------
//   CapFont
//---------------------------------------------------------

struct CapFont {
      QString face;
      };

//---------------------------------------------------------
//   Capella
//---------------------------------------------------------

class Capella {
      static const char* errmsg[];
      int curPos;

      QFile* f;
      char* author;
      char* keywords;
      char* comment;

      unsigned char beamRelMin0;
      unsigned char beamRelMin1;
      unsigned char beamRelMax0;
      unsigned char beamRelMax1;
      unsigned nRel;                // presentation parameter
      unsigned nAbs;
      bool bUseReadSize;
      bool bAllowCompression;
      bool bPrintLandscape;

      ChordObj* backgroundChord;
      bool bShowBarCount;           // Taktnumerierung zeigen
      unsigned char barNumberFrame; // 0=kein, 1=Rechteck, 2=Ellipse
      unsigned char nBarDistX;
      unsigned char nBarDistY;
      // LogFont       barNumFont;

      unsigned nFirstPage;          // Versatz fuer Seitenzaehlung

      unsigned leftPageMargins;     // Seitenraender
      unsigned topPageMargins;
      unsigned rightPageMargins;
      unsigned btmPageMargins;

      QList<CapFont*> fonts;
      QList<CapStaffLayout*> staves;      // staff layout

      int smallLineDist;            // layout
      int normalLineDist;
      int topDist;
      int interDist;
      unsigned char txtAlign;       // Stimmenbezeichnungen 0=links, 1=zentriert, 2=rechts
      unsigned char adjustVert;     // 0=nein, 1=außer letzte Seite, 3=alle Seiten
      bool redundantKeys;
      bool modernDoubleNote;
      bool bSystemSeparators;
      int nUnnamed;
      QFont namesFont;

      void readSimpleTextObj();
      void readSlurObj();
      void readTextObj();
      void readVoice(CapStaff*, int);
      void readStaff(CapSystem*);
      void readSystem();

   protected:
      void readStaveLayout(CapStaffLayout*, int);
      void readLayout();

   public:
      enum CapellaError { CAP_NO_ERROR, CAP_BAD_SIG, CAP_EOF, CAP_BAD_VOICE_SIG,
            CAP_BAD_STAFF_SIG, CAP_BAD_SYSTEM_SIG
            };

      Capella();
      ~Capella();
      void read(QFile*);
      QString error(CapellaError n) const { return QString(errmsg[int(n)]); }

      unsigned char readByte();
      char readChar();
      QColor readColor();
      int readInt();
      short readWord();
      unsigned readUnsigned();
      char* readString();
      void readExtra();
      void readDrawObjectArray();
      void read(void* p, qint64 len);
      QFont readFont();

      QList<CapSystem*> systems;
      };

#endif

