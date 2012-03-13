//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: rest.cpp 2080 2009-09-10 11:14:14Z wschweer $
//
//  Copyright (C) 2009 Werner Schweer and others
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
#include "tuplet.h"
#include "canonical.h"

//---------------------------------------------------------
//   write ChordRest
//---------------------------------------------------------

Segment* CScore::write(Xml& xml, DurationElement* cr)
      {
      if (cr->tuplet()) {
            xml.stag("Tuplet");
            Tuplet* t = cr->tuplet();
            foreach(DurationElement* e, t->elements())
                  write(xml, e);
            xml.etag();
            return static_cast<ChordRest*>(t->elements().back())->segment();
            }
      Fraction f(cr->fraction());
      if (cr->type() == CHORD) {
            xml.stag("Chord");
            xml.tag("len", QString("%1/%2").arg(f.numerator()).arg(f.denominator()));
            }
      else if (cr->type() == REST) {
            xml.stag("Rest");
            xml.tag("len", QString("%1/%2").arg(f.numerator()).arg(f.denominator()));
            }
      xml.etag();
      return static_cast<ChordRest*>(cr)->segment();
      }

//---------------------------------------------------------
//   CScore
//---------------------------------------------------------

CScore::CScore(Score* s, int track, Segment* segment)
      {
      score = s;
      buffer.open(QBuffer::ReadWrite);
      Xml xml(&buffer);

      xml.stag(QString("Track no=\"%1\"").arg(track));
      for (; segment; segment = segment->next()) {
            if (segment->subtype() == SegChordRest) {
                  ChordRest* cr = static_cast<ChordRest*>(segment->element(track));
                  if (cr)
                        segment = write(xml, cr);
                  else if (segment->tick() == segment->measure()->tick()) {
                        //
                        // fill empty voice with rest
                        //
                        xml.stag("IRest");
                        Fraction f(cr->measure()->fraction());
                        xml.tag("len", QString("%1/%2").arg(f.numerator()).arg(f.denominator()));
                        xml.etag();
                        }
                  }
            }
      xml.etag();
      buffer.close();
      }

