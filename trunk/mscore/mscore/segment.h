//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: segment.h,v 1.18 2006/03/30 07:32:34 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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
#include "spatium.h"

class Measure;
class Segment;
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
*/

class Segment : public Element {
   public:
      enum SegmentType {
            SegClef, SegKeySig, SegTimeSig,
            SegBarLine,       // usually a start repeat in first measure of system
            SegChordRest,
            SegBreath,
            SegEndBarLine, SegTimeSigAnnounce
            };
      static const char* segmentTypeNames[];

   private:
      QList<Element*> _elist;      ///< Element storage, size = staves * VOICES.
      QList<LyricsList> _lyrics;   ///< Lyrics storage, size  = staves.

      void init();

   public:
      Segment(Measure*);
      Segment(Measure*, int t);
      virtual Segment* clone() const    { return new Segment(*this); }
      virtual ElementType type() const  { return SEGMENT; }

      Segment* next() const             { return (Segment*)Element::next(); }
      Segment* prev() const             { return (Segment*)Element::prev(); }
      Segment* next1() const;
      Segment* prev1() const;

      Element* element(int track) const { return _elist[track];    }
      void removeElement(int track);
      void setElement(int track, Element* el);
      LyricsList* lyricsList(int staffIdx)             { return &_lyrics[staffIdx];  }
      const LyricsList* lyricsList(int staffIdx) const { return &_lyrics[staffIdx];  }
      void setLyrics(int staff, Lyrics* l);

      Measure* measure() const            { return (Measure*)parent(); }
      double x() const                    { return _pos.x();           }
      void setX(double v)                 { _pos.setX(v);              }

      void insertStaff(int staff);
      void removeStaff(int staff);

      virtual void add(Element*);
      virtual void remove(Element*);

      void sortStaves(QList<int>& src, QList<int>& dst);
      const char* name() const { return segmentTypeNames[subtype()]; }
      static SegmentType segmentType(int type);
      void setTime(int tick);
      void removeGeneratedElements();
      bool isEmpty() const;
      };

#endif

