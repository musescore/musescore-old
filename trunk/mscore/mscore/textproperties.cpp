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

#include "textproperties.h"
#include "text.h"
#include "score.h"

//---------------------------------------------------------
//   TextProp
//---------------------------------------------------------

TextProp::TextProp(bool os, Score* score, QWidget* parent)
   : QWidget(parent)
      {
      onlyStyle = os;
      setupUi(this);

      if (onlyStyle) {
            styledGroup->setVisible(false);
            unstyledGroup->setCheckable(false);
            unstyledGroup->setTitle(tr("Text Style"));
            }
      else {
            styles->clear();
            foreach(const TextStyle& st, score->style().textStyles())
                  styles->addItem(st.name());
            }

      QButtonGroup* g1 = new QButtonGroup(this);
      g1->addButton(alignLeft);
      g1->addButton(alignHCenter);
      g1->addButton(alignRight);

      QButtonGroup* g2 = new QButtonGroup(this);
      g2->addButton(alignTop);
      g2->addButton(alignVCenter);
      g2->addButton(alignBottom);

      QButtonGroup* g3 = new QButtonGroup(this);
      g3->addButton(circleButton);
      g3->addButton(boxButton);

      connect(mmUnit, SIGNAL(toggled(bool)), SLOT(mmToggled(bool)));
      connect(styledGroup, SIGNAL(toggled(bool)), SLOT(styledToggled(bool)));
      connect(unstyledGroup, SIGNAL(toggled(bool)), SLOT(unstyledToggled(bool)));
      }

//---------------------------------------------------------
//   mmToggled
//---------------------------------------------------------

void TextProp::mmToggled(bool val)
      {
      QString unit(val ? tr("mm", "millimeter unit") : tr("sp", "spatium unit"));
      xOffset->setSuffix(unit);
      yOffset->setSuffix(unit);
      }

//---------------------------------------------------------
//   setStyled
//---------------------------------------------------------

void TextProp::setStyled(bool val)
      {
      styledGroup->setChecked(val);
      unstyledGroup->setChecked(!val);
      }

//---------------------------------------------------------
//   setTextStyleType
//---------------------------------------------------------

void TextProp::setTextStyleType(TextStyleType st)
      {
      if (st == TEXT_STYLE_INVALID)
            st = TEXT_STYLE_TITLE;
      styles->setCurrentIndex(st);
      }

//---------------------------------------------------------
//   textStyleType
//---------------------------------------------------------

TextStyleType TextProp::textStyleType() const
      {
      return TextStyleType(styles->currentIndex());
      }

//---------------------------------------------------------
//   isStyled
//---------------------------------------------------------

bool TextProp::isStyled() const
      {
      return styledGroup->isChecked();
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void TextProp::setTextStyle(const TextStyle& s)
      {
      fontBold->setChecked(s.bold());
      fontItalic->setChecked(s.italic());
      fontUnderline->setChecked(s.underline());
      fontSize->setValue(s.size());
      color->setColor(s.foregroundColor());

      systemFlag->setChecked(s.systemFlag());
      int a = s.align();
      if (a & ALIGN_HCENTER)
            alignHCenter->setChecked(true);
      else if (a & ALIGN_RIGHT)
            alignRight->setChecked(true);
      else
            alignLeft->setChecked(true);

      if (a & ALIGN_VCENTER)
            alignVCenter->setChecked(true);
      else if (a & ALIGN_BOTTOM)
            alignBottom->setChecked(true);
      else if (a & ALIGN_BASELINE)
            alignBaseline->setChecked(true);
      else
            alignTop->setChecked(true);

      QString str;
      if (s.offsetType() == OFFSET_ABS) {
            xOffset->setValue(s.xoff() * INCH);
            yOffset->setValue(s.yoff() * INCH);
            mmUnit->setChecked(true);
            curUnit = 0;
            }
      else if (s.offsetType() == OFFSET_SPATIUM) {
            xOffset->setValue(s.xoff());
            yOffset->setValue(s.yoff());
            spatiumUnit->setChecked(true);
            curUnit = 1;
            }
      rxOffset->setValue(s.rxoff());
      ryOffset->setValue(s.ryoff());

      QFont f(s.family());
      f.setPixelSize(lrint(s.size()));
      f.setItalic(s.italic());
      f.setUnderline(s.underline());
      f.setBold(s.bold());
      fontSelect->setCurrentFont(f);

      frameColor->setColor(s.frameColor());
      frameWidth->setValue(s.frameWidth());
      frame->setChecked(s.hasFrame());
      paddingWidth->setValue(s.paddingWidth());
      frameRound->setValue(s.frameRound());
      circleButton->setChecked(s.circle());
      }

//---------------------------------------------------------
//   textStyle
//---------------------------------------------------------

TextStyle TextProp::textStyle() const
      {
      TextStyle s;
      if (curUnit == 0)
            s.setOffsetType(OFFSET_ABS);
      else if (curUnit == 1)
            s.setOffsetType(OFFSET_SPATIUM);
      s.setBold(fontBold->isChecked());
      s.setItalic(fontItalic->isChecked());
      s.setUnderline(fontUnderline->isChecked());
      s.setSize(fontSize->value());
      QFont f = fontSelect->currentFont();
      s.setFamily(f.family());
      s.setXoff(xOffset->value() / ((s.offsetType() == OFFSET_ABS) ? INCH : 1.0));
      s.setYoff(yOffset->value() / ((s.offsetType() == OFFSET_ABS) ? INCH : 1.0));
      s.setRxoff(rxOffset->value());
      s.setRyoff(ryOffset->value());
      s.setFrameColor(frameColor->color());
      s.setFrameWidth(frameWidth->value());
      s.setPaddingWidth(paddingWidth->value());
      s.setCircle(circleButton->isChecked());
      s.setFrameRound(frameRound->value());
      s.setHasFrame(frame->isChecked());
      s.setSystemFlag(systemFlag->isChecked());
      s.setForegroundColor(color->color());

      Align a = 0;
      if (alignHCenter->isChecked())
            a |= ALIGN_HCENTER;
      else if (alignRight->isChecked())
            a |= ALIGN_RIGHT;

      if (alignVCenter->isChecked())
            a |= ALIGN_VCENTER;
      else if (alignBottom->isChecked())
            a |= ALIGN_BOTTOM;
      else if (alignBaseline->isChecked())
            a |= ALIGN_BASELINE;
      s.setAlign(a);
      return s;
      }

//---------------------------------------------------------
//   styledToggled
//---------------------------------------------------------

void TextProp::styledToggled(bool val)
      {
      unstyledGroup->setChecked(!val);
      }

//---------------------------------------------------------
//   unstyledToggled
//---------------------------------------------------------

void TextProp::unstyledToggled(bool val)
      {
      styledGroup->setChecked(!val);
      }

