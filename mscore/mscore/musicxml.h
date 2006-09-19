//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: musicxml.h,v 1.11 2006/04/05 08:15:12 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include "globals.h"

class Instrument;
class Measure;
class Tuplet;
class Tie;
class Slur;
class Part;
class Score;
class Note;

struct MusicXmlWedge {
      int number;
      int startTick;
      qreal rx, ry;
      int spread;
      };

const int MAX_LYRICS = 8;
const int MAX_SLURS  = 8;

//---------------------------------------------------------
//   MusicXml
//---------------------------------------------------------

class MusicXml {
      Score* score;
      std::vector<int> voicelist[MAX_STAVES];

      Slur* slur[MAX_SLURS];

      Tie* tie;
      int voice;
      int move;

      QDomDocument* doc;
      int tick;         // current position
      int maxtick;      // maxtick of a measure, used to calculate measure len
      int lastMeasureLen;

      int lastLen;      // needed for chords
      int maxLyrics;

      int beats;        // current time parameters
      int beatType;
      int divisions;
      Tuplet* tuplet;   // current tuplet

      QString title;
      QString subTitle;
      QString composer;
      QString poet;
      QString translator;

      std::vector<MusicXmlWedge> wedgeList;

      //-----------------------------

      void addWedge(int no, int startPos, qreal rx, qreal ry, int spread);
      void genWedge(int no, int endPos, Measure*, int staff, qreal rx, qreal ry, int spread);

      void direction(Measure* measure, int staff, QDomNode node);
      void scorePartwise(QDomNode);
      void xmlPartList(QDomNode);
      void xmlPart(QDomNode, QString id);
      void xmlScorePart(QDomNode node, QString id);
      void xmlMeasure(Part*, QDomNode, int);
      void xmlAttributes(Measure*, int stave, QDomNode node);
      void xmlNote(Measure*, int stave, QDomNode node);

   public:
      MusicXml(QDomDocument* d);
      void import(Score*);
      };

#endif

