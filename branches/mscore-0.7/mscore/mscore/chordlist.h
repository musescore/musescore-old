//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: chordlist.h,v 1.3 2006/03/02 17:08:33 wschweer Exp $
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

#ifndef __CHORDLIST_H__
#define __CHORDLIST_H__

class ChordRest;

//---------------------------------------------------------
//   ChordRestList
//---------------------------------------------------------

class ChordRestList : public std::multimap <const int, ChordRest*, std::less<int> > {
   public:
      void add(ChordRest* n);
      const ChordRest* front() const { return (begin() != end()) ? begin()->second : 0;  }
      const ChordRest* back()  const { return (begin() != end()) ? rbegin()->second : 0; }
      };

typedef ChordRestList::iterator iChordRest;
typedef ChordRestList::reverse_iterator riChordRest;
typedef ChordRestList::const_iterator ciChordRest;

#endif

