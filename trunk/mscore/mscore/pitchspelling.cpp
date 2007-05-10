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

#include "analyse.h"

//
// "line of fifth's"
//    LOF
//

// table of alternative spellings for one octave
// each entry is an index in the LOF
//    tab1 does not contain double sharps
//    tab2 does not contain double flats

// int table[] = {
//       bb  b   -   #  ##
//       -   6, 13, 20, 27,  // F
//       0,  7, 14, 21, 28,  // C
//       1,  8, 15, 22, 39,  // G
//       2,  9, 16, 23, 30,  // D
//       3, 10, 17, 24, 31,  // A
//       4, 11, 18, 25, 32,  // E
//       5, 12, 19, 26, 33,  // B


static const int tab1[24] = {
      14,  2,  // 60  C   Dbb
      21,  9,  // 61  C#  Db
      16,  4,  // 62  D   Ebb
      23, 11,  // 63  D#  Eb
      18, 13,  // 64  E   Fb
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

static const bool enharmonicSpelling[35] = {
  //  f  c  g  d  a  e  b
      1, 1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      };

//---------------------------------------------------------
//   penalty
//---------------------------------------------------------

static int penalty(int lof1, int lof2)
      {
      int penalty  = enharmonicSpelling[lof1] * 4 + enharmonicSpelling[lof2] * 4;
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

static int computeWindow(const QList<SNote>& notes, int start, int end)
      {
      int p   = 10000;
      int idx = -1;
      int pitch[10];

      int i = start;
      int k = 0;
      while (i < end)
            pitch[k++] = notes[i++].pitch % 12;

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
                  pa += penalty(lof1a, lof2a);
                  pb += penalty(lof1b, lof2b);
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

void spell(QList<SNote>& notes)
      {
      int n = notes.size();

      int start = 0;
      while (start < n) {
            int end = start + WINDOW;
            if (end > n)
                  end = n;
            int opt = computeWindow(notes, start, end);
            const int* tab;
            if (opt < 0) {
                  tab = tab2;
                  opt *= -1;
                  }
            else
                  tab = tab1;

            if (start == 0) {
                  notes[0].tpc = tab[(notes[0].pitch % 12) * 2 + (opt & 1)];
                  int idx = (notes[1].pitch % 12) * 2 + ((opt & 2) >> 1);
                  notes[1].tpc = tab[idx];
                  notes[2].tpc = tab[idx];
                  }
            if ((end - start) >= 6) {
                  notes[start+3].tpc = tab[(notes[start+3].pitch % 12) * 2 + ((opt &  8) >> 3)];
                  notes[start+4].tpc = tab[(notes[start+4].pitch % 12) * 2 + ((opt & 16) >> 4)];
                  notes[start+5].tpc = tab[(notes[start+5].pitch % 12) * 2 + ((opt & 32) >> 5)];
                  }
            if (end == n) {
                  switch(end - start - 6) {
                        case 3:
                              notes[end-3].tpc = tab[(notes[end-3].pitch % 12) * 2 + ((opt & 64) >> 6)];
                        case 2:
                              notes[end-2].tpc = tab[(notes[end-2].pitch % 12) * 2 + ((opt & 128) >> 7)];
                        case 1:
                              notes[end-1].tpc = tab[(notes[end-1].pitch % 12) * 2 + ((opt & 256) >> 8)];
                              break;
                        }
                  }
            // advance to next window
            start += WINDOW_SHIFT;
            }
      }

