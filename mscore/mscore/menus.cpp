//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: menus.cpp,v 1.46 2006/04/12 14:58:10 wschweer Exp $
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

#include "score.h"
#include "palette.h"
#include "note.h"
#include "chordrest.h"
#include "dynamics.h"
#include "slur.h"
#include "sym.h"
#include "hairpin.h"
#include "canvas.h"
#include "mscore.h"
#include "edittempo.h"
#include "select.h"
#include "tempo.h"
#include "segment.h"
#include "undo.h"
#include "icons.h"
#include "bracket.h"
#include "ottava.h"
#include "trill.h"
#include "pedal.h"
#include "clef.h"
#include "timesig.h"
#include "barline.h"
#include "layoutbreak.h"
#include "timedialog.h"
#include "symboldialog.h"

//---------------------------------------------------------
//   genCreateMenu
//---------------------------------------------------------

QMenu* MuseScore::genCreateMenu()
      {
      QMenu* popup = new QMenu(tr("&Create"));

      popup->addAction(getAction("instruments"));
      popup->addAction(getAction("append-measure"));
      popup->addAction(tr("Measures..."),        this, SLOT(cmdAppendMeasures()));
      popup->addAction(tr("Barlines..."),        this, SLOT(barMenu()));
      popup->addAction(getAction("clefs"));
      popup->addAction(getAction("keys"));
      popup->addAction(getAction("times"));
      popup->addAction(tr("&Lines..."),          this, SLOT(lineMenu()));
      popup->addAction(tr("System Brackets..."), this, SLOT(bracketMenu()));
      popup->addAction(tr("Note Attributes..."), this, SLOT(noteAttributesMenu()));
      popup->addAction(tr("Accidentals..."),     this, SLOT(accidentalsMenu()));
      popup->addAction(getAction("dynamics"));

      QMenu* text = popup->addMenu(tr("Text..."));
      text->addAction(tr("Title"),        this, SLOT(cmdAddTitle()));
      text->addAction(tr("Subtitle"),     this, SLOT(cmdAddSubTitle()));
      text->addAction(tr("Composer"),     this, SLOT(cmdAddComposer()));
      text->addAction(tr("Poet"),         this, SLOT(cmdAddPoet()));
      text->addSeparator();
      text->addAction(getAction("lyrics"));
      text->addAction(getAction("fingering"));
      text->addAction(getAction("expression"));
      text->addAction(getAction("technik"));
      text->addAction(getAction("tempo"));
      text->addAction(getAction("metronome"));

      popup->addAction(getAction("symbols"));
      return popup;
      }

//---------------------------------------------------------
//   symbolMenu
//---------------------------------------------------------

void MuseScore::symbolMenu()
      {
      if (symbolPalette == 0)
            symbolPalette = new SymbolDialog(this);
      symbolPalette->show();
      symbolPalette->raise();
      }

//---------------------------------------------------------
//   clefMenu
//---------------------------------------------------------

void MuseScore::clefMenu()
      {
      if (clefPalette == 0) {
            QScrollArea* sa = new QScrollArea;
            sa->setWindowTitle(tr("MuseScore: Clefs"));
            clefPalette = sa;
            SymbolPalette* sp = new SymbolPalette(4, 4);
            sa->setWidget(sp);
            sp->setGrid(60, 80);
            sp->showStaff(true);
            for (int i = 0; i < 13; ++i) {
                  Clef* k = new ::Clef(0, i);
                  sp->addObject(i,  k, tr(clefTable[i].name));
                  }
            }
      clefPalette->show();
      clefPalette->raise();
      }

//---------------------------------------------------------
//   keyMenu
//---------------------------------------------------------

