//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: instrdialog.cpp,v 1.32 2006/03/13 21:35:59 wschweer Exp $
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

#include "mscore.h"
#include "instrdialog.h"
#include "canvas.h"
#include "score.h"
#include "system.h"
#include "clef.h"
#include "undo.h"
#include "staff.h"
#include "part.h"
#include "segment.h"
#include "layout.h"
#include "style.h"

InstrumentTemplateList instrumentTemplates;

//---------------------------------------------------------
//   StaffListItem
//---------------------------------------------------------

StaffListItem::StaffListItem(PartListItem* li)
   : QTreeWidgetItem(li, STAFF_LIST_ITEM)
      {
      op       = ITEM_KEEP;
      staff    = 0;
      setPartIdx(0);
      staffIdx = 0;
      setClef(0);
      }

StaffListItem::StaffListItem()
   : QTreeWidgetItem(STAFF_LIST_ITEM)
      {
      op       = ITEM_KEEP;
      staff    = 0;
      setPartIdx(0);
      staffIdx = 0;
      setClef(0);
      }

//---------------------------------------------------------
//   setPartIdx
//---------------------------------------------------------

void StaffListItem::setPartIdx(int val)
      {
      _partIdx = val;
      QString s("Staff %1");
      setText(0, s.arg(_partIdx + 1));
      }

//---------------------------------------------------------
//   setClef
//---------------------------------------------------------

void StaffListItem::setClef(int val)
      {
      _clef = val;
      setText(1, clefTable[_clef].name);
      }

//---------------------------------------------------------
//   PartListItem
//---------------------------------------------------------

PartListItem::PartListItem(Part* p, QTreeWidget* lv)
   : QTreeWidgetItem(lv, PART_LIST_ITEM)
      {
      part = p;
      it   = 0;
      op   = ITEM_KEEP;
      setText(0, p->trackName());
      }

PartListItem::PartListItem(const InstrumentTemplate* i, QTreeWidget* lv)
   : QTreeWidgetItem(lv, PART_LIST_ITEM)
      {
      part = 0;
      it   = i;
      op   = ITEM_ADD;
printf("part list item <%s>\n", it->name.toLatin1().data());
      setText(0, it->name);
      }

//---------------------------------------------------------
//   InstrumentTemplate
//---------------------------------------------------------

InstrumentTemplate::InstrumentTemplate(const QString& grp, const QString& n, const QString& sn,
   int noSystems, const int* ci, int br) : group(grp), name(n), shortName(sn)
      {
      midiProgram = 0;
      staves      = noSystems;
      minPitch    = 0;
      maxPitch    = 127;
      for (int i = 0; i < staves; ++i)
            clefIdx[i] = *ci++;
      bracket = br;
      }

//---------------------------------------------------------
//   InstrumentTemplateListItem
//---------------------------------------------------------

InstrumentTemplateListItem::InstrumentTemplateListItem(QString group, QTreeWidget* parent)
   : QTreeWidgetItem(parent) {
      _instrumentTemplate = 0;
      _group = group;
      setText(0, group);
      }

InstrumentTemplateListItem::InstrumentTemplateListItem(const InstrumentTemplate* i, InstrumentTemplateListItem* item)
   : QTreeWidgetItem(item) {
      _instrumentTemplate = i;
      _group = _instrumentTemplate->group;
      setText(0, i->name);
      }

