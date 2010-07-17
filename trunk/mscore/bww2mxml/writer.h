//=============================================================================
//  BWW to MusicXML converter
//  Part of MusE Score
//  Linux Music Score Editor
//  $Id$
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

#ifndef WRITER_H
#define WRITER_H

/**
 \file
 Definition of class Writer
 */

#include <QtCore/QString>

#include "symbols.h"

class QIODevice;

namespace Bww {

  /**
   The flags that need to be handled at the beginning of a measure.
   */

  struct MeasureBeginFlags {
    bool repeatBegin;
    bool endingFirst;
    bool endingSecond;
    MeasureBeginFlags() :
        repeatBegin(false),
        endingFirst(false),
        endingSecond(false)
    {}
  };

  /**
   The flags that need to be handled at the end of a measure.
   */

  struct MeasureEndFlags {
    bool repeatEnd;
    bool endingEnd;
    MeasureEndFlags() :
        repeatEnd(false),
        endingEnd(false)
    {}
  };

  /**
   The writer that generates the output.
   */

  class Writer
  {
  public:
    virtual void header(const QString title, const QString type,
                        const QString composer, const QString footer) = 0;
    virtual void tempo(const int beats, const int beat) = 0;
    virtual void trailer() = 0;
    virtual void beginMeasure() = 0;
    virtual void endMeasure() = 0;
    virtual void note(const QString pitch, const QString beam,
                      const QString type, const int dots,
                      bool tieStart = false, bool tieStop = false,
                      StartStop triplet = ST_NONE,
                      bool grace = false) = 0;
  };

} // namespace Bww

#endif // WRITER_H
