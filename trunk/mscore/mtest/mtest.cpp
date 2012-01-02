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

#include "config.h"
#include "libmscore/score.h"
#include "omr/omr.h"
#include "mscore/preferences.h"
#include "libmscore/instrtemplate.h"

bool debugMode = false;
bool noGui = true;

int bugs = 0;
Score* score;
MScore* mscore;
QString revision;

// dummy
#ifdef OMR
Omr::Omr(Score*) {}
void Omr::read(QDomElement) {}
void Omr::write(Xml&) const {}
#endif

extern bool testNote();
extern bool testMidi();

Preferences preferences;

Preferences::Preferences()
      {
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      QApplication app(argc, argv);
      mscore = new MScore;
      mscore->init();
      loadInstrumentTemplates("../../mscore/share/templates/instruments.xml");
      score = new Score(mscore->baseStyle());
      if (!testNote()) {
            printf("test note failed\n");
            ++bugs;
            }
      if (!testMidi()) {
            printf("test midi failed\n");
            ++bugs;
            }
      if (bugs)
            printf("test failed\n");
      else
            printf("test passed\n");
      return bugs;
      }

