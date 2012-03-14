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

#include <QtTest/QtTest>
#include "libmscore/score.h"
#include "libmscore/excerpt.h"
#include "libmscore/part.h"
#include "libmscore/undo.h"
#include "libmscore/measure.h"
#include "mtest/testutils.h"

#define DIR QString("libmscore/link/")

//---------------------------------------------------------
//   TestParts
//---------------------------------------------------------

class TestParts : public QObject, public MTest
      {
      Q_OBJECT

      void createParts(Score* score);
      void testPartCreation(const QString& test);

   private slots:
      void initTestCase();
      void createPart1();
      void createPart2();
      void appendMeasure();
      void insertMeasure();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestParts::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   createParts
//---------------------------------------------------------

void TestParts::createParts(Score* score)
      {
      //
      // create first part
      //
      QList<Part*> parts;
      parts.append(score->part(0));
      Score* nscore = ::createExcerpt(parts);
      QVERIFY(nscore);

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
      QVERIFY(nscore);

      nscore->setParentScore(score);
      nscore->setName(parts.front()->partName());
      score->undo(new AddExcerpt(nscore));
      nscore->style()->set(ST_createMultiMeasureRests, true);
      }

//---------------------------------------------------------
//   testPartCreation
//---------------------------------------------------------

void TestParts::testPartCreation(const QString& test)
      {
      Score* score = readScore(DIR + test + ".mscx");
      QVERIFY(score);
      QVERIFY(saveCompareScore(score, test + "-1.mscx", DIR + test + ".mscx"));
      createParts(score);
      QVERIFY(saveCompareScore(score, test + "-2.mscx", DIR + test + "-2o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   testAppendMeasure
//---------------------------------------------------------

void TestParts::appendMeasure()
      {
      Score* score = readScore(DIR + "part2.mscx");
      QVERIFY(score);
      createParts(score);

      score->startCmd();
      score->insertMeasure(MEASURE, 0);
      score->endCmd();

      QVERIFY(saveCompareScore(score, "part2-3.mscx", DIR + "part2-3o.mscx"));

      score->undo()->undo();
      score->endUndoRedo();

      QVERIFY(saveCompareScore(score, "part2-4.mscx", DIR + "part2-4o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   testInsertMeasure
//---------------------------------------------------------

void TestParts::insertMeasure()
      {
      Score* score = readScore(DIR + "part2.mscx");
      QVERIFY(score);
      createParts(score);

      score->startCmd();
      Measure* m = score->firstMeasure();
      score->insertMeasure(MEASURE, m);
      score->endCmd();

      QVERIFY(saveCompareScore(score, "part2-5.mscx", DIR + "part2-5o.mscx"));

      score->undo()->undo();
      score->endUndoRedo();

      QVERIFY(saveCompareScore(score, "part2-6.mscx", DIR + "part2-6o.mscx"));
      delete score;
      }

//---------------------------------------------------------
//   createPart1
//---------------------------------------------------------

void TestParts::createPart1()
      {
      testPartCreation("part1");
      }

//---------------------------------------------------------
//   createPart2
//---------------------------------------------------------

void TestParts::createPart2()
      {
      testPartCreation("part2");
      }

QTEST_MAIN(TestParts)

#include "tst_parts.moc"


