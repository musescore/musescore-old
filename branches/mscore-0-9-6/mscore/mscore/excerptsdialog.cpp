//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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

#include "excerptsdialog.h"
#include "mscore.h"
#include "score.h"
#include "part.h"
#include "excerpt.h"

//---------------------------------------------------------
//   ExcerptItem
//---------------------------------------------------------

ExcerptItem::ExcerptItem(Excerpt* e, QListWidget* parent)
   : QListWidgetItem(parent)
      {
      _excerpt = e;
      setText(e->name());
      }

//---------------------------------------------------------
//   PartItem
//---------------------------------------------------------

PartItem::PartItem(Part* p, QListWidget* parent)
   : QListWidgetItem(parent)
      {
      setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      setCheckState(Qt::Unchecked);
      _part = p;
      setText(p->trackName());
      }

//---------------------------------------------------------
//   ExcerptsDialog
//---------------------------------------------------------

ExcerptsDialog::ExcerptsDialog(Score* s, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setModal(true);
      score = s;

      // make a deep copy of all excerpts:

      QList<Excerpt*>* sel = score->excerpts();
      foreach(Excerpt* e, *sel)
            el.append(new Excerpt(*e));

      foreach(Excerpt* e, el) {
            ExcerptItem* ei = new ExcerptItem(e);
            excerptList->addItem(ei);
            }

      foreach(Part* p, *s->parts()) {
            PartItem* item = new PartItem(p);
            partList->addItem(item);
            }
      createExcerpt->setEnabled(false);

      connect(newButton, SIGNAL(clicked()), SLOT(newClicked()));
      connect(deleteButton, SIGNAL(clicked()), SLOT(deleteClicked()));
      connect(excerptList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
         SLOT(excerptChanged(QListWidgetItem*, QListWidgetItem*)));
      connect(excerptList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
         SLOT(createExcerptClicked(QListWidgetItem*)));
      connect(partList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
         SLOT(partDoubleClicked(QListWidgetItem*)));
      connect(createExcerpt, SIGNAL(clicked()), SLOT(createExcerptClicked()));

      if (!sel->isEmpty())
            excerptList->setCurrentRow(0);
      bool flag = excerptList->currentItem() != 0;
      editGroup->setEnabled(flag);
      deleteButton->setEnabled(flag);
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void ExcerptsDialog::accept()
      {
      QListWidgetItem* cur = excerptList->currentItem();
      excerptChanged(0, cur);

      //
      // set dirty if list changed
      //
      QList<Excerpt*>* sel = score->excerpts();
      bool dirty = false;
      if (sel->size() != el.size())
            dirty = true;
      else {
            int n = sel->size();
            for (int i = 0; i < n; ++i) {
                  if (*(sel->at(i)) != *(el.at(i))) {
                        dirty = true;
                        break;
                        }
                  }
            }
      if (dirty)
            score->setDirty(true);

      foreach(Excerpt* e, *sel)
            delete e;
      sel->clear();
      foreach(Excerpt* e, el)
            sel->append(e);
      el.clear();
      QDialog::accept();
      }

//---------------------------------------------------------
//   startExcerptsDialog
//---------------------------------------------------------

void MuseScore::startExcerptsDialog()
      {
      if (cs == 0)
            return;
      ExcerptsDialog ed(cs, 0);
      ed.exec();
      cs->setLayoutAll(true);
      cs->end();
      }

//---------------------------------------------------------
//   deleteClicked
//---------------------------------------------------------

void ExcerptsDialog::deleteClicked()
      {
      int idx = excerptList->currentRow();
      delete excerptList->item(idx);
      el.removeAt(idx);
      }

//---------------------------------------------------------
//   newClicked
//---------------------------------------------------------

void ExcerptsDialog::newClicked()
      {
      Excerpt* e = new Excerpt(score);
      QString name;
      for (int i = 1;; ++i) {
            name = tr("Part-%1").arg(i);
            Excerpt* ee = 0;
            foreach(ee, el) {
                  if (ee->name() == name)
                        break;
                  }
            if ((ee == 0) || (ee->name() != name))
                  break;
            }
      e->setName(name);
      el.append(e);
      ExcerptItem* ei = new ExcerptItem(e);
      excerptList->addItem(ei);
      excerptList->setCurrentRow(el.size() - 1);
      }

//---------------------------------------------------------
//   excerptChanged
//---------------------------------------------------------

void ExcerptsDialog::excerptChanged(QListWidgetItem* cur, QListWidgetItem* prev)
      {
      createExcerpt->setEnabled(true);
      if (prev) {
            Excerpt* pex = ((ExcerptItem*)prev)->excerpt();
            prev->setText(name->text());
            pex->setName(name->text());
            pex->setTitle(title->text());
            int n = partList->count();
            pex->parts()->clear();
            for (int i = 0; i < n; ++i) {
                  PartItem* pi = (PartItem*)partList->item(i);
                  if (pi->checkState() == Qt::Checked)
                        pex->parts()->append(pi->part());
                  }
            }
      if (cur) {
            Excerpt* e = ((ExcerptItem*)cur)->excerpt();
            name->setText(e->name());
            title->setText(e->title());

            // set selection:
            QList<Part*>* pl = e->parts();
            int n = partList->count();
            for (int i = 0; i < n; ++i) {
                  PartItem* pi = (PartItem*)partList->item(i);
                  int idx = pl->indexOf(pi->part());
                  pi->setCheckState(idx != -1 ? Qt::Checked : Qt::Unchecked);
                  }
            }
      else {
            name->setText("");
            title->setText("");
            int n = partList->count();
            for (int i = 0; i < n; ++i) {
                  PartItem* pi = (PartItem*)partList->item(i);
                  pi->setCheckState(Qt::Unchecked);
                  }
            }
      bool flag = excerptList->currentItem() != 0;
      editGroup->setEnabled(flag);
      deleteButton->setEnabled(flag);
      }

//---------------------------------------------------------
//   partDoubleClicked
//---------------------------------------------------------

void ExcerptsDialog::partDoubleClicked(QListWidgetItem* item)
      {
      PartItem* pi = (PartItem*)item;
      title->setText(pi->part()->trackName());
      }

//---------------------------------------------------------
//   createExcerptClicked
//---------------------------------------------------------

void ExcerptsDialog::createExcerptClicked()
      {
      QListWidgetItem* cur = excerptList->currentItem();
      if (cur == 0)
            return;
      createExcerptClicked(cur);
      }

//---------------------------------------------------------
//   createExcerpt
//---------------------------------------------------------

void ExcerptsDialog::createExcerptClicked(QListWidgetItem* cur)
      {
      excerptChanged(cur, cur);
      Excerpt* excerpt = ((ExcerptItem*)cur)->excerpt();
      if(excerpt->parts()->size() > 0) { //prevent creating empty score
            Score* nscore = score->createExcerpt(excerpt);
            nscore->rebuildMidiMapping();
            nscore->updateChannel();
            nscore->fixPpitch();
            nscore->layout();
            mscore->appendScore(nscore);
            nscore->setDirty(true);
            }
      }