InstrumentTemplateListItem::InstrumentTemplateListItem(const InstrumentTemplate* i, QTreeWidget* parent)
   : QTreeWidgetItem(parent) {
      _instrumentTemplate = i;
      _group = _instrumentTemplate->group;
      setText(0, i->name);
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString InstrumentTemplateListItem::text(int col) const
      {
      switch (col) {
            case 0:
                  return _instrumentTemplate ?
                     _instrumentTemplate->name : _group;
            default:
                  return QString("");
            }
      }

//---------------------------------------------------------
//   InstrumentsDialog
//---------------------------------------------------------

InstrumentsDialog::InstrumentsDialog(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      cs = 0;
      QString curGroup;

      instrumentList->setSelectionMode(QAbstractItemView::SingleSelection);
      partiturList->setSelectionMode(QAbstractItemView::SingleSelection);

      instrumentList->setHeaderLabels(QStringList("Instrument List"));
      QStringList header = (QStringList() << tr("Staves") << tr("Clef"));
      partiturList->setHeaderLabels(header);
      InstrumentTemplateListItem* group = 0;
      for (ciInstrumentTemplate i = instrumentTemplates.begin(); i != instrumentTemplates.end(); ++i) {
            if (curGroup != i->group) {
                  curGroup = i->group;
                  group    = new InstrumentTemplateListItem(curGroup, instrumentList);
                  }
            new InstrumentTemplateListItem(&*i, group);
            }

      addButton->setEnabled(false);
      removeButton->setEnabled(false);
      upButton->setEnabled(false);
      downButton->setEnabled(false);
      editButton->setEnabled(false);
      aboveButton->setEnabled(false);
      belowButton->setEnabled(false);
      }

//---------------------------------------------------------
//   genPartList
//---------------------------------------------------------

void InstrumentsDialog::genPartList()
      {
      partiturList->clear();

      PartList* pl = cs->parts();
      for (iPart ip = pl->begin(); ip != pl->end(); ++ip) {
            PartListItem* pli = new PartListItem(*ip, partiturList);
            StaffList* sl = (*ip)->staves();
            for (iStaff is = sl->begin(); is != sl->end(); ++is) {
                  StaffListItem* sli = new StaffListItem(pli);
                  Staff* s      = *is;
                  sli->staff    = s;
                  sli->setPartIdx(s->rstaff());
                  sli->staffIdx = cs->staff(s);
                  sli->setClef((*is)->clef()->clef(0));
                  }
            partiturList->setItemExpanded(pli, true);
            }
      }

//---------------------------------------------------------
//   on_instrumentList_itemSelectionChanged
//---------------------------------------------------------

void InstrumentsDialog::on_instrumentList_itemSelectionChanged()
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();
      bool flag = true;
      if (item == 0 || item->parent() == 0)
            flag = false;
      addButton->setEnabled(flag);
      editButton->setEnabled(flag);
      }

//---------------------------------------------------------
//   on_partiturList_itemSelectionChanged
//---------------------------------------------------------

void InstrumentsDialog::on_partiturList_itemSelectionChanged()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();
      bool flag = item != 0;
      removeButton->setEnabled(flag);
      upButton->setEnabled(flag);
      downButton->setEnabled(flag);
      aboveButton->setEnabled(item && item->type() == STAFF_LIST_ITEM);
      belowButton->setEnabled(item && item->type() == STAFF_LIST_ITEM);
      }

//---------------------------------------------------------
//   on_instrumentList
//---------------------------------------------------------

void InstrumentsDialog::on_instrumentList_itemActivated(QTreeWidgetItem*, int)
      {
      on_addButton_clicked();
      }

//---------------------------------------------------------
//   on_addButton_clicked
//    add instrument to partitur
//---------------------------------------------------------

void InstrumentsDialog::on_addButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      if (wi.isEmpty())
            return;
      InstrumentTemplateListItem* item = (InstrumentTemplateListItem*)wi.front();
      const InstrumentTemplate* it     = item->instrumentTemplate();
      PartListItem* pli                = new PartListItem(it, partiturList);
      pli->op = ITEM_ADD;

      int n = it->staves;
      for (int i = 0; i < n; ++i) {
            StaffListItem* sli = new StaffListItem(pli);
            sli->op       = ITEM_ADD;
            sli->staff    = 0;
            sli->setPartIdx(i);
            sli->staffIdx = -1;
            sli->setClef(it->clefIdx[i]);
            }
      partiturList->setItemExpanded(pli, true);
      partiturList->clearSelection();     // should not be necessary
      partiturList->setItemSelected(pli, true);
      }

//---------------------------------------------------------
//   on_removeButton_clicked
//    remove instrument from partitur
//---------------------------------------------------------

void InstrumentsDialog::on_removeButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();
      QTreeWidgetItem* parent = item->parent();

      if (parent) {
            if (((StaffListItem*)item)->op == ITEM_ADD) {
                  if (parent->childCount() == 1) {
                        partiturList->takeTopLevelItem(partiturList->indexOfTopLevelItem(parent));
                        delete parent;
                        }
                  else {
                        parent->takeChild(parent->indexOfChild(item));
                        delete item;
                        }
                  }
            else {
                  ((StaffListItem*)item)->op = ITEM_DELETE;
                  partiturList->setItemHidden(item, true);
                  }
            }
      else {
            if (((PartListItem*)item)->op == ITEM_ADD)
                  delete item;
            else {
                  ((PartListItem*)item)->op = ITEM_DELETE;
                  partiturList->setItemHidden(item, true);
                  }
            }
      partiturList->clearSelection();
      }

//---------------------------------------------------------
//   on_upButton_clicked
//    move instrument up in partitur
//---------------------------------------------------------