void MuseScore::keyMenu()
      {
      static const char* keyNames[] = {
            "g-major, e-minor",     "ces",
            "d-major, h-minor",     "ges-major, es-minor",
            "a-major, fis-minor",   "des-minor, b-minor",
            "e-major, cis-minor",   "as-major, f-minor",
            "h-major, gis-minor",   "es-major, c-minor",
            "fis-major, dis-minor", "b-major, g-minor",
            "cis",                  "f-major, d-minor",
            "c-major, a-minor"
            };

      if (keyPalette == 0) {
            keyPalette = new QScrollArea;
            keyPalette->setWindowTitle(tr("MuseScore: Key Signature"));
            SymbolPalette* sp = new SymbolPalette(4, 4);
            sp->setGrid(80, 60);
            ((QScrollArea*)keyPalette)->setWidget(sp);
            sp->showStaff(true);
            for (int i = 0; i < 7; ++i) {
                  KeySig* k = new KeySig(cs);
                  k->setSubtype(i+1);
                  sp->addObject(i * 2,  k, keyNames[i*2]);
                  }
            for (int i = -7; i < 0; ++i) {
                  KeySig* k = new KeySig(cs);
                  k->setSubtype(i);
                  sp->addObject((7 + i) * 2 + 1,  k, keyNames[(7 + i) * 2 + 1]);
                  }
            KeySig* k = new KeySig(cs);
            k->setSubtype(0);
            sp->addObject(14,  k, keyNames[14]);
            k = new KeySig(cs);
            k->setSubtype(0);
            sp->addObject(15,  k, keyNames[14]);
            }
      keyPalette->show();
      keyPalette->raise();
      }

//---------------------------------------------------------
//   timeMenu
//---------------------------------------------------------

void MuseScore::timeMenu()
      {
      if (timePalette == 0)
            timePalette = new TimeDialog(this);
      timePalette->show();
      timePalette->raise();
      }

//---------------------------------------------------------
//   lineMenu
//---------------------------------------------------------

void MuseScore::lineMenu()
      {
      if (linePalette == 0) {
            linePalette = new QScrollArea;
            linePalette->setWindowTitle(tr("MuseScore: Lines"));
            SymbolPalette* sp = new SymbolPalette(4, 4);
            ((QScrollArea*)linePalette)->setWidget(sp);
            sp->setGrid(100, 30);

            double l = _spatium * 8;

            Hairpin* gabel0 = new Hairpin(cs);
            gabel0->setSubtype(0);
            gabel0->setLen(l);
            sp->addObject(0, gabel0, tr("crescendo"));

            Hairpin* gabel1 = new Hairpin(cs);
            gabel1->setSubtype(1);
            gabel1->setLen(l);
            sp->addObject(1, gabel1, tr("diminuendo"));

            Volta* volta1 = new Volta(cs);
            volta1->setLen(l);
            volta1->setSubtype(PRIMA_VOLTA);
            sp->addObject(4, volta1, tr("prima volta"));

            Volta* volta2 = new Volta(cs);
            volta2->setLen(l);
            volta2->setSubtype(SECONDA_VOLTA);
            sp->addObject(5, volta2, tr("seconda volta"));

            Volta* volta3 = new Volta(cs);
            volta3->setLen(l);
            volta3->setSubtype(TERZA_VOLTA);
            sp->addObject(6, volta3, tr("terza volta"));

            Volta* volta4 = new Volta(cs);
            volta4->setLen(l);
            volta4->setSubtype(SECONDA_VOLTA2);
            sp->addObject(7, volta4, tr("seconda volta"));

            Ottava* ottava1 = new Ottava(cs);
            ottava1->setSubtype(0);
            ottava1->setLen(l);
            sp->addObject(8, ottava1, tr("8va"));

            Ottava* ottava2 = new Ottava(cs);
            ottava2->setSubtype(1);
            ottava2->setLen(l);
            sp->addObject(9, ottava2, tr("15va"));

            Ottava* ottava3 = new Ottava(cs);
            ottava3->setSubtype(2);
            ottava3->setLen(l);
            sp->addObject(10, ottava3, tr("8vb"));

            Ottava* ottava4 = new Ottava(cs);
            ottava4->setSubtype(3);
            ottava4->setLen(l);
            sp->addObject(11, ottava4, tr("15vb"));

            Pedal* pedal = new Pedal(cs);
            pedal->setLen(l);
            sp->addObject(12, pedal, tr("pedal"));

            Trill* trill = new Trill(0);
            trill->setLen(l);
            sp->addObject(13, trill, tr("trill line"));

            }
      linePalette->show();
      linePalette->raise();
      }

//---------------------------------------------------------
//   bracketMenu
//---------------------------------------------------------

