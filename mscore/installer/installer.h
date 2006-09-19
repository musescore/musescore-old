//=========================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: installer.h,v 1.2 2006/02/20 21:50:20 wschweer Exp $
//
//  (C) Copyright 2006 Werner Schweer (ws@seh.de)
//=========================================================

#ifndef __INSTALLER_H__
#define __INSTALLER_H__

#include "installerbase.h"

//---------------------------------------------------------
//   Installer
//---------------------------------------------------------

class Installer : public QStackedWidget, public Ui::InstallerBase {
	Q_OBJECT

      bool installFonts(const QStringList&);
      void log(const QString s) {
            installLog->addItem(s);
            }

   private slots:
      void installOk();
      void goBackLicence();
      void goCheckOk();
      void licenceOk();

   public:
      Installer(QWidget* parent = 0);
      };

#endif

