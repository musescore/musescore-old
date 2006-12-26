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

//---------------------------------------------------------
//   genCreateMenu
//---------------------------------------------------------

QMenu* MuseScore::genCreateMenu()
      {
      QMenu* popup = new QMenu(tr("&Create"));
      popup->addAction(tr("&Instruments...\tI"), this, SLOT(editInstrList()));
      popup->addAction(tr("Measure"),            this, SLOT(cmdAppendMeasure()), Qt::CTRL+Qt::Key_B);
      popup->addAction(tr("Measures..."),        this, SLOT(cmdAppendMeasures()));
      popup->addAction(tr("Barlines..."),        this, SLOT(barMenu()));
      popup->addAction(tr("Clef...\tY"),         this, SLOT(clefMenu()));
      popup->addAction(tr("&Key...\tK"),         this, SLOT(keyMenu()));
      popup->addAction(tr("&Time...\tT"),        this, SLOT(timeMenu()));
      popup->addAction(tr("&Lines..."),          this, SLOT(lineMenu()));
      popup->addAction(tr("System Brackets..."), this, SLOT(bracketMenu()));
      popup->addAction(tr("Note Attributes..."), this, SLOT(noteAttributesMenu()));
      popup->addAction(tr("Accidentals..."),     this, SLOT(accidentalsMenu()));
      popup->addAction(tr("Dynamics...\tL"),     this, SLOT(dynamicsMenu()));

      QMenu* text = popup->addMenu(tr("Text..."));
      text->addAction(tr("Title"),        this, SLOT(cmdAddTitle()));
      text->addAction(tr("Subtitle"),     this, SLOT(cmdAddSubTitle()));
      text->addAction(tr("Composer"),     this, SLOT(cmdAddComposer()));
      text->addAction(tr("Poet"),         this, SLOT(cmdAddPoet()));
      text->addSeparator();
      text->addAction(tr("Lyrics"),       this, SLOT(addLyrics()),     Qt::CTRL+Qt::Key_L);
      text->addAction(tr("Fingering..."), this, SLOT(fingeringMenu()));
      text->addAction(tr("Expression"),   this, SLOT(addExpression()), Qt::CTRL+Qt::Key_E);
      text->addAction(tr("Technik"),      this, SLOT(addTechnik()),    Qt::CTRL+Qt::Key_T);
      text->addAction(tr("Tempo..."),     this, SLOT(addTempo()),      Qt::CTRL+Qt::ALT+Qt::Key_T);
      text->addAction(tr("Metronome"),    this, SLOT(addMetronome()),  Qt::CTRL+Qt::ALT+Qt::Key_M);

      popup->addAction(tr("Symbols...\tZ"), this, SLOT(symbolMenu1()));
      return popup;
      }

//---------------------------------------------------------
//   symbolMenu
//---------------------------------------------------------

