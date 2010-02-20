//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "globals.h"
#include "mreader.h"
#include "scan.h"
#include "scanview.h"
#include "xml.h"
#include "utils.h"

//---------------------------------------------------------
//   MuseReader
//---------------------------------------------------------

MuseReader::MuseReader(QWidget* parent)
   : QMainWindow(parent)
      {
      setupUi(this);
      lastOpenPath = QDir::currentPath();

      _scanView = new ScanView(this);
      setCentralWidget(_scanView);

      QLabel* label = new QLabel(tr("Page:"), this);
      label->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
      label->setLineWidth(0);
      label->setMidLineWidth(0);

      statusBar()->addWidget(label);
      pageNumber = new QSpinBox(this);
      pageNumber->setRange(1, 1000);
      statusBar()->addWidget(pageNumber);

      label = new QLabel(tr("of "), this);
      statusBar()->addWidget(label);
      maxPages = new QLabel;
      statusBar()->addWidget(maxPages);

      label = new QLabel("    ");
      statusBar()->addWidget(label);

      label = new QLabel(tr("x:"), this);
      label->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
      statusBar()->addWidget(label);
      xLabel = new QLabel(this);
      xLabel->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
      statusBar()->addWidget(xLabel);

      label = new QLabel(tr("y:"), this);
      label->setFrameStyle(0);
      statusBar()->addWidget(label);
      yLabel = new QLabel(this);
      yLabel->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
      statusBar()->addWidget(yLabel);

      connect(_scanView,    SIGNAL(pageNumberChanged(int)), pageNumber, SLOT(setValue(int)));
      connect(_scanView,    SIGNAL(xPosChanged(int)), xLabel, SLOT(setNum(int)));
      connect(_scanView,    SIGNAL(yPosChanged(int)), yLabel, SLOT(setNum(int)));
      connect(pageNumber,   SIGNAL(valueChanged(int)), _scanView, SLOT(gotoPage(int)));
      connect(actionSaveAs, SIGNAL(triggered()), SLOT(save()));
      connect(actionOpen,   SIGNAL(triggered()), SLOT(load()));
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MuseReader::closeEvent(QCloseEvent*)
      {
      writeSettings();
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void MuseReader::load()
      {
      QString fn = QFileDialog::getOpenFileName(
         this,
         tr("MuseReader: Load PDF"),
         lastOpenPath,
         tr("PDF (*.pdf *.PDF)")
         );
      if (fn.isEmpty())
            return;
      if (!load(fn)) {
            QString s = tr("Open File\n") + fn + tr("\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(this, tr("MuseReader: Read File"), s);
            }
      }

//---------------------------------------------------------
//   load
//    return true on success
//---------------------------------------------------------

bool MuseReader::load(const QString& fn)
      {
      Scan* scan = new Scan;
      if (!scan->read(fn)) {
            delete scan;
            return false;
            }
      lastOpenPath = fn;
      _scanView->setScan(scan);
      pageNumber->setRange(1, _scanView->scan()->numPages());
      maxPages->setNum(scan->pagesInDocument());
      return true;
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void MuseReader::save()
      {
      if (_scanView == 0 || _scanView->scan() == 0)
            return;
      QString fn = QFileDialog::getSaveFileName(
         this,
         tr("MuseReader: Save Score"),
         lastOpenPath,
         tr("MuseScore (*.mscx)")
         );
      if (fn.isEmpty())
            return;
      if (!save(fn)) {
            QString s = tr("Save Score as\n") + fn + tr("\failed: ")
               + QString(strerror(errno));
            QMessageBox::critical(this, tr("MuseReader: Save As"), s);
            }
      }

//---------------------------------------------------------
//   save
//    return true on success
//---------------------------------------------------------

bool MuseReader::save(const QString& fn)
      {
      QFile f(fn);
      if (!f.open(QIODevice::WriteOnly))
            return false;
      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"1.12\"");
      _scanView->scan()->save(xml);
      xml.etag();
      f.close();
      return true;
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void MuseReader::readSettings()
      {
      QSettings settings;
      settings.beginGroup("MainWindow");
      resize(settings.value("size", QSize(950, 700)).toSize());
      move(settings.value("pos", QPoint(10, 10)).toPoint());
      if (settings.value("maximized", false).toBool())
            showMaximized();
      restoreState(settings.value("state").toByteArray());
      settings.endGroup();
      lastOpenPath = settings.value("lastOpenPath", lastOpenPath).toString();
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void MuseReader::writeSettings()
      {
      QSettings settings;
      settings.beginGroup("MainWindow");
      settings.setValue("size", size());
      settings.setValue("pos", pos());
      settings.setValue("maximized", isMaximized());
      settings.setValue("state", saveState());
      settings.endGroup();
      settings.setValue("lastOpenPath", lastOpenPath);
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      initUtils();
      QApplication::setGraphicsSystem(QString("opengl"));
      QApplication* app = new QApplication(argc, argv);

      int c;
      while ((c = getopt(argc, argv, "Vd:m:p:")) != EOF) {
            switch(c) {
                  case 'V':
                        printf("%s version 0.0.1\n", argv[0]);
                        return 0;
                  case 'd':
                        debugMode = atoi(optarg);
                        break;
                  case 'm':
                        maxPages = atoi(optarg);
                        if (maxPages < 1)
                              maxPages = 1;
                        break;
                  case 'p':
                        startPage = atoi(optarg) - 1;
                        if (startPage < 0)
                              startPage = 0;
                        if (maxPages == 0)
                              maxPages = 1;
                        break;

                  default:
                        fprintf(stderr, "%s: bad argument\n", argv[0]);
                        fprintf(stderr, "  -V      print version\n");
                        fprintf(stderr, "  -d nn   set debug level\n");
                        fprintf(stderr, "  -p nn   start at page nn\n");
                        fprintf(stderr, "  -m nn   process only nn pages\n");
                        return -1;
                  }
            }
      QCoreApplication::setOrganizationName("MusE");
      QCoreApplication::setOrganizationDomain("musescore.org");
      QCoreApplication::setApplicationName("MuseReader");
      MuseReader* mr = new MuseReader;
      mr->readSettings();
      if (argc - optind == 1) {
            if (!mr->load(argv[optind]))
                  fprintf(stderr, "cannot load <%s>\n", argv[optind]);
            }
      mr->show();
      return app->exec();
      }

