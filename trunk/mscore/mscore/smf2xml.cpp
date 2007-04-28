//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: mscore.cpp,v 1.105 2006/09/15 09:34:57 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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
bool debugMode  = false;
bool mergeNotes = false;

//---------------------------------------------------------
//   convert
//---------------------------------------------------------

static void convert(QIODevice* in, QIODevice* out)
      {
      Xml xml(out);

      MidiFile mf;
      mf.read(in);
      if (mergeNotes)
            mf.process1();

      xml.header();
      xml.stag("SMF");
      xml.tag("format", mf.format());

      MidiTrackList* tl = mf.tracks();
      foreach(MidiTrack* t, *tl) {
            xml.stag("Track");
            foreach (const MidiEvent* e, t->events()) {
                  switch(e->type) {
                        case ME_NOTEOFF:
                              xml.tagE(QString("NoteOff tick=\"%1\" channel=\"%2\" pitch=\"%3\" velo=\"%4\"")
                                 .arg(e->tick).arg(e->channel).arg(e->pitch()).arg(e->velo()));
                              break;
                        case ME_NOTEON:
                              if (e->len) {
                                    xml.tagE(QString("Note  tick=\"%1\" channel=\"%2\" len=\"%3\" pitch=\"%4\" velo=\"%5\"")
                                       .arg(e->tick).arg(e->channel).arg(e->len).arg(e->pitch()).arg(e->velo()));
                                    }
                              else {
                                    xml.tagE(QString("NoteOn  tick=\"%1\" channel=\"%2\" pitch=\"%3\" velo=\"%4\"")
                                       .arg(e->tick).arg(e->channel).arg(e->pitch()).arg(e->velo()));
                                    }
                              break;
                        case ME_META:
                              switch(e->dataA) {
                                    case META_TRACK_NAME:
                                          xml.tag("TrackName",  QString("tick=\"%1\"").arg(e->tick), QString((char*)(e->data)));
                                          break;
                                    case META_LYRIC:
                                          xml.tag("Lyric",  QString("tick=\"%1\"").arg(e->tick), QString((char*)(e->data)));
                                          break;
                                    case META_KEY_SIGNATURE:
                                          {
                                          const char* keyTable[] = {
                                                "Ges", "Des", "As", "Es", "B", "F",
                                                "C",
                                                "G", "D", "A", "E", "H", "Fis"
                                                };
                                          int key = e->data[0] + 7;
                                          if (key < 0 || key > 14) {
                                                printf("bad key signature\n");
                                                key = 0;
                                                }
                                          QString sex(e->data[1] ? "Minor" : "Major");
                                          QString keyName(keyTable[key]);
                                          xml.tag("Key",  QString("tick=\"%1\"").arg(e->tick), QString("%1 %2").arg(keyName).arg(sex));
                                          }
                                          break;
                                    case META_TIME_SIGNATURE:
                                          xml.tagE(QString("TimeSig tick=\"%1\" num=\"%2\" denom=\"%3\" metro=\"%4\" quarter=\"%5\"")
                                             .arg(e->tick)
                                             .arg(int(e->data[0]))
                                             .arg(int(e->data[1]))
                                             .arg(int(e->data[2]))
                                             .arg(int(e->data[3])));
                                          break;

                                    case META_TEMPO:
                                    default:
                                          xml.tagE(QString("Meta tick=\"%1\" type=\"%2\"").arg(e->tick).arg(int(e->dataA)));
                                          break;
                                    }
                              break;
                        case ME_POLYAFTER:
                        case ME_CONTROLLER:
                        case ME_PROGRAM:
                        case ME_AFTERTOUCH:
                        case ME_PITCHBEND:
                        case ME_SYSEX:
                        default:
                              xml.tagE(QString("Event tick=\"%1\" type=\"%2\"").arg(e->tick).arg(e->type));
                              break;
                        }
                  }
            xml.etag();
            }
      xml.etag();
      }

//---------------------------------------------------------
//   printVersion
//---------------------------------------------------------

static void printVersion()
      {
      printf("This is smf2xml version %s\n", versionString);
      }

//---------------------------------------------------------
//   usage
//---------------------------------------------------------

static void usage()
      {
      printVersion();
      printf("Usage: smf2xml [args] [infile] [outfile]\n");
      printf("   args:\n"
             "      -v   print version\n"
             "      -d   debug mode\n"
             "      -n   merge note on/off events\n"
            );
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      int c;
      while ((c = getopt(argc, argv, "vdn")) != EOF) {
            switch (c) {
                  case 'v':
                        printVersion();
                        return 0;
                  case 'd':
                        debugMode = true;
                        break;
                  case 'n':
                        mergeNotes = true;
                        break;
                  default:
                        usage();
                        return -1;
                  }
            }
      for (int i = 0; i < argc; ++i)
            printf("arg %d <%s>\n", i, argv[i]);
      printf("argc %d  optind %d\n", argc, optind);

      QIODevice* in = 0;
      QIODevice* out = 0;

      switch (argc - optind) {
            case 2:
                  out = new QFile(argv[1 + optind]);
                  if (!out->open(QIODevice::WriteOnly)) {
                        printf("cannot open output file <%s>: %s\n", argv[2], strerror(errno));
                        return -3;
                        }
            case 1:
                  in = new QFile(argv[0 + optind]);
                  if (!in->open(QIODevice::ReadOnly)) {
                        printf("cannot open input file <%s>: %s\n", argv[1], strerror(errno));
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
      convert(in, out);
      out->close();
      in->close();
      delete out;
      delete in;
      return 0;
      }

