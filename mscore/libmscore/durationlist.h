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

//---------------------------------------------------------
//   DurationList
//---------------------------------------------------------

class DurationList : public QList<DurationElement*>
      {
      Fraction _duration;

      Tuplet* writeTuplet(Tuplet* tuplet, Measure* measure, int tick);
      void append(DurationElement*);
      void appendGap(const Fraction&);

   public:
      DurationList() {}
      DurationList(int track, const Measure* fm, const Measure* lm) { read(track, fm, lm); }
      void read(int track, const Measure* fm, const Measure* lm);
      bool canWrite(const Fraction& f);
      bool write(int track, Measure*);
      Fraction duration() const  { return _duration; }
      };

#endif
