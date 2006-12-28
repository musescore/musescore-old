//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: layoutbreak.h,v 1.1 2006/03/27 14:16:24 wschweer Exp $
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

#ifndef __LAYOUTBREAK_H__
#define __LAYOUTBREAK_H__

#include "element.h"

// layout break subtypes:

enum { LAYOUT_BREAK_PAGE, LAYOUT_BREAK_LINE };

//---------------------------------------------------------
//   LayoutBreak
//    symbols for line break, page break etc.
//---------------------------------------------------------

class LayoutBreak : public Element {
      QPainterPath path;

      virtual void draw1(Painter&);
      virtual void layout();

   public:
      LayoutBreak(Score*);
      virtual Element* clone() const { return new LayoutBreak(*this); }
      virtual ElementType type() const { return LAYOUT_BREAK; }
      };

#endif



