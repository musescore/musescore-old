//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
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

#include "chordedit.h"
#include "harmony.h"

//---------------------------------------------------------
//   ChordEdit
//---------------------------------------------------------

ChordEdit::ChordEdit(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      rootGroup = new QButtonGroup(this);
      rootGroup->addButton(rootC,   0);
      rootGroup->addButton(rootDb,  1);
      rootGroup->addButton(rootD,   2);
      rootGroup->addButton(rootEb,  3);
      rootGroup->addButton(rootE,   4);
      rootGroup->addButton(rootF,   5);
      rootGroup->addButton(rootFis, 6);
      rootGroup->addButton(rootG,   7);
      rootGroup->addButton(rootAb,  8);
      rootGroup->addButton(rootA,   9);
      rootGroup->addButton(rootBb, 10);
      rootGroup->addButton(rootB,  11);

      extensionGroup = new QButtonGroup(this);
      extensionGroup->addButton(extMaj,    2);
      extensionGroup->addButton(ext2,     15);
      extensionGroup->addButton(extMaj7,   6);
      extensionGroup->addButton(extMaj9,   7);
      extensionGroup->addButton(ext6,      5);
      extensionGroup->addButton(ext69,    14);

      extensionGroup->addButton(extm,     16);
      extensionGroup->addButton(extm7,    19);
      extensionGroup->addButton(extm6,    23);
      extensionGroup->addButton(extm9,    20);
      extensionGroup->addButton(extmMaj7, 18);
      extensionGroup->addButton(extm7b5,  32);
      extensionGroup->addButton(extdim,   33);
      extensionGroup->addButton(ext7,     64);
      extensionGroup->addButton(ext9,     70);
      extensionGroup->addButton(ext13,    65);
      extensionGroup->addButton(ext7b9,   76);
      extensionGroup->addButton(extsus,  184);
      extensionGroup->addButton(ext7Sus, 128);
      extensionGroup->addButton(extOther,  0);

      extOtherCombo->clear();
      for (int i = 0; i < 185; ++i) {           // HACK
            const char* p = Harmony::getExtensionName(i);
            if (p)
                  extOtherCombo->addItem(p, i);
            }
      connect(rootGroup, SIGNAL(buttonClicked(int)), SLOT(chordChanged()));
      connect(extensionGroup, SIGNAL(buttonClicked(int)), SLOT(chordChanged()));
      connect(extOtherCombo, SIGNAL(currentIndexChanged(int)), SLOT(chordChanged()));
      connect(bassNote, SIGNAL(currentIndexChanged(int)), SLOT(chordChanged()));
      connect(extOther, SIGNAL(toggled(bool)), SLOT(otherToggled(bool)));

      extOtherCombo->setEnabled(false);
      chordChanged();
      }

//---------------------------------------------------------
//   setRoot
//---------------------------------------------------------

void ChordEdit::setRoot(int val)
      {
      QAbstractButton* button = rootGroup->button(val);
      if (button)
            button->setChecked(true);
      else
            printf("root button %d not found\n", val);
      chordChanged();
      }

//---------------------------------------------------------
//   setExtension
//---------------------------------------------------------

void ChordEdit::setExtension(int val)
      {
      QAbstractButton* button = extensionGroup->button(val);
      if (button)
            button->setChecked(true);
      else {
            extOther->setChecked(true);
            int idx = extOtherCombo->findData(val);
            if (idx != -1)
                  extOtherCombo->setCurrentIndex(idx);
            }
      chordChanged();
      }

//---------------------------------------------------------
//   setBase
//---------------------------------------------------------

void ChordEdit::setBase(int val)
      {
      bassNote->setCurrentIndex(val);
      chordChanged();
      }

//---------------------------------------------------------
//   extension
//---------------------------------------------------------

int ChordEdit::extension()
      {
      int id = extensionGroup->checkedId();
      if (id == -1)
            return 0;
      else if (id == 0) {
            int idx = extOtherCombo->currentIndex();
            return extOtherCombo->itemData(idx).toInt();
            }
      else
            return id;
      }

//---------------------------------------------------------
//   root
//---------------------------------------------------------

int ChordEdit::root()
      {
      return rootGroup->checkedId();
      }

//---------------------------------------------------------
//   base
//---------------------------------------------------------

int ChordEdit::base()
      {
      return bassNote->currentIndex();
      }

//---------------------------------------------------------
//   otherToggled
//---------------------------------------------------------

void ChordEdit::otherToggled(bool val)
      {
      extOtherCombo->setEnabled(val);
      }

//---------------------------------------------------------
//   chordChanged
//---------------------------------------------------------

void ChordEdit::chordChanged()
      {
      QString s = Harmony::harmonyName(root(), extension(), base());
      chordLabel->setText(s);
      }

