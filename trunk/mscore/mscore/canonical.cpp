//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: rest.cpp 2080 2009-09-10 11:14:14Z wschweer $
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
 Build a canonical internal representation of a score. This representation
 is independent of measures and barlines. It is used for reformatting the score after changing the
 time signature and when inserting or removing single chords/rests.

      - no measures
      - each track(voice) is filled with rests
      - tick position is invalid
      - tied notes over bar lines are concatenated
*/

#include "score.h"
#include "segment.h"
#include "rest.h"
#include "measure.h"
#include "durationtype.h"

//---------------------------------------------------------
//   buildCanonical
//---------------------------------------------------------

QBuffer Score::buildCanonical(int track)
      {
      QBuffer buffer;
      buffer.open(QBuffer::ReadWrite);
      Xml xml(buffer);

      xml.stag(QString("Track no=\"%1\"").arg(track);
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            bool firstCR = true;
            for (Segment* s = m->first(); s; s = s->next()) {
                  if (s->subtype() == Segment::SegChordRest) {
                        ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                        if (e) {
                              if (e->type() == CHORD) {
                                    xml.stag("Chord");
                                    xml.tag("len", m->fraction());
                                    }
                              else if (e->type() == REST) {
                                    xml.stag("Rest");
                                    xml.tag("len", m->fraction());
                                    }
                              xml.etag();
                              }
                        else if (firstCR) {
                              //
                              // fill empty voice with rest
                              //
                              xml.tag("IRest");
                              xml.tag("len", m->fraction());
                              xml.etag();
                              }
                        }
                  firstCR = false;
                  }
            }
      xml.etag();
      buffer.close();
      return buffer;
      }

