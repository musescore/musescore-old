//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#ifndef __CHORDLINE_H__
#define __CHORDLINE_H__

#include "element.h"

class Chord;
class Painter;

//---------------------------------------------------------
//   ChordLine
//    bezier line attached to top note of a chord
//    implements fall, doit, plop, bend
//---------------------------------------------------------

class ChordLine : public Element {
      QPainterPath path;
      bool modified;

   public:
      ChordLine(Score*);
      ChordLine(const ChordLine&);

      virtual ChordLine* clone() const { return new ChordLine(*this); }
      virtual ElementType type() const { return CHORDLINE; }
      virtual void setSubtype(int);
      Chord* chord() const             { return (Chord*)(parent()); }

      virtual void read(QDomElement);
      virtual void write(Xml& xml) const;
      virtual void layout();
      virtual void draw(Painter*) const;

      virtual void editDrag(int, const QPointF&);
      virtual void updateGrips(int*, QRectF*) const;
      };

#endif

