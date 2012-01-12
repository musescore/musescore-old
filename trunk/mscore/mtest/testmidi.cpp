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
#include "libmscore/durationtype.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "mcursor.h"

extern bool saveMidi(Score*, const QString&);
extern bool importMidi(Score*, const QString&);
extern MScore* mscore;

//---------------------------------------------------------
//   compareScores
//---------------------------------------------------------

bool compareScores(Score* score1, Score* score2)
      {
      int staves = score1->nstaves();
      if (score2->nstaves() != staves) {
            printf("   stave count different\n");
            return false;
            }
      Segment* s1 = score1->firstMeasure()->first();
      Segment* s2 = score2->firstMeasure()->first();

      int tracks = staves * VOICES;
      for (;;) {
            for (int track = 0; track < tracks; ++track) {
                  Element* e1 = s1->element(track);
                  Element* e2 = s2->element(track);
                  if ((e1 && !e2) || (e2 && !e1)) {
                        printf("   elements different\n");
                        return false;
                        }
                  if (e1 == 0)
                        continue;
                  if (e1->type() != e2->type()) {
                        printf("   %s != %s\n", e1->name(), e2->name());
                        return false;
                        }
                  printf("      ok: %s\n", e1->name());
                  }
            s1 = s1->next1();
            s2 = s2->next1();
            if ((s1 && !s2) || (s2 && !s2)) {
                  printf("   segment count different\n");
                  return false;
                  }
            if (s1 == 0)
                  break;
            }
      return true;
      }

//---------------------------------------------------------
//   testMidi
//---------------------------------------------------------

bool testMidi()
      {
      printf("====test midi\n");

      MCursor c;
      c.createScore("test1");
      c.addPart("Voice");
      c.move(0, 0);     // move to track 0 tick 0

      c.addKeySig(0);
      c.addTimeSig(Fraction(4,4));
      c.addChord(60, TDuration(TDuration::V_QUARTER));
      c.addChord(61, TDuration(TDuration::V_QUARTER));
      c.addChord(62, TDuration(TDuration::V_QUARTER));
      c.addChord(63, TDuration(TDuration::V_QUARTER));
      Score* score = c.score();

      score->doLayout();
      score->rebuildMidiMapping();
      c.saveScore();
      saveMidi(score, "test.mid");

      Score* score2 = new Score(mscore->baseStyle());
      if (!importMidi(score2, "test.mid")) {
            printf("import midi failed\n");
            abort();
            }
      score2->doLayout();

      bool rv = true;
      // compare
      if (!compareScores(score, score2)) {
            printf("   failed: readback midi file is different\n");
            rv = false;
            }
      delete score;
      delete score2;
      return rv;
      }

