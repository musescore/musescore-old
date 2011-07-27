//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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
#include "scoreview.h"
#include "mscore.h"
#include "edittempo.h"
#include "select.h"
#include "al/tempo.h"
#include "segment.h"
#include "undo.h"
#include "icons.h"
#include "bracket.h"
#include "ottava.h"
#include "textline.h"
#include "trill.h"
#include "pedal.h"
#include "clef.h"
#include "timesig.h"
#include "barline.h"
#include "layoutbreak.h"
#include "timedialog.h"
#include "symboldialog.h"
#include "volta.h"
#include "keysig.h"
#include "breath.h"
#include "arpeggio.h"
#include "tremolo.h"
#include "repeat.h"
#include "tempotext.h"
#include "glissando.h"
#include "articulation.h"
#include "chord.h"
#include "drumset.h"
#include "spacer.h"
#include "measure.h"

extern bool useFactorySettings;

//---------------------------------------------------------
//   showPalette
//---------------------------------------------------------

void MuseScore::showPalette(bool visible)
      {
      QAction* a = getAction("toggle-palette");
      if (paletteBox == 0) {
            paletteBox = new PaletteBox(this);

            connect(paletteBox, SIGNAL(paletteVisible(bool)), a, SLOT(setChecked(bool)));
            addDockWidget(Qt::LeftDockWidgetArea, paletteBox);

            if (!useFactorySettings) {
                  QFile f(dataPath + "/" + "mscore-palette.xml");
                  if (f.exists()) {
                        if (paletteBox->read(&f)) {
                              paletteBox->setShown(visible);
                              a->setChecked(visible);
                              return;
                              }
                        }
                  }

            //-----------------------------------
            //    notes
            //-----------------------------------

            Palette* notePalette = new Palette;
            notePalette->setName(tr("Grace Notes"));
            notePalette->setGrid(27, 40);
            notePalette->setDrawGrid(true);

            Icon* ik = new Icon(gscore);
            ik->setSubtype(ICON_ACCIACCATURA);
            ik->setAction(getAction("pad-acciaccatura"));
            notePalette->append(ik, tr("Acciaccatura"));

            ik = new Icon(gscore);
            ik->setSubtype(ICON_APPOGGIATURA);
            ik->setAction(getAction("pad-appoggiatura"));
            notePalette->append(ik, tr("Appoggiatura"));

            ik = new Icon(gscore);
            ik->setSubtype(ICON_GRACE4);
            ik->setAction(getAction("pad-grace4"));
            notePalette->append(ik, tr("Quarter grace note"));

            ik = new Icon(gscore);
            ik->setSubtype(ICON_GRACE16);
            ik->setAction(getAction("pad-grace16"));
            notePalette->append(ik, tr("16th grace note"));

            ik = new Icon(gscore);
            ik->setSubtype(ICON_GRACE32);
            ik->setAction(getAction("pad-grace32"));
            notePalette->append(ik, tr("32nd grace note"));

            paletteBox->addPalette(notePalette);

            //-----------------------------------
            //    drums
            //-----------------------------------

            drumPalette = new Palette;
            drumPalette->setName(tr("Drums"));
            drumPalette->setMag(0.8);
            drumPalette->setSelectable(true);
            drumPalette->setGrid(42, 60);
            drumPalette->setDrumPalette(true);

            paletteBox->addPalette(drumPalette);

            //-----------------------------------
            //    clefs
            //-----------------------------------

            Palette* sp = new Palette;
            sp->setName(tr("Clefs"));
            sp->setMag(0.8);
            sp->setGrid(33, 60);
            sp->setYOffset(1.0);
            int clefs[20] = {
                  0, 1, 2, 3, 16, 9, 10, 11, 12, 15, 4, 17, 18, 5, 6, 7, 8, 14, 13, CLEF_PERC2
                  };

            for (int i = 0; i < 20; ++i) {
                  int j = clefs[i];
                  Clef* k = new ::Clef(gscore, j);
                  sp->append(k, qApp->translate("clefTable", clefTable[j].name));
                  }
            paletteBox->addPalette(sp);

            //-----------------------------------
            //    key signatures
            //-----------------------------------

            sp = new Palette;
            sp->setName(tr("Key Signatures"));
            sp->setMag(0.8);
            sp->setGrid(56, 45);
            sp->setYOffset(6.0);

            for (int i = 0; i < 7; ++i) {
                  KeySig* k = new KeySig(gscore);
                  k->setSubtype(i+1);
                  sp->append(k, qApp->translate("MuseScore", keyNames[i*2]));
                  }
            for (int i = -7; i < 0; ++i) {
                  KeySig* k = new KeySig(gscore);
                  k->setSubtype(i & 0xff);
                  sp->append(k, qApp->translate("MuseScore", keyNames[(7 + i) * 2 + 1]));
                  }
            KeySig* k = new KeySig(gscore);
            k->setSubtype(0);
            sp->append(k, qApp->translate("MuseScore", keyNames[14]));
            paletteBox->addPalette(sp);

            //-----------------------------------
            //    Time
            //-----------------------------------

            sp = new Palette;
            sp->setName(tr("Time Signatures"));
            sp->setMag(.8);
            sp->setGrid(42, 38);

            sp->append(new TimeSig(gscore, 2, 2), "2/2");
            sp->append(new TimeSig(gscore, 4, 2), "2/4");
            sp->append(new TimeSig(gscore, 4, 3), "3/4");
            sp->append(new TimeSig(gscore, 4, 4), "4/4");
            sp->append(new TimeSig(gscore, 4, 5), "5/4");
            sp->append(new TimeSig(gscore, 4, 6), "6/4");
            sp->append(new TimeSig(gscore, 8, 3), "3/8");
            sp->append(new TimeSig(gscore, 8, 6), "6/8");
            sp->append(new TimeSig(gscore, 8, 9), "9/8");
            sp->append(new TimeSig(gscore, 8, 12), "12/8");
            sp->append(new TimeSig(gscore, TSIG_FOUR_FOUR), tr("4/4 common time"));
            sp->append(new TimeSig(gscore, TSIG_ALLA_BREVE), tr("2/2 alla breve"));
            paletteBox->addPalette(sp);

            //-----------------------------------
            //    Bar Lines
            //-----------------------------------

            sp = new Palette;
            sp->setName(tr("Barlines"));
            sp->setMag(0.8);
            sp->setGrid(42, 38);

            struct {
                  BarType type;
                  const char* name;
                  } t[] = {
                  { NORMAL_BAR,       QT_TR_NOOP("Normal") },
                  { BROKEN_BAR,       QT_TR_NOOP("Dashed") },
                  { END_BAR,          QT_TR_NOOP("End Bar") },
                  { DOUBLE_BAR,       QT_TR_NOOP("Double Bar") },
                  { START_REPEAT,     QT_TR_NOOP("Start Repeat") },
                  { END_REPEAT,       QT_TR_NOOP("End Repeat") },
                  { END_START_REPEAT, QT_TR_NOOP("End-Start Repeat") },
                  };
            for (unsigned i = 0; i < sizeof(t)/sizeof(*t); ++i) {
                  BarLine* b  = new BarLine(gscore);
                  b->setSubtype(t[i].type);
                  sp->append(b, tr(t[i].name));
                  }
            paletteBox->addPalette(sp);

            //-----------------------------------
            //    Lines
            //-----------------------------------

            sp = new Palette;
            sp->setName(tr("Lines"));
            sp->setMag(.8);
            sp->setGrid(84, 23);

            Slur* slur = new Slur(gscore);
            slur->setId(0);
            sp->append(slur, tr("Slur"));

            Hairpin* gabel0 = new Hairpin(gscore);
            gabel0->setSubtype(0);
            sp->append(gabel0, tr("Crescendo"));

            Hairpin* gabel1 = new Hairpin(gscore);
            gabel1->setSubtype(1);
            sp->append(gabel1, tr("Diminuendo"));

            Volta* volta = new Volta(gscore);
            volta->setSubtype(Volta::VOLTA_CLOSED);
            volta->setText("1.");
            QList<int> il;
            il.append(1);
            volta->setEndings(il);
            sp->append(volta, tr("Prima volta"));

            volta = new Volta(gscore);
            volta->setSubtype(Volta::VOLTA_CLOSED);
            volta->setText("2.");
            il.clear();
            il.append(2);
            volta->setEndings(il);
            sp->append(volta, tr("Seconda volta"));

            volta = new Volta(gscore);
            volta->setSubtype(Volta::VOLTA_CLOSED);
            volta->setText("3.");
            il.clear();
            il.append(3);
            volta->setEndings(il);
            sp->append(volta, tr("Terza volta"));

            volta = new Volta(gscore);
            volta->setSubtype(Volta::VOLTA_OPEN);
            volta->setText("2.");
            il.clear();
            il.append(2);
            volta->setEndings(il);
            sp->append(volta, tr("Seconda volta 2"));

            Ottava* ottava = new Ottava(gscore);
            ottava->setSubtype(0);
            sp->append(ottava, tr("8va"));

            ottava = new Ottava(gscore);
            ottava->setSubtype(1);
            sp->append(ottava, tr("15ma"));

            ottava = new Ottava(gscore);
            ottava->setSubtype(2);
            sp->append(ottava, tr("8vb"));

            ottava = new Ottava(gscore);
            ottava->setSubtype(3);
            sp->append(ottava, tr("15mb"));

            Pedal* pedal = new Pedal(gscore);
            sp->append(pedal, tr("pedal"));

            Trill* trill = new Trill(gscore);
            sp->append(trill, tr("Trill line"));

            TextLine* textLine = new TextLine(gscore);
            textLine->setBeginText("VII");
            textLine->setEndHook(true);
            textLine->setEndHookHeight(Spatium(1.5));
            sp->append(textLine, tr("Text line"));

            TextLine* line = new TextLine(gscore);
            line->setDiagonal(true);
            sp->append(line, tr("Line"));

            paletteBox->addPalette(sp);

            //-----------------------------------
            //    Arpeggios
            //-----------------------------------

            sp = new Palette();
            sp->setName(tr("Arpeggio && Glissando"));
            sp->setGrid(27, 60);

            for (int i = 0; i < 4; ++i) {
                  Arpeggio* a = new Arpeggio(gscore);
                  a->setSubtype(i);
                  sp->append(a, tr("Arpeggio"));
                  }
            for (int i = 0; i < 2; ++i) {
                  Glissando* a = new Glissando(gscore);
                  a->setSubtype(i);
                  sp->append(a, tr("Glissando"));
                  }
            paletteBox->addPalette(sp);

            //-----------------------------------
            //    Symbols: Breath
            //-----------------------------------

            sp = new Palette();
            sp->setName(tr("Breath && Pauses"));
            sp->setGrid(42, 40);

            for (int i = 0; i < 4; ++i) {
                  Breath* a = new Breath(gscore);
                  a->setSubtype(i);
                  if (i < 2)
                        sp->append(a, tr("Breath"));
                  else
                        sp->append(a, tr("Caesura"));
                  }

            paletteBox->addPalette(sp);

            //-----------------------------------
            //    Brackets
            //-----------------------------------

            sp = new Palette;
            sp->setName(tr("Brackets"));
            sp->setMag(0.7);
            sp->setGrid(42, 60);

            Bracket* b1 = new Bracket(gscore);
            b1->setSubtype(BRACKET_NORMAL);
            Bracket* b2 = new Bracket(gscore);
            b2->setSubtype(BRACKET_AKKOLADE);
            // b1->setHeight(_spatium * 7);
            // b2->setHeight(_spatium * 7);

            sp->append(b1, tr("Bracket"));
            sp->append(b2, tr("Akkolade"));

            paletteBox->addPalette(sp);

            //-----------------------------------
            //    Attributes, Ornaments
            //-----------------------------------

            unsigned nn = ARTICULATIONS;
            sp = new Palette;
            sp->setName(tr("Articulations && Ornaments"));
            sp->setGrid(42, 25);

            for (unsigned i = 0; i < nn; ++i) {
                  Articulation* s = new Articulation(gscore);
                  s->setSubtype(i);
                  sp->append(s, qApp->translate("articulation", qPrintable(s->subtypeName())));
                  }
            paletteBox->addPalette(sp);

            //-----------------------------------
            //    Accidentals
            //-----------------------------------

            sp = new Palette;
            sp->setName(tr("Accidentals"));
            sp->setGrid(33, 36);

            for (int i = ACC_SHARP; i < ACC_END; ++i) {
                  Accidental* s = new Accidental(gscore);
                  s->setSubtype(i);
                  sp->append(s, qApp->translate("accidental", s->subtypeUserName()));
                  }
            AccidentalBracket* ab = new AccidentalBracket(gscore);
            ab->setSubtype(0);
            sp->append(ab, qApp->translate("Accidental", "round bracket"));
            paletteBox->addPalette(sp);

            //-----------------------------------
            //    Dynamics
            //-----------------------------------

            sp = new Palette;
            sp->setName(tr("Dynamics"));
            sp->setMag(.8);
            sp->setGrid(42, 28);
//            sp->setYOffset(-12.0);

            static const char* dynS[] = {
                  "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff"
                  };
            for (unsigned i = 0; i < sizeof(dynS)/sizeof(*dynS); ++i) {
                  Dynamic* dynamic = new Dynamic(gscore);
                  dynamic->setSubtype(dynS[i]);
                  sp->append(dynamic, dynamic->subtypeName());
                  }
            paletteBox->addPalette(sp);

            //-----------------------------------
            //    Fingering
            //-----------------------------------

            sp = new Palette;
            sp->setName(tr("Fingering"));
            sp->setMag(1.5);
            sp->setGrid(28, 30);
            sp->setDrawGrid(true);

            const char finger[] = "012345pimac";
            for (unsigned i = 0; i < strlen(finger); ++i) {
                  Text* k = new Text(gscore);
                  k->setSubtype(TEXT_FINGERING);
                  k->setTextStyle(TEXT_STYLE_FINGERING);
                  k->setText(QString(finger[i]));
                  sp->append(k, tr("Fingering %1").arg(finger[i]));
                  }
            const char stringnumber[] = "0123456";
            for (unsigned i = 0; i < strlen(stringnumber); ++i) {
                  Text* k = new Text(gscore);
                  k->setSubtype(TEXT_STRING_NUMBER);
                  k->setTextStyle(TEXT_STYLE_STRING_NUMBER);
                  k->setText(QString(stringnumber[i]));
                  sp->append(k, tr("String number %1").arg(stringnumber[i]));
                  }

            paletteBox->addPalette(sp);

            //-----------------------------------
            //    Noteheads
            //-----------------------------------

            sp = new Palette;
            sp->setName(tr("Note Heads"));
            sp->setMag(1.3);
            sp->setGrid(33, 36);
            sp->setDrawGrid(true);
            for (int i = 0; i < HEAD_GROUPS; ++i) {
                  int sym = noteHeads[0][i][1];
                  if (i == HEAD_BREVIS_ALT)
                        sym = noteHeads[0][i][3];
                  NoteHead* nh = new NoteHead(gscore);
                  nh->setSym(sym);
                  sp->append(nh, qApp->translate("symbol", symbols[sym].name()));
                  }

            paletteBox->addPalette(sp);

            //-----------------------------------
            //    Tremolo
            //-----------------------------------

            sp = new Palette;
            sp->setName(tr("Tremolo"));
            sp->setGrid(27, 40);
            sp->setDrawGrid(true);
            const char* tremoloName[] = {
                  QT_TR_NOOP("1 through stem"),
                  QT_TR_NOOP("2 through stem"),
                  QT_TR_NOOP("3 through stem"),
                  QT_TR_NOOP("1 between notes"),
                  QT_TR_NOOP("2 between notes"),
                  QT_TR_NOOP("3 between notes")
                  };

            for (int i = 0; i < 6; ++i) {
                  Tremolo* tremolo = new Tremolo(gscore);
                  tremolo->setSubtype(i);
                  sp->append(tremolo, tr(tremoloName[i]));
                  }
            paletteBox->addPalette(sp);

            //-----------------------------------
            //    Repeats
            //-----------------------------------

            sp = new Palette;
            sp->setName(tr("Repeats"));
            sp->setMag(0.65);
            sp->setGrid(84, 28);
            sp->setDrawGrid(true);

            RepeatMeasure* rm = new RepeatMeasure(gscore);
            sp->append(rm, tr("Repeat measure sign"));

            Marker* mk = new Marker(gscore);
            mk->setMarkerType(MARKER_SEGNO);
            sp->append(mk, tr("Segno"));

            mk = new Marker(gscore);
            mk->setMarkerType(MARKER_CODA);
            sp->append(mk, tr("Coda"));

            mk = new Marker(gscore);
            mk->setMarkerType(MARKER_VARCODA);
            sp->append(mk, tr("Varied coda"));

            mk = new Marker(gscore);
            mk->setMarkerType(MARKER_CODETTA);
            sp->append(mk, tr("Codetta"));

            mk = new Marker(gscore);
            mk->setMarkerType(MARKER_FINE);
            sp->append(mk, tr("Fine"));

            Jump* jp = new Jump(gscore);
            jp->setJumpType(JUMP_DC);
            sp->append(jp, tr("Da Capo"));

            jp = new Jump(gscore);
            jp->setJumpType(JUMP_DC_AL_FINE);
            sp->append(jp, tr("Da Capo al Fine"));

            jp = new Jump(gscore);
            jp->setJumpType(JUMP_DC_AL_CODA);
            sp->append(jp, tr("Da Capo al Coda"));

            jp = new Jump(gscore);
            jp->setJumpType(JUMP_DS_AL_CODA);
            sp->append(jp, tr("D.S al Coda"));

            jp = new Jump(gscore);
            jp->setJumpType(JUMP_DS_AL_FINE);
            sp->append(jp, tr("D.S al Fine"));

            jp = new Jump(gscore);
            jp->setJumpType(JUMP_DS);
            sp->append(jp, tr("D.S"));

            mk = new Marker(gscore);
            mk->setMarkerType(MARKER_TOCODA);
            sp->append(mk, tr("To Coda"));

            paletteBox->addPalette(sp);

            //-----------------------------------
            //    breaks
            //-----------------------------------

            sp = new Palette;
            sp->setName(tr("Breaks && Spacer"));
            sp->setMag(.7);
            sp->setGrid(42, 36);
            sp->setDrawGrid(true);

            LayoutBreak* lb = new LayoutBreak(gscore);
            lb->setSubtype(LAYOUT_BREAK_LINE);
            sp->append(lb, tr("Line break"));

            lb = new LayoutBreak(gscore);
            lb->setSubtype(LAYOUT_BREAK_PAGE);
            sp->append(lb, tr("Page break"));

            Spacer* spacer = new Spacer(gscore);
            spacer->setSpace(Spatium(3));
            sp->append(spacer, tr("Staff spacer"));

            paletteBox->addPalette(sp);

            //-----------------------------------
            //    beam properties
            //-----------------------------------

            sp = new Palette;
            sp->setName(tr("Beam Properties"));
            sp->setGrid(27, 40);
            sp->setDrawGrid(true);

            ik = new Icon(gscore);
            ik->setSubtype(ICON_SBEAM);
            ik->setAction(getAction("beam-start"));
            sp->append(ik, tr("Start beam"));

            ik = new Icon(gscore);
            ik->setSubtype(ICON_MBEAM);
            ik->setAction(getAction("beam-mid"));
            sp->append(ik, tr("Middle of beam"));

            ik = new Icon(gscore);
            ik->setSubtype(ICON_NBEAM);
            ik->setAction(getAction("no-beam"));
            sp->append(ik, tr("No beam"));

            ik = new Icon(gscore);
            ik->setSubtype(ICON_BEAM32);
            ik->setAction(getAction("beam32"));
            sp->append(ik, tr("Start subbeam"));

            ik = new Icon(gscore);
            ik->setSubtype(ICON_AUTOBEAM);
            ik->setAction(getAction("auto-beam"));
            sp->append(ik, tr("Auto beam"));

            paletteBox->addPalette(sp);

            //-----------------------------------
            //    Symbols
            //-----------------------------------

            sp = new Palette;
            sp->setName(tr("Symbols"));
            sp->setGrid(42, 45);
            sp->setDrawGrid(true);

            sp->append(accDiscantSym);
            sp->append(accDotSym);
            sp->append(accFreebaseSym);
            sp->append(accStdbaseSym);
            sp->append(accBayanbaseSym);
            sp->append(accOldEESym);

            paletteBox->addPalette(sp);
            }
      paletteBox->setShown(visible);
      a->setChecked(visible);
      }

