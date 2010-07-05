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
      bool xml = false;

      fprintf(stderr, "%s: convert sound file\n", argv[0]);

      int c;
      while ((c = getopt(argc, argv, "x")) != EOF) {
            switch(c) {
                  case 'x':
                        xml = true;
                        break;
                  default:
                        fprintf(stderr, "usage: %s [-flags] infile outfile\n", argv[0]);
                        exit(1);
                  }
            }

      argc -= optind;
      argv += optind;

      if (argc != 2) {
            fprintf(stderr, "usage: %s [-flags] infile outfile\n", argv[0]);
            exit(1);
            }

      SoundFont sf(argv[0]);

      if (!sf.read()) {
            fprintf(stderr, "sf read error\n");
            exit(3);
            }

      QFile fo(argv[1]);
      if (!fo.open(QIODevice::WriteOnly)) {
            fprintf(stderr, "cannot open <%s>\n", argv[2]);
            exit(2);
            }
      if (xml)
            sf.writeXml(&fo);
      else
            sf.write(&fo);
      fo.close();
      return 0;
      }

