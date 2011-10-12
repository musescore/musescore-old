//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: importmidi.cpp 2721 2010-02-15 19:41:28Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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
#include "libmscore/score.h"
#include "omr/omr.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/rest.h"
#include "omr/omrpage.h"
#include "libmscore/segment.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/page.h"
#include "libmscore/clef.h"
#include "libmscore/bracket.h"
#include "libmscore/mscore.h"
#include "musescore.h"

//---------------------------------------------------------
//   importPdf
//---------------------------------------------------------

bool MuseScore::importPdf(Score* score, const QString& path)
      {
      Omr* omr = new Omr(path, score);
      if (!omr->readPdf()) {
            delete omr;
            return false;
            }
      score->setOmr(omr);
      score->setSpatium(omr->spatiumMM() * DPMM);
      score->style()->set(StyleVal(ST_pageFillLimit, 1.0));
      score->style()->set(StyleVal(ST_lastSystemFillLimit, 0.0));
      score->style()->set(StyleVal(ST_staffLowerBorder, 0.0));
      score->style()->set(StyleVal(ST_measureSpacing, 1.0));

      PageFormat pF(*score->pageFormat());
      pF.setEvenLeftMargin(5.0 * DPMM / DPI);
      pF.setEvenTopMargin(0);
      pF.setEvenBottomMargin(0);
      pF.setOddLeftMargin(5.0 * DPMM / DPI);
      pF.setOddTopMargin(0);
      pF.setOddBottomMargin(0);
      score->setPageFormat(pF);

      score->style()->set(StyleVal(ST_systemDistance,   Spatium(omr->systemDistance())));
      score->style()->set(StyleVal(ST_akkoladeDistance, Spatium(omr->staffDistance())));

      Part* part   = new Part(score);
      Staff* staff = new Staff(score, part, 0);
      part->staves()->push_back(staff);
      score->staves().insert(0, staff);
      staff = new Staff(score, part, 1);
      part->staves()->push_back(staff);
      score->staves().insert(1, staff);
      part->staves()->front()->setBarLineSpan(part->nstaves());
      score->insertPart(part, 0);

      Duration d(Duration::V_MEASURE);
      Measure* measure = 0;
      int tick = 0;
      foreach(const OmrPage* omrPage, omr->pages()) {
            int nsystems = omrPage->systems().size();
            for (int k = 0; k < nsystems; ++k) {
                  const OmrSystem& omrSystem = omrPage->systems().at(k);
                  int numMeasures = omrSystem.barLines.size() - 1;
                  if (numMeasures < 1)
                        numMeasures = 1;
                  else if (numMeasures > 50)
                        numMeasures = 50;
                  for (int i = 0; i < numMeasures; ++i) {
                        measure = new Measure(score);
                        measure->setTick(tick);

		            Rest* rest = new Rest(score, d);
                        rest->setDuration(Fraction(4,4));
                        rest->setTrack(0);
                        Segment* s = measure->getSegment(SegChordRest, tick);
		            s->add(rest);
		            rest = new Rest(score, d);
                        rest->setDuration(Fraction(4,4));
                        rest->setTrack(4);
		            s->add(rest);

                        score->measures()->add(measure);
                        tick += MScore::division * 4;
                        }
                  if (k < (nsystems-1)) {
                        LayoutBreak* b = new LayoutBreak(score);
                        b->setSubtype(LAYOUT_BREAK_LINE);
                        measure->add(b);
                        }
                  }
            if (measure) {
                  LayoutBreak* b = new LayoutBreak(score);
                  b->setSubtype(LAYOUT_BREAK_PAGE);
                  measure->add(b);
                  }
            }

      //---create bracket

      score->staff(0)->setBracket(0, BRACKET_AKKOLADE);
      score->staff(0)->setBracketSpan(0, 2);

      //---create clefs

      measure = score->firstMeasure();
      Clef* clef = new Clef(score);
      clef->setClefType(CLEF_G);
      clef->setTrack(0);
      Segment* segment = measure->getSegment(SegClef, 0);
      segment->add(clef);

      clef = new Clef(score);
      clef->setClefType(CLEF_F);
      clef->setTrack(4);
      segment->add(clef);

      score->setShowOmr(true);
      omr->page(0)->readHeader(score);
      score->rebuildMidiMapping();
      return true;
      }

