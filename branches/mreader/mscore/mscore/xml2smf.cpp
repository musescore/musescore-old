//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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
#include "xml.h"
#include "midifile.h"

static const char versionString[] = "0.1";

int division         = 480;
bool debugMode       = false;
bool noRunningStatus = false;

//---------------------------------------------------------
//   printVersion
//---------------------------------------------------------

static void printVersion()
      {
      printf("This is xml2smf version %s\n", versionString);
      }

//---------------------------------------------------------
//   usage
//---------------------------------------------------------

static void usage()
      {
      printVersion();
      printf("Usage: xml2smf [args] [infile] [outfile]\n");
      printf("   args:\n"
             "      -v      print version\n"
             "      -d      debug mode\n"
             "      -D nn   change division to nn\n"
             "      -r      do not use running status for smf output\n"
            );
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      int c;
      int division = -1;
      while ((c = getopt(argc, argv, "vdD:r")) != EOF) {
            switch (c) {
                  case 'v':
                        printVersion();
                        return 0;
                  case 'd':
                        debugMode = true;
                        break;
                  case 'D':
                        division = atoi(optarg);
                        break;
                  case 'r':
                        noRunningStatus = true;
                        break;
                  default:
                        usage();
                        return -1;
                  }
            }
      QIODevice* in = 0;
      QIODevice* out = 0;

      switch (argc - optind) {
            case 2:
                  out = new QFile(argv[1 + optind]);
                  if (!out->open(QIODevice::WriteOnly)) {
                        printf("cannot open output file <%s>: %s\n", argv[optind+1], strerror(errno));
                        return -3;
                        }
            case 1:
                  in = new QFile(argv[optind]);
                  docName = argv[optind];
                  if (!in->open(QIODevice::ReadOnly)) {
                        printf("cannot open input file <%s>: %s\n", argv[optind], strerror(errno));
                        return -4;
                        }
                  break;
            case 0:
                  break;
            default:
                  usage();
                  return -2;
                  break;
            }
      if (in == 0) {
            docName = "stdin";
            in = new QFile;
            ((QFile*)in)->open(stdin, QIODevice::ReadOnly);
            }
      if (out == 0) {
            out = new QFile;
            ((QFile*)out)->open(stdout, QIODevice::WriteOnly);
            }

      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(in, false, &err, &line, &column)) {
            printf("error reading file at line %d column %d: %s\n",
               line, column, err.toLatin1().data());
            return 1;
            }

      MidiFile mf;
      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "SMF") {
                  mf.readXml(e);
                  break;
                  }
            }
      in->close();

      if (division != -1)
            mf.changeDivision(division);

      mf.setFormat(1);
      mf.setNoRunningStatus(noRunningStatus);
      mf.write(out);
      out->close();
      delete out;
      delete in;
      return 0;
      }

//---------------------------------------------------------
//   quantizeLen
//---------------------------------------------------------

int quantizeLen(int len, int raster)
      {
      int rl = ((len + raster - 1) / raster) * raster;
      rl /= 2;
      if (rl == 0)
            rl = 1;
      rl = ((len + rl - 1) / rl) * rl;
      return rl;
      }


