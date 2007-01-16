//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: textstyle.cpp,v 1.8 2006/03/02 17:08:43 wschweer Exp $
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

#include "style.h"
#include "textstyle.h"
#include "globals.h"

//---------------------------------------------------------
//   TextStyleDialog
//---------------------------------------------------------

TextStyleDialog::TextStyleDialog(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      styles = textStyles;
      QFontDatabase fdb;
      QStringList families = fdb.families();
      fonts = 0;
      for (QStringList::Iterator f = families.begin(); f != families.end(); ++f) {
            QString family = *f;
            if (fdb.isSmoothlyScalable(family)) {
                  fontName->addItem(family);
                  ++fonts;
                  }
            }
      textNames->setSelectionMode(QListWidget::SingleSelection);
      textNames->clear();
      for (iTextStyle i = styles.begin(); i != styles.end(); ++i) {
            TextStyle* s = &*i;
            textNames->addItem(s->name);
            }

      connect(textNames,     SIGNAL(currentRowChanged(int)), SLOT(nameSelected(int)));
      connect(buttonOk,      SIGNAL(clicked()), SLOT(ok()));
      connect(buttonApply,   SIGNAL(clicked()), SLOT(apply()));
      connect(fontBold,      SIGNAL(clicked()), SLOT(fontChanged()));
      connect(fontUnderline, SIGNAL(clicked()), SLOT(fontChanged()));
      connect(fontItalic,    SIGNAL(clicked()), SLOT(fontChanged()));
      connect(fontSize,      SIGNAL(valueChanged(int)), SLOT(fontSizeChanged(int)));
      connect(fontName,      SIGNAL(activated(int)), SLOT(fontNameChanged(int)));
      connect(leftH,         SIGNAL(clicked()), SLOT(alignLeftH()));
      connect(rightH,        SIGNAL(clicked()), SLOT(alignRightH()));
      connect(centerH,       SIGNAL(clicked()), SLOT(alignCenterH()));
      connect(topV,          SIGNAL(clicked()), SLOT(alignTopV()));
      connect(bottomV,       SIGNAL(clicked()), SLOT(alignBottomV()));
      connect(centerV,       SIGNAL(clicked()), SLOT(alignCenterV()));
      connect(unitMM,        SIGNAL(clicked()), SLOT(setUnitMM()));
      connect(unitPercent,   SIGNAL(clicked()), SLOT(setUnitPercent()));
      connect(unitSpace,     SIGNAL(clicked()), SLOT(setUnitSpace()));

      current = -1;
      textNames->setCurrentItem(textNames->item(0));
      }

void TextStyleDialog::alignLeftH()
      {
      TextStyle* s = &styles[current];
      s->align &= ~(ALIGN_LEFT | ALIGN_RIGHT | ALIGN_HCENTER);
      s->align |= ALIGN_LEFT;
      }

void TextStyleDialog::alignRightH()
      {
      TextStyle* s = &styles[current];
      s->align &= ~(ALIGN_LEFT | ALIGN_RIGHT | ALIGN_HCENTER);
      s->align |= ALIGN_RIGHT;
      }

void TextStyleDialog::alignCenterH()
      {
      TextStyle* s = &styles[current];
      s->align &= ~(ALIGN_LEFT | ALIGN_RIGHT | ALIGN_HCENTER);
      s->align |= ALIGN_HCENTER;
      }

void TextStyleDialog::alignTopV()
      {
      TextStyle* s = &styles[current];
      s->align &= ~(ALIGN_TOP | ALIGN_BOTTOM | ALIGN_VCENTER);
      s->align |= ALIGN_TOP;
      }

void TextStyleDialog::alignBottomV()
      {
      TextStyle* s = &styles[current];
      s->align &= ~(ALIGN_TOP | ALIGN_BOTTOM | ALIGN_VCENTER);
      s->align |= ALIGN_BOTTOM;
      }

void TextStyleDialog::alignCenterV()
      {
      TextStyle* s = &styles[current];
      s->align &= ~(ALIGN_TOP | ALIGN_BOTTOM | ALIGN_VCENTER);
      s->align |= ALIGN_VCENTER;
      }

//---------------------------------------------------------
//   setUnitMM
//---------------------------------------------------------

void TextStyleDialog::setUnitMM()
      {
      TextStyle* s = &styles[current];
      s->offsetType = OFFSET_ABS;
      }

//---------------------------------------------------------
//   setUnitPercent
//---------------------------------------------------------

