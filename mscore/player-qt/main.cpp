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
#include <QtDeclarative/QDeclarativeContext>

#include "libmscore/mscore.h"
#include "scoreview.h"
#include "omr/omr.h"
#include "seq.h"
#include "runtime.h"

bool debugMode = false;
QString revision;

Runtime* runtime;

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      QApplication app(argc, argv);

      QWidget wi(0);
      PDPI = wi.logicalDpiX();    // physical resolution
      // PDPI = 150.0;

      DPI  = PDPI;                // logical drawing resolution
      DPMM = DPI / INCH;          // dots/mm

      runtime = new Runtime;

      MScore::init();
      seq = new Seq;
      if (!seq->init()) {
            printf("cannot initialize sequencer\n");
            exit(-1);
            }

      qmlRegisterType<ScoreView>("MuseScore", 1, 0, "ScoreView");

      QDeclarativeView view;
      view.setResizeMode(QDeclarativeView::SizeRootObjectToView);
      QDeclarativeContext* ctxt = view.rootContext();
      ctxt->setContextProperty(QLatin1String("runtime"), runtime);

      // registering only for exposing the Runtime::Orientation enum
      qmlRegisterUncreatableType<Runtime>("Qt", 4, 7, "Orientation", QString());
      qmlRegisterUncreatableType<Runtime>("QtQuick", 1, 0, "Orientation", QString());

      view.setSource(QUrl("qrc:/mplayer.qml"));
      view.show();
      return app.exec();
      }

