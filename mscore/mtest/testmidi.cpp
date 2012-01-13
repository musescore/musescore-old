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
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/keysig.h"
#include "mcursor.h"

extern bool saveMidi(Score*, const QString&);
extern bool importMidi(Score*, const QString&);
extern MScore* mscore;

//---------------------------------------------------------
//   compareElements
//---------------------------------------------------------

bool compareElements(Element* e1, Element* e2)
      {
      if (e1->type() != e2->type())
            return false;
      if (e1->type() == TIMESIG) {
            }
      else if (e1->type() == KEYSIG) {
            KeySig* ks1 = static_cast<KeySig*>(e1);
            KeySig* ks2 = static_cast<KeySig*>(e2);
            if (ks1->keySignature() != ks2->keySignature()) {
                  printf("      key signature %d  !=  %d\n",
                     ks1->keySignature(), ks2->keySignature());
                  return false;
                  }
            }
      else if (e1->type() == CLEF) {
            }
      else if (e1->type() == REST) {
            }
      else if (e1->type() == CHORD) {
            Chord* c1 = static_cast<Chord*>(e1);
            Chord* c2 = static_cast<Chord*>(e2);
            if (c1->duration() != c2->duration()) {
                  Fraction f1 = c1->duration();
                  Fraction f2 = c2->duration();
                  printf("      chord duration %d/%d  !=  %d/%d\n",
                     f1.numerator(), f1.denominator(),
                     f2.numerator(), f2.denominator()
                     );
                  return false;
                  }
            if (c1->notes().size() != c2->notes().size()) {
                  printf("      != note count\n");
                  return false;
                  }
            int n = c1->notes().size();
            for (int i = 0; i < n; ++i) {
                  Note* n1 = c1->notes()[i];
                  Note* n2 = c2->notes()[i];
                  if (n1->pitch() != n2->pitch()) {
                        printf("      != pitch note %d\n", i);
                        return false;
                        }
                  if (n1->tpc() != n2->tpc()) {
                        printf("      note tcp %d != %d\n", n1->tpc(), n2->tpc());
                        // return false;
                        }
                  }
            }

      return true;
      }

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
                  if (!compareElements(e1, e2)) {
                        printf("   %s != %s\n", e1->name(), e2->name());
                        return false;
                        }
                  printf("   ok: %s\n", e1->name());
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
//   testMidi1
//---------------------------------------------------------

static bool testMidi1()
      {
      printf("  -read/write\n");

      MCursor c;
      c.createScore("test1a");
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
      saveMidi(score, "test1.mid");

      Score* score2 = new Score(mscore->baseStyle());
      score2->setName("test1b");
      if (!importMidi(score2, "test1.mid")) {
            printf("import midi failed\n");
            abort();
            }
      score2->doLayout();
      score2->rebuildMidiMapping();
      MCursor c2(score2);
      c2.saveScore();

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

//---------------------------------------------------------
//   testMid2
//    write/read midi file with timesig 3/4
//---------------------------------------------------------

static bool testMidi2()
      {
      printf("  -timesig\n");

      MCursor c;
      c.createScore("test2a");
      c.addPart("Voice");
      c.move(0, 0);     // move to track 0 tick 0

      c.addKeySig(0);
      c.addTimeSig(Fraction(3,4));
      c.addChord(60, TDuration(TDuration::V_QUARTER));
      c.addChord(61, TDuration(TDuration::V_QUARTER));
      c.addChord(62, TDuration(TDuration::V_QUARTER));
      Score* score = c.score();

      score->doLayout();
      score->rebuildMidiMapping();
      c.saveScore();
      saveMidi(score, "test2.mid");

      Score* score2 = new Score(mscore->baseStyle());
      score2->setName("test2b");
      if (!importMidi(score2, "test2.mid")) {
            printf("import midi failed\n");
            abort();
            }
      score2->doLayout();
      score2->rebuildMidiMapping();
      MCursor c2(score2);
      c2.saveScore();

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

//---------------------------------------------------------
//   testMidi3
//---------------------------------------------------------

static bool testMidi3()
      {
      printf("  -keysig\n");

      MCursor c;
      c.createScore("test3a");
      c.addPart("Voice");
      c.move(0, 0);     // move to track 0 tick 0

      c.addKeySig(1);
      c.addTimeSig(Fraction(4,4));
      c.addChord(60, TDuration(TDuration::V_QUARTER));
      c.addChord(61, TDuration(TDuration::V_QUARTER));
      c.addChord(62, TDuration(TDuration::V_QUARTER));
      c.addChord(63, TDuration(TDuration::V_QUARTER));
      Score* score = c.score();

      score->doLayout();
      score->rebuildMidiMapping();
      c.saveScore();
      saveMidi(score, "test3.mid");

      Score* score2 = new Score(mscore->baseStyle());
      score2->setName("test3b");
      if (!importMidi(score2, "test3.mid")) {
            printf("import midi failed\n");
            abort();
            }
      score2->doLayout();
      score2->rebuildMidiMapping();
      MCursor c2(score2);
      c2.saveScore();

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

//---------------------------------------------------------
//   testMidi
//---------------------------------------------------------

bool testMidi()
      {
      printf("====test midi\n");

      if (!testMidi1())
            return false;
      if (!testMidi2())
            return false;
      if (!testMidi3())
            return false;
      return true;
      }

