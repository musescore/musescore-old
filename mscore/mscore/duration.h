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
      int _ticks;       // for reading obsolete scores version < 1.12
                        // must be converted to _duration

   public:
      DurationElement(Score* s);
      DurationElement(const DurationElement& e);

      virtual Measure* measure() const               { return (Measure*)(parent()); }

      const Duration& duration() const               { return _duration;        }
      void setDurationType(Duration::DurationType t) { _duration.setType(t);    }
      void setDurationType(const QString& s)         { _duration.setType(s);    }
      void setDurationVal(int ticks)                 { _duration.setVal(ticks); }
      void setDuration(const Duration& v)            { _duration = v;           }

      void setTuplet(Tuplet* t)                      { _tuplet = t;      }
      Tuplet* tuplet() const                         { return _tuplet;   }
      virtual Beam* beam() const                     { return 0;         }
      int ticks() const;
      int tickLen() const                            { return ticks(); }
      void setDots(int n)                            { _duration.setDots(n); }
      int dots() const                               { return _duration.dots(); }
      void convertTicks();   // for scores version < 1.12
      };

#endif

