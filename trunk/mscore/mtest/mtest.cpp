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
#include "all.h"

//---------------------------------------------------------
//   process
//---------------------------------------------------------

static void process(const QString& cmd)
      {
      QStringList args;
      QProcess::execute(cmd, args);
      }

//---------------------------------------------------------
//   scanDir
//---------------------------------------------------------

static void scanDir(QDir d)
      {
      QFileInfoList l = d.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
      foreach(const QFileInfo& fi, l) {
            if (fi.isDir()) {
                  scanDir(QDir(fi.filePath()));
                  }
            else if (fi.isExecutable()) {
                  QString s(fi.filePath());
                  if (fi.baseName().startsWith("tst_"))
                        process(s);
                  }
            }
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      QDir wd(QDir::current());
      scanDir(wd);
      return 0;
      }

