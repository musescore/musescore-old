//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

typedef QList<Lyrics*> LyricsList;
typedef LyricsList::iterator iLyrics;
typedef LyricsList::const_iterator ciLyrics;

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

enum SegmentType {
      SegClef                 = 0x1,
      SegKeySig               = 0x2,
      SegTimeSig              = 0x4,
      SegStartRepeatBarLine   = 0x8,
      SegBarLine              = 0x10,
      SegGrace                = 0x20,
      SegChordRest            = 0x40,
      SegBreath               = 0x80,
      SegEndBarLine           = 0x100,
      SegTimeSigAnnounce      = 0x200,
      SegKeySigAnnounce       = 0x400,
      SegAll                  = 0xfff
      };
typedef QFlags<SegmentType> SegmentTypes;
Q_DECLARE_OPERATORS_FOR_FLAGS(SegmentTypes)

class Segment : public Element {
      Segment* _next;
      Segment* _prev;
      mutable bool empty;     // cached value

   private:
      QList<Element*> _elist;      ///< Element storage, size = staves * VOICES.
      QList<LyricsList> _lyrics;   ///< Lyrics storage, size  = staves.

      void init();
      void checkEmpty() const;

   public:
      Segment(Measure*);
      Segment(Measure*, int t);
      ~Segment();

      virtual Segment* clone() const    { return new Segment(*this); }
      virtual ElementType type() const  { return SEGMENT; }

      Segment* next() const             { return _next;   }
      Segment* next(SegmentTypes) const;

      void setNext(Segment* e)          { _next = e;      }
      Segment* prev() const             { return _prev;   }
      void setPrev(Segment* e)          { _prev = e;      }

      Segment* next1() const;
      Segment* next1(SegmentTypes) const;
      Segment* prev1() const;

      Segment* nextCR(int track = -1) const;

      ChordRest* nextChordRest(int track, bool backwards = false) const;

      Element* element(int track) const { return _elist.value(track);   }
      void removeElement(int track);
      void setElement(int track, Element* el);
      LyricsList* lyricsList(int staffIdx)             { return &_lyrics[staffIdx];  }
      const LyricsList* lyricsList(int staffIdx) const { return &_lyrics[staffIdx];  }

      Measure* measure() const            { return (Measure*)parent(); }
      double x() const                    { return _pos.x();           }
      void setX(double v)                 { _pos.setX(v);              }

      void insertStaff(int staff);
      void removeStaff(int staff);

      virtual void add(Element*);
      virtual void remove(Element*);

      void sortStaves(QList<int>& dst);
      const char* subTypeName() const;
      static SegmentType segmentType(int type);
      void setTime(int tick);
      void removeGeneratedElements();
      bool isEmpty() const               { return empty; }
      void fixStaffIdx();
      bool isChordRest() const           { return subtype() == SegChordRest; }
      bool isGrace() const               { return subtype() == SegGrace; }
      };

#endif

