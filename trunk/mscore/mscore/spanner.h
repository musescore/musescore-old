//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#ifndef __SPANNER_H__
#define __SPANNER_H__

#include "element.h"

class Segment;

//---------------------------------------------------------
//   Spanner
//---------------------------------------------------------

class Spanner : public Element {
      Element* _startElement;
      Element* _endElement;
      Anchor _anchor;         // enum Anchor { ANCHOR_SEGMENT, ANCHOR_MEASURE};

      int _tick1, _tick2;       // used for backward compatibility

      int _id;    // used for xml serialization

   public:
      Spanner(Score*);
      Spanner(const Spanner&);

      virtual ElementType type() const = 0;

      void setStartElement(Element* e)    { _startElement = e;    }
      void setEndElement(Element* e)      { _endElement = e;      }
      Element* startElement() const       { return _startElement; }
      Element* endElement() const         { return _endElement;   }

      //
      // used for backward compatibility:
      //
      void __setTick1(int v)   { _tick1 = v;    }
      void __setTick2(int v)   { _tick2 = v;    }
      int __tick1() const      { return _tick1; }
      int __tick2() const      { return _tick2; }

      int id() const           { return _id; }
      void setId(int v)        { _id = v;    }

      Anchor anchor() const    { return _anchor;   }
      void setAnchor(Anchor a) { _anchor = a;      }
      };
#endif


