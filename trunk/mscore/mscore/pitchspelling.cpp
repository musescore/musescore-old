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

//  This file contains the implementation of an pitch spelling
//  algorithmus from Emilios Cambouropoulos as published in:
//  "Automatic Pitch Spelling: From Numbers to Sharps and Flats"

#include "midifile.h"
#include "score.h"
#include "layout.h"
#include "globals.h"
#include "chord.h"
#include "note.h"
#include "staff.h"
#include "key.h"

//    TPC - tonal pitch classes
//    "line of fifth's" LOF
//
//       bb  b   -   #  ##
//       -   6, 13, 20, 27,  // F
//       0,  7, 14, 21, 28,  // C
//       1,  8, 15, 22, 39,  // G
//       2,  9, 16, 23, 30,  // D
//       3, 10, 17, 24, 31,  // A
//       4, 11, 18, 25, 32,  // E
//       5, 12, 19, 26, 33,  // B


// table of alternative spellings for one octave
// each entry is the TPC of the note
//    tab1 does not contain double sharps
//    tab2 does not contain double flats

static const int tab1[24] = {
      14,  2,  // 60  C   Dbb
      21,  9,  // 61  C#  Db
      16,  4,  // 62  D   Ebb
      23, 11,  // 63  D#  Eb
      18,  6,  // 64  E   Fb
      13,  1,  // 65  F   Gbb
      20,  8,  // 66  F#  Gb
      15,  3,  // 67  G   Abb
      22, 10,  // 68  G#  Ab
      17,  5,  // 69  A   Bbb
      24, 12,  // 70  A#  Bb
      19,  7,  // 71  B   Cb
      };

static const int tab2[24] = {
      26, 14,  // 60  B#  C
      21,  9,  // 61  C#  Db
      28, 16,  // 62  C## D
      23, 11,  // 63  D#  Eb
      30, 18,  // 64  D## E
      25, 13,  // 65  E#  F
      20,  8,  // 66  F#  Gb
      27, 15,  // 67  F## G
      22, 10,  // 68  G#  Ab
      39, 17,  // 69  G## A
      24, 12,  // 70  A#  Bb
      31,  7,  // 71  A## Cb
      };

// intervall classes ABCD - 0123 for LOF distances
// int intervalClass[12] = {
//      0, 1, 1, 1, 1, 2, 3, 2, 2, 2, 3, 3
//      };

int intervalPenalty[13] = {
      0, 0, 0, 0, 0, 0, 1, 3, 1, 1, 1, 3, 3
      };

//       bb  b   -   #  ##
//       -   6, 13, 20, 27,  // F
//       0,  7, 14, 21, 28,  // C
//       1,  8, 15, 22, 39,  // G
//       2,  9, 16, 23, 30,  // D
//       3, 10, 17, 24, 31,  // A
//       4, 11, 18, 25, 32,  // E
//       5, 12, 19, 26, 33,  // B

//---------------------------------------------------------
//   enharmonicSpelling
//    TODO: fill tables with more sensible data
//---------------------------------------------------------

