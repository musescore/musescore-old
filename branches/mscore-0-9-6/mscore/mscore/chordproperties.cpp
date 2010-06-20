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
      int headGroup = note->headGroup();
      int headGroupIndex = 0;
      for (int i = 0; i < HEAD_GROUPS; ++i) {
            noteHeadGroup->setItemData(i, QVariant(heads[i]));
            if(headGroup == heads[i]) {
                  headGroupIndex = i;
                  }
            }

      noteHeadGroup->setCurrentIndex(headGroupIndex);
      noteHeadType->setCurrentIndex(int(note->headType()));

      ValueType vt  = note->veloType();
      _velo         = note->velocity();
      _veloOffset   = note->veloOffset();
      _veloType->setCurrentIndex(int(vt));
      veloTypeChanged(vt);

      vt                = note->onTimeType();
      _ontimeOffset     = note->onTimeOffset();
      _ontimeUserOffset = note->onTimeUserOffset();
      _ontimeOffsetType->setCurrentIndex(int(vt));
      ontimeOffsetTypeChanged(vt);

      vt                 = note->offTimeType();
      _offtimeOffset     = note->offTimeOffset();
      _offtimeUserOffset = note->offTimeUserOffset();
      _offtimeOffsetType->setCurrentIndex(int(vt));
      offtimeOffsetTypeChanged(vt);

      connect(_veloType,          SIGNAL(activated(int)),    SLOT(veloTypeChanged(int)));
      connect(_ontimeOffsetType,  SIGNAL(activated(int)),    SLOT(ontimeOffsetTypeChanged(int)));
      connect(_offtimeOffsetType, SIGNAL(activated(int)),    SLOT(offtimeOffsetTypeChanged(int)));
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
      switch(_ontimeOffsetType->currentIndex()) {
            case AUTO_VAL:
            case USER_VAL:   _ontimeOffset = val; break;
            case OFFSET_VAL: _ontimeUserOffset = val; break;
            }
      }

//---------------------------------------------------------
//   ontimeOffsetTypeChanged
//---------------------------------------------------------

void ChordProperties::ontimeOffsetTypeChanged(int vt)
      {
      switch(vt) {
            case AUTO_VAL:
                  ontimeOffset->setReadOnly(true);
                  ontimeOffset->setSuffix("");
                  ontimeOffset->setRange(0, 127);
                  ontimeOffset->setValue(_ontimeOffset);
                  break;
            case USER_VAL:
                  ontimeOffset->setReadOnly(false);
                  ontimeOffset->setSuffix("");
                  ontimeOffset->setRange(-1000, 1000);
                  ontimeOffset->setValue(_ontimeOffset);
                  break;
            case OFFSET_VAL:
                  ontimeOffset->setReadOnly(false);
                  ontimeOffset->setSuffix("%");
                  ontimeOffset->setRange(-200, 200);
                  ontimeOffset->setValue(_ontimeUserOffset);
                  break;
            }
      }

//---------------------------------------------------------
//   offtimeOffsetChanged
//---------------------------------------------------------

void ChordProperties::offtimeOffsetChanged(int val)
      {
      switch(_offtimeOffsetType->currentIndex()) {
            case AUTO_VAL:
            case USER_VAL:   _offtimeOffset = val; break;
            case OFFSET_VAL: _offtimeUserOffset = val;   break;
            }
      }

//---------------------------------------------------------
//   offtimeOffsetTypeChanged
//---------------------------------------------------------

void ChordProperties::offtimeOffsetTypeChanged(int vt)
      {
      switch(vt) {
            case AUTO_VAL:
                  offtimeOffset->setReadOnly(true);
                  offtimeOffset->setSuffix("");
                  offtimeOffset->setRange(0, 127);
                  offtimeOffset->setValue(_offtimeOffset);
                  break;
            case USER_VAL:
                  offtimeOffset->setReadOnly(false);
                  offtimeOffset->setSuffix("");
                  offtimeOffset->setRange(-1000, +1000);
                  offtimeOffset->setValue(_offtimeOffset);
                  break;
            case OFFSET_VAL:
                  offtimeOffset->setReadOnly(false);
                  offtimeOffset->setSuffix("%");
                  offtimeOffset->setRange(-200, 200);
                  offtimeOffset->setValue(_ontimeUserOffset);
                  break;
            }
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

