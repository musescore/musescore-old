//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: instrdialog.cpp,v 1.32 2006/03/13 21:35:59 wschweer Exp $
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

#include "config.h"
#include "mscore.h"
#include "instrdialog.h"
#include "instrtemplate.h"
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
#include "editinstrument.h"
#include "drumset.h"
#include "slur.h"

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
      setText(0, it->name);
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

InstrumentTemplateListItem::InstrumentTemplateListItem(InstrumentTemplate* i, InstrumentTemplateListItem* item)
   : QTreeWidgetItem(item) {
      _instrumentTemplate = i;
      _group = _instrumentTemplate->group;
      setText(0, i->name);
      }

InstrumentTemplateListItem::InstrumentTemplateListItem(InstrumentTemplate* i, QTreeWidget* parent)
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
      editInstrument = 0;
      setupUi(this);
      cs = 0;

      instrumentList->setSelectionMode(QAbstractItemView::SingleSelection);
      partiturList->setSelectionMode(QAbstractItemView::SingleSelection);

      instrumentList->setHeaderLabels(QStringList("Instrument List"));
      QStringList header = (QStringList() << tr("Staves") << tr("Clef"));
      partiturList->setHeaderLabels(header);

      buildTemplateList();

      addButton->setEnabled(false);
      removeButton->setEnabled(false);
      upButton->setEnabled(false);
      downButton->setEnabled(false);
      editButton->setEnabled(false);
      aboveButton->setEnabled(false);
      belowButton->setEnabled(false);
      }

//---------------------------------------------------------
//   buildTemplateList
//---------------------------------------------------------

void InstrumentsDialog::buildTemplateList()
      {
      instrumentList->clear();
      InstrumentTemplateListItem* group = 0;
      QString curGroup;
      foreach(InstrumentTemplate* t, instrumentTemplates) {
            if (curGroup != t->group) {
                  curGroup = t->group;
                  group    = new InstrumentTemplateListItem(curGroup, instrumentList);
                  group->setFlags(Qt::ItemIsEnabled);
                  }
            new InstrumentTemplateListItem(t, group);
            }
      }

//---------------------------------------------------------
//   genPartList
//---------------------------------------------------------

