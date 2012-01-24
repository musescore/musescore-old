//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "musescore.h"
#include "masterpalette.h"
#include "symboldialog.h"
#include "palette.h"
#include "libmscore/score.h"
#include "libmscore/clef.h"
#include "libmscore/fingering.h"
#include "libmscore/spacer.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/dynamic.h"
#include "libmscore/bracket.h"
#include "libmscore/arpeggio.h"
#include "libmscore/glissando.h"
#include "libmscore/repeat.h"
#include "libmscore/breath.h"
#include "libmscore/harmony.h"
#include "libmscore/text.h"
#include "libmscore/tempotext.h"
#include "libmscore/instrchange.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/stafftext.h"
#include "libmscore/note.h"
#include "libmscore/tremolo.h"
#include "libmscore/chordline.h"

extern void populateIconPalette(Palette* p, const IconAction* a);
extern Palette* newKeySigPalette();
extern Palette* newBarLinePalette();
extern Palette* newLinesPalette();
extern Palette* newAccidentalsPalette();

//---------------------------------------------------------
//   showMasterPalette
//---------------------------------------------------------

void MuseScore::showMasterPalette()
      {
      QAction* a = getAction("masterpalette");

      if (masterPalette == 0) {
            masterPalette = new MasterPalette(0);
            connect(masterPalette, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
            }
      masterPalette->setShown(a->isChecked());
      }

//---------------------------------------------------------
//   createPalette
//---------------------------------------------------------

Palette* MasterPalette::createPalette(int w, int h, bool grid, double mag)
      {
      Palette* sp = new Palette;
      PaletteScrollArea* psa = new PaletteScrollArea(sp);
      psa->setRestrictHeight(false);
      sp->setMag(mag);
      sp->setGrid(w, h);
      sp->setDrawGrid(grid);
      stack->addWidget(psa);
      return sp;
      }

//---------------------------------------------------------
//   MasterPalette
//---------------------------------------------------------

MasterPalette::MasterPalette(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      double _spatium = gscore->spatium();

      //
      //   Grace notes
      //
      Palette* sp = createPalette(32, 40, true);
      static const IconAction gna[] = {
            { ICON_ACCIACCATURA, "acciaccatura" },
            { ICON_APPOGGIATURA, "appoggiatura" },
            { ICON_GRACE4,       "grace4" },
            { ICON_GRACE16,      "grace16" },
            { ICON_GRACE32,      "grace32" },
            { ICON_GRACE8B,      "grace8b" },
            { -1, "" }
            };
      populateIconPalette(sp, gna);

      //
      //   Clefs
      //
      sp = createPalette(60, 80, false);
      for (int i = 0; i < CLEF_MAX; ++i) {
            Clef* k = new ::Clef(gscore);
            k->setClefType(ClefType(i));
            sp->append(k, qApp->translate("clefTable", clefTable[i].name));
            }

      //
      //   KeySigs
      //
      stack->addWidget(newKeySigPalette());

      //
      //   TimeSig
      //
      stack->addWidget(new TimeDialog);

      //
      //   BarLines
      //
      PaletteScrollArea* psa = new PaletteScrollArea(newBarLinePalette());
      psa->setRestrictHeight(false);
      stack->addWidget(psa);

      //
      //   Lines
      //
      psa = new PaletteScrollArea(newLinesPalette());
      psa->setRestrictHeight(false);
      stack->addWidget(psa);

      //
      //    Arpeggio && Glissando
      //

      sp = createPalette(27, 60, false);
      for (int i = 0; i < 4; ++i) {
            Arpeggio* a = new Arpeggio(gscore);
            a->setSubtype(ArpeggioType(i));
            sp->append(a, tr("Arpeggio"));
            }
      for (int i = 0; i < 2; ++i) {
            Glissando* a = new Glissando(gscore);
            a->setSubtype(i);
            sp->append(a, tr("Glissando"));
            }

      //
      //    Symbols: Breath
      //

      sp = createPalette(42, 40, false);
      for (int i = 0; i < 4; ++i) {
            Breath* a = new Breath(gscore);
            a->setSubtype(i);
            if (i < 2)
                  sp->append(a, tr("Breath"));
            else
                  sp->append(a, tr("Caesura"));
            }

      //
      //   Brackets
      //
      sp = createPalette(40, 80, true);
      Bracket* b1 = new Bracket(gscore);
      b1->setSubtype(BRACKET_NORMAL);
      Bracket* b2 = new Bracket(gscore);
      b2->setSubtype(BRACKET_AKKOLADE);
      b1->setHeight(_spatium * 7);
      b2->setHeight(_spatium * 7);
      sp->append(b1, tr("Bracket"));
      sp->append(b2, tr("Akkolade"));

      //
      //   Articulation
      //
      sp = createPalette(42, 30, true);
      for (unsigned i = 0; i < ARTICULATIONS; ++i) {
            Articulation* s = new Articulation(gscore);
            s->setSubtype(ArticulationType(i));
            sp->append(s, qApp->translate("articulation", qPrintable(s->subtypeName())));
            }

      //
      //   Accidentals
      //
      psa = new PaletteScrollArea(newAccidentalsPalette());
      psa->setRestrictHeight(false);
      stack->addWidget(psa);

      //
      //   Dynamics
      //
      psa = new PaletteScrollArea(MuseScore::newDynamicsPalette());
      psa->setRestrictHeight(false);
      stack->addWidget(psa);

      //
      //   Fingering
      //
      sp = createPalette(28, 30, true, 1.5);
      const char finger[] = "012345pimac";
      for (unsigned i = 0; i < strlen(finger); ++i) {
            Fingering* f = new Fingering(gscore);
            f->setText(QString(finger[i]));
            sp->append(f, tr("Fingering %1").arg(finger[i]));
            }
      const char stringnumber[] = "0123456";
      for (unsigned i = 0; i < strlen(stringnumber); ++i) {
            Fingering* f = new Fingering(gscore);
            f->setTextStyle(TEXT_STYLE_STRING_NUMBER);
            f->setText(QString(stringnumber[i]));
            sp->append(f, tr("String number %1").arg(stringnumber[i]));
            }

      //-----------------------------------
      //    Noteheads
      //-----------------------------------

      sp = createPalette(33, 36, true, 1.3);
      for (int i = 0; i < HEAD_GROUPS; ++i) {
            int sym = noteHeads[0][i][1];
            if (i == HEAD_BREVIS_ALT)
                  sym = noteHeads[0][i][3];
            NoteHead* nh = new NoteHead(gscore);
            nh->setSym(sym);
            sp->append(nh, qApp->translate("symbol", symbols[0][sym].name()));
            }

      //-----------------------------------
      //    Tremolo
      //-----------------------------------

      sp = createPalette(27, 40, true);
      const char* tremoloName[] = {
            QT_TR_NOOP("1/8 through stem"),
            QT_TR_NOOP("1/16 through stem"),
            QT_TR_NOOP("1/32 through stem"),
            QT_TR_NOOP("1/64 through stem"),
            QT_TR_NOOP("1/8 between notes"),
            QT_TR_NOOP("1/16 between notes"),
            QT_TR_NOOP("1/32 between notes"),
            QT_TR_NOOP("1/64 between notes")
            };
      for (int i = TREMOLO_R8; i <= TREMOLO_C64; ++i) {
            Tremolo* tremolo = new Tremolo(gscore);
            tremolo->setSubtype(TremoloType(i));
            sp->append(tremolo, tr(tremoloName[i - TREMOLO_R8]));
            }

      //-----------------------------------
      //    Fall, Doit
      //-----------------------------------

      sp = createPalette(27, 40, true);
      const char* scorelineNames[] = {
            QT_TR_NOOP("fall"),
            QT_TR_NOOP("doit"),
            };
      ChordLine* cl = new ChordLine(gscore);
      cl->setSubtype(1);
      sp->append(cl, tr(scorelineNames[0]));

      cl = new ChordLine(gscore);
      cl->setSubtype(2);
      sp->append(cl, tr(scorelineNames[1]));

      //-----------------------------------
      //    Repeats
      //-----------------------------------

      psa = new PaletteScrollArea(MuseScore::newRepeatsPalette());
      psa->setRestrictHeight(false);
      stack->addWidget(psa);

      //-----------------------------------
      //    Text
      //-----------------------------------

      psa = new PaletteScrollArea(MuseScore::newTextPalette());
      psa->setRestrictHeight(false);
      stack->addWidget(psa);

      //-----------------------------------
      //   Breaks
      //-----------------------------------

      psa = new PaletteScrollArea(MuseScore::newBreaksPalette());
      psa->setRestrictHeight(false);
      stack->addWidget(psa);

      //-----------------------------------
      //    beam properties
      //-----------------------------------

      psa = new PaletteScrollArea(MuseScore::newBeamPalette());
      psa->setRestrictHeight(false);
      stack->addWidget(psa);

      //
      //   Symbols
      //
      stack->addWidget(new SymbolDialog);
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MasterPalette::closeEvent(QCloseEvent* ev)
      {
      emit closed(false);
      QWidget::closeEvent(ev);
      }


