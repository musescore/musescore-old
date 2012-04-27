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
#include "mtest/testutils.h"
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/chordrest.h"

#define DIR QString("libmscore/split/")

//---------------------------------------------------------
//   TestSplit
//---------------------------------------------------------

class TestSplit : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void split();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSplit::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   beam
//---------------------------------------------------------

void TestSplit::split()
      {
      const char* path = "split.mscx";
      Score* score = readScore(DIR + path);
      QVERIFY(score);
      Measure* m = score->firstMeasure();
      Segment* s = m->first(SegChordRest);
      s = s->next(SegChordRest);
      s = s->next(SegChordRest);
      ChordRest* cr = static_cast<ChordRest*>(s->element(0));

      score->cmdSplitMeasure(cr);

      QVERIFY(saveCompareScore(score, path, DIR + "split-ref.mscx"));
      delete score;
      }

QTEST_MAIN(TestSplit)
#include "tst_split.moc"

