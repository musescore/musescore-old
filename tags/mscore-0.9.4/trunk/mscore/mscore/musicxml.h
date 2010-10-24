//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: musicxml.h,v 1.11 2006/04/05 08:15:12 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

//---------------------------------------------------------
//   MusicXmlWedge
//---------------------------------------------------------

struct MusicXmlWedge {
      int number;
      int startTick;
      int subType;
      qreal rx, ry;
      bool above;
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

const int MAX_LYRICS      = 8;
const int MAX_PART_GROUPS = 8;
const int MAX_SLURS       = 8;
const int MAX_BRACKETS    = 8;

//---------------------------------------------------------
//   MusicXml
//---------------------------------------------------------

/**
 The MusicXML importer.
*/

class MusicXml {
      Score* score;
      std::vector<int> voicelist[MAX_STAVES];

      Slur* slur[MAX_SLURS];

      TextLine* bracket[MAX_BRACKETS];

      Tie* tie;
      int voice;
      int move;
      Volta* lastVolta;

      QDomDocument* doc;
      int tick;         ///< Current position in MusicXML time
      int maxtick;      ///< Maxtick of a measure, used to calculate measure len
      int lastMeasureLen;

      int lastLen;      ///< Needed for chords
      int maxLyrics;

      int divisions;
      Tuplet* tuplet;   ///< Current tuplet

      QString composer;
      QString poet;
      QString translator;

      std::vector<MusicXmlWedge> wedgeList;
      std::vector<MusicXmlPartGroup*> partGroupList;

      Ottava* ottava;   ///< Current ottava
      Trill* trill;     ///< Current trill
      Pedal* pedal;     ///< Current pedal

      //-----------------------------

      void addWedge(int no, int startPos, qreal rx, qreal ry, bool above, int subType);
      void genWedge(int no, int endPos, Measure*, int staff);

      void direction(Measure* measure, int staff, QDomElement node);
      void scorePartwise(QDomElement);
      void xmlPartList(QDomElement);
      void xmlPart(QDomElement, QString id);
      void xmlScorePart(QDomElement node, QString id);
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

extern const XmlChordExtension chordExtensions[];

#endif