void InstrumentsDialog::on_upButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();

      if (item->type() == PART_LIST_ITEM) {
            bool isExpanded = partiturList->isItemExpanded(item);
            int idx = partiturList->indexOfTopLevelItem(item);
            if (idx) {
                  QTreeWidgetItem* item = partiturList->takeTopLevelItem(idx);
                  partiturList->insertTopLevelItem(idx-1, item);
                  partiturList->setItemExpanded(item, isExpanded);
                  partiturList->setItemSelected(item, true);
                  }
            }
      else {
            QTreeWidgetItem* parent = item->parent();
            int idx = parent->indexOfChild(item);
            if (idx) {
                  QTreeWidgetItem* item = parent->takeChild(idx);
                  parent->insertChild(idx-1, item);
                  partiturList->setItemSelected(item, true);
                  }
            }
      }

//---------------------------------------------------------
//   on_downButton_clicked
//    move instrument down in partitur
//---------------------------------------------------------

void InstrumentsDialog::on_downButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();
      if (item->type() == PART_LIST_ITEM) {
            bool isExpanded = partiturList->isItemExpanded(item);
            int idx = partiturList->indexOfTopLevelItem(item);
            int n = partiturList->topLevelItemCount();
            if (idx < (n-1)) {
                  QTreeWidgetItem* item = partiturList->takeTopLevelItem(idx);
                  partiturList->insertTopLevelItem(idx+1, item);
                  partiturList->setItemExpanded(item, isExpanded);
                  partiturList->setItemSelected(item, true);
                  }
            }
      else {
            QTreeWidgetItem* parent = item->parent();
            int idx = parent->indexOfChild(item);
            int n = parent->childCount();
            if (idx < (n-1)) {
                  QTreeWidgetItem* item = parent->takeChild(idx);
                  parent->insertChild(idx+1, item);
                  partiturList->setItemSelected(item, true);
                  }
            }
      }

//---------------------------------------------------------
//   on_editButton_clicked
//    start instrument editor for selected instrument
//---------------------------------------------------------

void InstrumentsDialog::on_editButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      if (wi.isEmpty())
            return;
//      QTreeWidgetItem* item = wi.front();
      printf("edit not implemented\n");
      }

//---------------------------------------------------------
//   on_aboveButton_clicked
//---------------------------------------------------------

void InstrumentsDialog::on_aboveButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();
      if (item->type() != STAFF_LIST_ITEM)
            return;

      StaffListItem* sli  = (StaffListItem*)item;
      Staff* staff        = sli->staff;
      PartListItem* pli   = (PartListItem*)sli->parent();
      StaffListItem* nsli = new StaffListItem();
      nsli->staff         = staff;
      nsli->setClef(sli->clef());
      if (staff)
            nsli->op = ITEM_ADD;
      pli->insertChild(pli->indexOfChild(sli), nsli);
      partiturList->clearSelection();     // should not be necessary
      partiturList->setItemSelected(nsli, true);
      }

//---------------------------------------------------------
//   on_belowButton_clicked
//---------------------------------------------------------

void InstrumentsDialog::on_belowButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();
      if (item->type() != STAFF_LIST_ITEM)
            return;

      StaffListItem* sli  = (StaffListItem*)item;
      Staff* staff        = sli->staff;
      PartListItem* pli   = (PartListItem*)sli->parent();
      StaffListItem* nsli = new StaffListItem();
      nsli->staff         = staff;
      nsli->setClef(sli->clef());
      if (staff)
            nsli->op = ITEM_ADD;
      pli->insertChild(pli->indexOfChild(sli)+1, nsli);
      partiturList->clearSelection();     // should not be necessary
      partiturList->setItemSelected(nsli, true);
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void InstrumentsDialog::accept()
      {
      done(1);
      }

//---------------------------------------------------------
//   editInstrList
//---------------------------------------------------------

