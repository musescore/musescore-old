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

#ifndef __ARTICULATION_H__
#define __ARTICULATION_H__

#include "symbol.h"

class Painter;
class ChordRest;

//---------------------------------------------------------
//   ArticulationInfo
//    gives infos about note attributes
//---------------------------------------------------------

enum ArticulationAnchor {
      A_TOP_STAFF,
      A_BOTTOM_STAFF,
      A_CHORD,          // anchor depends on chord direction
      A_TOP_CHORD,      // attribute is alway placed at top of chord
      A_BOTTOM_CHORD,   // attribute is placed at bottom of chord
      };

enum { ARTICULATION_SHOW_IN_PITCHED_STAFF = 1, ARTICULATION_SHOW_IN_TABLATURE = 2 };

struct ArticulationInfo {
      int sym;
      QString name;
      int relVelocity;        // add velocity to note/chord in percent
      int relGateTime;        // add to gateTime in percent;
      int flags;
      static int name2index();
      };

//---------------------------------------------------------
//   Articulation
//    articulation marks
//---------------------------------------------------------

class Articulation : public Symbol {
      Q_DECLARE_TR_FUNCTIONS(Articulation)

      QString _channelName;
      ArticulationAnchor _anchor;
      virtual void draw(Painter*) const;

   public:
      Articulation(Score*);
      Articulation &operator=(const Articulation&);

      virtual Articulation* clone() const { return new Articulation(*this); }
      virtual ElementType type() const     { return ARTICULATION; }

      virtual void setSubtype(int);
      virtual void read(QDomElement);
      virtual void write(Xml& xml) const;
      ArticulationType articulationType() const { return ArticulationType(subtype()); }
      QString subtypeUserName() const { return articulationList[subtype()].name; }
      int relGateTime() const { return articulationList[subtype()].relGateTime; }
      int relVelocity() const { return articulationList[subtype()].relVelocity; }
      virtual QPointF pagePos() const;      ///< position in page coordinates

      ChordRest* chordRest() const;

      static ArticulationInfo articulationList[];

      virtual const QString subtypeName() const;
      virtual void setSubtype(const QString& s);
      ArticulationAnchor anchor() const     { return _anchor;      }
      void setAnchor(ArticulationAnchor v)  { _anchor = v;         }

      QString channelName() const           { return _channelName; }
      void setChannelName(const QString& s) { _channelName = s;    }


      static QString idx2name(int idx);
      static int name2idx(const QString& name);
      };

#endif