void TextStyleDialog::setUnitPercent()
      {
      TextStyle* s = &styles[current];
      s->offsetType = OFFSET_REL;
      }

//---------------------------------------------------------
//   setUnitSpace
//---------------------------------------------------------

void TextStyleDialog::setUnitSpace()
      {
      TextStyle* s = &styles[current];
      s->offsetType = OFFSET_SPATIUM;
      }

//---------------------------------------------------------
//   nameSelected
//---------------------------------------------------------

void TextStyleDialog::nameSelected(int n)
      {
      if (current != -1)
            saveStyle(current);
      TextStyle* s = &styles[n];

      fontBold->setChecked(s->bold);
      fontItalic->setChecked(s->italic);
      fontUnderline->setChecked(s->underline);
      fontSize->setValue(s->size);

      if (s->align & ALIGN_RIGHT)
            rightH->setChecked(true);
      else if (s->align & ALIGN_HCENTER)
            centerH->setChecked(true);
      else
            leftH->setChecked(true);
      if (s->align & ALIGN_BOTTOM)
            bottomV->setChecked(true);
      else if (s->align & ALIGN_VCENTER)
            centerV->setChecked(true);
      else
            topV->setChecked(true);
      referencePos->setCurrentIndex(s->anchor);

      QString str;
      if (s->offsetType == OFFSET_ABS) {
            xOffset->setValue(s->xoff/DPMM);
            yOffset->setValue(s->yoff/DPMM);
            unitMM->setChecked(true);
            }
      if (s->offsetType == OFFSET_REL) {
            xOffset->setValue(s->xoff);
            yOffset->setValue(s->yoff);
            unitPercent->setChecked(true);
            }
      else if (s->offsetType == OFFSET_SPATIUM) {
            xOffset->setValue(s->xoff);
            yOffset->setValue(s->yoff);
            unitSpace->setChecked(true);
            }

      QFont f(s->family);
      f.setPointSizeF(s->size);
      f.setItalic(s->italic);
      f.setUnderline(s->underline);
      f.setBold(s->bold);
      fontSample->setFont(f);
      fontSample->setText(tr("Ich und du, Müllers Kuh"));
      int i;
      for (i = 0; i < fonts; ++i) {
            QString ls = fontName->itemText(i);
            if (ls.toLower() == s->family.toLower()) {
                  fontName->setCurrentIndex(i);
                  break;
                  }
            }
      if (i == fonts) {
            printf("font not in list: <%s>\n", s->family.toLower().toLatin1().data());
            }
      current = n;
      }

//---------------------------------------------------------
//   fontChanged
//---------------------------------------------------------

void TextStyleDialog::fontChanged()
      {
      QFont f(fontSample->font());
      f.setItalic(fontItalic->isChecked());
      f.setUnderline(fontUnderline->isChecked());
      f.setBold(fontBold->isChecked());
      fontSample->setFont(f);
      }

//---------------------------------------------------------
//   fontSizeChanged
//---------------------------------------------------------

void TextStyleDialog::fontSizeChanged(int n)
      {
      QFont f(fontSample->font());
      f.setPointSizeF(n);
      fontSample->setFont(f);
      }

//---------------------------------------------------------
//   fontNameChanged
//---------------------------------------------------------

void TextStyleDialog::fontNameChanged(int)
      {
      QFont f(fontSample->font());
      f.setFamily(fontName->currentText());
      fontSample->setFont(f);
      }

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

void TextStyleDialog::saveStyle(int n)
      {
      TextStyle* s = &styles[n];
      s->bold      = fontBold->isChecked();
      s->italic    = fontItalic->isChecked();
      s->underline = fontUnderline->isChecked();
      s->size      = fontSize->value();
      s->anchor    = (Anchor)(referencePos->currentIndex());
      s->family    = strdup(fontName->currentText().toLatin1().data());  // memory leak

      bool ok;
      double val = xOffset->text().toDouble(&ok);
      if (ok)
            s->xoff = val * ((s->offsetType == OFFSET_ABS) ? DPMM : 1.0);
      else
            printf("bad xoff float value\n");

      val = yOffset->text().toDouble(&ok);
      if (ok)
            s->yoff = val * ((s->offsetType == OFFSET_ABS) ? DPMM : 1.0);
      else
            printf("bad yoff float value\n");
      }

//---------------------------------------------------------
//   ok
//---------------------------------------------------------

void TextStyleDialog::ok()
      {
      apply();
      done(0);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void TextStyleDialog::apply()
      {
      saveStyle(current);
      textStyles = styles;
      emit textStyleChanged();
      }

