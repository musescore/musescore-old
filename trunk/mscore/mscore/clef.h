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

#ifndef __CLEF_H__
#define __CLEF_H__

/**
 \file
 Definition of classes Clef and ClefList.
*/

#include "element.h"

class Xml;
class ScoreView;
class Segment;

static const int NO_CLEF = -1000;

//---------------------------------------------------------
//   Clef
//---------------------------------------------------------

/**
 Graphic representation of a clef.
*/

class Clef : public Compound {
      bool _showCourtesyClef;
      bool _small;

   public:
      Clef(Score*);
      Clef(const Clef&);
      Clef(Score*, int i);
      virtual Clef* clone() const      { return new Clef(*this); }
      virtual ElementType type() const { return CLEF; }
      virtual QPointF canvasPos() const;      ///< position in canvas coordinates
      Segment* segment() const         { return (Segment*)parent(); }
      Measure* measure() const         { return (Measure*)parent()->parent(); }

      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(ScoreView*, const QPointF&, const QPointF&, Element*);
      virtual void layout();
      virtual Space space() const;
      virtual void read(QDomElement);

      bool small() const                        { return _small; }
      void setSmall(bool val);
      int tick() const;

      bool showCourtesyClef() const       { return _showCourtesyClef; };
      void setShowCourtesyClef(bool v)    { _showCourtesyClef = v;    };
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);
      };

//---------------------------------------------------------
//   ClefInfo
//---------------------------------------------------------

/**
 Info about a clef.
*/

struct ClefInfo {
      const char* tag;        ///< comprehensive name for instruments.xml
      const char* sign;       ///< Name for musicXml.
      int line;               ///< Line for musicXml.
      int octChng;            ///< Octave change for musicXml.
      int yOffset;
      int pitchOffset;        ///< Pitch offset for line 0.
      char lines[14];
      const char* name;
      };

enum {
      CLEF_G, CLEF_G1, CLEF_G2, CLEF_G3,
      CLEF_F, CLEF_F8, CLEF_F15, CLEF_F_B, CLEF_F_C,
      CLEF_C1, CLEF_C2, CLEF_C3, CLEF_C4,
      CLEF_TAB, CLEF_PERC,
      CLEF_C5, CLEF_G4,
      CLEF_F_8VA, CLEF_F_15MA,
      CLEF_PERC2,
      CLEF_MAX
      };


extern const ClefInfo clefTable[];

//---------------------------------------------------------
//   ClefList
//---------------------------------------------------------

typedef std::map<const int, int>::iterator iClefEvent;
typedef std::map<const int, int>::const_iterator ciClefEvent;

/**
 List of Clefs during time.

 This list is instantiated for every Instrument
 to keep track of clef changes.
*/

class ClefList : public std::map<const int, int> {
   public:
      ClefList() {}
      int clef(int tick) const;
      void setClef(int tick, int idx);
      void read(QDomElement, Score*);
      void write(Xml&, const char* name) const;
      void removeTime(int, int);
      void insertTime(int, int);
      };

#endif

