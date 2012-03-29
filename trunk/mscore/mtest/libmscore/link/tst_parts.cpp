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
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/breath.h"
#include "libmscore/segment.h"
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
      void createPartBreath();
      void addBreath();
      void undoAddBreath();
      void undoRedoAddBreath();
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
//   test part creation
//---------------------------------------------------------

void TestParts::createPart1()
      {
      testPartCreation("part1");
      }

void TestParts::createPart2()
      {
      testPartCreation("part2");
      }

void TestParts::createPartBreath()
      {
      testPartCreation("part3");
      }

//---------------------------------------------------------
//   test creation of elements
//---------------------------------------------------------

void TestParts::addBreath()
      {
      Score* score = readScore(DIR + "part1-2o.mscx");
      Measure* m = score->firstMeasure();
      Segment* s = m->tick2segment(480, false);
      Chord* chord = static_cast<Chord*>(s->element(0));
      Note* note   = chord->upNote();
      DropData dd;
      dd.view = 0;
      Breath* b = new Breath(score);
      b->setSubtype(0);
      dd.element = b;
      note->drop(dd);

      chord = static_cast<Chord*>(s->element(4));
      note   = chord->upNote();
      b = new Breath(score);
      b->setSubtype(0);
      dd.element = b;
      note->drop(dd);

      QVERIFY(saveCompareScore(score, "part4.mscx", DIR + "part4o.mscx"));
      }

//---------------------------------------------------------
//   undoAddBreath
//---------------------------------------------------------

void TestParts::undoAddBreath()
      {
      Score* score = readScore(DIR + "part1-2o.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m = score->firstMeasure();
      Segment* s = m->tick2segment(480, false);
      Chord* chord = static_cast<Chord*>(s->element(0));
      Note* note   = chord->upNote();
      DropData dd;
      dd.view = 0;
      Breath* b = new Breath(score);
      b->setSubtype(0);
      dd.element = b;
      score->startCmd();
      note->drop(dd);
      score->setLayoutAll(true);
      score->endCmd();        // does layout

      score->undo()->undo();
      score->endUndoRedo();

      QVERIFY(saveCompareScore(score, "part5.mscx", DIR + "part5o.mscx"));
      }

//---------------------------------------------------------
//   undoRedoAddBreath
//---------------------------------------------------------

void TestParts::undoRedoAddBreath()
      {
      Score* score = readScore(DIR + "part1-2o.mscx");
      score->doLayout();
      foreach(Excerpt* e, score->excerpts())
            e->score()->doLayout();

      Measure* m = score->firstMeasure();
      Segment* s = m->tick2segment(480, false);
      Chord* chord = static_cast<Chord*>(s->element(0));
      Note* note   = chord->upNote();
      DropData dd;
      dd.view = 0;
      Breath* b = new Breath(score);
      b->setSubtype(0);
      dd.element = b;
      score->startCmd();
      note->drop(dd);
      score->setLayoutAll(true);
      score->endCmd();

      score->undo()->undo();
      score->endUndoRedo();

      score->undo()->redo();
      score->endUndoRedo();

      QVERIFY(saveCompareScore(score, "part6.mscx", DIR + "part6o.mscx"));
      }


QTEST_MAIN(TestParts)

#include "tst_parts.moc"
#include "mops.ui"