//---------------------------------------------------------
//   genCreateMenu
//---------------------------------------------------------

QMenu* MuseScore::genCreateMenu(QWidget* parent)
      {
      QMenu* popup = new QMenu(tr("&Create"), parent);
      popup->setObjectName("Create");

      popup->addAction(getAction("instruments"));

      QMenu* measures = popup->addMenu(tr("Measures"));
      measures->addAction(getAction("append-measure"));
      measures->addAction(getAction("append-measures"));
      measures->addAction(getAction("insert-measure"));
      measures->addAction(getAction("insert-measures"));
      measures->addAction(getAction("insert-hbox"));
      measures->addAction(getAction("insert-vbox"));
      measures->addAction(getAction("append-hbox"));
      measures->addAction(getAction("append-vbox"));

      popup->addAction(tr("Barlines..."),        this, SLOT(barMenu()));
      popup->addAction(getAction("clefs"));
      popup->addAction(getAction("keys"));
      popup->addAction(getAction("times"));
      popup->addAction(tr("&Lines..."),          this, SLOT(lineMenu()));
      popup->addAction(tr("Brackets..."), this, SLOT(bracketMenu()));
      popup->addAction(tr("Articulations && Ornaments..."), this, SLOT(noteAttributesMenu()));
      popup->addAction(tr("Accidentals..."),     this, SLOT(accidentalsMenu()));

      QMenu* text = popup->addMenu(tr("Text"));
      text->addAction(getAction("title-text"));
      text->addAction(getAction("subtitle-text"));
      text->addAction(getAction("composer-text"));
      text->addAction(getAction("poet-text"));
      text->addAction(getAction("copyright-text"));
      text->addSeparator();
      text->addAction(getAction("system-text"));
      text->addAction(getAction("staff-text"));
      text->addAction(getAction("chord-text"));
      text->addAction(getAction("rehearsalmark-text"));
      text->addSeparator();
      text->addAction(getAction("lyrics"));
      text->addAction(getAction("fingering"));
      text->addAction(getAction("dynamics"));
      text->addAction(getAction("tempo"));

      popup->addAction(getAction("symbols"));
      return popup;
      }

