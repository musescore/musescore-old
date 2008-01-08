//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: menus.cpp,v 1.46 2006/04/12 14:58:10 wschweer Exp $
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
#include "volta.h"
#include "keysig.h"
#include "breath.h"
#include "arpeggio.h"
#include "tremolo.h"
#include "repeat.h"

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

            //-----------------------------------
            //    clefs
            //-----------------------------------

            Palette* sp = new Palette(4, 4, .8);
            sp->setGrid(42, 60);
            sp->showStaff(true);
            for (int i = 0; i < 15; ++i) {
                  Clef* k = new ::Clef(gscore, i);
                  sp->addObject(i,  k, tr(clefTable[i].name));
                  }
            paletteBox->addPalette(tr("Clefs"), sp);

            //-----------------------------------
            //    key signatures
            //-----------------------------------

            sp = new Palette(5, 3, .8);
            sp->setGrid(56, 45);
            sp->showStaff(true);
            for (int i = 0; i < 7; ++i) {
                  KeySig* k = new KeySig(gscore);
                  k->setSubtype(i+1);
                  sp->addObject(i,  k, keyNames[i*2]);
                  }
            for (int i = -7; i < 0; ++i) {
                  KeySig* k = new KeySig(gscore);
                  k->setSubtype(i & 0xff);
                  sp->addObject(14 + i,  k, keyNames[(7 + i) * 2 + 1]);
                  }
            KeySig* k = new KeySig(gscore);
            k->setSubtype(0);
            sp->addObject(14,  k, keyNames[14]);
            paletteBox->addPalette(tr("Keys"), sp);

            //-----------------------------------
            //    Time
            //-----------------------------------

            sp = new Palette(4, 3);
            sp->setGrid(56, 45);
            sp->showStaff(true);

      	sp->addObject(0,   new TimeSig(gscore, 2, 2), "2/2");
      	sp->addObject(1,   new TimeSig(gscore, 4, 2), "2/4");
      	sp->addObject(2,   new TimeSig(gscore, 4, 3), "3/4");
      	sp->addObject(3,   new TimeSig(gscore, 4, 4), "4/4");
      	sp->addObject(4,   new TimeSig(gscore, 4, 5), "5/4");
      	sp->addObject(5,   new TimeSig(gscore, 4, 6), "6/4");
      	sp->addObject(6,   new TimeSig(gscore, 8, 3), "3/8");
      	sp->addObject(7,   new TimeSig(gscore, 8, 6), "6/8");
      	sp->addObject(8,   new TimeSig(gscore, 8, 9), "9/8");
      	sp->addObject(9,   new TimeSig(gscore, 8, 12), "12/8");
      	sp->addObject(10,  new TimeSig(gscore, TSIG_FOUR_FOUR), "4/4 common time");
      	sp->addObject(11,  new TimeSig(gscore, TSIG_ALLA_BREVE), "(2+2)/4 alla breve");
            paletteBox->addPalette(tr("Time"), sp);

            //-----------------------------------
            //    Bar Lines
            //-----------------------------------

            sp = new Palette(2, 4);
            sp->setGrid(42, 50);
            sp->showStaff(true);

            struct {
                  BarType type;
                  QString name;
                  } t[] = {
                  { NORMAL_BAR,       "normal" },
                  { BROKEN_BAR,       "broken" },
                  { END_BAR,          "End Bar" },
                  { DOUBLE_BAR,       "Double Bar" },
                  { START_REPEAT,     "Start Repeat" },
                  { END_REPEAT,       "End Repeat" },
                  { END_START_REPEAT, "End-Start Repeat" },
                  };
            for (unsigned i = 0; i < sizeof(t)/sizeof(*t); ++i) {
                  BarLine* b  = new BarLine(gscore);
                  b->setHeight(point(Spatium(4)));
                  b->setSubtype(t[i].type);
                  sp->addObject(i,  b, t[i].name);
                  }
            paletteBox->addPalette(tr("Bar Lines"), sp);

            //-----------------------------------
            //    Lines
            //-----------------------------------

            sp = new Palette(6, 2);
            sp->setGrid(84, 30);

            double l = _spatium * 7;

            Hairpin* gabel0 = new Hairpin(gscore);
            gabel0->setSubtype(0);
            gabel0->setLen(l);
            sp->addObject(0, gabel0, tr("crescendo"));

            Hairpin* gabel1 = new Hairpin(gscore);
            gabel1->setSubtype(1);
            gabel1->setLen(l);
            sp->addObject(1, gabel1, tr("diminuendo"));

            Volta* volta = new Volta(gscore);
            volta->setLen(l);
            volta->setSubtype(Volta::VOLTA_CLOSED);
            volta->setText("1.");
            QList<int> il;
            il.append(1);
            volta->setEndings(il);
            sp->addObject(2, volta, tr("prima volta"));

            volta = new Volta(gscore);
            volta->setLen(l);
            volta->setSubtype(Volta::VOLTA_CLOSED);
            volta->setText("2.");
            il.clear();
            il.append(2);
            volta->setEndings(il);
            sp->addObject(3, volta, tr("seconda volta"));

            volta = new Volta(gscore);
            volta->setLen(l);
            volta->setSubtype(Volta::VOLTA_CLOSED);
            volta->setText("3.");
            il.clear();
            il.append(3);
            volta->setEndings(il);
            sp->addObject(4, volta, tr("terza volta"));

            volta = new Volta(gscore);
            volta->setLen(l);
            volta->setSubtype(Volta::VOLTA_OPEN);
            volta->setText("2.");
            il.clear();
            il.append(2);
            volta->setEndings(il);
            sp->addObject(5, volta, tr("seconda volta 2"));

            Ottava* ottava = new Ottava(gscore);
            ottava->setSubtype(0);
            ottava->setLen(l);
            sp->addObject(6, ottava, tr("8va"));

            ottava = new Ottava(gscore);
            ottava->setSubtype(1);
            ottava->setLen(l);
            sp->addObject(7, ottava, tr("15va"));

            ottava = new Ottava(gscore);
            ottava->setSubtype(2);
            ottava->setLen(l);
            sp->addObject(8, ottava, tr("8vb"));

            ottava = new Ottava(gscore);
            ottava->setSubtype(3);
            ottava->setLen(l);
            sp->addObject(9, ottava, tr("15vb"));

            Pedal* pedal = new Pedal(gscore);
            pedal->setLen(l);
            sp->addObject(10, pedal, tr("pedal"));

            Trill* trill = new Trill(gscore);
            trill->setLen(l);
            sp->addObject(11, trill, tr("trill line"));

            paletteBox->addPalette(tr("Lines"), sp);

            //-----------------------------------
            //    Arpeggios
            //-----------------------------------

            sp = new Palette(1, 4);
            sp->setGrid(42, 60);

            for (int i = 0; i < 4; ++i) {
                  Arpeggio* a = new Arpeggio(gscore);
                  a->setSubtype(i);
                  a->setHeight(_spatium * 4);
                  sp->addObject(i, a, tr("arpeggio"));
                  }
            paletteBox->addPalette(tr("Arpeggios"), sp);

            //-----------------------------------
            //    Symbols: Breath
            //-----------------------------------

            sp = new Palette(1, 4);
            sp->setGrid(42, 40);

            for (int i = 0; i < 2; ++i) {
                  Breath* a = new Breath(gscore);
                  a->setSubtype(i);
                  sp->addObject(i, a, tr("breath"));
                  }

            paletteBox->addPalette(tr("Breath"), sp);

            //-----------------------------------
            //    Brackets
            //-----------------------------------

            sp = new Palette(1, 4, .7);
            sp->setGrid(42, 60);

            Bracket* b1 = new Bracket(gscore);
            b1->setSubtype(BRACKET_NORMAL);
            Bracket* b2 = new Bracket(gscore);
            b2->setSubtype(BRACKET_AKKOLADE);
            b1->setHeight(_spatium * 7);
            b2->setHeight(_spatium * 7);

            sp->addObject(0, b1, "Bracket");
            sp->addObject(1, b2, "Akkolade");

            paletteBox->addPalette(tr("Brackets"), sp);

            //-----------------------------------
            //    Attributes
            //-----------------------------------

            static const char* atrSyms[] = {
                  "ufermata", "dfermata", "thumb", "sforzato",
                  "espressivo", "staccato", "ustaccatissimo", "dstaccatissimo",
                  "tenuto", "uportato", "dportato", "umarcato", "dmarcato",
                  "ouvert", "plusstop", "upbow", "downbow", "reverseturn",
                  "turn", "trill", "prall", "mordent", "prallprall",
                  "prallmordent", "upprall", "downprall", "upmordent",
	            "downmordent"
                  };

            unsigned nn = sizeof(atrSyms)/sizeof(*atrSyms);
            sp = new Palette((nn + 3) / 4, 4);
            sp->setGrid(42, 30);

            for (unsigned i = 0; i < nn; ++i) {
                  NoteAttribute* s = new NoteAttribute(gscore);
                  s->setSubtype(atrSyms[i]);
                  sp->addObject(i, s, s->subtypeName());
                  }
            paletteBox->addPalette(tr("Attributes"), sp);

            //-----------------------------------
            //    Accidentals
            //-----------------------------------

            sp = new Palette(2, 5);
            sp->setGrid(33, 36);

            for (int i = 1; i < 11; ++i) {
                  Accidental* s = new Accidental(gscore);
                  s->setSubtype(i);
                  sp->addObject(i-1, s, s->name());
                  }
            paletteBox->addPalette(tr("Accidentals"), sp);

            //-----------------------------------
            //    Dynamics
            //-----------------------------------

            sp = new Palette(2, 4, .9);
            sp->setGrid(42, 42);

            static const char* dynS[] = {
                  "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff"
                  };
            for (unsigned i = 0; i < sizeof(dynS)/sizeof(*dynS); ++i) {
                  Dynamic* dynamic = new Dynamic(gscore);
                  dynamic->setSubtype(dynS[i]);
                  sp->addObject(i, dynamic, dynamic->subtypeName());
                  }
            paletteBox->addPalette(tr("Dynamics"), sp);

            //-----------------------------------
            //    Fingering
            //-----------------------------------

            sp = new Palette(2, 6, 1.5);
            sp->setGrid(28, 30);
            sp->setDrawGrid(true);

            const char finger[] = "012345pimac";
            for (unsigned i = 0; i < strlen(finger); ++i) {
                  Text* k = new Text(gscore);
                  k->setSubtype(TEXT_FINGERING);
                  k->setText(QString(finger[i]));
                  sp->addObject(i, k, QString("fingering %1").arg(finger[i]));
                  }
            paletteBox->addPalette(tr("Fingering"), sp);

            //-----------------------------------
            //    Noteheads
            //-----------------------------------

            sp = new Palette(1, 4, 1.5);
            sp->setGrid(42, 40);
            sp->setDrawGrid(true);

            NoteHead* nh = new NoteHead(gscore);
            nh->setSym(quartheadSym);
            sp->addObject(0, nh, QString("normal"));

            nh = new NoteHead(gscore);
            nh->setSym(crossedheadSym);
            sp->addObject(1, nh, QString("crossed"));

            nh = new NoteHead(gscore);
            nh->setSym(diamondheadSym);
            sp->addObject(2, nh, QString("diamond"));

            nh = new NoteHead(gscore);
            nh->setSym(triangleheadSym);
            sp->addObject(3, nh, QString("triangle"));

            paletteBox->addPalette(tr("NoteHeads"), sp);

            //-----------------------------------
            //    Tremolo
            //-----------------------------------

            sp = new Palette(1, 4, 1.0);
            sp->setGrid(42, 40);
            sp->setDrawGrid(true);

            for (int i = 0; i < 3; ++i) {
                  Tremolo* tremolo = new Tremolo(gscore);
                  tremolo->setSubtype(i);
                  sp->addObject(i, tremolo, QString("normal"));
                  }
            paletteBox->addPalette(tr("Tremolo"), sp);

            //-----------------------------------
            //    Repeats
            //-----------------------------------

            sp = new Palette(7, 2, 0.8);
            sp->setGrid(84, 28);
            sp->setDrawGrid(true);

            RepeatMeasure* rm = new RepeatMeasure(gscore);
            sp->addObject(0, rm, tr("repeat measure"));

            Marker* mk = new Marker(gscore);
            mk->setMarkerType(MARKER_SEGNO);
            sp->addObject(1, mk, tr("Segno"));

            mk = new Marker(gscore);
            mk->setMarkerType(MARKER_CODA);
            sp->addObject(2, mk, tr("Coda"));

            mk = new Marker(gscore);
            mk->setMarkerType(MARKER_VARCODA);
            sp->addObject(3, mk, tr("VarCoda"));

            mk = new Marker(gscore);
            mk->setMarkerType(MARKER_CODETTA);
            sp->addObject(4, mk, tr("Codetta"));

            mk = new Marker(gscore);
            mk->setMarkerType(MARKER_FINE);
            sp->addObject(5, mk, tr("Fine"));

            Jump* jp = new Jump(gscore);
            jp->setJumpType(JUMP_DC);
            sp->addObject(6, jp, tr("da Capo"));

            jp = new Jump(gscore);
            jp->setJumpType(JUMP_DC_AL_FINE);
            sp->addObject(7, jp, tr("da Capo al Fine"));

            jp = new Jump(gscore);
            jp->setJumpType(JUMP_DC_AL_CODA);
            sp->addObject(8, jp, tr("da Capo al Coda"));

            jp = new Jump(gscore);
            jp->setJumpType(JUMP_DS_AL_CODA);
            sp->addObject(9, jp, tr("D.S al Coda"));

            jp = new Jump(gscore);
            jp->setJumpType(JUMP_DS_AL_FINE);
            sp->addObject(10, jp, tr("D.S al Fine"));

            jp = new Jump(gscore);
            jp->setJumpType(JUMP_DS);
            sp->addObject(11, jp, tr("D.S"));

            mk = new Marker(gscore);
            mk->setMarkerType(MARKER_TOCODA);
            sp->addObject(12, mk, tr("To Coda"));