void InstrumentsDialog::genPartList()
      {
      partiturList->clear();

      foreach(Part* p, *cs->parts()) {
            PartListItem* pli = new PartListItem(p, partiturList);
            foreach(Staff* s, *p->staves()) {
                  StaffListItem* sli = new StaffListItem(pli);
                  sli->staff    = s;
                  sli->setPartIdx(s->rstaff());
                  sli->staffIdx = s->idx();
                  sli->setClef(s->clef()->clef(0));
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
      bool flag = !wi.isEmpty();
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
      if (it == 0)
            return;
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
      QTreeWidgetItem* item = wi.front();

      InstrumentTemplateListItem* ti = (InstrumentTemplateListItem*) item;
      InstrumentTemplate* tp         = ti->instrumentTemplate();
      if (tp == 0)
            return;

      if (editInstrument == 0)
            editInstrument = new EditInstrument(this);

      editInstrument->setInstrument(tp);
      editInstrument->show();
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
      cs->startCmd();

      QTreeWidget* pl = instrList->partiturList;
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
                  part->setPitchOffset(t->transpose);
                  if (t->useDrumset) {
                        part->setUseDrumset(true);
                        part->setDrumset(new Drumset(*smDrumset));
                        }

                  pli->part = part;
                  QTreeWidgetItem* ci = 0;
                  rstaff = 0;
                  for (int cidx = 0; (ci = pli->child(cidx)); ++cidx) {
                        StaffListItem* sli = (StaffListItem*)ci;
                        Staff* staff = new Staff(cs, part, rstaff);
                        sli->staff = staff;
                        staff->setRstaff(rstaff);
                        // ++rstaff;
                        staff->clef()->setClef(0, sli->clef());
                        staff->setLines(t->staffLines[cidx]);
                        staff->setSmall(t->smallStaff[cidx]);
                        if (cidx == 0) {
                              staff->setBracket(0, t->bracket);
                              staff->setBracketSpan(0, t->staves);
                              }
                        cs->insertStaff(staff, staffIdx + rstaff);
                        cs->undoOp(UndoOp::InsertStaff, staff, staffIdx + rstaff);
                        ++rstaff;
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
                              cs->mainLayout()->systems()->clear();
                              Staff* staff = sli->staff;
                              int sidx = staff->idx();
                              int eidx = sidx + 1;
                              for (MeasureBase* mb = cs->mainLayout()->first(); mb; mb = mb->next()) {
                                    if (mb->type() != MEASURE)
                                          continue;
                                    Measure* m = (Measure*)mb;
                                    m->cmdRemoveStaves(sidx, eidx);
                                    }
                              cs->undoOp(UndoOp::RemoveStaff, staff, sidx);
                              cs->removeStaff(staff);
                              }
                        else if (sli->op == ITEM_ADD) {
                              Staff* staff = new Staff(cs, part, rstaff);
                              sli->staff = staff;
                              staff->setRstaff(rstaff);
                              ++rstaff;
                              staff->clef()->setClef(0, sli->clef());

                              cs->insertStaff(staff, staffIdx);
                              cs->undoOp(UndoOp::InsertStaff, staff, staffIdx);

                              for (MeasureBase* mb = cs->mainLayout()->first(); mb; mb = mb->next()) {
                                    if (mb->type() != MEASURE)
                                          continue;
                                    Measure* m = (Measure*)mb;
                                    m->cmdAddStaves(staffIdx, staffIdx+1);
                                    }

                              ++staffIdx;
                              }
                        else {
                              ++staffIdx;
                              ++rstaff;
                              }
                        }
                  }
            }
      //
      //    sort staves
      //
      QList<Staff*> dst;
      for (int idx = 0; idx < pl->topLevelItemCount(); ++idx) {
            PartListItem* pli = (PartListItem*)pl->topLevelItem(idx);
            if (pli->op == ITEM_DELETE)
                  continue;
            QTreeWidgetItem* ci = 0;
            for (int cidx = 0; (ci = pli->child(cidx)); ++cidx) {
                  StaffListItem* sli = (StaffListItem*) ci;
                  if (sli->op == ITEM_DELETE)
                        continue;
                  dst.push_back(sli->staff);
                  }
            }
      QList<int> sl;
      QList<int> dl;

      for (int idx = 0; idx < cs->staves().size(); ++idx)
            sl.push_back(idx);

      foreach(Staff* staff, dst) {
            int idx = cs->staves().indexOf(staff);
            if (idx == -1)
                  printf("staff in dialog(%p) not found in score\n", staff);
            else
                  dl.push_back(idx);
            }

      if (sl.size() != dl.size())
            printf("cannot happen: sl(%d) != dl(%d)\n", sl.size(), dl.size());
      bool sort = false;
      for (int i = 0; i < sl.size(); ++i) {
            if (sl[i] != dl[i]) {
                  sort = true;
                  break;
                  }
            }
      if (sort) {
            cs->sortStaves(sl, dl);
            cs->undoOp(sl, dl);
            }
      cs->setLayoutAll(true);
      cs->endCmd();
      }

//---------------------------------------------------------
//   cmdInsertPart
//    insert before staffIdx
//---------------------------------------------------------

void Score::cmdInsertPart(Part* part, int staffIdx)
      {
      undoOp(UndoOp::InsertPart, part, staffIdx);
      insertPart(part, staffIdx);

      int sidx = this->staffIdx(part);
      int eidx = sidx + part->nstaves();
      for (MeasureBase* mb = _layout->first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            m->cmdAddStaves(sidx, eidx);
            }
      }

//---------------------------------------------------------
//   cmdRemovePart
//---------------------------------------------------------

void Score::cmdRemovePart(Part* part)
      {
      int sidx = staffIdx(part);
      int n    = part->nstaves();
      int eidx = sidx + n;

      for (MeasureBase* mb = _layout->first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            m->cmdRemoveStaves(sidx, eidx);
            }

      int idx = eidx - 1;
      for (QList<Staff*>::iterator i = _staves.begin() + idx; n > 0; --n, --idx, --i) {
            undoOp(UndoOp::RemoveStaff, *i, idx);
            part->staves()->removeAll(*i);
            }
      _staves.erase(_staves.begin() + sidx, _staves.begin() + eidx);
      _parts.removeAt(_parts.indexOf(part));
      undoOp(UndoOp::RemovePart, part, sidx);
      }

//---------------------------------------------------------
//   insertPart
//---------------------------------------------------------

