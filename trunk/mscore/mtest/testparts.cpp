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

#include "mtest.h"
#include "libmscore/score.h"
#include "libmscore/excerpt.h"
#include "libmscore/part.h"
#include "libmscore/undo.h"
#include "libmscore/measure.h"
#include "testutils.h"

//---------------------------------------------------------
//   createParts
//---------------------------------------------------------

static bool createParts(Score* score)
      {
      //
      // create first part
      //
      QList<Part*> parts;
      parts.append(score->part(0));
      Score* nscore = ::createExcerpt(parts);
      if (nscore == 0) {
            printf("         failed\n");
            return false;
            }
      nscore->setParentScore(score);
      nscore->setName(parts.front()->partName());
      score->undo(new AddExcerpt(nscore));
      nscore->style()->set(ST_createMultiMeasureRests, true);

      //
      // create second part
      //
      parts.clear();
      parts.append(score->part(1));
      nscore = ::createExcerpt(parts);
      if (nscore == 0) {
            printf("         failed\n");
            return false;
            }
      nscore->setParentScore(score);
      nscore->setName(parts.front()->partName());
      score->undo(new AddExcerpt(nscore));
      nscore->style()->set(ST_createMultiMeasureRests, true);
      return true;
      }

//---------------------------------------------------------
//   testPartCreation
//---------------------------------------------------------

static bool testPartCreation(MTest* mt, const QString& test)
      {
      printf("      %s\n", qPrintable(test));
      Score* score = mt->readScore(test + ".mscx");
      if (score == 0) {
            printf("         failed\n");
            return false;
            }

      if (!mt->saveCompareScore(score, test + "-1.mscx", test + ".mscx")) {
            printf("         failed\n");
            return false;
            }

      if (!createParts(score))
            return false;

      if (!mt->saveCompareScore(score, test + "-2.mscx", test + "-2o.mscx")) {
            printf("         failed\n");
            return false;
            }
      printf("         ok\n");
      delete score;
      return true;
      }

//---------------------------------------------------------
//   testAppendMeasure
//---------------------------------------------------------

static bool testAppendMeasure(MTest* mt, const QString& test)
      {
      printf("      %s\n", qPrintable(test));
      Score* score = mt->readScore("part2.mscx");
      if (score == 0) {
            printf("         failed\n");
            return false;
            }

      if (!createParts(score))
            return false;

      score->startCmd();
      score->insertMeasure(MEASURE, 0);
      score->endCmd();

      if (!mt->saveCompareScore(score, "part2-3.mscx", "part2-3o.mscx")) {
            printf("         failed\n");
            return false;
            }

      score->undo()->undo();
      score->endUndoRedo();

      if (!mt->saveCompareScore(score, "part2-4.mscx", "part2-4o.mscx")) {
            printf("         failed\n");
            return false;
            }

      printf("         ok\n");
      delete score;
      return true;
      }

//---------------------------------------------------------
//   testInsertMeasure
//---------------------------------------------------------

static bool testInsertMeasure(MTest* mt, const QString& test)
      {
      printf("      %s\n", qPrintable(test));
      Score* score = mt->readScore("part2.mscx");
      if (score == 0) {
            printf("         failed\n");
            return false;
            }

      if (!createParts(score))
            return false;

      score->startCmd();
      Measure* m = score->firstMeasure();
      score->insertMeasure(MEASURE, m);
      score->endCmd();

      if (!mt->saveCompareScore(score, "part2-5.mscx", "part2-5o.mscx")) {
            printf("         failed\n");
            return false;
            }

      score->undo()->undo();
      score->endUndoRedo();

      if (!mt->saveCompareScore(score, "part2-6.mscx", "part2-6o.mscx")) {
            printf("         failed\n");
            return false;
            }

      printf("         ok\n");
      delete score;
      return true;
      }

//---------------------------------------------------------
//   testParts
//---------------------------------------------------------

bool testParts(MTest* mt)
      {
      if (!testPartCreation(mt, "part1"))
            return false;
      if (!testPartCreation(mt, "part2"))
            return false;
      if (!testAppendMeasure(mt, "append measure"))
            return false;
      if (!testInsertMeasure(mt, "insert measure"))
            return false;
      return true;
      }

