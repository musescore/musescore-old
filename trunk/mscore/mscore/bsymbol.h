//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: element.h,v 1.58 2006/04/12 14:58:10 wschweer Exp $
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

#ifndef __BSYMBOL_H__
#define __BSYMBOL_H__

#include "element.h"
#include "style.h"

//---------------------------------------------------------
//   BSymbol
//    base class for Symbol and Image
//---------------------------------------------------------

class BSymbol : public Element {

   public:
      BSymbol(Score* s) : Element(s) {}
      BSymbol &operator=(const BSymbol&);

      virtual bool isMovable() const { return true; }

      void setAnchor(Anchor a) { setSubtype(int(a)); }
      Anchor anchor() const    { return (Anchor)subtype(); }
      };

#endif

