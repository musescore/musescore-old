//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: partedit.cpp,v 1.4 2006/03/13 21:35:59 wschweer Exp $
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
#include "score.h"
#include "part.h"
#include "partedit.h"
#include "seq.h"
#include "synti.h"

//---------------------------------------------------------
//   PartEdit
//---------------------------------------------------------

PartEdit::PartEdit()
   : QWidget()
      {
      setupUi(this);
      connect(patch,    SIGNAL(activated(int)),    SLOT(patchChanged(int)));
      connect(volume,   SIGNAL(valueChanged(double,int)), SLOT(volChanged(double)));
      connect(pan,      SIGNAL(valueChanged(double,int)), SLOT(panChanged(double)));
      connect(chorus,   SIGNAL(valueChanged(double,int)), SLOT(chorusChanged(double)));
      connect(reverb,   SIGNAL(valueChanged(double,int)), SLOT(reverbChanged(double)));
      connect(mute,     SIGNAL(toggled(bool)),     SLOT(muteChanged(bool)));
      connect(solo,     SIGNAL(toggled(bool)),     SLOT(soloChanged(bool)));
      connect(showPart, SIGNAL(toggled(bool)),     SLOT(showPartChanged(bool)));
      }

//---------------------------------------------------------
//   setPart
//---------------------------------------------------------

void PartEdit::setPart(Part* p)
      {
      part = p;
      const Instrument* i = part->instrument();
      partName->setText(part->trackName());
      mute->setChecked(i->mute);
      solo->setChecked(i->solo);
      showPart->setChecked(part->show());
      volume->setValue(i->volume);
      reverb->setValue(i->reverb);
      chorus->setValue(i->chorus);
      pan->setValue(i->pan);
      patch->setCurrentIndex(i->midiProgram);
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
//   updateAll
//---------------------------------------------------------

void InstrumentListEditor::updateAll(Score* cs)
      {
      QList<Part*>* pl = cs->parts();

      int n = pl->size() - vb->count();
      while (n < 0) {
            QWidgetItem* wi = (QWidgetItem*)(vb->itemAt(0));
            vb->removeItem(wi);
            delete wi->widget();
            delete wi;
            ++n;
            }
      while (n > 0) {
            PartEdit* pe = new PartEdit;
            vb->addWidget(pe);
            Synth* synti = seq->synth();
            const MidiPatch* p = 0;
            while (synti) {
                  p = synti->getPatchInfo(0, p);
                  if (p == 0)
                        break;
                  pe->patch->addItem(p->name);
                  }
            --n;
            }
      QString s;
      int idx = 0;
      foreach (Part* part, *pl) {
            QWidgetItem* wi = (QWidgetItem*)(vb->itemAt(idx));
            PartEdit* pe    = (PartEdit*)(wi->widget());
            pe->setPart(part);
            ++idx;
            }
      }

//---------------------------------------------------------
//   startInstrumentEditor
//---------------------------------------------------------

void MuseScore::startInstrumentListEditor()
      {
      if (iledit == 0)
            iledit = new InstrumentListEditor(0);
      iledit->updateAll(cs);
      iledit->show();
      }

//---------------------------------------------------------
//   patchChanged
//---------------------------------------------------------

void PartEdit::patchChanged(int n)
      {
      Synth* synti = seq->synth();
      const MidiPatch* p = 0;
      for (int idx = 0; synti; ++idx) {
            p = synti->getPatchInfo(0, p);
            if (p == 0)
                  break;
            if (idx == n) {
                  Instrument* i = part->instrument();
                  synti->setController(i->midiChannel, CTRL_PROGRAM, p->prog);
                  i->midiProgram = p->prog;
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   volChanged
//---------------------------------------------------------

void PartEdit::volChanged(double val)
      {
// printf("volChanged %d\n", lrint(val));
      Synth* synti = seq->synth();
      Instrument* i = part->instrument();
      int iv = lrint(val);
      synti->setController(i->midiChannel, CTRL_VOLUME, iv);
      i->volume = iv;
      }

//---------------------------------------------------------
//   panChanged
//---------------------------------------------------------

void PartEdit::panChanged(double val)
      {
// printf("panChanged %d\n", lrint(val));
      Synth* synti = seq->synth();
      Instrument* i = part->instrument();
      int iv = lrint(val);
      synti->setController(i->midiChannel, CTRL_PAN, iv);
      i->pan = iv;
      }

//---------------------------------------------------------
//   reverbChanged
//---------------------------------------------------------

void PartEdit::reverbChanged(double val)
      {
// printf("reverbChanged %d\n", lrint(val));
      Synth* synti = seq->synth();
      Instrument* i = part->instrument();
      int iv = lrint(val);
      synti->setController(i->midiChannel, CTRL_REVERB_SEND, iv);
      i->reverb = iv;
      }

//---------------------------------------------------------
//   chorusChanged
//---------------------------------------------------------

void PartEdit::chorusChanged(double val)
      {
// printf("chorusChanged %d\n", lrint(val));
      Synth* synti = seq->synth();
      Instrument* i = part->instrument();
      int iv = lrint(val);
      synti->setController(i->midiChannel, CTRL_CHORUS_SEND, iv);
      i->chorus = iv;
      }

void PartEdit::muteChanged(bool val)
      {
      Instrument* i = part->instrument();
      i->mute = val;
      }

void PartEdit::soloChanged(bool val)
      {
      Instrument* i = part->instrument();
      i->solo = val;
      }

void PartEdit::showPartChanged(bool val)
      {
      part->setShow(val);
      part->score()->setLayoutAll(true);
      part->score()->end();
      }

