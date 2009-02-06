//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
//
//  $Id: repeatflag.cpp,v 1.00 2007/08/23 14:00:00 dikrau Exp $
//
//  Copyright (C) 2007- Dieter Krause (dikrau@users.sourceforge.net)
//
// repeatflag: contains function to handel properties of repeats and volta
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

#if 0       // TODO: MeasureBase

#include "repeatflag.h"
#include "repeat.h"
#include "measure.h"
#include "barline.h"

Element* actElement;

void RepeatFlag::setRepeatFlagProps(Element* e, RepeatFlag* rf)
      {
      QString en;

      if (e->type() == REPEAT)
            en = e->subtypeName();
      if (e->type() == BAR_LINE) {
            switch (e->subtype()) {
                  case START_REPEAT:
                        en = "Start Repeat";
                        break;
                  case END_REPEAT:
                        en = "End Repeat";
                        break;
                  case END_START_REPEAT:
                        en = "End/Start Repeat";
                        break;
                  }
            }
      if (e->type() == VOLTA || e->type() == VOLTA_SEGMENT)
            en = "Volta";
      if (e->type() == REPEAT_MEASURE)
            en = "Repeat last Measure";

	RepeatFlagDialog* repeatFlagDialog = new RepeatFlagDialog();

      repeatFlagDialog->repeatTypeName->setText(en);
      repeatFlagDialog->cycles->setValue(rf->cycle());
      repeatFlagDialog->no->setValue(rf->no());
      repeatFlagDialog->cyclesToPlay->setText(rf->cycleList());
      repeatFlagDialog->destno->setValue(rf->destNo());
	repeatFlagDialog->show();
      }

RepeatFlag::RepeatFlag()
      {
      setDefaults(this);
      }

void RepeatFlag::setDefaults (RepeatFlag* rf)
      {
      rf->setRepeatFlag(0);
      rf->setNo(1);
      rf->setCycle(2);
      rf->setDestNo(1);
      rf->setCycleList("1");
      }

RepeatFlag::~RepeatFlag()
      {
      }


void RepeatFlag::setMeasureRepeatFlag(Element* el, int type)
      {
#if 0       // ws
      int flag;


      flag = el->subtype();
      if (!el->repeatFlag()) {
            RepeatFlag* rf = new RepeatFlag();
            setDefaults(rf);
            if (el->subtype() == RepeatCoda)
                  rf->setCycleList("2");
            if (el->type() == REPEAT_MEASURE)
                  flag = RepeatMeasureFlag;
            if (el->type() == BAR_LINE) {
                  switch (type) {
                        case START_REPEAT:
                              {
                              flag = RepeatStart;
                              break;
                              }
                        case END_REPEAT:
                              {
                              flag = RepeatEnd;
                              break;
                              }
                        default:
                              break;
                        }
                  }
            rf->setRepeatFlag(flag);
            el->setRepeatFlag(rf);
            }
#endif
      }


//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool RepeatFlag::genPropertyMenu(QMenu* popup) const
      {
      QAction* a = popup->addAction(QT_TR_NOOP("Setup..."));
      a->setData("sets");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void RepeatFlag::propertyAction(const QString& s, Element* el)
      {
#if 0 // WS
      RepeatFlag* rf;

      if (s == "sets") {
            actElement = el;
            rf = el->repeatFlag();
            if (rf) {
                  setRepeatFlagProps(el,rf);
                  }
            }
#endif
      }

RepeatFlag* RepeatFlag::findRepElement(Measure* m, int rtype)
      {
      RepList* rl;

      for (rl = rList; rl; rl = rl->next()) {
            if (rl->measure() == m && rl->getRf()->repeatFlag() == rtype)
                  return rl->getRf();
            }
      return 0x00;
      }

RepeatFlag* RepeatFlag::findCodaElement(int n)
      {
      RepList* rl;

      for (rl = rList; rl; rl = rl->next()) {
            if (rl->getRf()->repeatFlag() == RepeatCoda && rl->getRf()->no() == n)
                  return rl->getRf();
            }
      return 0x00;
      }

RepeatFlag* RepeatFlag::findCodettaElement(int n)
      {
      RepList* rl;

      for (rl = rList; rl; rl = rl->next()) {
            if (rl->getRf()->repeatFlag() == RepeatCodetta && rl->getRf()->no() == n)
                  return rl->getRf();
            }
      return 0x00;
      }

#endif

