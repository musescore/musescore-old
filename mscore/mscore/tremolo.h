//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: note.h,v 1.45 2006/03/03 16:20:42 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

// Tremolo subtypes:
enum { TREMOLO_1, TREMOLO_2, TREMOLO_3 };

//---------------------------------------------------------
//   Tremolo
//---------------------------------------------------------

class Tremolo : public Element {
      QPainterPath path;

   public:
      Tremolo(Score*);
      Tremolo &operator=(const Tremolo&);
      virtual Tremolo* clone() const  { return new Tremolo(*this); }
      virtual ElementType type() const { return TREMOLO; }
      virtual void draw(QPainter&) const;
      virtual void layout(ScoreLayout*);
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      };

#endif

