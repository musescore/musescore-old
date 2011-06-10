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

#ifndef __SPANNER_H__
#define __SPANNER_H__

#include "element.h"

class Segment;
class Spanner;
class System;

enum SpannerSegmentType {
      SEGMENT_SINGLE, SEGMENT_BEGIN, SEGMENT_MIDDLE, SEGMENT_END
      };

//---------------------------------------------------------
//   SpannerSegment
//---------------------------------------------------------

class SpannerSegment : public Element {

   protected:
      System* _system;

   public:
      SpannerSegment(Score* s);
      SpannerSegment(const SpannerSegment&);
      virtual SpannerSegment* clone() const = 0;
      Spanner* spanner() const                         { return (Spanner*)parent();  }
      void setSpannerSegmentType(SpannerSegmentType s) { setSubtype(s);              }
      SpannerSegmentType spannerSegmentType() const    { return SpannerSegmentType(subtype()); }
      void setSystem(System* s)                        { _system = s;                }
      };

//---------------------------------------------------------
//   Spanner
//---------------------------------------------------------

class Spanner : public Element {
      Element* _startElement;
      Element* _endElement;
      Anchor _anchor;         // enum Anchor { ANCHOR_SEGMENT, ANCHOR_MEASURE};

      QList<SpannerSegment*> segments;

      int _id;                // used for xml serialization

   protected:
      Element* oStartElement; // start/end element at startEdit()
      Element* oEndElement;
      qreal _yoffset;        // in spatium units

   public:
      Spanner(Score*);
      Spanner(const Spanner&);
      ~Spanner();

      virtual ElementType type() const = 0;

      void setStartElement(Element* e) { _startElement = e;    }
      void setEndElement(Element* e)   { _endElement = e;      }
      Element* startElement() const    { return _startElement; }
      Element* endElement() const      { return _endElement;   }

      int id() const           { return _id; }
      void setId(int v)        { _id = v;    }

      Anchor anchor() const    { return _anchor;   }
      void setAnchor(Anchor a) { _anchor = a;      }

      const QList<SpannerSegment*>& spannerSegments() const { return segments; }
      QList<SpannerSegment*>& spannerSegments()             { return segments; }

      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void scanElements(void* data, void (*func)(void*, Element*));
      virtual void setSelected(bool f);
      void setYoff(qreal d) { _yoffset = d;        }
      qreal yoff() const    { return _yoffset;     }
      };
#endif

