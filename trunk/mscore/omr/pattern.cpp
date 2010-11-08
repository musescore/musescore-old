//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
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

#include "pattern.h"

//---------------------------------------------------------
//   patternMatch
//    compare two patterns for similarity
//    return
//          1.0   - identical
//          0.5   - 50% of all pixel match
//          0.0   - no match
//---------------------------------------------------------

double Pattern::match(Pattern* a) const
      {
      if (a->n != n)
            return 0.0;
      int k = 0;
      for (int i = 0; i < a->n; ++i) {
            int v = a->img[i] ^ b->img[i];
            k += bitsSetTable[v & 0xff]
               + bitsSetTable[(v >> 8) & 0xff]
               + bitsSetTable[(v >> 16) & 0xff]
               + bitsSetTable[v >> 24];
            }
      return 1.0 - (double(k) / (n * sizeof(int)));
      }

