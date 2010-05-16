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

#ifndef __TREMOLO_H__
#define __TREMOLO_H__

#include "symbol.h"

class Chord;

// Tremolo subtypes:
enum { TREMOLO_1, TREMOLO_2, TREMOLO_3 };

//---------------------------------------------------------
//   Tremolo
//---------------------------------------------------------

class Tremolo : public Element {
      Chord* _chord1;
      Chord* _chord2;
      QPainterPath path;

   public:
      Tremolo(Score*);
      Tremolo &operator=(const Tremolo&);
      virtual Tremolo* clone() const  { return new Tremolo(*this); }
      virtual ElementType type() const { return TREMOLO; }
      virtual void draw(QPainter&, ScoreView*) const;
      virtual void layout();
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);

      Chord* chord1() const { return _chord1; }
      Chord* chord2() const { return _chord2; }

      void setChords(Chord* c1, Chord* c2) {
            _chord1 = c1;
            _chord2 = c2;
            }

      bool twoNotes() const { return subtype() > 2; } // is it a two note tremolo?
      };

#endif

