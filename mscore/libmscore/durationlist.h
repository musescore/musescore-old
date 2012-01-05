//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __DURATIONLIST_H__
#define __DURATIONLIST_H__

#include "fraction.h"

class DurationElement;
class Measure;
class Tuplet;
class Segment;
class Spanner;

//---------------------------------------------------------
//   DurationList
//---------------------------------------------------------

class DurationList : public QList<DurationElement*>
      {
      Fraction _duration;

      Tuplet* writeTuplet(Tuplet* tuplet, Measure* measure, int tick) const;
      void append(DurationElement*, QHash<Spanner*, Spanner*>*);
      void appendGap(const Fraction&);

   public:
      DurationList() {}
      void read(int track, const Segment* fs, const Segment* ls, QHash<Spanner*, Spanner*>*);
      bool canWrite(const Fraction& f) const;
      bool write(int track, Measure*, QHash<Spanner*, Spanner*>*) const;
      Fraction duration() const  { return _duration; }
      };

//---------------------------------------------------------
//   ScoreRange
//---------------------------------------------------------

class ScoreRange {
      mutable QHash<Spanner*, Spanner*> spannerMap;
      QList<DurationList> tracks;

   public:
      void read(Segment* first, Segment* last, int startTrack, int endTrack);
      bool canWrite(const Fraction&) const;
      bool write(int track, Measure*) const;
      Fraction duration() const;
      };

#endif

