//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2008-2009 Werner Schweer and others
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

#ifndef __DURATION_H__
#define __DURATION_H__

#include "element.h"
#include "durationtype.h"

class Tuplet;
class Beam;

//---------------------------------------------------------
//   DurationElement
//---------------------------------------------------------

class DurationElement : public Element {
      Duration _duration;
      Tuplet* _tuplet;

   protected:
      mutable int _tickLen;   // is temporary changed in const write() method

   public:
      DurationElement(Score* s);
      DurationElement(const DurationElement& e);

      virtual Measure* measure() const = 0;
      Duration duration() const                    { return _duration; }
      virtual void setDuration(const Duration t)   { _duration = t;    }

      void setTuplet(Tuplet* t)                    { _tuplet = t;      }
      Tuplet* tuplet() const                       { return _tuplet;   }
      virtual Beam* beam() const                   { return 0;         }
      virtual int tickLen() const                  { return _tickLen;  }
      virtual void setTickLen(int t) const         { _tickLen = t;     }
      int ticks() const;
      };

#endif



