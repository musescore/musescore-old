//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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

#include "mscore.h"
#include "score.h"
#include "part.h"
#include "partedit.h"
#include "seq.h"
#include "undo.h"
#include "synti.h"
#include "synthcontrol.h"

//---------------------------------------------------------
//   PartEdit
//---------------------------------------------------------

PartEdit::PartEdit(QWidget* parent)
   : QWidget(parent, Qt::Dialog)
      {
      setupUi(this);
      connect(patch,    SIGNAL(activated(int)),           SLOT(patchChanged(int)));
      connect(volume,   SIGNAL(valueChanged(double,int)), SLOT(volChanged(double)));
      connect(pan,      SIGNAL(valueChanged(double,int)), SLOT(panChanged(double)));
      connect(chorus,   SIGNAL(valueChanged(double,int)), SLOT(chorusChanged(double)));
      connect(reverb,   SIGNAL(valueChanged(double,int)), SLOT(reverbChanged(double)));
      connect(mute,     SIGNAL(toggled(bool)),            SLOT(muteChanged(bool)));
      connect(solo,     SIGNAL(toggled(bool)),            SLOT(soloToggled(bool)));
      connect(drumset,  SIGNAL(toggled(bool)),            SLOT(drumsetToggled(bool)));
      }

//---------------------------------------------------------
//   setPart
//---------------------------------------------------------

void PartEdit::setPart(Part* p, Channel* a)
      {
      channel = a;
      part    = p;
      QString s = part->trackName();
      if (!a->name.isEmpty())
            s += "-" + a->name;
      partName->setText(s);
      mute->setChecked(a->mute);
      solo->setChecked(a->solo);
      volume->setValue(a->volume);
      reverb->setValue(a->reverb);
      chorus->setValue(a->chorus);
      pan->setValue(a->pan);
      patch->setCurrentIndex(a->program);
      drumset->setChecked(p->useDrumset());
      }

//---------------------------------------------------------
//   InstrumentListEditor
//---------------------------------------------------------

InstrumentListEditor::InstrumentListEditor(QWidget* parent)
   : QScrollArea(parent)
      {
      setWindowTitle(tr("MuseScore: Part List"));
      setWidgetResizable(true);
      setWindowFlags(Qt::Dialog);
      QWidget* area = new QWidget(this);
      vb = new QVBoxLayout;
      vb->setMargin(0);
      vb->setSpacing(0);
      area->setLayout(vb);
      setWidget(area);
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void InstrumentListEditor::closeEvent(QCloseEvent* ev)
      {
      QAction* a = getAction("toggle-mixer");
      a->setChecked(false);
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   updateAll
//---------------------------------------------------------

void InstrumentListEditor::updateAll(Score* score)
      {
      cs = score;
      int n = -vb->count();
      if(cs) {
            QList<MidiMapping>* mm = cs->midiMapping();
            n = mm->size() - vb->count();
            }
      while (n < 0) {
            QWidgetItem* wi = (QWidgetItem*)(vb->itemAt(0));
            vb->removeItem(wi);
            delete wi->widget();
            delete wi;
            ++n;
            }
      while (n > 0) {
            PartEdit* pe = new PartEdit;
            connect(pe, SIGNAL(soloChanged(bool)), SLOT(updateSolo(bool)));
            vb->addWidget(pe);
            --n;
            }
      patchListChanged();
      }

//---------------------------------------------------------
//   patchListChanged
//---------------------------------------------------------

void InstrumentListEditor::patchListChanged()
      {
      if(!cs)
            return;
      QString s;
      int idx = 0;
      QList<MidiMapping>* mm = cs->midiMapping();
      foreach (const MidiMapping& m, *mm) {
            QWidgetItem* wi = (QWidgetItem*)(vb->itemAt(idx));
            PartEdit* pe    = (PartEdit*)(wi->widget());
            bool drum       = m.part->useDrumset();
            const QList<MidiPatch*> pl = seq->getPatchInfo();
            pe->patch->clear();
            foreach(const MidiPatch* p, pl) {
                  if (p->drum == drum)
                        pe->patch->addItem(p->name, QVariant::fromValue<void*>((void*)p));
                  }
            pe->setPart(m.part, m.articulation);
            idx++;
            }
      }

//---------------------------------------------------------
//   showMixer
//---------------------------------------------------------

void MuseScore::showMixer(bool val)
      {
      if (iledit == 0) {
            iledit = new InstrumentListEditor(this);
            if (synthControl) {
                  connect(synthControl, SIGNAL(soundFontChanged()), iledit,
                     SLOT(patchListChanged()));
                  }
            }
      iledit->updateAll(cs);
      iledit->setShown(val);
      }

//---------------------------------------------------------
//   patchChanged
//---------------------------------------------------------

void PartEdit::patchChanged(int n)
      {
      MidiPatch* p = (MidiPatch*)patch->itemData(n, Qt::UserRole).value<void*>();
      Score* score = part->score();
      score->startCmd();
      score->undo()->push(new ChangePatch(part, channel, p->prog, p->bank));
      score->endCmd();
      }

//---------------------------------------------------------
//   volChanged
//---------------------------------------------------------

void PartEdit::volChanged(double val)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_VOLUME, iv);
      channel->volume = iv;
      }