void MuseScore::bracketMenu()
      {
      if (bracketPalette == 0) {
            bracketPalette = new QScrollArea;
            bracketPalette->setWindowTitle(tr("MuseScore: System Brackets"));
            SymbolPalette* sp = new SymbolPalette(1, 4);
            ((QScrollArea*)bracketPalette)->setWidget(sp);
            sp->setGrid(40, 80);

            Bracket* b1 = new Bracket(cs);
            b1->setSubtype(BRACKET_NORMAL);
            Bracket* b2 = new Bracket(cs);
            b2->setSubtype(BRACKET_AKKOLADE);
            b1->setHeight(_spatium * 7);
            b2->setHeight(_spatium * 7);

            sp->addObject(0, b1, "Bracket");
            sp->addObject(1, b2, "Akkolade");

            }
      bracketPalette->show();
      bracketPalette->raise();
      }

//---------------------------------------------------------
//   noteAttributesMenu
//---------------------------------------------------------

void MuseScore::noteAttributesMenu()
      {
      if (noteAttributesPalette == 0) {
            noteAttributesPalette = new QScrollArea;
            noteAttributesPalette->setWindowTitle(tr("MuseScore: Note Attributes"));
            SymbolPalette* sp = new SymbolPalette(3, 10);
            ((QScrollArea*)noteAttributesPalette)->setWidget(sp);
            sp->setGrid(60, 60);

            for (int i = 0; i < NOTE_ATTRIBUTES; ++i) {
                  NoteAttribute* s = new NoteAttribute(cs);
                  s->setSubtype(i);
                  sp->addObject(i, s, s->name());

                  }
            }
      noteAttributesPalette->show();
      noteAttributesPalette->raise();
      }

//---------------------------------------------------------
//   accidentalsMenu
//---------------------------------------------------------

void MuseScore::accidentalsMenu()
      {
      if (accidentalsPalette == 0) {
            accidentalsPalette = new QScrollArea;
            accidentalsPalette->setWindowTitle(tr("MuseScore: Accidentals"));
            SymbolPalette* sp = new SymbolPalette(2, 8);
            ((QScrollArea*)accidentalsPalette)->setWidget(sp);
            sp->setGrid(60, 60);

            for (int i = 0; i < 16; ++i) {
                  Accidental* s = new Accidental(cs);
                  s->setSubtype(i);
                  sp->addObject(i, s, s->name());
                  }
            }
      accidentalsPalette->show();
      accidentalsPalette->raise();
      }

//---------------------------------------------------------
//   dynamicsMenu
//---------------------------------------------------------

void MuseScore::dynamicsMenu()
      {
      if (dynamicsPalette == 0) {
            dynamicsPalette = new QScrollArea;
            dynamicsPalette->setWindowTitle(tr("MuseScore: Dynamics"));
            SymbolPalette* sp = new SymbolPalette(6, 6);
            ((QScrollArea*)dynamicsPalette)->setWidget(sp);
            sp->setGrid(90, 40);

            for (int i = 0; i < 27; ++i) {
                  Dynamic* dynamic = new Dynamic(cs, i+1);
                  sp->addObject(i, dynamic, dynamic->subtypeName());
                  }
            Dynamic* d = new Dynamic(cs, "crescendo");
            sp->addObject(27, d,  "crescendo");

            sp->addObject(28, new Dynamic(cs, "diminuendo"), "diminuendo");
            sp->addObject(29, new Dynamic(cs, "dolce"),      "dolce");
            sp->addObject(30, new Dynamic(cs, "espressivo"), "espessivo");
            sp->addObject(31, new Dynamic(cs, "legato"),   "legato");
            sp->addObject(32, new Dynamic(cs, "leggiero"), "leggiero");
            sp->addObject(33, new Dynamic(cs, "marcato"),  "marcato");
            sp->addObject(34, new Dynamic(cs, "mero"),     "mero");
            sp->addObject(35, new Dynamic(cs, "molto"),    "molto");
//            sp->addObject(36, new Dynamic(cs, "morendo"),  "morendo");
//            sp->addObject(37, new Dynamic(cs, "calando"),  "calando");

            }
      dynamicsPalette->show();
      dynamicsPalette->raise();
      }

//---------------------------------------------------------
//   barMenu
//---------------------------------------------------------

