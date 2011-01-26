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
#include "globals.h"

class Xml;
class ScoreView;
class Segment;
class Painter;

static const int NO_CLEF = -1000;

//---------------------------------------------------------
//   ClefInfo
///   Info about a clef.
//---------------------------------------------------------

struct ClefInfo {
      const char* tag;        ///< comprehensive name for instruments.xml
      const char* sign;       ///< Name for musicXml.
      int line;               ///< Line for musicXml.
      int octChng;            ///< Octave change for musicXml.
      int yOffset;
      int pitchOffset;        ///< Pitch offset for line 0.
      char lines[14];
      const char* name;
      StaffGroup staffGroup;
      };

extern const ClefInfo clefTable[];

//---------------------------------------------------------
//   ClefTypeList
//---------------------------------------------------------

struct ClefTypeList {
      ClefType _concertClef;
      ClefType _transposingClef;

      ClefTypeList() {}
      ClefTypeList(ClefType a, ClefType b) : _concertClef(a), _transposingClef(b) {}
      bool operator==(const ClefTypeList& t) const;
      bool operator!=(const ClefTypeList& t) const;
      };

//---------------------------------------------------------
//   ClefList
//---------------------------------------------------------

typedef std::map<const int, ClefTypeList>::iterator iClefEvent;
typedef std::map<const int, ClefTypeList>::const_iterator ciClefEvent;

/**
 List of Clefs during time.

 This list is instantiated for every Instrument
 to keep track of clef changes.
*/

class ClefList : public std::map<const int, ClefTypeList> {
   public:
      ClefList() {}
      ClefTypeList clef(int tick) const;
      void setClef(int tick, const ClefTypeList&);
      void read(QDomElement, Score*);
      void removeTime(int, int);
      void insertTime(int, int);
      };

//---------------------------------------------------------
//   Clef
///   Graphic representation of a clef.
//---------------------------------------------------------

class Clef : public Element {
      QList<Element*> elements;
      bool _showCourtesyClef;
      bool _small;

      ClefTypeList _clefTypes;

   public:
      Clef(Score*);
      Clef(const Clef&);
      virtual Clef* clone() const      { return new Clef(*this); }
      virtual ElementType type() const { return CLEF; }

      virtual QPointF canvasPos() const;      ///< position in canvas coordinates
      Segment* segment() const         { return (Segment*)parent(); }
      Measure* measure() const         { return (Measure*)parent()->parent(); }

      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(ScoreView*, const QPointF&, const QPointF&, Element*);
      virtual void layout();
      virtual void draw(Painter*) const;
      virtual Space space() const;
      virtual void read(QDomElement);
      virtual void write(Xml&) const;

      void add(Element* e, double x, double y);
      bool small() const                        { return _small; }
      void setSmall(bool val);
      int tick() const;

      bool showCourtesyClef() const       { return _showCourtesyClef; };
      void setShowCourtesyClef(bool v)    { _showCourtesyClef = v;    };
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);

      virtual const QString subtypeName() const;
      virtual void setSubtype(const QString& s);
      static ClefType clefType(const QString& s);

      ClefType clefType() const             { return ClefType(subtype());         }
      ClefTypeList clefTypeList() const     { return _clefTypes;                  }
      ClefType concertClef() const          { return _clefTypes._concertClef;     }
      ClefType transposingClef() const      { return _clefTypes._transposingClef; }
      void setConcertClef(ClefType val)     { _clefTypes._concertClef = val;      }
      void setTransposingClef(ClefType val) { _clefTypes._transposingClef = val;  }
      void setClefType(ClefType i);
      void setClefType(const ClefTypeList& ctl) { _clefTypes = ctl; }
      };

#endif

