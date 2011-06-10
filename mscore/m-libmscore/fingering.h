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

#ifndef __FINGERING_H__
#define __FINGERING_H__

#include "text.h"

//---------------------------------------------------------
//   Fingering
//---------------------------------------------------------

class Fingering : public Text {

   public:
      Fingering(Score* s);
      virtual Fingering* clone() const { return new Fingering(*this); }
      virtual ElementType type() const { return FINGERING; }

      virtual void layout();
      virtual void read(XmlReader*);
      };

#endif

