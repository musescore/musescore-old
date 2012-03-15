//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/instrtemplate.h"
#include "omr/omr.h"
#include "testutils.h"
#include "mscore/preferences.h"

bool debugMode = false;
bool noGui = true;
QString revision;

Score* score;

// dummy
#ifdef OMR
Omr::Omr(Score*) {}
void Omr::read(QDomElement) {}
void Omr::write(Xml&) const {}
#endif

//---------------------------------------------------------
//   Preferences
//---------------------------------------------------------

Preferences preferences;
Preferences::Preferences()
      {
      }

//---------------------------------------------------------
//   writeReadElement
//    writes and element and reads it back
//---------------------------------------------------------

Element* MTest::writeReadElement(Element* element)
      {
      //
      // write element
      //
      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      Xml xml(&buffer);
      xml.header();
      element->write(xml);
      buffer.close();

      //
      // read element
      //
//      printf("%s\n", buffer.buffer().data());

      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(buffer.buffer(), &err, &line, &column)) {
            printf("writeReadElement: error reading paste data at line %d column %d: %s\n",
               line, column, qPrintable(err));
            printf("%s\n", buffer.buffer().data());
            return 0;
            }
      docName = "--";
      QDomElement e = doc.documentElement();
      QString tag(e.tagName());
      element = Element::name2Element(e.tagName(), score);
      element->read(e);
      return element;
      }

//---------------------------------------------------------
//   MTest
//---------------------------------------------------------

MTest::MTest()
      {
      }

//---------------------------------------------------------
//   readScore
//---------------------------------------------------------

Score* MTest::readScore(const QString& name)
      {
      Score* score = new Score(mscore->baseStyle());
      score->setTestMode(true);
      QString path = root + "/" + name;
      if (!score->loadMsc(path)) {
            QWARN(qPrintable(QString("readScore: cannot load <%1>\n").arg(path)));
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

bool MTest::saveCompareScore(Score* score, const QString& saveName, const QString& compareWith)
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
//   initMTest
//---------------------------------------------------------

void MTest::initMTest()
      {
      DPI  = 120;
      PDPI = 120;
      DPMM = DPI / INCH;

      mscore = new MScore;
      mscore->init();

      root = TESTROOT "/mtest";
      loadInstrumentTemplates(":/instruments.xml");
      score = new Score(mscore->baseStyle());
      }

