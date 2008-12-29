//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: partedit.cpp,v 1.4 2006/03/13 21:35:59 wschweer Exp $
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

//---------------------------------------------------------
//   PartEdit
//---------------------------------------------------------

PartEdit::PartEdit(QWidget* parent)
   : QWidget(parent, Qt::Dialog)
      {
      setupUi(this);
      connect(patch,    SIGNAL(activated(int)),    SLOT(patchChanged(int)));
      connect(volume,   SIGNAL(valueChanged(double,int)), SLOT(volChanged(double)));
      connect(pan,      SIGNAL(valueChanged(double,int)), SLOT(panChanged(double)));
      connect(chorus,   SIGNAL(valueChanged(double,int)), SLOT(chorusChanged(double)));
      connect(reverb,   SIGNAL(valueChanged(double,int)), SLOT(reverbChanged(double)));
      connect(mute,     SIGNAL(toggled(bool)),     SLOT(muteChanged(bool)));
      connect(solo,     SIGNAL(toggled(bool)),     SLOT(soloToggled(bool)));
      connect(drumset,  SIGNAL(toggled(bool)),     SLOT(drumsetToggled(bool)));
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
      QList<MidiMapping>* mm = cs->midiMapping();

      int n = mm->size() - vb->count();
      while (n < 0) {
            QWidgetItem* wi = (QWidgetItem*)(vb->itemAt(0));
            vb->removeItem(wi);
            delete wi->widget();
            delete wi;
            ++n;
            }
      while (n > 0) {
            PartEdit* pe = new PartEdit;
            connect(pe, SIGNAL(soloChanged()), SLOT(updateSolo()));
            connect(this, SIGNAL(soloChanged()), pe, SLOT(updateSolo()));
            vb->addWidget(pe);
            --n;
            }
      QString s;
      int idx = 0;
      foreach (const MidiMapping& m, *mm) {
            QWidgetItem* wi = (QWidgetItem*)(vb->itemAt(idx));
            PartEdit* pe    = (PartEdit*)(wi->widget());
            pe->setPart(m.part, m.articulation);
            const MidiPatch* p = 0;
            for (;;) {
                  p = seq->getPatchInfo(m.part->useDrumset(), p);
                  if (p == 0)
                        break;
                  pe->patch->addItem(p->name);
                  }
            ++idx;
            }
      }

//---------------------------------------------------------
//   showMixer
//---------------------------------------------------------

void MuseScore::showMixer(bool val)
      {
      if (iledit == 0)
            iledit = new InstrumentListEditor(0);
      iledit->updateAll(cs);
      iledit->setShown(val);
      }

//---------------------------------------------------------
//   patchChanged
//---------------------------------------------------------

void PartEdit::patchChanged(int n)
      {
      const MidiPatch* p = 0;
      for (int idx = 0;; ++idx) {
            p = seq->getPatchInfo(part->useDrumset(), p);
            if (p == 0)
                  break;
            if (idx == n) {
                  channel->program = p->prog;
                  channel->hbank   = p->hbank;
                  channel->lbank   = p->lbank;

                  MidiOutEvent event;
                  int idx    = channel->channel;
                  event.port = part->score()->midiPort(idx);
                  event.type = ME_CONTROLLER | part->score()->midiChannel(idx);

                  if (channel->hbank != -1) {
                        event.a    = CTRL_HBANK;
                        event.b    = channel->hbank;
                        seq->sendEvent(event);
                        }
                  if (channel->lbank != -1) {
                        event.a    = CTRL_LBANK;
                        event.b    = channel->lbank;
                        seq->sendEvent(event);
                        }
                  event.type = ME_PROGRAM | part->score()->midiChannel(idx);
                  event.a    = channel->program;
                  event.b    = part->useDrumset();
                  seq->sendEvent(event);
                  return;
                  }
            }
      printf("patch %d not found\n", n);
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
      if (val) {
            foreach(Part* part, *part->score()->parts()) {
                  Instrument* instr = part->instrument();
                  foreach(Channel* a, instr->channel) {
                        a->soloMute = (channel != a);
                        a->solo = (channel == a);
                        }
                  }
            }
      else {
            foreach(Part* part, *part->score()->parts()) {
                  Instrument* instr = part->instrument();
                  foreach(Channel* a, instr->channel) {
                        a->soloMute = false;
                        a->solo = false;
                        }
                  }
            }
      emit soloChanged();
      }

//---------------------------------------------------------
//   drumsetToggled
//---------------------------------------------------------

void PartEdit::drumsetToggled(bool val)
      {
      part->setUseDrumset(val);
      patch->clear();
      const MidiPatch* p = 0;
      for (;;) {
            p = seq->getPatchInfo(val, p);
            if (p == 0)
                  break;
            patch->addItem(p->name);
            }
      patch->setCurrentIndex(channel->program);
      }

//---------------------------------------------------------
//   updateSolo
//---------------------------------------------------------

void PartEdit::updateSolo()
      {
      solo->setChecked(channel->solo);
      }

//---------------------------------------------------------
//   soloChanged
//---------------------------------------------------------

void InstrumentListEditor::updateSolo()
      {
      emit soloChanged();
      }

