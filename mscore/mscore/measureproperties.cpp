//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2007 Werner Schweer and others
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

#include "measureproperties.h"
#include "measure.h"
#include "al/sig.h"
#include "score.h"
#include "repeat.h"
#include "undo.h"

//---------------------------------------------------------
//   MeasureProperties
//---------------------------------------------------------

MeasureProperties::MeasureProperties(Measure* _m, QWidget* parent)
   : QDialog(parent)
      {
      m = _m;
      setupUi(this);
      const AL::SigEvent ev(m->score()->sigmap()->timesig(m->tick()));

      actualZ->setValue(ev.fraction().numerator());
      actualN->setValue(ev.fraction().denominator());
      nominalZ->setValue(ev.getNominal().numerator());
      nominalN->setValue(ev.getNominal().denominator());

      irregular->setChecked(m->irregular());
      breakMultiMeasureRest->setChecked(m->getBreakMultiMeasureRest());
      int n  = m->repeatCount();
      count->setValue(n);
      count->setEnabled(m->repeatFlags() & RepeatEnd);
      layoutStretch->setValue(m->userStretch());
      measureNumberOffset->setValue(m->noOffset());

      Score* score = m->score();
      int rows = score->nstaves();
      staves->setRowCount(rows);
      staves->setColumnCount(3);

      for (int staffIdx = 0; staffIdx < rows; ++staffIdx) {
            QTableWidgetItem* item = new QTableWidgetItem(QString("%1").arg(staffIdx+1));
            staves->setItem(staffIdx, 0, item);
            MStaff* ms = m->mstaff(staffIdx);

            item = new QTableWidgetItem(tr("visible"));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setCheckState(ms->_visible ? Qt::Checked : Qt::Unchecked);
            if (rows == 1)                // cannot be invisible if only one row
                  item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            staves->setItem(staffIdx, 1, item);

            item = new QTableWidgetItem(tr("stemless"));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setCheckState(ms->_slashStyle ? Qt::Checked : Qt::Unchecked);
            staves->setItem(staffIdx, 2, item);
            }
      staves->verticalHeader()->hide();
      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(bboxClicked(QAbstractButton*)));
      }

//---------------------------------------------------------
//   bboxClicked
//---------------------------------------------------------

void MeasureProperties::bboxClicked(QAbstractButton* button)
      {
      QDialogButtonBox::ButtonRole br = buttonBox->buttonRole(button);
      switch(br) {
            case QDialogButtonBox::ApplyRole:
                  apply();
                  break;

            case QDialogButtonBox::AcceptRole:
                  apply();
                  // fall through

            case QDialogButtonBox::RejectRole:
                  close();
                  break;

            default:
                  printf("EditStaff: unknown button %d\n", int(br));
                  break;
            }
      }

//---------------------------------------------------------
//   visible
//---------------------------------------------------------

bool MeasureProperties::visible(int staffIdx)
      {
      QTableWidgetItem* item = staves->item(staffIdx, 1);
      return item->checkState() == Qt::Checked;
      }

//---------------------------------------------------------
//   slashStyle
//---------------------------------------------------------

bool MeasureProperties::slashStyle(int staffIdx)
      {
      QTableWidgetItem* item = staves->item(staffIdx, 2);
      return item->checkState() == Qt::Checked;
      }

//---------------------------------------------------------
//   sig
//---------------------------------------------------------

AL::SigEvent MeasureProperties::sig() const
      {
      AL::SigEvent e(Fraction(actualZ->value(), actualN->value()),
         Fraction(nominalZ->value(), nominalN->value()));
      return e;
      }

//---------------------------------------------------------
//   isIrregular
//---------------------------------------------------------

bool MeasureProperties::isIrregular() const
      {
      return irregular->isChecked();
      }

//---------------------------------------------------------
//   repeatCount
//---------------------------------------------------------

int MeasureProperties::repeatCount() const
      {
      return count->value();
      }

// HACK: to reconstruct siglist
struct MS {
      Measure* measure;
      AL::SigEvent se;
      };

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void MeasureProperties::apply()
      {
      Score* score = m->score();

      for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            MStaff* ms = m->mstaff(staffIdx);
            bool v = visible(staffIdx);
            bool s = slashStyle(staffIdx);
            if (ms->visible() != v || ms->slashStyle() != s)
                  score->undo()->push(new ChangeMStaffProperties(ms, v, s));
            }
      if (isIrregular() != m->irregular()
         || breakMultiMeasureRest->isChecked() != m->breakMultiMeasureRest()
         || repeatCount() != m->repeatCount()
         || layoutStretch->value() != m->userStretch()
         || measureNumberOffset->value() != m->noOffset()) {
            score->undo()->push(new ChangeMeasureProperties(m, breakMultiMeasureRest->isChecked(),
               repeatCount(), layoutStretch->value(), measureNumberOffset->value(), isIrregular()));
            }

      score->setDirty();
      score->select(0, SELECT_SINGLE, 0);
      AL::TimeSigMap* sl = score->sigmap();

      AL::SigEvent oev = sl->timesig(m->tick());
      AL::SigEvent newEvent = sig();

      if (!(oev == newEvent)) {
            // HACK: buildup evList to reconstruct siglist
            QList<MS> evList;
            for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
                  MS ms;
                  ms.measure = measure;
                  if (m == measure)
                        ms.se = newEvent;
                  else
                        ms.se = sl->timesig(measure->tick());
                  evList.append(ms);
                  }

            int oldLen = oev.ticks;
            int newLen = newEvent.ticks;

            AL::SigEvent oldEvent;
            AL::iSigEvent i = sl->find(m->tick());
            if (i != sl->end())
                  oldEvent = i->second;

            score->undoFixTicks();
            score->undoChangeSig(m->tick(), oldEvent, newEvent);
            //
            // change back to nominal values if there is
            // not already another TimeSig
            //
            i = sl->find(m->tick() + oldLen);
            if (i == sl->end()) {
                  score->undoChangeSig(m->tick() + newLen, AL::SigEvent(), oev);
                  }
            // score->select(0, SELECT_SINGLE, 0);
            m->adjustToLen(oldLen, newLen);

            // HACK: reconstruct siglist
            sl->clear();
            AL::SigEvent ev;
            int ctick = 0;
            foreach(MS ms, evList) {
                  ms.measure->moveTicks(ctick - ms.measure->tick());
                  if (!(ms.se == ev))
                        sl->add(ctick, ms.se);
                  ctick += sl->ticksMeasure(ctick);
                  ev = ms.se;
                  }
            score->undoFixTicks();
            score->select(m, SELECT_RANGE, 0);
            }
      score->renumberMeasures();
      score->setLayoutAll(true);
      score->end();
      }

