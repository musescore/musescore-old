//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __CLEF_H__
#define __CLEF_H__

/**
 \file
 Definition of classes Clef and ClefList.
*/

#include "element.h"
#include "mscore.h"

class Xml;
class MuseScoreView;
class Segment;
class Painter;

static const int NO_CLEF = -1000;

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
//   Clef
///   Graphic representation of a clef.
//---------------------------------------------------------

class Clef : public Element {
      QList<Element*> elements;
      bool _showCourtesyClef;
      bool _showPreviousClef;       // show clef type at position tick-1
                                    // used for first clef on staff immediatly followed
                                    // by a different clef at same tick position
      bool _small;

      ClefTypeList _clefTypes;

   public:
      Clef(Score*);
      Clef(const Clef&);
      virtual Clef* clone() const      { return new Clef(*this); }
      virtual ElementType type() const { return CLEF; }

      virtual QPointF pagePos() const;      ///< position in page coordinates
      Segment* segment() const         { return (Segment*)parent(); }
      Measure* measure() const         { return (Measure*)parent()->parent(); }

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(const DropData&);
      virtual void layout();
      virtual void draw(Painter*) const;
      virtual void read(QDomElement);
      virtual void write(Xml&) const;
      virtual void add(Element* e, qreal x, qreal y);
      virtual QVariant getProperty(int propertyId) const;
      virtual void setProperty(int propertyId, const QVariant&);

      virtual Space space() const      { return Space(0.0, bbox().x() * 2.0 + width()); }

      bool small() const               { return _small; }
      void setSmall(bool val);
      int tick() const;

      bool showCourtesyClef() const       { return _showCourtesyClef; };
      void setShowCourtesyClef(bool v)    { _showCourtesyClef = v;    };

      virtual const QString subtypeName() const;
      virtual void setSubtype(const QString& s);
      static ClefType clefType(const QString& s);

      ClefType clefType() const;
      ClefTypeList clefTypeList() const     { return _clefTypes;                  }
      ClefType concertClef() const          { return _clefTypes._concertClef;     }
      ClefType transposingClef() const      { return _clefTypes._transposingClef; }
      void setConcertClef(ClefType val)     { _clefTypes._concertClef = val;      }
      void setTransposingClef(ClefType val) { _clefTypes._transposingClef = val;  }
      void setClefType(ClefType i);
      void setClefType(const ClefTypeList& ctl) { _clefTypes = ctl; }
      };

#endif

