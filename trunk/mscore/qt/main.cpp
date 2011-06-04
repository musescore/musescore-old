//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
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

#include <stdio.h>
#include <QtGui/QApplication>
#include <QtGui/QFontDatabase>
#include "scoreview.h"

extern void setDefaultStyle();
extern void initStaffTypes();
extern void initSyms();

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

      //
      //  load internal fonts
      //
      static const char* fonts[] = {
            "mscore-20", "mscore1-20", "MuseJazz", "FreeSans", "FreeSerif",
            "gonville-20",
            };
      for (unsigned i = 0; i < sizeof(fonts)/sizeof(*fonts); ++i) {
            if (-1 == QFontDatabase::addApplicationFont(QString(":/fonts/%1.ttf").arg(fonts[i]))) {
                  if (-1 == QFontDatabase::addApplicationFont(QString(":/fonts/%1.otf").arg(fonts[i]))) {
                        fprintf(stderr, "mscoreplay: fatal error: cannot load internal font <%s>\n", fonts[i]);
                        exit(-1);
                        }
                  }
            }

      setDefaultStyle();
      initStaffTypes();

      ScoreView* view = new ScoreView;
      view->loadFile(argv[1]);
      view->show();
      return qApp->exec();
      }

