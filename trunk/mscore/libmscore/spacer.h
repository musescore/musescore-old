//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2008-2011 Werner Schweer
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

#ifndef __SPACER_H__
#define __SPACER_H__

#include "element.h"

class Painter;

// spacer subtypes
enum { SPACER_UP, SPACER_DOWN };

//---------------------------------------------------------
//   Spacer
//---------------------------------------------------------

class Spacer : public Element {
      Spatium _space;
//      QPainterPath path;

   public:
      Spacer(Score*);
      Spacer(const Spacer&);
      virtual Spacer* clone() const    { return new Spacer(*this); }
      virtual ElementType type() const { return SPACER; }

      virtual void read(XmlReader*);
      void setSpace(const Spatium& sp) { _space = sp;   }
      Spatium getSpace() const         { return _space; }
      virtual void layout();
      virtual void draw(Painter*) const;
      };

#endif
