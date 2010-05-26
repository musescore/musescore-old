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

#include "chordproperties.h"
#include "chord.h"
#include "note.h"

//---------------------------------------------------------
//   ChordProperties
//---------------------------------------------------------

ChordProperties::ChordProperties(const Note* note, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      const Chord* chord = note->chord();
      smallCheckBox->setChecked(chord->small());
      noStemCheckBox->setChecked(chord->noStem());
      extraLeadingSpace->setValue(chord->extraLeadingSpace().val());
      extraTrailingSpace->setValue(chord->extraTrailingSpace().val());
      tuningSpinBox->setValue(note->tuning());
      userMirror->setCurrentIndex(int(note->userMirror()));
      stemDirection->setCurrentIndex(int(chord->stemDirection()));

      //
      // fix order of note heads
      //
      static const int heads[] = {
            HEAD_NORMAL, HEAD_CROSS, HEAD_DIAMOND, HEAD_TRIANGLE,
            HEAD_SLASH, HEAD_XCIRCLE, HEAD_DO, HEAD_RE, HEAD_MI, HEAD_FA, HEAD_LA, HEAD_TI
            };
      for (int i = 0; i < HEAD_GROUPS; ++i)
            noteHeadGroup->setItemData(i, QVariant(heads[i]));

      noteHeadGroup->setCurrentIndex(note->headGroup());
      noteHeadType->setCurrentIndex(int(note->headType()));

      ValueType vt  = note->veloType();
      _velo         = note->velocity();
      _veloOffset   = note->veloOffset();
      _veloType->setCurrentIndex(int(vt));
      veloTypeChanged(vt);

      _ontimeUserOffset = note->onTimeUserOffset();
      _offtimeUserOffset = note->offTimeUserOffset();

      connect(_veloType,          SIGNAL(activated(int)),    SLOT(veloTypeChanged(int)));
      connect(velocity,           SIGNAL(valueChanged(int)), SLOT(velocityChanged(int)));
      connect(ontimeOffset,       SIGNAL(valueChanged(int)), SLOT(ontimeOffsetChanged(int)));
      connect(offtimeOffset,      SIGNAL(valueChanged(int)), SLOT(offtimeOffsetChanged(int)));
      }

//---------------------------------------------------------
//   getHeadGroup
//---------------------------------------------------------

int ChordProperties::getHeadGroup() const
      {
      return noteHeadGroup->itemData(noteHeadGroup->currentIndex()).toInt();
      }

//---------------------------------------------------------
//   velocityChanged
//---------------------------------------------------------

void ChordProperties::velocityChanged(int val)
      {
      switch(_veloType->currentIndex()) {
            case AUTO_VAL:
            case USER_VAL:   _velo = val; break;
            case OFFSET_VAL: _veloOffset = val; break;
            }
      }

//---------------------------------------------------------
//   veloTypeChanged
//---------------------------------------------------------

void ChordProperties::veloTypeChanged(int vt)
      {
      switch(vt) {
            case AUTO_VAL:
                  velocity->setReadOnly(true);
                  velocity->setSuffix("");
                  velocity->setRange(0, 127);
                  velocity->setValue(_velo);
                  break;
            case USER_VAL:
                  velocity->setReadOnly(false);
                  velocity->setSuffix("");
                  velocity->setRange(0, 127);
                  velocity->setValue(_velo);
                  break;
            case OFFSET_VAL:
                  velocity->setReadOnly(false);
                  velocity->setSuffix("%");
                  velocity->setRange(-200, 200);
                  velocity->setValue(_veloOffset);
                  break;
            }
      }

//---------------------------------------------------------
//   ontimeOffsetChanged
//---------------------------------------------------------

void ChordProperties::ontimeOffsetChanged(int val)
      {
      _ontimeUserOffset = val;
      }

//---------------------------------------------------------
//   offtimeOffsetChanged
//---------------------------------------------------------

void ChordProperties::offtimeOffsetChanged(int val)
      {
      _offtimeUserOffset = val;
      }

//---------------------------------------------------------
//   small
//---------------------------------------------------------

bool ChordProperties::small() const
      {
      return smallCheckBox->isChecked();
      }

//---------------------------------------------------------
//   getUserMirror
//---------------------------------------------------------

int ChordProperties::getUserMirror() const
      {
      return userMirror->currentIndex();
      }

//---------------------------------------------------------
//   noStem
//---------------------------------------------------------

bool ChordProperties::noStem() const
      {
      return noStemCheckBox->isChecked();
      }

//---------------------------------------------------------
//   getStemDirection
//---------------------------------------------------------

int ChordProperties::getStemDirection() const
      {
      return stemDirection->currentIndex();
      }

//---------------------------------------------------------
//   leadingSpace
//---------------------------------------------------------

double ChordProperties::leadingSpace() const
      {
      return extraLeadingSpace->value();
      }

//---------------------------------------------------------
//   trailingSpace
//---------------------------------------------------------

double ChordProperties::trailingSpace() const
      {
      return extraTrailingSpace->value();
      }

//---------------------------------------------------------
//   tuning
//---------------------------------------------------------

double ChordProperties::tuning() const
      {
      return tuningSpinBox->value();
      }