void MuseScore::editInstrList()
      {
      if (!instrList)
            instrList = new InstrumentsDialog(this);
      instrList->setScore(cs);
      instrList->genPartList();
      int rv = instrList->exec();
      if (rv == 0)
            return;

      //
      // process modified partitur list
      //
      QTreeWidget* pl = instrList->partiturList;
      cs->startCmd();
      Part* part   = 0;
      int staffIdx = 0;
      int rstaff   = 0;

      QTreeWidgetItem* item = 0;
      for (int idx = 0; (item = pl->topLevelItem(idx)); ++idx) {
            rstaff = 0;
            PartListItem* pli = (PartListItem*)item;
            if (pli->op == ITEM_DELETE)
                  cs->cmdRemovePart(pli->part);
            else if (pli->op == ITEM_ADD) {
                  const InstrumentTemplate* t = ((PartListItem*)item)->it;
                  part = new Part(cs);
                  part->setMidiProgram(t->midiProgram);
                  part->setMinPitch(t->minPitch);
                  part->setMaxPitch(t->maxPitch);
                  part->setShortName(t->shortName);
                  part->setTrackName(t->name);
                  part->setLongName(t->name);

                  pli->part = part;
                  QTreeWidgetItem* ci = 0;
                  rstaff = 0;
                  for (int cidx = 0; (ci = pli->child(cidx)); ++cidx) {
                        StaffListItem* sli = (StaffListItem*)ci;
                        Staff* staff = new Staff(cs, part, rstaff);
                        sli->staff = staff;
                        staff->setRstaff(rstaff);
                        ++rstaff;
                        staff->clef()->setClef(0, sli->clef());
                        if (cidx == 0) {
                              staff->setBracket(t->bracket);
                              staff->setBracketSpan(t->staves);
                              }
                        part->staves()->push_back(staff);
                        cs->staves()->insert(cs->staves()->begin() + staffIdx, staff);
                        cs->undoOp(UndoOp::InsertStaff, staff, staffIdx);
                        }
                  cs->cmdInsertPart(part, staffIdx);
                  staffIdx += rstaff;
                  }
            else {
                  part = pli->part;
                  QTreeWidgetItem* ci = 0;
                  for (int cidx = 0; (ci = pli->child(cidx)); ++cidx) {
                        StaffListItem* sli = (StaffListItem*)ci;
                        if (sli->op == ITEM_DELETE) {
                              cs->systems()->clear();
                              Staff* staff = sli->staff;
                              int sidx = cs->staff(staff);
                              int eidx = sidx + 1;
                              for (Measure* m = cs->scoreLayout()->first(); m; m = m->next())
                                    m->cmdRemoveStaves(sidx, eidx);
                              cs->undoOp(UndoOp::RemoveStaff, staff, sidx);
                              cs->removeStaff(staff);
                              }
                        else if (sli->op == ITEM_ADD) {
                              Staff* staff = new Staff(cs, part, rstaff);
                              sli->staff = staff;
                              staff->clef()->setClef(0, sli->clef());

                              for (Measure* m = cs->scoreLayout()->first(); m; m = m->next())
                                    m->cmdAddStaves(staffIdx, staffIdx+1);

                              cs->insertStaff(staff, staffIdx);
                              cs->undoOp(UndoOp::InsertStaff, staff, staffIdx);

                              ++staffIdx;
                              ++rstaff;
                              }
                        }
                  }
            }
      //
      //    sort staves
      //
      StaffList dst;
      for (int idx = 0; (item = pl->topLevelItem(idx)); ++idx) {
            PartListItem* pli = (PartListItem*)item;
            if (pli->op == ITEM_DELETE)
                  continue;
            QTreeWidgetItem* ci = 0;
            for (int cidx = 0; (ci = item->child(cidx)); ++cidx) {
                  StaffListItem* sli = (StaffListItem*) ci;
                  if (sli->op == ITEM_DELETE)
                        continue;
                  dst.push_back(sli->staff);
                  }
            }
      std::list<int> sl;
      std::list<int> dl;
      StaffList* src = cs->staves();
      int idx = 0;
      for (iStaff i = src->begin(); i != src->end(); ++i, ++idx)
            sl.push_back(idx);

      for (iStaff i = dst.begin(); i != dst.end(); ++i) {
            Staff* staff = *i;
            int idx = 0;
            iStaff ii = src->begin();
            for (; ii != src->end(); ++ii, ++idx) {
                  if (*ii == staff) {
                        dl.push_back(idx);
                        break;
                        }
                  }
            if (ii == src->end())
                  printf("staff in dialog(%p) not found in score\n", staff);
            }

      if (sl.size() != dl.size())
            printf("cannot happen: sl(%zd) != dl(%zd)\n", sl.size(), dl.size());
      bool sort = false;
      std::list<int>::iterator isl = sl.begin();
      std::list<int>::iterator dsl = dl.begin();
      for (unsigned int i = 0; i < sl.size(); ++i, ++isl, ++dsl) {
            if (*isl != *dsl) {
                  sort = true;
                  break;
                  }
            }
      if (sort) {
            cs->sortStaves(sl, dl);
            cs->undoOp(sl, dl);
            }
      cs->layout();
      cs->endCmd(true);
      }

//---------------------------------------------------------
//   cmdInsertPart
//    insert before staffIdx
//---------------------------------------------------------

