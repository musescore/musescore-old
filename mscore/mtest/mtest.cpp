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
#include "config.h"
#include "libmscore/score.h"
#include "omr/omr.h"
#include "mscore/preferences.h"
#include "libmscore/instrtemplate.h"

MTest* mtest;

bool debugMode = false;
bool noGui = true;

int bugs = 0;
Score* score;
QString revision;

// dummy
#ifdef OMR
Omr::Omr(Score*) {}
void Omr::read(QDomElement) {}
void Omr::write(Xml&) const {}
#endif

extern bool testNote(MTest*);
extern bool testMidi(MTest*);
extern bool testHairpin(MTest*);
extern bool testParts(MTest*);

//---------------------------------------------------------
//   Preferences
//---------------------------------------------------------

Preferences preferences;
Preferences::Preferences()
      {
      }

struct Mtest {
      const char* name;
      bool (*proc)(MTest*);
      };

struct Mtest tests[] = {
      { "note",    testNote    },
      { "hairpin", testHairpin },
      { "midi",    testMidi    },
      { "parts",   testParts   },
      { 0, 0 }
      };

//---------------------------------------------------------
//   MTest
//---------------------------------------------------------

MTest::MTest()
      {
      mscore = new MScore;
      mscore->init();
      }

//---------------------------------------------------------
//   readScore
//---------------------------------------------------------

Score* MTest::readScore(const QString& name)
      {
      Score* score = new Score(mscore->baseStyle());
      score->setTestMode(true);
      if (!score->loadMsc(root + "/" + name)) {
            delete score;
            return 0;
            }
      score->doLayout();
      return score;
      }

//---------------------------------------------------------
//   saveScore
//---------------------------------------------------------

bool MTest::saveScore(Score* score, const QString& name)
      {
      QFileInfo fi(name);
      score->setTestMode(true);
      return score->saveFile(fi);
      }

//---------------------------------------------------------
//   saveCompareScore
//---------------------------------------------------------

bool MTest::saveCompareScore(Score* score,
   const QString& saveName, const QString& compareWith)
      {
      saveScore(score, saveName);

      QString cmd = "diff";
      QStringList args;
      args.append(saveName);
      args.append(root + "/" + compareWith);
      int n = QProcess::execute(cmd, args);
      if (n) {
            printf("   <%s", qPrintable(cmd));
            foreach(const QString& s, args)
                  printf(" %s", qPrintable(s));
            printf("> failed\n");
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      DPI  = 120;
      PDPI = 120;
      DPMM = DPI / INCH;

      QApplication app(argc, argv);

      mtest = new MTest;
      mtest->root = TESTROOT "/mtest";

      loadInstrumentTemplates("../../mscore/share/templates/instruments.xml");
      score = new Score(mtest->mscore->baseStyle());

      for (int i = 0;; ++i) {
            if (tests[i].name == 0)
                  break;
            printf("====Test <%s>====\n", tests[i].name);
            if ((*tests[i].proc)(mtest))
                  printf("   passed\n");
            else {
                  ++bugs;
                  printf("   failed\n");
                  }
            }
      if (bugs)
            printf("==%d tests failed==\n", bugs);
      else
            printf("==passed==\n");
      return bugs;
      }

