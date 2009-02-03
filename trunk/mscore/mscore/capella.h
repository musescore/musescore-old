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

#if 0
static const char* timeNames[] = { "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/64",
      "1/128", "1/256", "breve" };
#endif

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

   public:
      CapClef(Capella* c) : NoteObj(T_CLEF), CapellaObj(c) {}
      void read();
      const char* name() {
            static const char* formName[] = { "G", "C", "F", "=", " ", "*" };
            return formName[form];
            }
      int clef() const;

      LINE line;
      OCT  oct;
      };

//---------------------------------------------------------
//   CapKey
//---------------------------------------------------------

class CapKey : public NoteObj, public CapellaObj {

   public:
      CapKey(Capella* c) : NoteObj(T_KEY), CapellaObj(c) {}
      void read();
      int signature;
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
      int voiceNo;
      };

//---------------------------------------------------------
//   CapStaff
//---------------------------------------------------------

struct CapStaff {
      unsigned char numerator;      // default time signature
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

enum { CAP_GROUP, CAP_TRANSPOSABLE, CAP_METAFILE, CAP_SIMPLE_TEXT, CAP_TEXT, CAP_RECT_ELLIPSE,
      CAP_LINE, CAP_POLYGON, CAP_WAVY_LINE, CAP_SLUR, CAP_NOTE_LINES, CAP_WEDGE, CAP_VOLTA,
      CAP_BRACKET, CAP_GUITAR, CAP_TRILL
      };

class BasicDrawObj : public CapellaObj {
   public:
      unsigned char modeX, modeY, distY, flags;
      int nRefNote;
      int nNotes;
      bool background;
      int pageRange;
      int type;

      BasicDrawObj(int t, Capella* c) : CapellaObj(c) { type = t; }
      void read();
      };

//---------------------------------------------------------
//   BasicRectObj
//---------------------------------------------------------

class BasicRectObj : public BasicDrawObj {
   public:
      BasicRectObj(int t, Capella* c) : BasicDrawObj(t, c) {}
      void read();

      QPoint relPos;
      int width;
      int yxRatio;
      int height;
      };

//---------------------------------------------------------
//   GroupObj
//---------------------------------------------------------

class GroupObj : public BasicDrawObj {
   public:
      GroupObj(Capella* c) : BasicDrawObj(CAP_GROUP, c) {}
      void read();

      QPoint relPos;
      QList<BasicDrawObj*> objects;
      };

//---------------------------------------------------------
//   TransposableObj
//---------------------------------------------------------

class TransposableObj : public BasicDrawObj {
   public:
      TransposableObj(Capella* c) : BasicDrawObj(CAP_TRANSPOSABLE, c) {}
      void read();

      QPoint relPos;
      char b;
      QList<BasicDrawObj*> variants;
      };

//---------------------------------------------------------
//   MetafileObj
//---------------------------------------------------------

class MetafileObj : public BasicDrawObj {
   public:
      MetafileObj(Capella* c) : BasicDrawObj(CAP_METAFILE, c) {}
      void read();
      };

//---------------------------------------------------------
//   LineObj
//---------------------------------------------------------

class LineObj : public BasicDrawObj {

   public:
      LineObj(Capella* c) : BasicDrawObj(CAP_LINE, c) {}
      LineObj(int t, Capella* c) : BasicDrawObj(t, c) {}
      void read();

      QPoint pt1, pt2;
      QColor color;
      char lineWidth;
      };

//---------------------------------------------------------
//   RectEllipseObj
//---------------------------------------------------------

class RectEllipseObj : public LineObj {    // special
   public:
      RectEllipseObj(Capella* c) : LineObj(CAP_RECT_ELLIPSE, c) {}
      void read();

      int radius;
      bool bFilled;
      QColor clrFill;
      };

//---------------------------------------------------------
//   PolygonObj
//---------------------------------------------------------

class PolygonObj : public BasicDrawObj {
   public:
      PolygonObj(Capella* c) : BasicDrawObj(CAP_POLYGON, c) {}
      void read();

      bool bFilled;
      unsigned lineWidth;
      QColor clrFill;
      QColor clrLine;
      };

//---------------------------------------------------------
//   WavyLineObj
//---------------------------------------------------------

class WavyLineObj : public LineObj {
   public:
      WavyLineObj(Capella* c) : LineObj(CAP_WAVY_LINE, c) {}
      void read();

