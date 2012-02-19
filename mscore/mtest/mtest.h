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

#ifndef __MTEST_H__
#define __MTEST_H__

#define TEST(a) if (!(a)) \
      printf("Test failed in <%s>: " #a "\n", __func__), passed = false;

class MScore;
class Score;

//---------------------------------------------------------
//   MTest
//---------------------------------------------------------

class MTest {
   public:
      MScore* mscore;
      QString root;     // root path of test source

      MTest();
      Score* readScore(const QString& name);
      bool saveScore(Score*, const QString& name);
      bool saveCompareScore(Score*,
         const QString& saveName, const QString& compareWith);
      };

extern MTest *mtest;
#endif

