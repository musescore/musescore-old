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

#define DIR QString("libmscore/join/")

//---------------------------------------------------------
//   TestJoin
//---------------------------------------------------------

class TestJoin : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void split();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestJoin::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   beam
//---------------------------------------------------------

void TestJoin::split()
      {
      const char* path = "join.mscx";
      Score* score = readScore(DIR + path);
      QVERIFY(score);
      Measure* m1 = score->firstMeasure();
      Measure* m2 = m1->nextMeasure()->nextMeasure();

      QVERIFY(m1 != 0);
      QVERIFY(m2 == 0);

      score->cmdJoinMeasure(m1, m2);

      QVERIFY(saveCompareScore(score, path, DIR + "join-ref.mscx"));
      delete score;
      }

QTEST_MAIN(TestJoin)
#include "tst_join.moc"