//                  "alSegno"

            paletteBox->addPalette(tr("Repeats"), sp);

            //-----------------------------------
            //    breaks
            //-----------------------------------

            sp = new Palette(1, 4, .7);
            sp->setGrid(42, 36);
            sp->setDrawGrid(true);

            LayoutBreak* lb = new LayoutBreak(gscore);
            lb->setSubtype(LAYOUT_BREAK_LINE);
            sp->addObject(0, lb, tr("break line"));

            lb = new LayoutBreak(gscore);
            lb->setSubtype(LAYOUT_BREAK_PAGE);
            sp->addObject(1, lb, tr("break page"));

            paletteBox->addPalette(tr("Breaks"), sp);
            //-----------------------------------
            //    Symbols
            //-----------------------------------

            sp = new Palette(42, 4);
            sp->setGrid(42, 45);
            sp->setDrawGrid(true);
            sp->addObject(0, wholerestSym);
            sp->addObject(1, halfrestSym);
            sp->addObject(2, outsidewholerestSym);
            sp->addObject(3, outsidehalfrestSym);
            sp->addObject(4, longarestSym);
            sp->addObject(5, breverestSym);
            sp->addObject(6, quartrestSym);
            sp->addObject(7, eighthrestSym);
            sp->addObject(8, clasquartrestSym);
            sp->addObject(9, sixteenthrestSym);
            sp->addObject(10, thirtysecondrestSym);
            sp->addObject(11, sixtyfourthrestSym);
            sp->addObject(12, hundredtwentyeighthrestSym);
            sp->addObject(16, zeroSym);
            sp->addObject(17, oneSym);
            sp->addObject(18, twoSym);
            sp->addObject(19, threeSym);
            sp->addObject(20, fourSym);
            sp->addObject(21, fiveSym);
            sp->addObject(22, sixSym);
            sp->addObject(23, sevenSym);
            sp->addObject(24, eightSym);
            sp->addObject(25, nineSym);
            sp->addObject(31, sharpSym);
            sp->addObject(32, naturalSym);
            sp->addObject(33, flatSym);
            sp->addObject(34, flatflatSym);
            sp->addObject(35, sharpsharpSym);
            sp->addObject(36, rightparenSym);
            sp->addObject(37, leftparenSym);
            sp->addObject(38, dotSym);
            sp->addObject(39, brevisheadSym);
            sp->addObject(40, wholeheadSym);
            sp->addObject(41, halfheadSym);
            sp->addObject(42, quartheadSym);
            sp->addObject(43, wholediamondheadSym);
            sp->addObject(44, halfdiamondheadSym);
            sp->addObject(45, diamondheadSym);
            sp->addObject(46, wholetriangleheadSym);
            sp->addObject(47, halftriangleheadSym);
            sp->addObject(48, triangleheadSym);
            sp->addObject(49, wholeslashheadSym);
            sp->addObject(50, halfslashheadSym);
            sp->addObject(51, quartslashheadSym);
            sp->addObject(52, wholecrossedheadSym);
            sp->addObject(53, halfcrossedheadSym);
            sp->addObject(54, crossedheadSym);
            sp->addObject(55, xcircledheadSym);

            sp->addObject(57, ufermataSym);
            sp->addObject(58, dfermataSym);
            sp->addObject(59, thumbSym);
            sp->addObject(60, sforzatoaccentSym);
            sp->addObject(61, staccatoSym);
            sp->addObject(62, ustaccatissimoSym);
            sp->addObject(63, dstaccatissimoSym);
            sp->addObject(64, tenutoSym);
            sp->addObject(65, uportatoSym);
            sp->addObject(66, dportatoSym);
            sp->addObject(67, umarcatoSym);
            sp->addObject(68, dmarcatoSym);
            sp->addObject(69, ouvertSym);
            sp->addObject(70, plusstopSym);
            sp->addObject(71, upbowSym);
            sp->addObject(72, downbowSym);
            sp->addObject(73, reverseturnSym);
            sp->addObject(74, turnSym);
            sp->addObject(75, trillSym);
            sp->addObject(76, upedalheelSym);
            sp->addObject(77, dpedalheelSym);
            sp->addObject(78, upedaltoeSym);
            sp->addObject(79, dpedaltoeSym);
            sp->addObject(80, flageoletSym);
            sp->addObject(81, segnoSym);
            sp->addObject(82, codaSym);
            sp->addObject(83, rcommaSym);
            sp->addObject(84, lcommaSym);
            sp->addObject(85, arpeggioSym);
            sp->addObject(86, trillelementSym);
            sp->addObject(87, arpeggioarrowdownSym);
            sp->addObject(88, arpeggioarrowupSym);
            sp->addObject(89, trilelementSym);
            sp->addObject(90, prallSym);
            sp->addObject(91, mordentSym);
            sp->addObject(92, prallprallSym);
            sp->addObject(93, prallmordentSym);
            sp->addObject(94, upprallSym);
            sp->addObject(95, downprallSym);
            sp->addObject(96, upmordentSym);
            sp->addObject(97, downmordentSym);
            sp->addObject(98, lineprallSym);
            sp->addObject(99, pralldownSym);
            sp->addObject(101, prallupSym);
            sp->addObject(102, eighthflagSym);
            sp->addObject(103, sixteenthflagSym);
            sp->addObject(104, thirtysecondflagSym);
            sp->addObject(105, sixtyfourthflagSym);
            sp->addObject(106, deighthflagSym);
            sp->addObject(107, gracedashSym);
            sp->addObject(108, dgracedashSym);
            sp->addObject(109, dsixteenthflagSym);
            sp->addObject(110, dthirtysecondflagSym);
            sp->addObject(111, dsixtyfourthflagSym);
            sp->addObject(112, stemSym);
            sp->addObject(113, dstemSym);
            sp->addObject(114, altoclefSym);
            sp->addObject(115, caltoclefSym);
            sp->addObject(116, bassclefSym);
            sp->addObject(117, cbassclefSym);
            sp->addObject(118, trebleclefSym);
            sp->addObject(119, ctrebleclefSym);
            sp->addObject(120, percussionclefSym);
            sp->addObject(121, cpercussionclefSym);
            sp->addObject(122, tabclefSym);
            sp->addObject(123, ctabclefSym);
            sp->addObject(124, fourfourmeterSym);
            sp->addObject(125, allabreveSym);
            sp->addObject(126, pedalasteriskSym);
            sp->addObject(127, pedaldashSym);
            sp->addObject(128, pedaldotSym);
            sp->addObject(129, pedalPSym);
            sp->addObject(130, pedaldSym);
            sp->addObject(131, pedaleSym);
            sp->addObject(132, pedalPedSym);
            sp->addObject(133, accDiscantSym);
            sp->addObject(134, accDotSym);
            sp->addObject(135, accFreebaseSym);
            sp->addObject(136, accStdbaseSym);
            sp->addObject(137, accBayanbaseSym);
            sp->addObject(138, accSBSym);
            sp->addObject(139, accBBSym);
            sp->addObject(140, accOldEESym);
            sp->addObject(141, accOldEESSym);
            sp->addObject(142, wholedoheadSym);
            sp->addObject(143, halfdoheadSym);
            sp->addObject(144, doheadSym);
            sp->addObject(145, wholereheadSym);
            sp->addObject(146, halfreheadSym);
            sp->addObject(147, reheadSym);
            sp->addObject(148, wholemeheadSym);
            sp->addObject(149, halfmeheadSym);
            sp->addObject(150, meheadSym);
            sp->addObject(151, wholefaheadSym);
            sp->addObject(152, halffauheadSym);
            sp->addObject(152, fauheadSym);
            sp->addObject(153, halffadheadSym);
            sp->addObject(154, fadheadSym);
            sp->addObject(155, wholelaheadSym);
            sp->addObject(156, halflaheadSym);
            sp->addObject(157, laheadSym);
            sp->addObject(158, wholeteheadSym);
            sp->addObject(159, halfteheadSym);
            sp->addObject(160, letterfSym);
            sp->addObject(161, lettermSym);
            sp->addObject(162, letterpSym);
            sp->addObject(163, letterrSym);
            sp->addObject(164, lettersSym);
            sp->addObject(165, letterzSym);
            paletteBox->addPalette(tr("Symbols"), sp);
            }
      paletteBox->setShown(visible);
      a->setChecked(visible);
      }

