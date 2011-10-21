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
bool mergeNotes      = false;
bool separateChannel = false;
bool cleanup         = false;
bool smfOutput       = false;
bool noRunningStatus = false;

//---------------------------------------------------------
//   printVersion
//---------------------------------------------------------

static void printVersion()
      {
      qDebug("This is smf2xml version %s\n", versionString);
      }

//---------------------------------------------------------
//   usage
//---------------------------------------------------------

static void usage()
      {
      printVersion();
      qDebug("Usage: smf2xml [args] [infile] [outfile]\n");
      qDebug("   args:\n"
             "      -v      print version\n"
             "      -d      debug mode\n"
             "      -m      merge note on/off events\n"
             "      -s      separate channels into different tracks\n"
             "      -c      cleanup: quantize, remove overlaps\n"
             "      -D nn   change division to nn\n"
             "      -M      output smf instead of xml\n"
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
      while ((c = getopt(argc, argv, "vdmscD:Mr")) != EOF) {
            switch (c) {
                  case 'v':
                        printVersion();
                        return 0;
                  case 'd':
                        debugMode = true;
                        break;
                  case 'm':
                        mergeNotes = true;
                        break;
                  case 's':
                        separateChannel = true;
                        break;
                  case 'c':
                        cleanup = true;
                        break;
                  case 'D':
                        division = atoi(optarg);
                        break;
                  case 'M':
                        smfOutput = true;
                        break;
                  case 'r':
                        noRunningStatus = true;
                        break;
                  default:
                        usage();
                        return -1;
                  }
            }
      if (cleanup)
            mergeNotes = true;

      QIODevice* in = 0;
      QIODevice* out = 0;

      switch (argc - optind) {
            case 2:
                  out = new QFile(argv[1 + optind]);
                  if (!out->open(QIODevice::WriteOnly)) {
                        qDebug("cannot open output file <%s>: %s\n", argv[2], strerror(errno));
                        return -3;
                        }
            case 1:
                  in = new QFile(argv[0 + optind]);
                  if (!in->open(QIODevice::ReadOnly)) {
                        qDebug("cannot open input file <%s>: %s\n", argv[1], strerror(errno));
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
            in = new QFile;
            ((QFile*)in)->open(stdin, QIODevice::ReadOnly);
            }
      if (out == 0) {
            out = new QFile;
            ((QFile*)out)->open(stdout, QIODevice::WriteOnly);
            }

      MidiFile mf;
      mf.read(in);
      in->close();

      if (mergeNotes)
            mf.process1();
      if (separateChannel)
            mf.separateChannel();
      if (division != -1)
            mf.changeDivision(division);

      if (smfOutput) {
            mf.setNoRunningStatus(noRunningStatus);
            mf.write(out);
            }
      else {
            Xml xml(out);

            xml.header();
            xml.stag("SMF");
            xml.tag("format", mf.format());
            xml.tag("division", mf.division());

            QList<MidiTrack*>* tl = mf.tracks();
            foreach(MidiTrack* t, *tl) {
                  if (cleanup)
                        t->cleanup();
                  xml.stag("Track");
                  foreach (const Event& e, t->events())
                        e.write(xml);
                  xml.etag();
                  }
            xml.etag();
            }
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

