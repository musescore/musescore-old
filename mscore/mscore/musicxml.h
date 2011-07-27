//=============================================================================
//  MusE Score
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

#ifndef __MUSICXML_H__
#define __MUSICXML_H__

/**
 \file
 Definition of class MusicXML
*/

#include "globals.h"

class Instrument;
class Measure;
class Tuplet;
class Tie;
class Slur;
class Part;
class Score;
class Note;
class Ottava;
class Trill;
class Pedal;
class Volta;
class TextLine;
class Chord;

//---------------------------------------------------------
//   MusicXmlWedge
//---------------------------------------------------------

struct MusicXmlWedge {
      int number;
      int startTick;
      int subType;
      qreal rx;
      qreal ry;
      bool above;
      bool hasYoffset;
      qreal yoffset;
      };

//---------------------------------------------------------
//   MusicXmlPartGroup
//---------------------------------------------------------

struct MusicXmlPartGroup {
      int span;
      int start;
      int type;
      bool barlineSpan;
      };

const int MAX_LYRICS       = 8;
const int MAX_PART_GROUPS  = 8;
const int MAX_NUMBER_LEVEL = 6; // maximum number of overlapping MusicXML objects
const int MAX_BRACKETS     = 8;

//---------------------------------------------------------
//   CreditWords
//    a single parsed MusicXML credit-words element
//---------------------------------------------------------

struct CreditWords {
      double defaultX;
      double defaultY;
      QString justify;
      QString hAlign;
      QString vAlign;
      QString words;
      CreditWords(double a, double b, QString c, QString d, QString e, QString f) {
            defaultX = a;
            defaultY = b;
            justify  = c;
            hAlign   = d;
            vAlign   = e;
            words    = f;
            }
      };

typedef  QList<CreditWords*> CreditWordsList;
typedef  CreditWordsList::iterator iCreditWords;
typedef  CreditWordsList::const_iterator ciCreditWords;

//---------------------------------------------------------
//   MusicXml
//---------------------------------------------------------

/**
 The MusicXML "creator" meta-data element.
*/

class MusicXmlCreator {
      QString _type;
      QString _text;

   public:
      MusicXmlCreator(QString& tp, QString& txt) { _type = tp; _text = txt; }
      QString crType() const                     { return _type; }
      QString crText() const                     { return _text; }
      };

//---------------------------------------------------------
//   MusicXml
//---------------------------------------------------------

/**
 The MusicXML importer.
*/

class MusicXml {
      Score* score;
      std::vector<int> voicelist[MAX_STAVES];

      Slur* slur[MAX_NUMBER_LEVEL];

      TextLine* bracket[MAX_BRACKETS];

      Tie* tie;
      int voice;
      int move;
      Volta* lastVolta;

      QDomDocument* doc;
      int tick;         ///< Current position in MusicXML time
      int maxtick;      ///< Maxtick of a measure, used to calculate measure len
      int lastMeasureLen;
      unsigned int multiMeasureRestCount;       ///< Remaining measures in a multi measure rest
      bool startMultiMeasureRest;               ///< Multi measure rest started in this measure

      int lastLen;      ///< Needed for chords
      int maxLyrics;

      int divisions;
      Tuplet* tuplet;   ///< Current tuplet

      QString composer;
      QString poet;
      QString translator;
      CreditWordsList credits;

      std::vector<MusicXmlWedge> wedgeList;
      std::vector<MusicXmlPartGroup*> partGroupList;

      Ottava* ottava;    ///< Current ottava
      Trill* trill;      ///< Current trill
      Pedal* pedal;      ///< Current pedal
      Chord* tremStart;  ///< Starting chord for current tremolo
      BeamMode beamMode; ///< Current beam mode

      //-----------------------------

      void addWedge(int no, int startPos, qreal rx, qreal ry, bool above, bool hasYoffset, qreal yoffset, int subType);
      void genWedge(int no, int endPos, Measure*, int staff);
      void doCredits();
      void direction(Measure* measure, int staff, QDomElement node);
      void scorePartwise(QDomElement);
      void xmlPartList(QDomElement);
      void xmlPart(QDomElement, QString id);
      void xmlScorePart(QDomElement node, QString id, int& parts);
      Measure* xmlMeasure(Part*, QDomElement, int);
      void xmlAttributes(Measure*, int stave, QDomElement node);
      void xmlLyric(Measure* measure, int staff, QDomElement e);
      void xmlNote(Measure*, int stave, QDomElement node);
      void xmlHarmony(QDomElement node, int tick, Measure* m);
      void xmlClef(QDomElement, int staffIdx, Measure*);

   public:
      MusicXml(QDomDocument* d);
      void import(Score*);
      };

//---------------------------------------------------------
//   XmlChorExtension
//---------------------------------------------------------

struct XmlChordExtension {
      int idx;
      const char* xmlName;
      };

enum {
      NoSystem          = 0,
      TopSystem         = 1,
      NewSystem         = 2,
      NewPage           = 3
      };

extern const XmlChordExtension chordExtensions[];

#endif