void MuseScore::barMenu()
      {
      if (barPalette == 0) {
            barPalette = new QScrollArea;
            barPalette->setWindowTitle(tr("MuseScore: Barlines"));
            SymbolPalette* sp = new SymbolPalette(2, 4);
            ((QScrollArea*)barPalette)->setWidget(sp);
            sp->setGrid(60, 60);
            sp->showStaff(true);

            struct {
                  BarType type;
                  QString name;
                  } t[] = {
                  { INVISIBLE_BAR, "invisible" },
                  { BROKEN_BAR,    "broken" },
                  { NORMAL_BAR,    "normal" },
                  { END_BAR,       "End Bar" },
                  { DOUBLE_BAR,    "Double Bar" },
                  { START_REPEAT,  "Start Repeat" },
                  { END_REPEAT,    "End Repeat" },
                  };
            for (unsigned i = 0; i < sizeof(t)/sizeof(*t); ++i) {
                  BarLine* b  = new BarLine(cs);
                  b->setHeight(point(Spatium(4)));
                  b->setSubtype(t[i].type);
                  sp->addObject(i,  b, t[i].name);
                  }

            }
      barPalette->show();
      barPalette->raise();
      }

//---------------------------------------------------------
//   fingeringMenu
//---------------------------------------------------------

void MuseScore::fingeringMenu()
      {
      if (fingeringPalette == 0) {
            QScrollArea* sa = new QScrollArea;
            sa->setWindowTitle(tr("MuseScore: Fingering"));
            fingeringPalette = sa;
            SymbolPalette* sp = new SymbolPalette(2, 6, 1.5);
            sa->setWidget(sp);
            sp->setGrid(50, 50);

            const char finger[] = "012345pimac";
            Text* k;

            for (unsigned i = 0; i < strlen(finger); ++i) {
                  k = new Text(0);
                  k->setSubtype(TEXT_FINGERING);
                  k->setText(QString(finger[i]));
                  sp->addObject(i, k, QString("fingering %1").arg(finger[i]));
                  }
            }
      fingeringPalette->show();
      fingeringPalette->raise();
      }

//---------------------------------------------------------
//   addTempo
//---------------------------------------------------------

void Score::addTempo()
      {
      Element* e = sel->element();
      if (!e) {
            printf("no element selected\n");
            endCmd(true);
            return;
            }
      if (e->type() == NOTE)
            e = e->parent();
      if (e->type() != CHORD && e->type() != REST) {
            printf("no Chord/Rest selected\n");
            endCmd(true);
            return;
            }
      Measure* m = ((ChordRest*)e)->segment()->measure();
      int tick = e->tick();
      if (editTempo == 0)
            editTempo = new EditTempo(0);
      int rv = editTempo->exec();
      if (rv == 1) {
            double bpm = editTempo->bpm();
printf("tempo at %d(%s)  %s %f\n",
   e->tick(), e->name(), editTempo->text().toLatin1().data(), editTempo->bpm());
            tempomap->addTempo(tick, int(60000000.0/double(bpm)));
            TempoText* tt = new TempoText(this);
            tt->setTick(tick);
            tt->setStaff(e->staff());
            tt->setText(editTempo->text());
            tt->setTempo(bpm);
            tt->setParent(m);
            cmdAdd(tt);
            refresh |= tt->abbox();
            endCmd(true);
            }
      }

//---------------------------------------------------------
//   addMetronome
//---------------------------------------------------------

void Score::addMetronome()
      {
      printf("addMetronome: not implemented\n");
      endCmd(true);
      }

//---------------------------------------------------------
//   showLayoutBreakPalette
//---------------------------------------------------------

void MuseScore::showLayoutBreakPalette()
      {
      if (layoutBreakPalette == 0) {
            QScrollArea* sa = new QScrollArea;
            sa->setWindowTitle(tr("MuseScore: Layout Breaks"));
            layoutBreakPalette = sa;
            SymbolPalette* sp = new SymbolPalette(1, 3);
            sa->setWidget(sp);
            sp->setGrid(80, 80);
            LayoutBreak* lb = new LayoutBreak(cs);
            lb->setSubtype(LAYOUT_BREAK_LINE);
            sp->addObject(0, lb, tr("break line"));
            lb = new LayoutBreak(cs);
            lb->setSubtype(LAYOUT_BREAK_PAGE);
            sp->addObject(1, lb, tr("break page"));
            }
      layoutBreakPalette->show();
      layoutBreakPalette->raise();
      }


