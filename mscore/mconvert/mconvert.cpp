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

#include <stdio.h>

#include "config.h"
#include "libmscore/mscore.h"
#include "libmscore/score.h"
#include "libmscore/page.h"
#include "omr/omr.h"

QString revision;

// dummy
#ifdef OMR
Omr::Omr(Score*) {}
void Omr::read(QDomElement) {}
void Omr::write(Xml&) const {}
#endif

//---------------------------------------------------------
//   usage
//---------------------------------------------------------

static void usage()
      {
      printf("Usage: mconvert infile.mscx -o outfile.pdf\n");
      exit(-1);
      }

//---------------------------------------------------------
//   savePsPdf
//---------------------------------------------------------

static bool savePsPdf(Score* cs, const QString& saveName, QPrinter::OutputFormat format)
      {
      const PageFormat* pf = cs->pageFormat();
      QPrinter printerDev(QPrinter::HighResolution);

      printerDev.setPaperSize(pf->size(), QPrinter::Inch);

      printerDev.setCreator("MuseScore Version: " VERSION);
      printerDev.setFullPage(true);
      printerDev.setColorMode(QPrinter::Color);
      printerDev.setDocName(cs->name());
      printerDev.setDoubleSidedPrinting(pf->twosided());
      printerDev.setOutputFormat(format);
      printerDev.setOutputFileName(saveName);

      QPainter p(&printerDev);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);

      double mag = printerDev.logicalDpiX() / MScore::DPI;
      p.scale(mag, mag);

      const QList<Page*> pl = cs->pages();
      int pages    = pl.size();

      bool firstPage = true;
      for (int n = 0; n < pages; ++n) {
            if (!firstPage)
                  printerDev.newPage();
            firstPage = false;
            cs->print(&p, n);
            }
      p.end();
      return true;
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      new QApplication(argc, argv);

      QStringList args =  QCoreApplication::arguments();
      args.removeFirst();
      QString outName;

      for (int i = 0; i < args.size();) {
            QString s = args[i];
            if (s[0] != '-') {
                  ++i;
                  continue;
                  }
            switch(s[1].toAscii()) {
                  case 'v':
                        printf("MuseScore converter 0.1\n");
                        return 0;
                   case 'd':
                        MScore::debugMode = true;
                        break;
                  case 'L':
                        MScore::layoutDebug = true;
                        break;
                  case 'o':
                        if (args.size() - i < 2)
                              usage();
                        outName = args.takeAt(i + 1);
                        break;
                  default:
                        usage();
                  }
            args.removeAt(i);
            }
      if (outName.isEmpty() || args.size() != 1)
            usage();
      MScore::init();
      Score* score = new Score(MScore::defaultStyle());
      if (!score->loadMsc(args[0])) {
            printf("cannot load <%s>\n", qPrintable(args[0]));
            return -2;
            }
      score->doLayout();
      savePsPdf(score, outName, QPrinter::PdfFormat);
      return 0;
      }

