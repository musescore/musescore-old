//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2004-2009 Werner Schweer and others
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

#ifndef __ACCIDENTAL_H__
#define __ACCIDENTAL_H__

/**
 \file
 Definition of class Accidental
*/

#include "element.h"

// Accidental Subtype Values

enum {
      ACC_NONE, ACC_SHARP, ACC_FLAT, ACC_SHARP2, ACC_FLAT2, ACC_NATURAL
      };

//---------------------------------------------------------
//   Accidental
//---------------------------------------------------------

class Accidental : public Compound {
   public:
      Accidental(Score*);
      virtual Accidental* clone() const { return new Accidental(*this); }
      virtual ElementType type() const  { return ACCIDENTAL; }
      virtual void setSubtype(int v);
      virtual void setMag(double val);

      static int subtype2value(int);      // return effective pitch offset
      static int value2subtype(int);
      };
#endif