void MuseScore::symbolMenu1()
      {
      if (symbolPalette1 == 0) {
            QScrollArea* sa = new QScrollArea;
            sa->setWindowTitle(tr("MuseScore: Symbols 1"));
            symbolPalette1 = sa;
            SymbolPalette* pw = new SymbolPalette(11, 16);
            sa->setWidget(pw);

            pw->addObject(0, wholerestSym);
            pw->addObject(1, halfrestSym);
            pw->addObject(2, outsidewholerestSym);
            pw->addObject(3, outsidehalfrestSym);
            pw->addObject(4, longarestSym);
            pw->addObject(5, breverestSym);
            pw->addObject(6, quartrestSym);
            pw->addObject(7, eighthrestSym);
            pw->addObject(8, clasquartrestSym);
            pw->addObject(9, sixteenthrestSym);
            pw->addObject(10, thirtysecondrestSym);
            pw->addObject(11, sixtyfourthrestSym);
            pw->addObject(12, hundredtwentyeighthrestSym);
//            pw->addObject(13, neomensmaximarestSym);
//            pw->addObject(14, neomenslongarestSym);
//            pw->addObject(15, neomensbreverestSym);
            pw->addObject(16, zeroSym);
            pw->addObject(17, oneSym);
            pw->addObject(18, twoSym);
            pw->addObject(19, threeSym);
            pw->addObject(20, fourSym);
            pw->addObject(21, fiveSym);
            pw->addObject(22, sixSym);
            pw->addObject(23, sevenSym);
            pw->addObject(24, eightSym);
            pw->addObject(25, nineSym);
//            pw->addObject(26, neomenssemibrevisrestSym);
//            pw->addObject(27, neomensminimahalfrestSym);
//            pw->addObject(28, neomenssemiminimarestSym);
//            pw->addObject(29, neomensfusarestSym);
//            pw->addObject(30, neomenssemifusarestSym);
            pw->addObject(31, sharpSym);
            pw->addObject(32, naturalSym);
            pw->addObject(33, flatSym);
            pw->addObject(34, flatflatSym);
            pw->addObject(35, sharpsharpSym);
            pw->addObject(36, rightparenSym);
            pw->addObject(37, leftparenSym);
            pw->addObject(38, dotSym);
            pw->addObject(39, brevisheadSym);
            pw->addObject(40, wholeheadSym);
            pw->addObject(41, halfheadSym);
            pw->addObject(42, quartheadSym);
            pw->addObject(43, wholediamondheadSym);
            pw->addObject(44, halfdiamondheadSym);
            pw->addObject(45, diamondheadSym);
            pw->addObject(46, wholetriangleheadSym);
            pw->addObject(47, halftriangleheadSym);
            pw->addObject(48, triangleheadSym);
            pw->addObject(49, wholeslashheadSym);
            pw->addObject(50, halfslashheadSym);
            pw->addObject(51, quartslashheadSym);
            pw->addObject(52, wholecrossedheadSym);
            pw->addObject(53, halfcrossedheadSym);
            pw->addObject(54, crossedheadSym);
            pw->addObject(55, xcircledheadSym);

            pw->addObject(57, ufermataSym);
            pw->addObject(58, dfermataSym);
            pw->addObject(59, thumbSym);
            pw->addObject(60, sforzatoaccentSym);
            pw->addObject(61, staccatoSym);
            pw->addObject(62, ustaccatissimoSym);
            pw->addObject(63, dstaccatissimoSym);
            pw->addObject(64, tenutoSym);
            pw->addObject(65, uportatoSym);
            pw->addObject(66, dportatoSym);
            pw->addObject(67, umarcatoSym);
            pw->addObject(68, dmarcatoSym);
            pw->addObject(69, ouvertSym);
            pw->addObject(70, plusstopSym);
            pw->addObject(71, upbowSym);
            pw->addObject(72, downbowSym);
            pw->addObject(73, reverseturnSym);
            pw->addObject(74, turnSym);
            pw->addObject(75, trillSym);
            pw->addObject(76, upedalheelSym);
            pw->addObject(77, dpedalheelSym);
            pw->addObject(78, upedaltoeSym);
            pw->addObject(79, dpedaltoeSym);
            pw->addObject(80, flageoletSym);
            pw->addObject(81, segnoSym);
            pw->addObject(82, codaSym);
            pw->addObject(83, rcommaSym);
            pw->addObject(84, lcommaSym);
            pw->addObject(85, arpeggioSym);
            pw->addObject(86, trillelementSym);
            pw->addObject(87, arpeggioarrowdownSym);
            pw->addObject(88, arpeggioarrowupSym);
            pw->addObject(89, trilelementSym);
            pw->addObject(90, prallSym);
            pw->addObject(91, mordentSym);
            pw->addObject(92, prallprallSym);
            pw->addObject(93, prallmordentSym);
            pw->addObject(94, upprallSym);
            pw->addObject(95, downprallSym);
            pw->addObject(96, upmordentSym);
            pw->addObject(97, downmordentSym);
            pw->addObject(98, lineprallSym);
            pw->addObject(99, pralldownSym);
            pw->addObject(101, prallupSym);
            pw->addObject(102, eighthflagSym);
            pw->addObject(103, sixteenthflagSym);
            pw->addObject(104, thirtysecondflagSym);
            pw->addObject(105, sixtyfourthflagSym);
            pw->addObject(106, deighthflagSym);
            pw->addObject(107, gracedashSym);
            pw->addObject(108, dgracedashSym);
            pw->addObject(109, dsixteenthflagSym);
            pw->addObject(110, dthirtysecondflagSym);
            pw->addObject(111, dsixtyfourthflagSym);
            pw->addObject(112, stemSym);
            pw->addObject(113, dstemSym);
            pw->addObject(114, altoclefSym);
            pw->addObject(115, caltoclefSym);
            pw->addObject(116, bassclefSym);
            pw->addObject(117, cbassclefSym);
            pw->addObject(118, trebleclefSym);
            pw->addObject(119, ctrebleclefSym);
            pw->addObject(120, percussionclefSym);
            pw->addObject(121, cpercussionclefSym);
            pw->addObject(122, tabclefSym);
            pw->addObject(123, ctabclefSym);
            pw->addObject(124, fourfourmeterSym);
            pw->addObject(125, allabreveSym);
            pw->addObject(126, pedalasteriskSym);
            pw->addObject(127, pedaldashSym);
            pw->addObject(128, pedaldotSym);
            pw->addObject(129, pedalPSym);
            pw->addObject(130, pedaldSym);
            pw->addObject(131, pedaleSym);
            pw->addObject(132, pedalPedSym);
            pw->addObject(133, accDiscantSym);
            pw->addObject(134, accDotSym);
            pw->addObject(135, accFreebaseSym);
            pw->addObject(136, accStdbaseSym);
            pw->addObject(137, accBayanbaseSym);
            pw->addObject(138, accSBSym);
            pw->addObject(139, accBBSym);
            pw->addObject(140, accOldEESym);
            pw->addObject(141, accOldEESSym);
            pw->addObject(142, wholedoheadSym);
            pw->addObject(143, halfdoheadSym);
            pw->addObject(144, doheadSym);
            pw->addObject(145, wholereheadSym);
            pw->addObject(146, halfreheadSym);
            pw->addObject(147, reheadSym);
            pw->addObject(148, wholemeheadSym);
            pw->addObject(149, halfmeheadSym);
            pw->addObject(150, meheadSym);
            pw->addObject(151, wholefaheadSym);
            pw->addObject(152, halffauheadSym);
            pw->addObject(152, fauheadSym);
            pw->addObject(153, halffadheadSym);
            pw->addObject(154, fadheadSym);
            pw->addObject(155, wholelaheadSym);
            pw->addObject(156, halflaheadSym);
            pw->addObject(157, laheadSym);
            pw->addObject(158, wholeteheadSym);
            pw->addObject(159, halfteheadSym);
            pw->addObject(160, letterfSym);
            pw->addObject(161, lettermSym);
            pw->addObject(162, letterpSym);
            pw->addObject(163, letterrSym);
            pw->addObject(164, lettersSym);
            pw->addObject(165, letterzSym);
            }
      symbolPalette1->show();
      symbolPalette1->raise();
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
                  KeySig* k = new KeySig(cs, i+1, 0);
                  sp->addObject(i * 2,  k, keyNames[i*2]);
                  }
            for (int i = -7; i < 0; ++i) {
                  KeySig* k = new KeySig(cs, i, 0);
                  sp->addObject((7 + i) * 2 + 1,  k, keyNames[(7 + i) * 2 + 1]);
                  }
            KeySig* k = new KeySig(cs, 0, 0);
            sp->addObject(14,  k, keyNames[14]);
            k = new KeySig(cs, 0, 0);
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

            Bracket* b1 = new Bracket(cs, BRACKET_NORMAL);
            Bracket* b2 = new Bracket(cs, BRACKET_AKKOLADE);
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
                  Accidental* s = new Accidental(cs, i, false);
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

            sp->addObject( 0, new Dynamic(cs, DynPPPPPP),   "pppppp");
            sp->addObject( 1, new Dynamic(cs, DynPPPPP),    "ppppp");
            sp->addObject( 2, new Dynamic(cs, DynPPPP),     "pppp");
            sp->addObject( 3, new Dynamic(cs, DynPPP),      "ppp");
            sp->addObject( 4, new Dynamic(cs, DynPP),       "pianissimo");
            sp->addObject( 5, new Dynamic(cs, DynP),        "piano");
            sp->addObject( 6, new Dynamic(cs, DynMP),       "mezzopiano");
            sp->addObject( 7, new Dynamic(cs, DynMF),       "mezzoforte");
            sp->addObject( 8, new Dynamic(cs, DynF),        "forte");
            sp->addObject( 9, new Dynamic(cs, DynFF),       "fortissimo");
            sp->addObject(10, new Dynamic(cs, DynFFF),      "fff");
            sp->addObject(11, new Dynamic(cs, DynFFFF),     "ffff");
            sp->addObject(12, new Dynamic(cs, DynFFFFF),    "fffff");
            sp->addObject(13, new Dynamic(cs, DynFFFFFF),   "ffffff");
            sp->addObject(14, new Dynamic(cs, DynFP),       "fp");
            sp->addObject(15, new Dynamic(cs, DynSF),       "sforzando");
            sp->addObject(16, new Dynamic(cs, DynSFZ),      "sforzando");
            sp->addObject(17, new Dynamic(cs, DynSFFZ),     "sffz");
            sp->addObject(18, new Dynamic(cs, DynSFP),      "sfp");
            sp->addObject(19, new Dynamic(cs, DynSFPP),     "sfpp");
            sp->addObject(20, new Dynamic(cs, DynRFZ),      "rfz");
            sp->addObject(21, new Dynamic(cs, DynRF),       "rf");
            sp->addObject(22, new Dynamic(cs, DynFZ),       "fz");
            sp->addObject(23, new Dynamic(cs, DynM),        "m");
            sp->addObject(24, new Dynamic(cs, DynR),        "r");
            sp->addObject(25, new Dynamic(cs, DynS),        "s");
            sp->addObject(26, new Dynamic(cs, DynZ),        "z");

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
            SymbolPalette* sp = new SymbolPalette(1, 5);
            sa->setWidget(sp);
            sp->setGrid(50, 50);

            Fingering* k;
            k = new Fingering(0);
            k->setSubtype(1);
            sp->addObject(0, k, tr("fingering 1"));
            k = new Fingering(0);
            k->setSubtype(2);
            sp->addObject(1, k, tr("fingering 2"));
            k = new Fingering(0);
            k->setSubtype(3);
            sp->addObject(2, k, tr("fingering 3"));
            k = new Fingering(0);
            k->setSubtype(4);
            sp->addObject(3, k, tr("fingering 4"));
            k = new Fingering(0);
            k->setSubtype(5);
            sp->addObject(4, k, tr("fingering 5"));

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
            return;
            }
      if (e->type() == NOTE)
            e = e->parent();
      if (e->type() != CHORD && e->type() != REST) {
            printf("no Chord/Rest selected\n");
            return;
            }
      Measure* m = ((ChordRest*)e)->segment()->measure();
      int tick = e->tick();
      if (editTempo == 0)
            editTempo = new EditTempo(0);
      int rv = editTempo->exec();
      if (rv == 1) {
            int bpm = editTempo->bpm();
            printf("tempo at %d(%s)  %s %d\n",
               e->tick(), e->name(), editTempo->text().toLatin1().data(), editTempo->bpm());
            tempomap->addTempo(tick, int(60000000.0/double(bpm)));
            TempoText* tt = new TempoText(this);
            tt->setTick(tick);
            tt->setStaff(e->staff());
            tt->setText(editTempo->text());
            tt->setParent(m);
            startCmd();
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
            LayoutBreak* lb = new LayoutBreak(cs, LAYOUT_BREAK_LINE);
            sp->addObject(0, lb, tr("break line"));
            lb = new LayoutBreak(cs, LAYOUT_BREAK_PAGE);
            sp->addObject(1, lb, tr("break page"));
            }
      layoutBreakPalette->show();
      layoutBreakPalette->raise();
      }


