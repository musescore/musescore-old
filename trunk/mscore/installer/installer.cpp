//=========================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: installer.cpp,v 1.4 2006/03/15 08:55:39 wschweer Exp $
//
//  (C) Copyright 2006 Werner Schweer (ws@seh.de)
//=========================================================

#include "../mscore/config.h"
#include "installer.h"
#include "licence.h"
#include "fontconfig/fontconfig.h"
#include "getopt.h"

bool rootInstall;
QString installPrefix("/usr/local");

//---------------------------------------------------------
//   Installer
//---------------------------------------------------------

Installer::Installer(QWidget* parent)
   : QStackedWidget(parent)
      {
      setupUi(this);

      setWindowTitle(tr("Install MuseScore"));
      description->setHtml(tr("<b>MuseScore</b> is a WYSIWYG (What You See Is What You Get)"
         " program to create printed score.<br>\n"
         "Some highlights:\n<br><br>"
         "<ul>\n"
         "<li> WYSIWYG Design, notes are entered on a <i>virtual notesheet</i></item>\n"
         "<li>uses TrueType fonts for printing and screen display</item>\n"
         "</ul>\n"));
      licence->setPlainText(licenceTxt);
      setCurrentIndex(0);
      connect(installButton, SIGNAL(clicked()), SLOT(installOk()));
      connect(acceptLicence, SIGNAL(clicked()), SLOT(licenceOk()));
      connect(backLicence, SIGNAL(clicked()), SLOT(goBackLicence()));
      connect(checkOk, SIGNAL(clicked()), SLOT(goCheckOk()));
      }

//---------------------------------------------------------
//   goBackLicence
//---------------------------------------------------------

void Installer::goBackLicence()
      {
	setCurrentIndex(0);
      }

//---------------------------------------------------------
//   1 - installOk
//	start installation
//---------------------------------------------------------

void Installer::installOk()
      {
	setCurrentIndex(1);
      }

//---------------------------------------------------------
//   2 - licenceOk
//	licence was accepted
//---------------------------------------------------------

void Installer::licenceOk()
      {
      int fail = 0;
	setCurrentIndex(2);
      checkLog->addItem("System check running...");
      qApp->processEvents();
      if (fail)
      	checkLog->addItem("System check failed");
      else
      	checkLog->addItem("System sucessfully checked");
      }

//---------------------------------------------------------
//   3 - goCheckOk
//	system Check was OK
//---------------------------------------------------------

void Installer::goCheckOk()
      {
	setCurrentIndex(3);
      log("Installing...");

      QStringList fonts;
      fonts << ":/emmentaler_20.otf";
      installFonts(fonts);

      log("install /usr/bin/mscore");
      QFile f(":/data/mscore");
      if (!f.exists()) {
            log("file :/data/mscore does not exist\n");
            exit(2);
            }
      QFile df("mops");
      if (df.exists()) {
            if (!df.remove()) {
                  log(QString("remove existing file <%1> failed\n").arg(df.fileName()));
                  exit(3);
                  }
            }
      if (!f.copy("mops")) {
            log("copy failed\n");
            exit(1);
        	}
      if (!df.setPermissions(
         QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner
         | QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup
         | QFile::ReadOther | QFile::ExeOther
         )) {
            log(QString("set permissions on <%1> failed\n").arg(f.fileName()));
            exit(2);
            }
      qApp->processEvents();
      setCurrentIndex(4);
      }

//---------------------------------------------------------
//   installFonts
//    return false on error
//---------------------------------------------------------

bool Installer::installFonts(const QStringList& fl)
      {
      //
      // initialize font config subsystem
      //
      if (!FcInit()) {
            log("Initialize fontconfig library failed\n");
            return false;
            }
      int version = FcGetVersion();
      log(QString("FontConfig: found version %1").arg(version));
      FcConfig* fc = FcInitLoadConfig();
      if (fc == 0) {
            log("FontConfig: load default config failed\n");
            return false;
            }
      //
      // get list of font directories, select a dir we have write
      // access to
      //
      FcStrList* fontDirs = FcConfigGetFontDirs(fc);
      if (fontDirs == 0) {
            log("FontConfig: get font dirs failed\n");
            return false;
            }
      char* path = 0;
      for (;;) {
            FcChar8* p = FcStrListNext(fontDirs);
            if (p == 0)
                  break;
            log(QString("<%1>").arg(QString((char*)p)));
            QFileInfo qf((char*)p);
            if (qf.isWritable()) {
                  QFile::Permissions perm = qf.permissions();
                  log(QString(" ...writable %1").arg(int(perm)));
                  path = (char*)p;
                  }
            }
      if (path == 0) {
            log("FontConfig: cannot find suitable font directory\n");
            return false;
            }

      //
      // copy font(s) into font directory
      //
      path = "/home/ws/.fonts";

      if (path) {
            foreach (QString s, fl) {
                  log(QString("copy <%1> to <%1>").arg(s).arg(path));
                  if (!QFile::copy(s, path + s.mid(1))) {
                        log("copy file failed");
                        return false;
                        }
                  }
            }

      //
      // start fc-cache to update font cache
      //
      QStringList lst;
      lst << "-v";
      lst << path;
      printf("starting fc-cache...\n");
      if (QProcess::execute("fc-cache", lst)) {
            log("FontConfig: update font cache failed\n");
            return false;
            }
      FcFini();
      return true;
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      int id = geteuid();
      rootInstall = id == 0;

      if (!rootInstall)
            installPrefix = QDir::homePath();

      for (;;) {
            int optionIndex = 0;
            static struct option longOptions[] = {
                  { "prefix",  1, 0, 'p' },
                  { "help",    0, 0, 'h' },
                  { "local",   0, 0, 'l' },
                  { "version", 0, 0, 'v' },
                  { 0, 0, 0, 0 }
                  };
            int c = getopt_long(argc, argv, "p:hv", longOptions, &optionIndex);
            if (c == -1)
                  break;
            switch(c) {
                  case 'v':
                        printf("This installs MuseScore Version " VERSION "\n");
                        exit(0);
                  case 'p':
                        installPrefix = optarg;
                        break;
                  case 'l':
                        rootInstall = false;
                        break;
                  case 'h':
                  default:
                        printf("MuseScore " VERSION " Installer Usage: %s [args]\n"
                           "  args: -h        print help message\n"
                           "        -v        print version\n"
                           "        -p prefix install path prefix\n"
                           "        -l        local install\n",
                           argv[0]);
                        exit(0);
                  }
            }
      new QApplication(argc, argv);
      Installer* installer = new Installer;
      installer->show();
      qApp->setQuitOnLastWindowClosed(true);
      return qApp->exec();
      }

