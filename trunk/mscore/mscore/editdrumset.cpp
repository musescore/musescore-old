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
      }

//---------------------------------------------------------
//   updateList
//---------------------------------------------------------

void EditDrumset::updateList()
      {
      pitchList->clear();
      for (int i = 0; i < 128; ++i) {
            QTreeWidgetItem* item = new QTreeWidgetItem(pitchList);
            item->setText(0, QString("%1").arg(i));
            item->setText(1, nDrumset.name(i));
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
            item->setText(1, name);
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
            nDrumset.drum[pitch].stemDirection = Direction(stemDirection->currentIndex());
            previous->setText(1, nDrumset.name(pitch));
            }

      int pitch = current->data(0, Qt::UserRole).toInt();
      name->setText(nDrumset.name(pitch));
      staffLine->setValue(nDrumset.line(pitch));
      voice->setValue(nDrumset.voice(pitch));
      stemDirection->setCurrentIndex(int(nDrumset.stemDirection(pitch)));
      int nh = nDrumset.noteHead(pitch);
      noteHead->setCurrentIndex(nh + 1);
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
      xml.stag("museScore version=\"1.0\"");
      nDrumset.save(xml);
      xml.etag();
      if (f.error() != QFile::NoError) {
            QString s = QString("Write File failed: ") + f.errorString();
            QMessageBox::critical(this, tr("MuseScore: Write Drumset"), s);
            }
      }

