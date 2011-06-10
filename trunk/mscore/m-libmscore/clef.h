//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
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

#include <map>
#include "element.h"
#include "globals.h"

class Xml;
class Segment;
class Painter;

static const int NO_CLEF = -1000;

//---------------------------------------------------------
//   Clef
///   Graphic representation of a clef.
//---------------------------------------------------------

class Clef : public Element {
      QList<Element*> elements;
      bool _showCourtesyClef;
      bool _small;

   public:
      Clef(Score*);
      Clef(const Clef&);
      virtual Clef* clone() const      { return new Clef(*this); }
      virtual ElementType type() const { return CLEF; }

      virtual QPointF canvasPos() const;      ///< position in canvas coordinates
      Segment* segment() const         { return (Segment*)parent(); }
      Measure* measure() const         { return (Measure*)parent()->parent(); }

      virtual void layout();
      virtual void draw(Painter*) const;
      virtual Space space() const;
      virtual void read(XmlReader*);

      void add(Element* e, qreal x, qreal y);
      bool small() const                        { return _small; }
      void setSmall(bool val);
      int tick() const;

      bool showCourtesyClef() const       { return _showCourtesyClef; };
      void setShowCourtesyClef(bool v)    { _showCourtesyClef = v;    };

      virtual const QString subtypeName() const;
      virtual void setSubtype(const QString& s);
      void setClefType(ClefType i)     { Element::setSubtype(int(i)); }
      ClefType clefType() const        { return ClefType(subtype());  }
      static ClefType clefType(const QString& s);
      };

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
//   ClefList
//---------------------------------------------------------

typedef std::map<const int, ClefType>::iterator iClefEvent;
typedef std::map<const int, ClefType>::const_iterator ciClefEvent;

/**
 List of Clefs during time.

 This list is instantiated for every Instrument
 to keep track of clef changes.
*/

class ClefList : public std::map<const int, ClefType> {
   public:
      ClefList() {}
      ClefType clef(int tick) const;
      void setClef(int tick, ClefType idx);
//      void read(XmlReader*, Score*);
      void removeTime(int, int);
      void insertTime(int, int);
      };

#endif

