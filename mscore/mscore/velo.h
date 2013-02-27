//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2009 Werner Schweer and others
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

#ifndef __VELO_H__
#define __VELO_H__

/**
 \file
 Definition of classes VeloList.
*/

//---------------------------------------------------------
//   VeloList
//---------------------------------------------------------

typedef std::map<const int, int>::iterator iVeloEvent;
typedef std::map<const int, int>::const_iterator ciVeloEvent;

/**
 List of note velocity changes
*/

class VeloList : public std::map<const int, int> {
   public:
      VeloList() {}
      int velo(int tick) const;
      void setVelo(int tick, int velo);
      };

#endif

