//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
//
//  This file is in part based on code from KGuitar, a KDE tabulature editor
//     * copyright (C) 2002-2003 the KGuitar development team
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

#ifndef __CHORDINFO_H__
#define __CHORDINFO_H__

//---------------------------------------------------------
//   ChordInfo
//---------------------------------------------------------

class ChordInfo {
      int _tonic, _bass;
      int s[6];

      QString _name;

   public:
      ChordInfo() {}
      ChordInfo(int t, int b, int s3, int s5, int s7, int s9, int s11, int s13);
      QString name() const { return _name; }
      };

#endif

