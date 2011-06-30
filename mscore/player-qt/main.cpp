//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include <stdio.h>
#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/QDeclarativeView>

#include "libmscore/mscore.h"
#include "scoreview.h"
#include "omr/omr.h"
#include "seq.h"

bool debugMode = false;
QString revision;

// dummies:

Omr::Omr(Score*) {}
void Omr::write(Xml&) const {}
void Omr::read(QDomElement) {}

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      new QApplication(argc, argv);

      QWidget wi(0);
      PDPI = wi.logicalDpiX();    // physical resolution
      DPI  = PDPI;                // logical drawing resolution
      DPMM = DPI / INCH;          // dots/mm

      MScore::init();
      seq = new Seq;
      if (!seq->init()) {
            printf("cannot initialize sequencer\n");
            exit(-1);
            }

      qmlRegisterType<ScoreView>("MuseScore", 1, 0, "ScoreView");

      QDeclarativeView view;
      view.resize(1040, 780);
      view.setSource(QUrl("qrc:/mplayer.qml"));
      view.show();

      return qApp->exec();
      }