void Score::cmdInsertPart(Part* part, int staffIdx)
      {
      undoOp(UndoOp::InsertPart, part, staffIdx);
      insertPart(part, staffIdx);

      int sidx = staff(part);
      int eidx = sidx + part->nstaves();

      for (Measure* m = _layout->first(); m; m = m->next())
            m->cmdAddStaves(sidx, eidx);

/*      int rstaff = 0;
      for (int i = sidx; i < eidx; ++i, ++rstaff) {
            Staff* staff = part->staff(rstaff);
            _staves->insert(_staves->begin() + i, staff);
            undoOp(UndoOp::InsertStaff, staff, i);
            }
      */
      }

//---------------------------------------------------------
//   cmdRemovePart
//---------------------------------------------------------

void Score::cmdRemovePart(Part* part)
      {
      systems()->clear();

      int sidx = staff(part);
      int n    = part->nstaves();
      int eidx = sidx + n;

      for (Measure* m = _layout->first(); m; m = m->next())
            m->cmdRemoveStaves(sidx, eidx);

      int idx = eidx - 1;
      for (iStaff i = _staves->begin() + idx; n > 0; --n, --idx, --i) {
            undoOp(UndoOp::RemoveStaff, *i, idx);
            part->staves()->remove(*i);
            }
      _staves->erase(_staves->begin() + sidx, _staves->begin() + eidx);
      _parts->remove(part);
      undoOp(UndoOp::RemovePart, part, sidx);
      }

//---------------------------------------------------------
//   insertPart
//---------------------------------------------------------

void Score::insertPart(Part* part, int idx)
      {
      systems()->clear();
      int staff = 0;
      for (iPart i = _parts->begin(); i != _parts->end(); ++i) {
            if (staff >= idx) {
                  _parts->insert(i, part);
                  return;
                  }
            staff += (*i)->nstaves();
            }
      _parts->push_back(part);
      }

//---------------------------------------------------------
//   removePart
//---------------------------------------------------------

void Score::removePart(Part* part)
      {
      systems()->clear();
      _parts->remove(part);
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Score::insertStaff(Staff* staff, int idx)
      {
      systems()->clear();
      _staves->insert(_staves->begin() + idx, staff);
      staff->part()->insertStaff(staff);
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Score::removeStaff(Staff* staff)
      {
      systems()->clear();
      _staves->remove(staff);
      staff->part()->removeStaff(staff);
      }

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Score::sortStaves(std::list<int> src, std::list<int> dst)
      {
      systems()->clear();

      _parts->clear();
      Part* curPart = 0;
      StaffList* dl = new StaffList;
      for (std::list<int>::iterator i = dst.begin(); i != dst.end(); ++i) {
            int didx = *i;
            int sidx = 0;
            for (std::list<int>::iterator ii = src.begin(); ii != src.end(); ++ii, ++sidx) {
                  if (didx == *ii) {
                        Staff* staff = (*_staves)[sidx];
                        if (staff->part() != curPart) {
                              curPart = staff->part();
                              curPart->staves()->clear();
                              _parts->push_back(curPart);
                              }
                        curPart->staves()->push_back(staff);
                        dl->push_back(staff);
                        break;
                        }
                  }
            }
      delete _staves;
      _staves = dl;

      for (Measure* m = _layout->first(); m; m = m->next()) {
            m->sortStaves(src, dst);
            }
      }

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Measure::sortStaves(std::list<int>& src, std::list<int>& dst)
      {
      MStaffList ms;

      for (std::list<int>::iterator i = dst.begin(); i != dst.end(); ++i) {
            int didx = *i;
            int sidx = 0;
            for (std::list<int>::iterator ii = src.begin(); ii != src.end(); ++ii, ++sidx) {
                  if (didx == *ii) {
                        ms.push_back(staves[sidx]);
                        break;
                        }
                  }
            }
      staves = ms;
      for (Segment* s = first(); s; s = s->next()) {
            s->sortStaves(src, dst);
            }
      }

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Segment::sortStaves(std::list<int>& src, std::list<int>& dst)
      {
      std::vector<Element*> dl;

      for (std::list<int>::iterator i = dst.begin(); i != dst.end(); ++i) {
            int didx = *i;
            int sidx = 0;
            for (std::list<int>::iterator ii = src.begin(); ii != src.end(); ++ii, ++sidx) {
                  if (didx == *ii) {
                        int startTrack = sidx * VOICES;
                        int endTrack = startTrack + VOICES;
                        for (int k = startTrack; k < endTrack; ++k)
                              dl.push_back(_elist[k]);
                        break;
                        }
                  }
            }
      _elist = dl;
      }

