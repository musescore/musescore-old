//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: score.cpp 5149 2011-12-29 08:38:43Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "libmscore/mscore.h"
#include "libmscore/score.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/rest.h"
#include "libmscore/segment.h"
#include "libmscore/instrtemplate.h"

extern bool saveMidi(Score*, const QString&);
extern bool importMidi(Score*, const QString&);
extern MScore* mscore;

//---------------------------------------------------------
//   createTestScore
//---------------------------------------------------------

Score* createTestScore()
      {
      Score* score = new Score(mscore->baseStyle());
      Part* part   = new Part(score);
      Staff* staff = new Staff(score, part, 0);
      static const char* instr = "Voice";
      InstrumentTemplate* it = searchTemplate(instr);
      if (it == 0) {
            printf("did not found instrument <%s>\n", instr);
            abort();
            }
      part->initFromInstrTemplate(it);

      score->appendPart(part);
      score->insertStaff(staff, 0);
      Measure* measure = new Measure(score);
      measure->setTick(0);
      measure->setTimesig(Fraction(4,4));
      measure->setLen(Fraction(4,4));

      Rest* rest = new Rest(score, TDuration::V_MEASURE);
      rest->setTrack(0);
      rest->setDuration(measure->len());
      Segment* segment = measure->getSegment(rest, 0);
      segment->add(rest);
      score->measures()->add(measure);
      return score;
      }

//---------------------------------------------------------
//   saveTestScore
//---------------------------------------------------------

void saveTestScore(Score* score)
      {
      QFile fp("test.mscx");
      if (!fp.open(QIODevice::WriteOnly)) {
            printf("open test file failed\n");
            abort();
            }
      score->saveFile(&fp, false);
      fp.close();
      }

//---------------------------------------------------------
//   testMidi
//---------------------------------------------------------

bool testMidi()
      {
      printf("test midi\n");

      Score* score = createTestScore();
      score->doLayout();
      score->rebuildMidiMapping();
      saveMidi(score, "test.mid");
      Score* score2 = new Score(mscore->baseStyle());
      if (!importMidi(score2, "test.mid")) {
            printf("import midi failed\n");
            abort();
            }
      delete score;
      delete score2;

      return true;
      }


