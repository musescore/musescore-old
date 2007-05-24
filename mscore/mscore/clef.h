//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: clef.h,v 1.7 2006/03/02 17:08:33 wschweer Exp $
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

#ifndef __CLEF_H__
#define __CLEF_H__

/**
 \file
 Definition of classes Clef and ClefList.
*/

#include "element.h"

class Xml;
class Viewer;

static const int clefSmallBit = 0x1000;
static const int NO_CLEF = -1000;

//---------------------------------------------------------
//   Clef
//---------------------------------------------------------

/**
 Graphic representation of a clef.
*/

class Clef : public Compound {

   public:
      Clef(Score*);
      Clef(Score*, int i);
      virtual Clef* clone() const { return new Clef(*this); }
      virtual ElementType type() const { return CLEF; }

      void setSmall(bool val);
      virtual void setSubtype(int st);
      virtual void space(double& min, double& extra) const;
      virtual bool acceptDrop(Viewer*, const QPointF&, int, const QDomElement&) const;
      virtual Element* drop(const QPointF&, const QPointF&, int, const QDomElement&);
      };

//---------------------------------------------------------
//   ClefInfo
//---------------------------------------------------------

/**
 Info about a clef.
*/

struct ClefInfo {
      char* sign;             ///< Name for musicXml.
      int line;               ///< Line for musicXml.
      int octChng;            ///< Octave change for musicXml.
      int yOffset;
      int pitchOffset;        ///< Pitch offset for line 0.
      const char* name;
      ClefInfo(char* s, int l, int oc, int yo, int po, const char* n)
         : sign(s), line(l), octChng(oc), yOffset(yo), pitchOffset(po), name(n)
            {
            }
      };

enum {
      CLEF_G, CLEF_G1, CLEF_G2, CLEF_G3,
      CLEF_F, CLEF_F8, CLEF_F15, CLEF_F_B, CLEF_F_C,
      CLEF_C1, CLEF_C2, CLEF_C3, CLEF_C4,
      CLEF_TAB, CLEF_PERC
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

