//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __CHORDPROPERTIES_H__
#define __CHORDPROPERTIES_H__

#include "ui_chordproperties.h"
#include "libmscore/mscore.h"

class Note;

//---------------------------------------------------------
//   ChordProperties
//---------------------------------------------------------

class ChordProperties : public QDialog, Ui::ChordPropertyBase {
      Q_OBJECT

      int _velo;
      int _userVelocity;
      int _veloOffset;

      int _ontimeUserOffset;
      int _offtimeUserOffset;

   private slots:
      void veloTypeChanged(int);
      void velocityChanged(int);
      void ontimeOffsetChanged(int);
      void offtimeOffsetChanged(int);

   public:
      ChordProperties(const Note* c, QWidget* parent = 0);
      bool small() const;
      bool noStem() const;
      double tuning() const;
      int getUserMirror() const;
      int getStemDirection() const;

      int getHeadGroup() const;
      NoteHeadType getHeadType() const { return (NoteHeadType)noteHeadType->currentIndex(); }

      ValueType veloType() const    { return ValueType(_veloType->currentIndex()); }
      int velo() const              { return _velo;                       }
      int veloOffset() const        { return _veloOffset;                 }

      int onTimeUserOffset() const  { return _ontimeUserOffset;  }
      int offTimeUserOffset() const { return _offtimeUserOffset; }
      };

#endif

