//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <QtCore/QFile>
#include "sfont.h"

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      fprintf(stderr, "%s: compress sound file\n", argv[0]);
      if (argc != 3) {
            fprintf(stderr, "usage: %s infile outfile\n", argv[0]);
            exit(1);
            }
      SoundFont sf(argv[1]);

      if (!sf.read()) {
            fprintf(stderr, "sf read error\n");
            exit(3);
            }

      QFile fo(argv[2]);
      if (!fo.open(QIODevice::WriteOnly)) {
            fprintf(stderr, "cannot open <%s>\n", argv[2]);
            exit(2);
            }
//      sf.writeXml(&fo);
      sf.write(&fo);
      fo.close();
      return 0;
      }

