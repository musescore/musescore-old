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
#include "libmscore/mscore.h"
#include "scoreview.h"

bool debugMode = false;
QString revision;

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      if (argc != 2) {
            fprintf(stderr, "usage: %s <scorefile>\n", argv[0]);
            return -1;
            }
      new QApplication(argc, argv);

      QWidget wi(0);
      PDPI = wi.logicalDpiX();         // physical resolution
      DPI  = PDPI;                     // logical drawing resolution
      DPMM = DPI / INCH;      // dots/mm

      MScore::init();

      ScoreView* view = new ScoreView;
      view->loadFile(argv[1]);
      view->show();
      return qApp->exec();
      }