      unsigned waveLen;
      bool adapt;
      };

//---------------------------------------------------------
//   NotelinesObj
//---------------------------------------------------------

class NotelinesObj : public BasicDrawObj {
   public:
      NotelinesObj(Capella* c) : BasicDrawObj(CAP_NOTE_LINES, c) {}
      void read();

      int x0, x1, y;
      QColor color;
      };

//---------------------------------------------------------
//   VoltaObj
//---------------------------------------------------------

class VoltaObj : public BasicDrawObj {
   public:
      VoltaObj(Capella* c) : BasicDrawObj(CAP_VOLTA, c) {}
      void read();

      int x0, x1, y;
      QColor color;

      bool bLeft;
      bool bRight;
      bool bDotted;
      bool allNumbers;

      int from, to;
      };

//---------------------------------------------------------
//   GuitarObj
//---------------------------------------------------------

class GuitarObj : public BasicDrawObj {
   public:
      GuitarObj(Capella* c) : BasicDrawObj(CAP_GUITAR, c) {}
      void read();

      QPoint relPos;
      QColor color;
      short flags;
      int strings;      // 8 Saiten in 8 Halbbytes
      };

//---------------------------------------------------------
//   TrillObj
//---------------------------------------------------------

class TrillObj : public BasicDrawObj {
   public:
      TrillObj(Capella* c) : BasicDrawObj(CAP_TRILL, c) {}
      void read();

      int x0, x1, y;
      QColor color;
      bool trillSign;
      };

//---------------------------------------------------------
//   SlurObj
//---------------------------------------------------------

class SlurObj : public BasicDrawObj {
      QPoint bezierPoint[4];
      QColor color;

   public:
      SlurObj(Capella* c) : BasicDrawObj(CAP_SLUR, c) {}
      void read();
      unsigned char nEnd, nMid, nDotDist, nDotWidth;
      };

//---------------------------------------------------------
//   TextObj
//---------------------------------------------------------

class TextObj : public BasicRectObj {

   public:
      TextObj(Capella* c) : BasicRectObj(CAP_TEXT, c) {}
      ~TextObj() {}
      void read();

      QString text;
      };

//---------------------------------------------------------
//   SimpleTextObj
//---------------------------------------------------------

class SimpleTextObj : public BasicDrawObj {
      char* _text;
      QPoint relPos;
      unsigned char align;
      QFont _font;

   public:
      SimpleTextObj(Capella* c) : BasicDrawObj(CAP_SIMPLE_TEXT, c) { _text = 0;}
      ~SimpleTextObj() { if (_text) delete _text; }
      void read();
      QString text() const { return QString(_text); }
      QFont font() const { return _font; }
      };

//---------------------------------------------------------
//   BracketObj
//---------------------------------------------------------

class BracketObj : public LineObj {

   public:
      BracketObj(Capella* c) : LineObj(CAP_BRACKET, c) {}
      void read();

      char orientation, number;
      };

//---------------------------------------------------------
//   WedgeObj
//---------------------------------------------------------

class WedgeObj : public LineObj {

   public:
      WedgeObj(Capella* c) : LineObj(CAP_WEDGE, c) {}
      void read();

      int height;
      bool decresc;
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
      BasicDurationalObj(Capella* c) : CapellaObj(c) {}
      void read();
      int ticks() const;
      bool invisible;
      QList<BasicDrawObj*> objects;
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
   public:
      unsigned char beamMode;
      char notationStave;
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
      char stemDir;           // -1 down, 0 auto, 1 up, 3 no stem
      };

//---------------------------------------------------------
//   RestObj
//---------------------------------------------------------

class RestObj : public BasicDurationalObj, public NoteObj {
      bool bVerticalCentered;
      int vertShift;

   public:
      RestObj(Capella*);
      void read();
      unsigned fullMeasures;  // >0, multi measure rest (counting measures)
      };

//---------------------------------------------------------
//   CapFont
//---------------------------------------------------------

struct CapFont {
      QString face;
      };

//---------------------------------------------------------
//   CapBracket
//---------------------------------------------------------

struct CapBracket {
      int from, to;
      bool curly;
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

      QList<QFont> fonts;
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
      int readLong();
      short readWord();
      int readDWord();
      unsigned readUnsigned();
      char* readString();
      void readExtra();
      QList<BasicDrawObj*> readDrawObjectArray();
      void read(void* p, qint64 len);
      QFont readFont();
      QPoint readPoint();

      QList<CapSystem*> systems;
      QList<CapBracket> brackets;
      ChordObj* backgroundChord;
      };

#endif

