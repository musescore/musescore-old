//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
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

#include "editdrumset.h"
#include "mscore.h"
#include "xml.h"
#include "utils.h"
#include "chord.h"
#include "score.h"
#include "note.h"

enum { COL_PITCH, COL_NOTE, COL_SHORTCUT, COL_NAME };

//---------------------------------------------------------
//   EditDrumset
//---------------------------------------------------------

EditDrumset::EditDrumset(Drumset* ds, QWidget* parent)
   : QDialog(parent)
      {
      oDrumset = ds;
#if 0
      for (int i = 0; i < 128; ++i) {
            nDrumset.drum[i].name          = ds->drum[i].name;
            nDrumset.drum[i].notehead      = ds->drum[i].notehead;
            nDrumset.drum[i].line          = ds->drum[i].line;
            nDrumset.drum[i].voice         = ds->drum[i].voice;
            nDrumset.drum[i].stemDirection = ds->drum[i].stemDirection;
            }
#endif
      nDrumset = *ds;
      setupUi(this);

      drumNote->setGrid(70, 80);
      drumNote->setDrawGrid(true);
      drumNote->setReadOnly(true);

      updateList();

      noteHead->addItem(tr("invalid"));
      noteHead->addItem(tr("Normal Head"));
      noteHead->addItem(tr("Cross Head"));
      noteHead->addItem(tr("Diamond Head"));
      noteHead->addItem(tr("Triangle Head"));

      loadButton = new QPushButton(tr("Load"));
      saveButton = new QPushButton(tr("Save"));
      buttonBox->addButton(loadButton, QDialogButtonBox::ActionRole);
      buttonBox->addButton(saveButton, QDialogButtonBox::ActionRole);

      connect(pitchList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
         SLOT(itemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(bboxClicked(QAbstractButton*)));
      connect(name, SIGNAL(textChanged(const QString&)), SLOT(nameChanged(const QString&)));
      connect(noteHead, SIGNAL(currentIndexChanged(int)), SLOT(valueChanged()));
      connect(staffLine, SIGNAL(valueChanged(int)), SLOT(valueChanged()));
      connect(voice, SIGNAL(valueChanged(int)), SLOT(valueChanged()));
      connect(stemDirection, SIGNAL(currentIndexChanged(int)), SLOT(valueChanged()));
      }

//---------------------------------------------------------
//   updateList
//---------------------------------------------------------

void EditDrumset::updateList()
      {
      pitchList->clear();
      for (int i = 0; i < 128; ++i) {
            QTreeWidgetItem* item = new QTreeWidgetItem(pitchList);
            item->setText(COL_PITCH, QString("%1").arg(i));
            item->setText(COL_NOTE, pitch2string(i));
            if (nDrumset.shortcut(i) == 0)
                  item->setText(COL_SHORTCUT, "");
            else {
                  QString s(QChar(nDrumset.shortcut(i)));
                  item->setText(COL_SHORTCUT, s);
                  }
            item->setText(COL_NAME, nDrumset.name(i));
            item->setData(0, Qt::UserRole, i);
            }
      }

//---------------------------------------------------------
//   nameChanged
//---------------------------------------------------------

void EditDrumset::nameChanged(const QString& name)
      {
      QTreeWidgetItem* item = pitchList->currentItem();
      if (item)
            item->setText(COL_NAME, name);
      }

//---------------------------------------------------------
//   bboxClicked
//---------------------------------------------------------

void EditDrumset::bboxClicked(QAbstractButton* button)
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

            case QDialogButtonBox::ActionRole:
                  if (button == loadButton)
                        load();
                  else if (button == saveButton)
                        save();
                  else
                        printf("EditDrumSet: unknown action button\n");
                  break;

            default:
                  printf("EditDrumSet: unknown button\n");
                  break;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void EditDrumset::apply()
      {
      *oDrumset = nDrumset;
      }

//---------------------------------------------------------
//   itemChanged
//---------------------------------------------------------

void EditDrumset::itemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
      {
      if (previous) {
            int pitch = previous->data(0, Qt::UserRole).toInt();
            nDrumset.drum[pitch].name          = name->text();
            nDrumset.drum[pitch].notehead      = noteHead->currentIndex() - 1;
            nDrumset.drum[pitch].line          = staffLine->value();
            nDrumset.drum[pitch].voice         = voice->value();
            if (shortcut->currentIndex() == 7)
                  nDrumset.drum[pitch].shortcut = 0;
            else
                  nDrumset.drum[pitch].shortcut = "ABCDEFG"[shortcut->currentIndex()];
            nDrumset.drum[pitch].stemDirection = Direction(stemDirection->currentIndex());
            previous->setText(COL_NAME, nDrumset.name(pitch));
            }
      name->blockSignals(true);
      staffLine->blockSignals(true);
      voice->blockSignals(true);
      stemDirection->blockSignals(true);
      noteHead->blockSignals(true);

      int pitch = current->data(0, Qt::UserRole).toInt();
      name->setText(nDrumset.name(pitch));
      staffLine->setValue(nDrumset.line(pitch));
      voice->setValue(nDrumset.voice(pitch));
      stemDirection->setCurrentIndex(int(nDrumset.stemDirection(pitch)));
      int nh = nDrumset.noteHead(pitch);
      noteHead->setCurrentIndex(nh + 1);
      if (nDrumset.shortcut(pitch) == 0)
            shortcut->setCurrentIndex(7);
      else
            shortcut->setCurrentIndex(nDrumset.shortcut(pitch) - 'A');

      name->blockSignals(false);
      staffLine->blockSignals(false);
      voice->blockSignals(false);
      stemDirection->blockSignals(false);
      noteHead->blockSignals(false);

      updateExample();
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void EditDrumset::valueChanged()
      {
      int pitch = pitchList->currentItem()->data(0, Qt::UserRole).toInt();
      nDrumset.drum[pitch].name          = name->text();
      nDrumset.drum[pitch].notehead      = noteHead->currentIndex() - 1;
      nDrumset.drum[pitch].line          = staffLine->value();
      nDrumset.drum[pitch].voice         = voice->value();
      nDrumset.drum[pitch].stemDirection = Direction(stemDirection->currentIndex());
      updateExample();
      }

//---------------------------------------------------------
//   updateExample
//---------------------------------------------------------

void EditDrumset::updateExample()
      {
      int pitch = pitchList->currentItem()->data(0, Qt::UserRole).toInt();
      if (!nDrumset.isValid(pitch)) {
            drumNote->add(0,  0, "");
            return;
            }
      int line      = nDrumset.line(pitch);
      int noteHead  = nDrumset.noteHead(pitch);
      int voice     = nDrumset.voice(pitch);
      Direction dir = nDrumset.stemDirection(pitch);
      bool up;
      if (dir == UP)
            up = true;
      else if (dir == DOWN)
            up = false;
      else
            up = line > 4;
      Chord* chord = new Chord(gscore);
      chord->setTickLen(division);
      chord->setDuration(Duration::V_QUARTER);
      chord->setStemDirection(dir);
      chord->setTrack(voice);
      Note* note = new Note(gscore);
      note->setParent(chord);
      note->setTrack(voice);
      note->setPitch(pitch);
      note->setLine(line);
      note->setPos(0.0, _spatium * .5 * line);
      note->setHeadGroup(noteHead);
      chord->add(note);
      Stem* stem = new Stem(gscore);
      stem->setLen(Spatium(up ? -3.0 : 3.0));
      chord->setStem(stem);
      stem->setPos(note->stemPos(up));
      drumNote->add(0,  chord, nDrumset.name(pitch));
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void EditDrumset::load()
      {
      QString name = QFileDialog::getOpenFileName(
         this, tr("MuseScore: Load Drumset"),
         ".",
         tr("MuseScore drumset (*.drm)")
         );
      if (name.isEmpty())
            return;

      QFile fp(name);
      if (!fp.open(QIODevice::ReadOnly))
            return;

      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(&fp, false, &err, &line, &column)) {
            printf("error reading file %s at line %d column %d: %s\n",
               qPrintable(name), line, column, qPrintable(err));
            return;
            }

      docName = name;
      nDrumset.clear();
      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull();  ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        QString val(ee.text());
                        if (tag == "Drum")
                              nDrumset.load(ee);
                        else
                              domError(ee);
                        }
                  }
            }
      fp.close();
      updateList();
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void EditDrumset::save()
      {
      QString name = QFileDialog::getSaveFileName(
         this, tr("MuseScore: Save Drumset"),
         ".",
         tr("MuseScore drumset (*.drm)")
         );
      if (name.isEmpty())
            return;

      QFile f(name);
      if (!f.open(QIODevice::WriteOnly)) {
            QString s = tr("Open File\n") + f.fileName() + tr("\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(mscore, tr("MuseScore: Open File"), s);
            return;
            }

      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      nDrumset.save(xml);
      xml.etag();
      if (f.error() != QFile::NoError) {
            QString s = QString("Write File failed: ") + f.errorString();
            QMessageBox::critical(this, tr("MuseScore: Write Drumset"), s);
            }
      }

