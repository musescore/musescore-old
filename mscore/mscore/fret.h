//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef __FRET_H__
#define __FRET_H__

#include "element.h"

//---------------------------------------------------------
//   FretDiagram
//---------------------------------------------------------

class FretDiagram : public Element {
      int _strings;
      int _frets;
      char* _dots;
      char* _marker;
      char* _fingering;
      double lw1;
      double lw2;             // top line
      double stringDist;
      double fretDist;

   public:
      FretDiagram(Score* s);
      ~FretDiagram();
      virtual void draw(QPainter&, ScoreView*) const;
      virtual FretDiagram* clone() const { return new FretDiagram(*this); }
      virtual bool isMovable() const     { return true; }

      virtual ElementType type() const { return FRET_DIAGRAM; }
      virtual void layout();
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);

      int strings() const { return _strings; }
      void setStrings(int n) { _strings = n; }
      void setDot(int string, int fret);
      void setMarker(int string, int marker);
      void setFingering(int string, int finger);
      };

#endif



