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

#ifndef __HOOK_H__
#define __HOOK_H__

#include "symbol.h"

//---------------------------------------------------------
//   Hook
//---------------------------------------------------------

class Hook : public Symbol {
      int val;
      bool _small;

   public:
      Hook(Score*);
      virtual Element* clone() const { return new Hook(*this); }
      virtual ElementType type() const { return HOOK; }
      int idx() const             { return val; }
      void setIdx(int v, bool);
      };

#endif

