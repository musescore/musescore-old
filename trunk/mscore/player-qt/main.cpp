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

#include "config.h"
#include "libmscore/mscore.h"
#include "scoreview.h"
#include "omr/omr.h"
#include "seq.h"
#include "runtime.h"

QString revision;

Runtime* runtime;

// dummy
#ifdef OMR
Omr::Omr(Score*) {}
void Omr::read(QDomElement) {}
void Omr::write(Xml&) const {}
#endif

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      QApplication app(argc, argv);

      QWidget wi(0);
      MScore::PDPI = wi.logicalDpiX();    // physical resolution
      MScore::DPI  = MScore::PDPI;                // logical drawing resolution
      MScore::DPMM = MScore::DPI / INCH;          // dots/mm

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