//---------------------------------------------------------
//   symbolMenu
//---------------------------------------------------------

void MuseScore::symbolMenu()
      {
      if (symbolDialog == 0)
            symbolDialog = new SymbolDialog(this);
      symbolDialog->show();
      symbolDialog->raise();
      }

//---------------------------------------------------------
//   clefMenu
//---------------------------------------------------------

void MuseScore::clefMenu()
      {
      if (clefPalette == 0) {
            Palette* sp = new Palette;
            sp->setGrid(60, 80);
            sp->resize(360, 400);
            clefPalette = new PaletteScrollArea(sp);
            clefPalette->setRestrictHeight(false);
            clefPalette->setWindowTitle(tr("MuseScore: Clefs"));
            for (int i = 0; i < CLEF_MAX; ++i) {
                  Clef* k = new ::Clef(gscore, i);
                  sp->append(k, qApp->translate("clefTable", clefTable[i].name));
                  }
            }
      clefPalette->show();
      clefPalette->raise();
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
            Palette* sp = new Palette();
            sp->resize(400, 300);
            linePalette = new PaletteScrollArea(sp);
            linePalette->setRestrictHeight(false);
            linePalette->setWindowTitle(tr("MuseScore: Lines"));
            sp->setGrid(100, 30);

            double l = gscore->spatium() * 8;

            Hairpin* gabel0 = new Hairpin(gscore);
            gabel0->setSubtype(0);
            gabel0->setLen(l);
            sp->append(gabel0, tr("Crescendo"));

            Hairpin* gabel1 = new Hairpin(gscore);
            gabel1->setSubtype(1);
            gabel1->setLen(l);
            sp->append(gabel1, tr("Diminuendo"));

            Volta* volta = new Volta(gscore);
            volta->setLen(l);
            volta->setText("1.");
            QList<int> il;
            il.clear();
            il.append(1);
            volta->setEndings(il);
            volta->setSubtype(Volta::VOLTA_CLOSED);

            sp->append(volta, tr("Prima volta"));

            volta = new Volta(gscore);
            volta->setLen(l);
            volta->setText("2.");
            il.clear();
            il.append(2);
            volta->setEndings(il);
            volta->setSubtype(Volta::VOLTA_CLOSED);
            sp->append(volta, tr("Seconda volta"));

            volta = new Volta(gscore);
            volta->setLen(l);
            volta->setText("3.");
            il.clear();
            il.append(3);
            volta->setEndings(il);
            volta->setSubtype(Volta::VOLTA_CLOSED);
            sp->append(volta, tr("Terza volta"));

            volta = new Volta(gscore);
            volta->setLen(l);
            volta->setText("2.");
            il.clear();
            il.append(2);
            volta->setEndings(il);
            volta->setSubtype(Volta::VOLTA_OPEN);
            sp->append(volta, tr("Seconda volta"));

            //--------

            Ottava* ottava = new Ottava(gscore);
            ottava->setSubtype(0);
            ottava->setLen(l);
            sp->append(ottava, tr("8va"));

            ottava = new Ottava(gscore);
            ottava->setSubtype(1);
            ottava->setLen(l);
            sp->append(ottava, tr("15ma"));

            ottava = new Ottava(gscore);
            ottava->setSubtype(2);
            ottava->setLen(l);
            sp->append(ottava, tr("8vb"));

            ottava = new Ottava(gscore);
            ottava->setSubtype(3);
            ottava->setLen(l);
            sp->append(ottava, tr("15mb"));

            //-------

            Pedal* pedal = new Pedal(gscore);
            pedal->setLen(l);
            sp->append(pedal, tr("Pedal"));

            Trill* trill = new Trill(gscore);
            trill->setLen(l);
            sp->append(trill, tr("Trill line"));

            TextLine* textLine = new TextLine(gscore);
            textLine->setBeginText("VII");
            sp->append(textLine, tr("Text line"));
            textLine->setEndHook(true);
            textLine->setEndHookHeight(Spatium(1.5));

            TextLine* line = new TextLine(gscore);
            line->setDiagonal(true);
            sp->append(line, tr("Line"));
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
            Palette* sp = new Palette();
            bracketPalette = new PaletteScrollArea(sp);
            bracketPalette->setRestrictHeight(false);
            bracketPalette->setWindowTitle(tr("MuseScore: Brackets"));
            sp->setGrid(40, 80);

            double _spatium = gscore->spatium();
            Bracket* b1 = new Bracket(gscore);
            b1->setSubtype(BRACKET_NORMAL);
            Bracket* b2 = new Bracket(gscore);
            b2->setSubtype(BRACKET_AKKOLADE);
            b1->setHeight(_spatium * 7);
            b2->setHeight(_spatium * 7);

            sp->append(b1, tr("Bracket"));
            sp->append(b2, tr("Akkolade"));

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
            Palette* sp = new Palette();
            sp->resize(400, 300);
            noteAttributesPalette = new PaletteScrollArea(sp);
            noteAttributesPalette->setRestrictHeight(false);
            noteAttributesPalette->setWindowTitle(tr("MuseScore: Articulations & Ornaments"));
            unsigned nn = ARTICULATIONS;
            sp->setGrid(42, 30);

            for (unsigned i = 0; i < nn; ++i) {
                  Articulation* s = new Articulation(gscore);
                  s->setSubtype(i);
                  sp->append(s, qApp->translate("articulation", qPrintable(s->subtypeName())));
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
            Palette* sp = new Palette();
            sp->resize(400, 300);
            accidentalsPalette = new PaletteScrollArea(sp);
            accidentalsPalette->setRestrictHeight(false);
            accidentalsPalette->setWindowTitle(tr("MuseScore: Accidentals"));
            sp->setGrid(40, 50);

            for (int i = ACC_SHARP; i < ACC_END; ++i) {
                  Accidental* s = new Accidental(gscore);
                  s->setSubtype(i);
                  sp->append(s, qApp->translate("accidental", s->subtypeUserName()));
                  }
            AccidentalBracket* ab = new AccidentalBracket(gscore);
            ab->setSubtype(0);
            sp->append(ab, qApp->translate("Accidental", "round bracket"));
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
            Palette* sp = new Palette();
            dynamicsPalette = new PaletteScrollArea(sp);
            dynamicsPalette->setRestrictHeight(false);
            dynamicsPalette->setWindowTitle(tr("MuseScore: Dynamics"));
            sp->setGrid(90, 40);
            sp->resize(300, 200);

            for (int i = 0; i < 27; ++i) {
                  Dynamic* dynamic = new Dynamic(gscore);
                  dynamic->setSubtype(dynList[i + 1].tag);
                  sp->append(dynamic, dynamic->subtypeName());
                  }

            const char* expr[] = {
                  "crescendo", "diminuendo", "dolce", "espressivo",
                  "legato", "leggiero", "marcato", "mero", "molto"
                  };
            for (unsigned int i = 0; i < sizeof(expr) / sizeof(*expr); ++i) {
                  Dynamic* d = new Dynamic(gscore);
                  d->setSubtype(expr[i]);
                  sp->append(d,  expr[i]);
                  }
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
            Palette* sp = new Palette();
            sp->resize(300, 200);
            barPalette = new PaletteScrollArea(sp, 0);
            barPalette->setRestrictHeight(false);
            barPalette->setWindowTitle(tr("MuseScore: Barlines"));
            sp->setGrid(42, 38);

            struct {
                  BarType type;
                  const char* name;
                  } t[] = {
                  { NORMAL_BAR,       QT_TR_NOOP("Normal") },
                  { BROKEN_BAR,       QT_TR_NOOP("Dashed") },
                  { END_BAR,          QT_TR_NOOP("End Bar") },
                  { DOUBLE_BAR,       QT_TR_NOOP("Double Bar") },
                  { START_REPEAT,     QT_TR_NOOP("Start Repeat") },
                  { END_REPEAT,       QT_TR_NOOP("End Repeat") },
                  { END_START_REPEAT, QT_TR_NOOP("End-Start Repeat") },
                  };
            for (unsigned i = 0; i < sizeof(t)/sizeof(*t); ++i) {
                  BarLine* b  = new BarLine(gscore);
                  b->setHeight(4 * gscore->spatium());
                  b->setSubtype(t[i].type);
                  sp->append(b, tr(t[i].name));
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
            Palette* sp = new Palette;
            sp->setMag(1.5);
            sp->resize(300, 200);
            fingeringPalette = new PaletteScrollArea(sp);
            fingeringPalette->setRestrictHeight(false);
            fingeringPalette->setWindowTitle(tr("MuseScore: Fingering"));
            sp->setGrid(28, 30);

            const char finger[] = "012345pimac";
            Text* k;

            for (unsigned i = 0; i < strlen(finger); ++i) {
                  k = new Text(gscore);
                  k->setSubtype(TEXT_FINGERING);
                  k->setTextStyle(TEXT_STYLE_FINGERING);
                  k->setText(QString(finger[i]));
                  sp->append(k, tr("Fingering %1").arg(finger[i]));
                  }
            const char stringnumber[] = "0123456";
            for (unsigned i = 0; i < strlen(stringnumber); ++i) {
                  Text* k = new Text(gscore);
                  k->setSubtype(TEXT_STRING_NUMBER);
                  k->setTextStyle(TEXT_STYLE_STRING_NUMBER);
                  k->setText(QString(stringnumber[i]));
                  sp->append(k, tr("String number %1").arg(stringnumber[i]));
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
      ChordRest* cr = getSelectedChordRest();
      if (!cr)
            return;
      Measure* m = cr->segment()->measure();
      int tick = cr->tick();
      if (editTempo == 0)
            editTempo = new EditTempo(0);
      int rv = editTempo->exec();
      if (rv == 1) {
            double bps = editTempo->bpm() / 60.0;
            _tempomap->addTempo(tick, bps);
            TempoText* tt = new TempoText(this);
            tt->setTick(tick);
            tt->setTrack(0);
            tt->setText(editTempo->text());
            tt->setTempo(bps);
            tt->setParent(m);
            cmdAdd(tt);
            refresh |= tt->abbox();
            }
      }

//---------------------------------------------------------
//   addMetronome
//---------------------------------------------------------

void Score::addMetronome()
      {
      printf("addMetronome: not implemented\n");
      }

//---------------------------------------------------------
//   showLayoutBreakPalette
//---------------------------------------------------------

void MuseScore::showLayoutBreakPalette()
      {
      if (layoutBreakPalette == 0) {
            Palette* sp = new Palette;
            layoutBreakPalette = new PaletteScrollArea(sp);
            layoutBreakPalette->setRestrictHeight(false);
            layoutBreakPalette->setWindowTitle(tr("MuseScore: Breaks & Spacer"));
            sp->setGrid(80, 80);
            sp->resize(240,80);
            LayoutBreak* lb = new LayoutBreak(gscore);
            lb->setSubtype(LAYOUT_BREAK_LINE);
            sp->append(lb, tr("Line break"));
            lb = new LayoutBreak(gscore);
            lb->setSubtype(LAYOUT_BREAK_PAGE);
            sp->append(lb, tr("Page break"));
            Spacer* spacer = new Spacer(gscore);
            spacer->setSpace(Spatium(3));
            sp->append(spacer, tr("Staff spacer"));
            }
      layoutBreakPalette->show();
      layoutBreakPalette->raise();
      }

//---------------------------------------------------------
//   updateDrumset
//---------------------------------------------------------

void MuseScore::updateDrumset()
      {
      if (cs == 0 || paletteBox == 0 || drumPalette == 0)
            return;

      double _spatium = gscore->spatium();
      const InputState& padState = cs->inputState();
      Drumset* ds        = padState.drumset();
      if (ds != drumset) {
            drumset = ds;
            drumPalette->clear();
            if (drumset) {
                  int drumInstruments = 0;
                  for (int pitch = 0; pitch < 128; ++pitch) {
                        if (drumset->isValid(pitch))
                              ++drumInstruments;
                        }
                  int i = 0;
                  for (int pitch = 0; pitch < 128; ++pitch) {
                        if (!ds->isValid(pitch))
                              continue;
                        bool up;
                        int line      = ds->line(pitch);
                        int noteHead  = ds->noteHead(pitch);
                        int voice     = ds->voice(pitch);
                        Direction dir = ds->stemDirection(pitch);
                        if (dir == UP)
                              up = true;
                        else if (dir == DOWN)
                              up = false;
                        else
                              up = line > 4;

                        Chord* chord = new Chord(gscore);
                        chord->setDuration(Duration::V_QUARTER);
                        chord->setStemDirection(dir);
                        chord->setTrack(voice);
                        Note* note = new Note(gscore);
                        note->setParent(chord);
                        note->setTrack(voice);
                        note->setPitch(pitch);
                        note->setTpcFromPitch();
                        note->setLine(line);
                        note->setPos(0.0, _spatium * .5 * line);
                        note->setHeadGroup(noteHead);
                        chord->add(note);
                        Stem* stem = new Stem(gscore);
                        stem->setLen((up ? -3.0 : 3.0) * _spatium);
                        chord->setStem(stem);
                        stem->setPos(note->stemPos(up));
                        drumPalette->append(chord, qApp->translate("drumset", qPrintable(drumset->name(pitch))));
                        ++i;
                        }
                  }
            }
      if (drumset) {
            int i = 0;
            drumPalette->setSelected(-1);
            for (int pitch = 0; pitch < 128; ++pitch) {
                  if (drumset->isValid(pitch)) {
                        if (pitch == padState.drumNote()) {
                              drumPalette->setSelected(i);
                              break;
                              }
                        ++i;
                        }
                  }
            }
      drumPalette->update();
      }

//---------------------------------------------------------
//   drumPaletteSelected
//---------------------------------------------------------

void MuseScore::drumPaletteSelected(int idx)
      {
      if (cs == 0)
            return;
      InputState& padState = cs->inputState();
      Drumset* ds          = padState.drumset();
      if (ds == 0)
            return;
      int i = 0;
      for (int pitch = 0; pitch < 128; ++pitch) {
            if (!drumset->isValid(pitch))
                  continue;
            if (i == idx) {
                  padState.setDrumNote(pitch);
                  padState.track    = (padState.track / VOICES) * VOICES + ds->voice(pitch);
                  cs->setPadState();
                  break;
                  }
            ++i;
            }
      }

