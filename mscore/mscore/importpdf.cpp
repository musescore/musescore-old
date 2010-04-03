//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: importmidi.cpp 2721 2010-02-15 19:41:28Z wschweer $
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

#include "importpdf.h"
#include "score.h"
#include "omr/omr.h"
#include "part.h"
#include "staff.h"
#include "measure.h"
#include "al/al.h"
#include "rest.h"
#include "omr/omrpage.h"

//---------------------------------------------------------
//   importPdf
//---------------------------------------------------------

bool Score::importPdf(const QString& path)
      {
      _omr = new Omr(path, this);
      if (!_omr->read()) {
            delete _omr;
            _omr = 0;
            return false;
            }
      _spatium = _omr->spatiumMM() * DPMM;
      setStyle(ST_systemDistance,
         StyleVal(ST_systemDistance, Spatium(_omr->systemDistance())));
      setStyle(ST_akkoladeDistance,
         StyleVal(ST_akkoladeDistance, Spatium(_omr->staffDistance())));

      Part* part = new Part(this);
      Staff* staff = new Staff(this, part, 0);
      part->staves()->push_back(staff);
      staves().insert(0, staff);
      staff = new Staff(this, part, 1);
      part->staves()->push_back(staff);
      staves().insert(1, staff);
      part->staves()->front()->setBarLineSpan(part->nstaves());
      insertPart(part, 0);

      int numMeasures = 4;
      Duration d(Duration::V_MEASURE);
      for (int i = 0; i < numMeasures; ++i) {
            int tick = i * AL::division * 4;
            Measure* measure = new Measure(this);
		Rest* rest = new Rest(this, tick, d);
            rest->setTrack(0);
            Segment* s = measure->getSegment(rest);
		s->add(rest);
		rest = new Rest(this, tick, d);
            rest->setTrack(4);
		s->add(rest);

            measures()->add(measure);
            }
      setShowOmr(true);

      _omr->page(0)->readHeader(this);

      return true;
      }

