//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

/**
 \file
 Definition of class Segment.
*/

#ifndef __SEGMENT_H__
#define __SEGMENT_H__

#include "element.h"

class Measure;
class Segment;
class ChordRest;
class Lyrics;
class Painter;
class Spanner;
class System;

//---------------------------------------------------------
//   Segment
//---------------------------------------------------------

/**
 The Segment class stores all elements inside a staff.

 A Segment is typed, i.e. all Elements in a Segment are of the same type.
 All Elements also start at the same tick. The Segment can store one Element for
 each voice in each staff in the score. It also stores the lyrics for each staff.
 Some elements (Clef, KeySig, TimeSig etc.) are assumed to always have voice zero
 and can be found in _elist[staffIdx * VOICES];

 Segments are children of Measures and store Clefs, KeySigs, TimeSigs,
 BarLines and ChordRests.

 special case SegGrace:
      - tick()                 is play position of acciaccatura and appoggiatura notes
      - tick() - tickOffset()  is logical position, relevant for layout
                               (logicl position is the tick position of the next main note)
*/

class Segment : public Element {
      Segment* _next;               // linked list of segments inside a measure
      Segment* _prev;

      mutable bool empty;           // cached value
      int _tick;

      QList<Spanner*> _spannerFor;
      QList<Spanner*> _spannerBack;
      QList<Element*> _annotations;

      QList<Element*> _elist;      ///< Element storage, size = staves * VOICES.

      void init();
      void checkEmpty() const;
      void addSpanner(Spanner*);
      void removeSpanner(Spanner*);

   public:
      Segment(Measure*);
      Segment(Measure*, SegmentType, int tick);
      ~Segment();

      virtual Segment* clone() const    { return new Segment(*this); }
      virtual ElementType type() const  { return SEGMENT; }

      Segment* next() const             { return _next;   }
      Segment* next(SegmentTypes) const;

      void setNext(Segment* e)          { _next = e;      }
      Segment* prev() const             { return _prev;   }
      Segment* prev(SegmentTypes) const;
      void setPrev(Segment* e)          { _prev = e;      }

      Segment* next1() const;
      Segment* next1(SegmentTypes) const;
      Segment* prev1() const;
      Segment* prev1(SegmentTypes) const;

      Segment* nextCR(int track = -1) const;

      ChordRest* nextChordRest(int track, bool backwards = false) const;

      Element* element(int track) const    { return _elist.value(track);  }
      const QList<Element*>& elist() const { return _elist; }

      void removeElement(int track);
      void setElement(int track, Element* el);
      const QList<Lyrics*>* lyricsList(int staffIdx) const;

      Measure* measure() const            { return (Measure*)parent(); }
      System* system() const              { return (System*)parent()->parent(); }
      double x() const                    { return ipos().x();         }
      void setX(double v)                 { rxpos() = v;               }

      void insertStaff(int staff);
      void removeStaff(int staff);

      virtual void add(Element*);
      virtual void remove(Element*);
      void swapElements(int i1, int i2);

      void sortStaves(QList<int>& dst);
      const char* subTypeName() const;
      SegmentType segmentType() const     { return SegmentType(subtype()); }
      static SegmentType segmentType(ElementType type);
      void removeGeneratedElements();
      bool isEmpty() const               { return empty; }
      void fixStaffIdx();
      bool isChordRest() const           { return subtype() == SegChordRest; }
      bool isGrace() const               { return subtype() == SegGrace; }
      void setTick(int);
      int tick() const;
      int rtick() const { return _tick; } // tickposition relative to measure start

      QList<Spanner*> spannerFor() const { return _spannerFor;  }
      QList<Spanner*> spannerBack() const { return _spannerBack;       }
      void addSpannerBack(Spanner* e)     { _spannerBack.append(e);    }
      void removeSpannerBack(Spanner* e)  { _spannerBack.removeOne(e); }
      void addSpannerFor(Spanner* e)      { _spannerFor.append(e);    }
      void removeSpannerFor(Spanner* e)   { _spannerFor.removeOne(e); }

      const QList<Element*>& annotations() const  { return _annotations;  }
      void removeAnnotation(Element* e)           { _annotations.removeOne(e); }
      };

#endif