//---------------------------------------------------------
//   panChanged
//---------------------------------------------------------

void PartEdit::panChanged(double val)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_PANPOT, iv);
      channel->pan = iv;
      }

//---------------------------------------------------------
//   reverbChanged
//---------------------------------------------------------

void PartEdit::reverbChanged(double val)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_REVERB_SEND, iv);
      channel->reverb = iv;
      }

//---------------------------------------------------------
//   chorusChanged
//---------------------------------------------------------

void PartEdit::chorusChanged(double val)
      {
      int iv = lrint(val);
      seq->setController(channel->channel, CTRL_CHORUS_SEND, iv);
      channel->chorus = iv;
      }

//---------------------------------------------------------
//   muteChanged
//---------------------------------------------------------

void PartEdit::muteChanged(bool val)
      {
      channel->mute = val;
      }

//---------------------------------------------------------
//   soloToggled
//---------------------------------------------------------

void PartEdit::soloToggled(bool val)
      {
      channel->solo = val;
      channel->soloMute = !val;
      if (val) {
            mute->setChecked(false);
            foreach(Part* part, *part->score()->parts()) {
                  for (int i = 0; i < part->channel().size(); ++i) {
                        Channel* a = &part->channel(i);
                        a->soloMute = (channel != a && !a->solo);
                        a->solo     = (channel == a || a->solo);
                        }
                  }
            emit soloChanged(true);
            }
      else { //do nothing except if it's the last solo to be switched off
            bool found = false;
            foreach(Part* part, *part->score()->parts()) {
                  for (int i = 0; i < part->channel().size(); ++i) {
                        Channel* a = &part->channel(i);
                        if(a->solo){
                            found = true;
                            break;
                            }
                        }
                  }
            if(!found){
                foreach(Part* part, *part->score()->parts()) {
                  for (int i = 0; i < part->channel().size(); ++i) {
                        Channel* a = &part->channel(i);
                        a->soloMute = false;
                        a->solo     = false;
                        }
                  }
                  emit soloChanged(false);
                }
            }
      }

//---------------------------------------------------------
//   drumsetToggled
//---------------------------------------------------------

void PartEdit::drumsetToggled(bool val)
      {
      part->setUseDrumset(val);
      patch->clear();
      const QList<MidiPatch*> pl = seq->getPatchInfo();
      foreach(MidiPatch* p, pl) {
            if (val == p->drum)
                  patch->addItem(p->name, QVariant::fromValue<void*>((void*)p));
            }
      patch->setCurrentIndex(channel->program);
      }

//---------------------------------------------------------
//   updateSolo
//---------------------------------------------------------

void InstrumentListEditor::updateSolo(bool val)
      {
      for(int i = 0; i <vb->count(); i++ ){
            QWidgetItem* wi = (QWidgetItem*)(vb->itemAt(i));
            PartEdit* pe    = (PartEdit*)(wi->widget());
            pe->mute->setEnabled(!val);
            }
      }

