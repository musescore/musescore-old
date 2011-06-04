//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
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
#include "font.h"

class Tablature;
class Chord;
class Painter;

//---------------------------------------------------------
//   FretDiagram
//---------------------------------------------------------

class FretDiagram : public Element {
      int _strings;
      int maxStrings;
      int _frets;
      int _fretOffset;
      int _maxFrets;
      char* _dots;
      char* _marker;
      char* _fingering;

      qreal lw1;
      qreal lw2;             // top line
      qreal stringDist;
      qreal fretDist;
      Font font;

   public:
      FretDiagram(Score* s);
      FretDiagram(const FretDiagram&);
      ~FretDiagram();
      virtual void draw(Painter*) const;
      virtual FretDiagram* clone() const { return new FretDiagram(*this); }
      Segment* segment() const           { return (Segment*)parent(); }
      Measure* measure() const           { return (Measure*)parent()->parent(); }

      virtual ElementType type() const   { return FRET_DIAGRAM; }
      virtual void layout();
      virtual void read(XmlReader*);
      virtual QPointF canvasPos() const;

      int strings() const    { return _strings; }
      int frets()   const    { return _frets; }
      void setStrings(int n);
      void setFrets(int n)   { _frets = n; }
      void setDot(int string, int fret);
      void setMarker(int string, int marker);
      void setFingering(int string, int finger);
      int fretOffset() const      { return _fretOffset; }
      void setFretOffset(int val) { _fretOffset = val;  }
      int maxFrets() const        { return _maxFrets;   }
      void setMaxFrets(int val)   { _maxFrets = val;    }

      char* dots()      { return _dots;   }
      char* marker()    { return _marker; }
      char* fingering() { return _fingering; }
      void init(Tablature*, Chord*);
      };

#endif