void Score::insertPart(Part* part, int idx)
      {
      int staff = 0;
      for (QList<Part*>::iterator i = _parts.begin(); i != _parts.end(); ++i) {
            if (staff >= idx) {
                  _parts.insert(i, part);
                  return;
                  }
            staff += (*i)->nstaves();
            }
      _parts.push_back(part);
      }

//---------------------------------------------------------
//   removePart
//---------------------------------------------------------

void Score::removePart(Part* part)
      {
      _parts.removeAt(_parts.indexOf(part));
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Score::insertStaff(Staff* staff, int idx)
      {
      _staves.insert(idx, staff);
      staff->part()->insertStaff(staff);

      int track = idx * VOICES;
      foreach(Element* e, *_layout->gel()) {
            if (e->type() == SLUR) {
                  Slur* slur = (Slur*)e;
                  if (slur->track() >= track) {
                        slur->setTrack(slur->track() + VOICES);
                        }
                  if (slur->track2() >= track)
                        slur->setTrack2(slur->track2() + VOICES);
                  }
            }
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Score::removeStaff(Staff* staff)
      {
      int idx = staff->idx();
      _staves.removeAll(staff);
      staff->part()->removeStaff(staff);
      int track = idx * VOICES;
      foreach(Element* e, *_layout->gel()) {
            if (e->type() == SLUR) {
                  Slur* slur = (Slur*)e;
                  if (slur->track() > track) {
                        slur->setTrack(slur->track() - VOICES);
                        }
                  if (slur->track2() > track)
                        slur->setTrack2(slur->track2() - VOICES);
                  }
            }
      }

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Score::sortStaves(QList<int> src, QList<int> dst)
      {
      _layout->systems()->clear();  //??
      _parts.clear();
      Part* curPart = 0;
      QList<Staff*> dl;
      for (QList<int>::iterator i = dst.begin(); i != dst.end(); ++i) {
            int didx = *i;
            int sidx = 0;
            for (QList<int>::iterator ii = src.begin(); ii != src.end(); ++ii, ++sidx) {
                  if (didx == *ii) {
                        Staff* staff = _staves[sidx];
                        if (staff->part() != curPart) {
                              curPart = staff->part();
                              curPart->staves()->clear();
                              _parts.push_back(curPart);
                              }
                        curPart->staves()->push_back(staff);
                        dl.push_back(staff);
                        break;
                        }
                  }
            }
      _staves = dl;

      for (MeasureBase* mb = _layout->first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            m->sortStaves(src, dst);
            }
      }

//---------------------------------------------------------
//   on_saveButton_clicked
//---------------------------------------------------------

void InstrumentsDialog::on_saveButton_clicked()
      {
      QString name = QFileDialog::getSaveFileName(
         this,
         tr("MuseScore: Save Instrument List"),
         ".",
         tr("MuseScore Instruments (*.xml);;")
         );
      if (name.isEmpty())
            return;
      QString ext(".xml");
      QFileInfo info(name);

      if (info.completeSuffix().isEmpty())
            info.setFile(info.filePath() + ext);
      QFile f(info.filePath());
      if (!f.open(QIODevice::WriteOnly)) {
            QString s = tr("Open Instruments File\n") + f.fileName() + tr("\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(mscore, tr("MuseScore: Open Instruments file"), s);
            return;
            }

      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      QString curGroup;
      foreach(InstrumentTemplate* t, instrumentTemplates) {
            if (curGroup != t->group) {
                  if (!curGroup.isEmpty())
                        xml.etag();
                  xml.stag(QString("instrument-group name=\"%1\"").arg(t->group));
                  curGroup = t->group;
                  }
            t->write(xml);
            }
      if (!curGroup.isEmpty())
            xml.etag();
      xml.etag();
      if (f.error() != QFile::NoError) {
            QString s = QString("Write Style failed: ") + f.errorString();
            QMessageBox::critical(this, tr("MuseScore: Write Style"), s);
            }
      }

//---------------------------------------------------------
//   on_loadButton_clicked
//---------------------------------------------------------

void InstrumentsDialog::on_loadButton_clicked()
      {
      QString fn = QFileDialog::getOpenFileName(
         this, tr("MuseScore: Load Instrument List"),
          mscoreGlobalShare + "/templates",
         tr("MuseScore Instruments (*.xml);;"
            "All files (*)"
            )
         );
      if (fn.isEmpty())
            return;
      QFile f(fn);
      if (!loadInstrumentTemplates(fn)) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: load Style failed:"),
               QString(strerror(errno)),
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            return;
            }
      buildTemplateList();
      }