//---------------------------------------------------------
//   genCreateMenu
//---------------------------------------------------------

QMenu* MuseScore::genCreateMenu()
      {
      QMenu* popup = new QMenu(tr("&Create"));

      popup->addAction(getAction("instruments"));

      QMenu* measures = popup->addMenu(tr("Measures..."));
      measures->addAction(getAction("append-measure"));
      measures->addAction(getAction("append-measures"));
      measures->addAction(getAction("insert-measure"));
      measures->addAction(getAction("insert-measures"));
      measures->addAction(getAction("insert-hbox"));
      measures->addAction(getAction("insert-vbox"));

      popup->addAction(tr("Barlines..."),        this, SLOT(barMenu()));
      popup->addAction(getAction("clefs"));
      popup->addAction(getAction("keys"));
      popup->addAction(getAction("times"));
      popup->addAction(tr("&Lines..."),          this, SLOT(lineMenu()));
      popup->addAction(tr("System Brackets..."), this, SLOT(bracketMenu()));
      popup->addAction(tr("Note Attributes..."), this, SLOT(noteAttributesMenu()));
      popup->addAction(tr("Accidentals..."),     this, SLOT(accidentalsMenu()));

      QMenu* text = popup->addMenu(tr("Text..."));
      text->addAction(getAction("title-text"));
      text->addAction(getAction("subtitle-text"));
      text->addAction(getAction("composer-text"));
      text->addAction(getAction("poet-text"));
      text->addAction(getAction("copyright-text"));
      text->addSeparator();
      text->addAction(getAction("system-text"));
      text->addAction(getAction("chord-text"));
      text->addAction(getAction("rehearsalmark-text"));
      text->addSeparator();
      text->addAction(getAction("lyrics"));
      text->addAction(getAction("fingering"));
      text->addAction(getAction("dynamics"));
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
            QScrollArea* sa = new QScrollArea;
            sa->setWindowTitle(tr("MuseScore: Clefs"));
            clefPalette = sa;
            Palette* sp = new Palette(4, 4);
            sa->setWidget(sp);
            sp->setGrid(60, 80);
            sp->showStaff(true);
            for (int i = 0; i < 15; ++i) {
                  Clef* k = new ::Clef(gscore, i);
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
      if (keyPalette == 0) {
            keyPalette = new QScrollArea;
            keyPalette->setWindowTitle(tr("MuseScore: Key Signature"));
            Palette* sp = new Palette(4, 4);
            sp->setGrid(80, 60);
            ((QScrollArea*)keyPalette)->setWidget(sp);
            sp->showStaff(true);
            for (int i = 0; i < 7; ++i) {
                  KeySig* k = new KeySig(gscore);
                  k->setSubtype(i+1);
                  sp->addObject(i * 2,  k, keyNames[i*2]);
                  }
            for (int i = -7; i < 0; ++i) {
                  KeySig* k = new KeySig(gscore);
                  k->setSubtype(i & 0xff);
                  sp->addObject((7 + i) * 2 + 1,  k, keyNames[(7 + i) * 2 + 1]);
                  }
            KeySig* k = new KeySig(gscore);
            k->setSubtype(0);
            sp->addObject(14,  k, keyNames[14]);
            k = new KeySig(gscore);
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
            Palette* sp = new Palette(4, 4);
            ((QScrollArea*)linePalette)->setWidget(sp);
            sp->setGrid(100, 30);

            double l = _spatium * 8;

            Hairpin* gabel0 = new Hairpin(gscore);
            gabel0->setSubtype(0);
            gabel0->setLen(l);
            sp->addObject(0, gabel0, tr("crescendo"));

            Hairpin* gabel1 = new Hairpin(gscore);
            gabel1->setSubtype(1);
            gabel1->setLen(l);
            sp->addObject(1, gabel1, tr("diminuendo"));

            Volta* volta = new Volta(gscore);
            volta->setLen(l);
            volta->setText("1.");
            QList<int> il;
            il.clear();
            il.append(1);
            volta->setEndings(il);
            volta->setSubtype(Volta::VOLTA_CLOSED);

            sp->addObject(4, volta, tr("prima volta"));

            volta = new Volta(gscore);
            volta->setLen(l);
            volta->setText("2.");
            il.clear();
            il.append(2);
            volta->setEndings(il);
            volta->setSubtype(Volta::VOLTA_CLOSED);
            sp->addObject(5, volta, tr("seconda volta"));

            volta = new Volta(gscore);
            volta->setLen(l);
            volta->setText("3.");
            il.clear();
            il.append(3);
            volta->setEndings(il);
            volta->setSubtype(Volta::VOLTA_CLOSED);
            sp->addObject(6, volta, tr("terza volta"));

            volta = new Volta(gscore);
            volta->setLen(l);
            volta->setText("2.");
            il.clear();
            il.append(2);
            volta->setEndings(il);
            volta->setSubtype(Volta::VOLTA_OPEN);
            sp->addObject(7, volta, tr("seconda volta"));

            //--------

            Ottava* ottava = new Ottava(gscore);
            ottava->setSubtype(0);
            ottava->setLen(l);
            sp->addObject(8, ottava, tr("8va"));

            ottava = new Ottava(gscore);
            ottava->setSubtype(1);
            ottava->setLen(l);
            sp->addObject(9, ottava, tr("15va"));

            ottava = new Ottava(gscore);
            ottava->setSubtype(2);
            ottava->setLen(l);
            sp->addObject(10, ottava, tr("8vb"));

            ottava = new Ottava(gscore);
            ottava->setSubtype(3);
            ottava->setLen(l);
            sp->addObject(11, ottava, tr("15vb"));

            //-------

            Pedal* pedal = new Pedal(gscore);
            pedal->setLen(l);
            sp->addObject(12, pedal, tr("pedal"));

            Trill* trill = new Trill(gscore);
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
            Palette* sp = new Palette(1, 4);
            ((QScrollArea*)bracketPalette)->setWidget(sp);
            sp->setGrid(40, 80);

            Bracket* b1 = new Bracket(gscore);
            b1->setSubtype(BRACKET_NORMAL);
            Bracket* b2 = new Bracket(gscore);
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
            Palette* sp = new Palette(3, 10);
            ((QScrollArea*)noteAttributesPalette)->setWidget(sp);
            sp->setGrid(60, 60);

            static const char* syms[] = {
                  "ufermata", "dfermata", "thumb", "sforzato",
                  "espressivo", "staccato", "ustaccatissimo", "dstaccatissimo",
                  "tenuto", "uportato", "dportato", "umarcato", "dmarcato",
                  "ouvert", "plusstop", "upbow", "downbow", "reverseturn",
                  "turn", "trill", "prall", "mordent", "prallprall",
                  "prallmordent", "upprall", "downprall", "upmordent",
	            "downmordent"
                  };
            for (unsigned i = 0; i < sizeof(syms)/sizeof(*syms); ++i) {
                  NoteAttribute* s = new NoteAttribute(gscore);
                  s->setSubtype(syms[i]);
                  sp->addObject(i, s, s->subtypeName());
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
            Palette* sp = new Palette(2, 8);
            ((QScrollArea*)accidentalsPalette)->setWidget(sp);
            sp->setGrid(60, 60);

            for (int i = 0; i < 16; ++i) {
                  Accidental* s = new Accidental(gscore);
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
            Palette* sp = new Palette(6, 6);
            ((QScrollArea*)dynamicsPalette)->setWidget(sp);
            sp->setGrid(90, 40);

            for (int i = 0; i < 27; ++i) {
                  Dynamic* dynamic = new Dynamic(gscore);
                  dynamic->setSubtype(i + 1);
                  sp->addObject(i, dynamic, dynamic->subtypeName());
                  }

            char* expr[] = {
                  "crescendo", "diminuendo", "dolce", "espressivo",
                  "legato", "leggiero", "marcato", "mero", "molto"
                  };
            for (unsigned int i = 0; i < sizeof(expr) / sizeof(*expr); ++i) {
                  Dynamic* d = new Dynamic(gscore);
                  d->setSubtype(0);
                  d->setText(expr[i]);
                  sp->addObject(27+i, d,  expr[i]);
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
            barPalette = new QScrollArea;
            barPalette->setWindowTitle(tr("MuseScore: Barlines"));
            Palette* sp = new Palette(2, 4);
            ((QScrollArea*)barPalette)->setWidget(sp);
            sp->setGrid(60, 60);
            sp->showStaff(true);

            struct {
                  BarType type;
                  QString name;
                  } t[] = {
                  { BROKEN_BAR,    "broken" },
                  { NORMAL_BAR,    "normal" },
                  { END_BAR,       "End Bar" },
                  { DOUBLE_BAR,    "Double Bar" },
                  { START_REPEAT,  "Start Repeat" },
                  { END_REPEAT,    "End Repeat" },
                  { END_START_REPEAT, "End-Start Repeat" },
                  };
            for (unsigned i = 0; i < sizeof(t)/sizeof(*t); ++i) {
                  BarLine* b  = new BarLine(gscore);
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
            Palette* sp = new Palette(2, 6, 1.5);
            sa->setWidget(sp);
            sp->setGrid(50, 50);

            const char finger[] = "012345pimac";
            Text* k;

            for (unsigned i = 0; i < strlen(finger); ++i) {
                  k = new Text(gscore);
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
      if (!e || (e->type() != NOTE && e->type() != REST)) {
            QMessageBox::information(0, "MuseScore: Text Entry",
               tr("No note or rest selected:\n"
                  "please select a note or rest were you want to\n"
                  "set tempo."));
            return;
            }
      if (e->type() == NOTE)
            e = e->parent();
      Measure* m = ((ChordRest*)e)->segment()->measure();
      int tick = e->tick();
      if (editTempo == 0)
            editTempo = new EditTempo(0);
      int rv = editTempo->exec();
      if (rv == 1) {
            double bps = editTempo->bpm() / 60.0;
            tempomap->addTempo(tick, bps);
            TempoText* tt = new TempoText(this);
            tt->setTick(tick);
            tt->setStaffIdx(e->staffIdx());
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
            QScrollArea* sa = new QScrollArea;
            sa->setWindowTitle(tr("MuseScore: Layout Breaks"));
            layoutBreakPalette = sa;
            Palette* sp = new Palette(1, 3);
            sa->setWidget(sp);
            sp->setGrid(80, 80);
            LayoutBreak* lb = new LayoutBreak(gscore);
            lb->setSubtype(LAYOUT_BREAK_LINE);
            sp->addObject(0, lb, tr("break line"));
            lb = new LayoutBreak(gscore);
            lb->setSubtype(LAYOUT_BREAK_PAGE);
            sp->addObject(1, lb, tr("break page"));
            }
      layoutBreakPalette->show();
      layoutBreakPalette->raise();
      }