static const bool enharmonicSpelling[15][34] = {
      {
  //  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
  //  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
  //  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
  //  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
  //  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//Bb  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 1, 1,
      0, 0, 0, 1, 1, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//F   f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//C   f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
  //  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
  //  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
  //  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
  //  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
  //  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
  //  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
  //  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      }
      };

//---------------------------------------------------------
//   penalty
//---------------------------------------------------------

static int penalty(int lof1, int lof2, int k)
      {
      int penalty  = enharmonicSpelling[k][lof1] * 4 + enharmonicSpelling[k][lof2] * 4;
      int distance = lof2 > lof1 ? lof2 - lof1 : lof1 - lof2;
      if (distance > 12)
            penalty += 3;
      else
            penalty += intervalPenalty[distance];
      return penalty;
      }

static const int WINDOW       = 9;
static const int WINDOW_SHIFT = 3;
static const int ASIZE        = 1024;   // 2 ** WINDOW

//---------------------------------------------------------
//   computeWindow
//---------------------------------------------------------

static int computeWindow(const QList<MidiNote*>& notes, int start, int end, int keyIdx)
      {
      int p   = 10000;
      int idx = -1;
      int pitch[10];

      int i = start;
      int k = 0;
      while (i < end)
            pitch[k++] = notes[i++]->pitch() % 12;

      for (; k < 10; ++k)
            pitch[k] = pitch[k-1];

      for (int i = 0; i < 512; ++i) {
            int pa    = 0;
            int pb    = 0;
            int l     = pitch[0] * 2 + (i & 1);
            int lof1a = tab1[l];
            int lof1b = tab2[l];

            for (int k = 1; k < 10; ++k) {
                  int l = pitch[k] * 2 + ((i & (1 << k)) >> k);
                  int lof2a = tab1[l];
                  int lof2b = tab2[l];
                  pa += penalty(lof1a, lof2a, keyIdx);
                  pb += penalty(lof1b, lof2b, keyIdx);
                  lof1a = lof2a;
                  lof1b = lof2b;
                  }
            if (pa < pb) {
                  if (pa < p) {
                        p   = pa;
                        idx = i;
                        }
                  }
            else {
                  if (pb < p) {
                        p   = pb;
                        idx = i * -1;
                        }
                  }
            }
      return idx;
      }

//---------------------------------------------------------
//   computeWindow
//---------------------------------------------------------

static int computeWindow(const QList<Note*>& notes, int start, int end, int keyIdx)
      {
      int p   = 10000;
      int idx = -1;
      int pitch[10];

      int i = start;
      int k = 0;
      while (i < end)
            pitch[k++] = notes[i++]->pitch() % 12;

      for (; k < 10; ++k)
            pitch[k] = pitch[k-1];

      for (int i = 0; i < 512; ++i) {
            int pa    = 0;
            int pb    = 0;
            int l     = pitch[0] * 2 + (i & 1);
            int lof1a = tab1[l];
            int lof1b = tab2[l];

            for (int k = 1; k < 10; ++k) {
                  int l = pitch[k] * 2 + ((i & (1 << k)) >> k);
                  int lof2a = tab1[l];
                  int lof2b = tab2[l];
                  pa += penalty(lof1a, lof2a, keyIdx);
                  pb += penalty(lof1b, lof2b, keyIdx);
                  lof1a = lof2a;
                  lof1b = lof2b;
                  }
            if (pa < pb) {
                  if (pa < p) {
                        p   = pa;
                        idx = i;
                        }
                  }
            else {
                  if (pb < p) {
                        p   = pb;
                        idx = i * -1;
                        }
                  }
            }
      return idx;
      }

//---------------------------------------------------------
//   spell
//---------------------------------------------------------

void spell(QList<MidiNote*>& notes, int key)
      {
      int n = notes.size();

      int start = 0;
      while (start < n) {
            int end = start + WINDOW;
            if (end > n)
                  end = n;
            int opt = computeWindow(notes, start, end, key + 7);
            const int* tab;
            if (opt < 0) {
                  tab = tab2;
                  opt *= -1;
                  }
            else
                  tab = tab1;

            if (start == 0) {
                  notes[0]->setTpc(tab[(notes[0]->pitch() % 12) * 2 + (opt & 1)]);
                  notes[1]->setTpc(tab[(notes[1]->pitch() % 12) * 2 + ((opt & 2)>>1)]);
                  notes[2]->setTpc(tab[(notes[2]->pitch() % 12) * 2 + ((opt & 4)>>2)]);
                  }
            if ((end - start) >= 6) {
                  notes[start+3]->setTpc(tab[(notes[start+3]->pitch() % 12) * 2 + ((opt &  8) >> 3)]);
                  notes[start+4]->setTpc(tab[(notes[start+4]->pitch() % 12) * 2 + ((opt & 16) >> 4)]);
                  notes[start+5]->setTpc(tab[(notes[start+5]->pitch() % 12) * 2 + ((opt & 32) >> 5)]);
                  }
            if (end == n) {
                  int n = end - start;
                  int k;
                  switch(n - 6) {
                        case 3:
                              k = end - start - 3;
                              notes[end-3]->setTpc(tab[(notes[end-3]->pitch() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        case 2:
                              k = end - start - 2;
                              notes[end-2]->setTpc(tab[(notes[end-2]->pitch() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        case 1:
                              k = end - start - 1;
                              notes[end-1]->setTpc(tab[(notes[end-1]->pitch() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        }
                  break;
                  }
            // advance to next window
            start += 3;
            }
      }

//---------------------------------------------------------
//   spell
//---------------------------------------------------------

void spell(QList<Note*> notes, int key)
      {
      int n = notes.size();

      int start = 0;
      while (start < n) {
            int end = start + WINDOW;
            if (end > n)
                  end = n;
            int opt = computeWindow(notes, start, end, key + 7);
            const int* tab;
            if (opt < 0) {
                  tab = tab2;
                  opt *= -1;
                  }
            else
                  tab = tab1;

            if (start == 0) {
                  notes[0]->setTpc(tab[(notes[0]->pitch() % 12) * 2 + (opt & 1)]);
                  notes[1]->setTpc(tab[(notes[1]->pitch() % 12) * 2 + ((opt & 2)>>1)]);
                  notes[2]->setTpc(tab[(notes[2]->pitch() % 12) * 2 + ((opt & 4)>>2)]);
                  }
            if ((end - start) >= 6) {
                  notes[start+3]->setTpc(tab[(notes[start+3]->pitch() % 12) * 2 + ((opt &  8) >> 3)]);
                  notes[start+4]->setTpc(tab[(notes[start+4]->pitch() % 12) * 2 + ((opt & 16) >> 4)]);
                  notes[start+5]->setTpc(tab[(notes[start+5]->pitch() % 12) * 2 + ((opt & 32) >> 5)]);
                  }
            if (end == n) {
                  int n = end - start;
                  int k;
                  switch(n - 6) {
                        case 3:
                              k = end - start - 3;
                              notes[end-3]->setTpc(tab[(notes[end-3]->pitch() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        case 2:
                              k = end - start - 2;
                              notes[end-2]->setTpc(tab[(notes[end-2]->pitch() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        case 1:
                              k = end - start - 1;
                              notes[end-1]->setTpc(tab[(notes[end-1]->pitch() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        }
                  break;
                  }
            // advance to next window
            start += 3;
            }
      }

//---------------------------------------------------------
//   spell
//---------------------------------------------------------

void Score::spell()
      {
      for (int i = 0; i < nstaves(); ++i) {
            QList<Note*> notes;
            int key = staff(i)->keymap()->key(0);
            for(Measure* m = _layout->first(); m; m = m->next()) {
                  for (Segment* s = m->first(); s; s = s->next()) {
                        int strack = i * VOICES;
                        int etrack = strack + VOICES;
                        for (int track = strack; track < etrack; ++track) {
                              Element* e = s->element(track);
                              if (e && e->type() == CHORD) {
                                    Chord* chord = (Chord*) e;
                                    const NoteList* nl = chord->noteList();
                                    for (ciNote in = nl->begin(); in != nl->end(); ++in) {
                                          Note* note = in->second;
                                          notes.append(note);
                                          }
                                    }
                              }
                        }
                  }
            ::spell(notes, key);
            }
      }

