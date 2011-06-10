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

#ifndef __ARPEGGIO_H__
#define __ARPEGGIO_H__

#include "element.h"

class Chord;
class Painter;

// Arpeggio types
enum { ARP_NORMAL, ARP_UP, ARP_DOWN, ARP_BRACKET};

//---------------------------------------------------------
//   Arpeggio
//---------------------------------------------------------

class Arpeggio : public Element {
      Spatium _userLen1;
      Spatium _userLen2;
      qreal _height;
      int _span;              // spanning staves

   public:
      Arpeggio(Score* s);
      virtual Arpeggio* clone() const  { return new Arpeggio(*this); }
      virtual ElementType type() const { return ARPEGGIO; }
      Chord* chord() const             { return (Chord*)parent(); }
      virtual QRectF bbox() const;
      virtual void draw(Painter*) const;

      void read(XmlReader* e);
      int span() const      { return _span; }
      void setSpan(int val) { _span = val; }

      void setHeight(qreal);
      };

#endif

