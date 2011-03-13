//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: trill.h 3229 2010-06-27 14:55:28Z wschweer $
//
//  Copyright (C) 2010 Werner Schweer and others
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

class Note;

//---------------------------------------------------------
//   Fingering
//---------------------------------------------------------

class Fingering : public Text {

   public:
      Fingering(Score* s);
      virtual Fingering* clone() const { return new Fingering(*this); }
      virtual ElementType type() const { return FINGERING; }

      Note* note() const { return (Note*)parent(); }

      virtual void layout();
      virtual void write(Xml&) const;
      virtual void read(QDomElement);
      virtual void toDefault();
      };

#endif

