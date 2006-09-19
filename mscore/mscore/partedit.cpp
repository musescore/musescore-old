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
      connect(volume,   SIGNAL(valueChanged(int)), SLOT(volChanged(int)));
      connect(pan,      SIGNAL(valueChanged(int)), SLOT(panChanged(int)));
      connect(chorus,   SIGNAL(valueChanged(int)), SLOT(chorusChanged(int)));
      connect(reverb,   SIGNAL(valueChanged(int)), SLOT(reverbChanged(int)));
      connect(mute,     SIGNAL(toggled(bool)),     SLOT(muteChanged(bool)));
      connect(solo,     SIGNAL(toggled(bool)),     SLOT(soloChanged(bool)));
      connect(showPart, SIGNAL(toggled(bool)),     SLOT(showPartChanged(bool)));
      connect(channel,  SIGNAL(valueChanged(int)), SLOT(channelChanged(int)));
      connect(minPitch, SIGNAL(valueChanged(int)), SLOT(minPitchChanged(int)));
      connect(maxPitch, SIGNAL(valueChanged(int)), SLOT(maxPitchChanged(int)));
      connect(partName, SIGNAL(textChanged(const QString&)), SLOT(partNameChanged(const QString&)));
      connect(shortName, SIGNAL(textChanged(const QString&)), SLOT(shortNameChanged(const QString&)));
      connect(longName, SIGNAL(textChanged(const QString&)), SLOT(longNameChanged(const QString&)));
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
      showPart->setChecked(i->show);
      channel->setValue(i->midiChannel);
      volume->setValue(i->volume);
      reverb->setValue(i->reverb);
      chorus->setValue(i->chorus);
      pan->setValue(i->pan);
      patch->setCurrentIndex(i->midiProgram);   // TODO!
      shortName->setText(part->shortName().text());
      longName->setText(part->longName().text());
      minPitch->setValue(i->minPitch);
      maxPitch->setValue(i->maxPitch);
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
      PartList* pl = cs->parts();

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
      for (iPart ip = pl->begin(); ip != pl->end(); ++ip, ++idx) {
            QWidgetItem* wi = (QWidgetItem*)(vb->itemAt(idx));
            PartEdit* pe    = (PartEdit*)(wi->widget());
            pe->setPart(*ip);
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

void PartEdit::volChanged(int val)
      {
// printf("volChanged %d\n", val);
      Synth* synti = seq->synth();
      Instrument* i = part->instrument();
      synti->setController(i->midiChannel, CTRL_VOLUME, val);
      i->volume = val;
      }

//---------------------------------------------------------
//   panChanged
//---------------------------------------------------------

void PartEdit::panChanged(int val)
      {
// printf("panChanged %d\n", val);
      Synth* synti = seq->synth();
      Instrument* i = part->instrument();
      synti->setController(i->midiChannel, CTRL_PAN, val);
      i->pan = val;
      }

//---------------------------------------------------------
//   reverbChanged
//---------------------------------------------------------

void PartEdit::reverbChanged(int val)
      {
// printf("reverbChanged %d\n", val);
      Synth* synti = seq->synth();
      Instrument* i = part->instrument();
      synti->setController(i->midiChannel, CTRL_REVERB_SEND, val);
      i->reverb = val;
      }

//---------------------------------------------------------
//   chorusChanged
//---------------------------------------------------------

void PartEdit::chorusChanged(int val)
      {
// printf("chorusChanged %d\n", val);
      Synth* synti = seq->synth();
      Instrument* i = part->instrument();
      synti->setController(i->midiChannel, CTRL_CHORUS_SEND, val);
      i->chorus = val;
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
      Instrument* i = part->instrument();
      i->show = val;
      }

void PartEdit::channelChanged(int val)
      {
      Instrument* i = part->instrument();
      i->midiChannel = val;
      }

void PartEdit::minPitchChanged(int val)
      {
      Instrument* i = part->instrument();
      i->minPitch = val;
      }

void PartEdit::maxPitchChanged(int val)
      {
      Instrument* i = part->instrument();
      i->maxPitch = val;
      }

void PartEdit::partNameChanged(const QString& s)
      {
      part->setTrackName(s);
      }

void PartEdit::shortNameChanged(const QString& s)
      {
//TODO      part->setShortName(s);
      }

void PartEdit::longNameChanged(const QString& s)
      {
//TODO      part->setLongName(s);
      }

