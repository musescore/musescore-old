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

//---------------------------------------------------------
//   showPalette
//---------------------------------------------------------

void MuseScore::showPalette(bool visible)
      {
      QAction* a = getAction("toggle-palette");
      if (paletteBox == 0) {
            paletteBox = new PaletteBox(this);

#if 1
            QFile f(dataPath + "/" + "mscore-palette.xml");
            if (f.exists()) {
                  paletteBox->read(&f);
                  paletteBox->setShown(visible);
                  a->setChecked(visible);
                  return;
                  }
#endif

            connect(paletteBox, SIGNAL(paletteVisible(bool)), a, SLOT(setChecked(bool)));
            addDockWidget(Qt::LeftDockWidgetArea, paletteBox);

            //-----------------------------------
            //    notes
            //-----------------------------------

            notePalette = new Palette(1.0);
            notePalette->setGrid(27, 40);
            notePalette->showStaff(false);
            notePalette->setDrawGrid(true);

            Icon* ik = new Icon(gscore);
            ik->setSubtype(ICON_ACCIACCATURA);
            ik->setAction(getAction("pad-acciaccatura"));
            notePalette->append(ik, tr("acciaccatura"));

            ik = new Icon(gscore);
            ik->setSubtype(ICON_APPOGGIATURA);
            ik->setAction(getAction("pad-appoggiatura"));
            notePalette->append(ik, tr("appoggiatura"));

	      ik = new Icon(gscore);
            ik->setSubtype(ICON_GRACE4);
            ik->setAction(getAction("pad-grace4"));
            notePalette->append(ik, tr("grace-4"));

	      ik = new Icon(gscore);
            ik->setSubtype(ICON_GRACE16);
            ik->setAction(getAction("pad-grace16"));
            notePalette->append(ik, tr("grace-16"));

	      ik = new Icon(gscore);
            ik->setSubtype(ICON_GRACE32);
            ik->setAction(getAction("pad-grace32"));
            notePalette->append(ik, tr("grace-32"));

            paletteBox->addPalette(qApp->translate("NotePalette", "Notes"), notePalette);

            //-----------------------------------
            //    drums
            //-----------------------------------

            drumPalette = new Palette();
            drumPalette->setMag(0.8);
            drumPalette->setSelectable(true);
            drumPalette->setGrid(42, 60);
            drumPalette->showStaff(true);
            drumPalette->setDrumPalette(true);

//            updateDrumset();
            paletteBox->addPalette(tr("Drums"), drumPalette);
//            connect(drumPalette, SIGNAL(boxClicked(int)), SLOT(drumPaletteSelected(int)));

            //-----------------------------------
            //    clefs
            //-----------------------------------

            Palette* sp = new Palette(0.8);
            sp->setGrid(33, 62);
            sp->showStaff(true);
            sp->setYOffset(1.0);

            for (int i = 0; i < CLEF_MAX; ++i) {
                  Clef* k = new ::Clef(gscore, i);
                  sp->append(k, tr(clefTable[i].name));
                  }
            paletteBox->addPalette(tr("Clefs"), sp);

            //-----------------------------------
            //    key signatures
            //-----------------------------------

            sp = new Palette(.8);
            sp->setGrid(56, 45);
            sp->showStaff(true);
            sp->setYOffset(6.0);

            for (int i = 0; i < 7; ++i) {
                  KeySig* k = new KeySig(gscore);
                  k->setSubtype(i+1);
                  sp->append(k, tr(keyNames[i*2]));
                  }
            for (int i = -7; i < 0; ++i) {
                  KeySig* k = new KeySig(gscore);
                  k->setSubtype(i & 0xff);
                  sp->append(k, tr(keyNames[(7 + i) * 2 + 1]));
                  }
            KeySig* k = new KeySig(gscore);
            k->setSubtype(0);
            sp->append(k, keyNames[14]);
            paletteBox->addPalette(tr("Keys"), sp);

            //-----------------------------------
            //    Time
            //-----------------------------------

            sp = new Palette(.8);
            sp->setGrid(42, 38);
            sp->showStaff(true);

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
      	sp->append(new TimeSig(gscore, TSIG_ALLA_BREVE), tr("(2+2)/4 alla breve"));
            paletteBox->addPalette(tr("Time"), sp);

            //-----------------------------------
            //    Bar Lines
            //-----------------------------------

            sp = new Palette(.8);
            sp->setGrid(42, 38);
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
                  b->setSubtype(t[i].type);
                  sp->append(b, t[i].name);
                  }
            paletteBox->addPalette(tr("Bar Lines"), sp);

            //-----------------------------------
            //    Lines
            //-----------------------------------

            sp = new Palette(.8);
            sp->setGrid(84, 23);

            Hairpin* gabel0 = new Hairpin(gscore);
            gabel0->setSubtype(0);
            sp->append(gabel0, tr("crescendo"));

            Hairpin* gabel1 = new Hairpin(gscore);
            gabel1->setSubtype(1);
            sp->append(gabel1, tr("diminuendo"));

            Volta* volta = new Volta(gscore);
            volta->setSubtype(Volta::VOLTA_CLOSED);
            volta->setText("1.");
            QList<int> il;
            il.append(1);
            volta->setEndings(il);
            sp->append(volta, tr("prima volta"));

            volta = new Volta(gscore);
            volta->setSubtype(Volta::VOLTA_CLOSED);
            volta->setText("2.");
            il.clear();
            il.append(2);
            volta->setEndings(il);
            sp->append(volta, tr("seconda volta"));

            volta = new Volta(gscore);
            volta->setSubtype(Volta::VOLTA_CLOSED);
            volta->setText("3.");
            il.clear();
            il.append(3);
            volta->setEndings(il);
            sp->append(volta, tr("terza volta"));

            volta = new Volta(gscore);
            volta->setSubtype(Volta::VOLTA_OPEN);
            volta->setText("2.");
            il.clear();
            il.append(2);
            volta->setEndings(il);
            sp->append(volta, tr("seconda volta 2"));

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
            sp->append(trill, tr("trill line"));

            TextLine* textLine = new TextLine(gscore);
            textLine->setText("VII");
            sp->append(textLine, tr("text line"));

            TextLine* line = new TextLine(gscore);
            line->setHasText(false);
            line->setHook(false);
            sp->append(line, tr("line"));

            paletteBox->addPalette(tr("Lines"), sp);

            //-----------------------------------
            //    Arpeggios
            //-----------------------------------

            sp = new Palette();
            sp->setGrid(27, 60);

            for (int i = 0; i < 3; ++i) {
                  Arpeggio* a = new Arpeggio(gscore);
                  a->setSubtype(i);
                  // a->setHeight(_spatium * 4);
                  sp->append(a, tr("arpeggio"));
                  }
            for (int i = 0; i < 2; ++i) {
                  Glissando* a = new Glissando(gscore);
                  a->setSubtype(i);
                  // a->setSize(QSizeF(_spatium * 2, _spatium * 4));
                  sp->append(a, tr("glissando"));
                  }
            paletteBox->addPalette(tr("Arpeggio/Glissando"), sp);

            //-----------------------------------
            //    Symbols: Breath
            //-----------------------------------

            sp = new Palette();
            sp->setGrid(42, 40);

            for (int i = 0; i < 2; ++i) {
                  Breath* a = new Breath(gscore);
                  a->setSubtype(i);
                  sp->append(a, tr("breath"));
                  }

            paletteBox->addPalette(tr("Breath"), sp);

            //-----------------------------------
            //    Brackets
            //-----------------------------------

            sp = new Palette(.7);
            sp->setGrid(42, 60);

            Bracket* b1 = new Bracket(gscore);
            b1->setSubtype(BRACKET_NORMAL);
            Bracket* b2 = new Bracket(gscore);
            b2->setSubtype(BRACKET_AKKOLADE);
            // b1->setHeight(_spatium * 7);
            // b2->setHeight(_spatium * 7);

            sp->append(b1, "Bracket");
            sp->append(b2, "Akkolade");

            paletteBox->addPalette(tr("Brackets"), sp);

            //-----------------------------------
            //    Attributes, Ornaments
            //-----------------------------------

            unsigned nn = NOTE_ATTRIBUTES;
            sp = new Palette();
            sp->setGrid(42, 25);

            for (unsigned i = 0; i < nn; ++i) {
                  Articulation* s = new Articulation(gscore);
                  s->setSubtype(i);
                  sp->append(s, s->subtypeName());
                  }
            paletteBox->addPalette(tr("Articulations, Ornaments"), sp);

            //-----------------------------------
            //    Accidentals
            //-----------------------------------

            sp = new Palette();
            sp->setGrid(33, 36);

            for (int i = 1; i < 11; ++i) {
                  Accidental* s = new Accidental(gscore);
                  s->setSubtype(i);
                  sp->append(s, s->name());
                  }
            for (int i = 16; i < 26; ++i) {
                  Accidental* s = new Accidental(gscore);
                  s->setSubtype(i);
                  sp->append(s, s->name());
                  }
            paletteBox->addPalette(tr("Accidentals"), sp);

            //-----------------------------------
            //    Dynamics
            //-----------------------------------

            sp = new Palette(.9);
            sp->setGrid(42, 28);
            sp->setYOffset(-12.0);

            static const char* dynS[] = {
                  "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff"
                  };
            for (unsigned i = 0; i < sizeof(dynS)/sizeof(*dynS); ++i) {
                  Dynamic* dynamic = new Dynamic(gscore);
                  dynamic->setSubtype(dynS[i]);
                  sp->append(dynamic, dynamic->subtypeName());
                  }
            paletteBox->addPalette(tr("Dynamics"), sp);

            //-----------------------------------
            //    Fingering
            //-----------------------------------

            sp = new Palette(1.5);
            sp->setGrid(28, 30);
            sp->setDrawGrid(true);

            const char finger[] = "012345pimac";
            for (unsigned i = 0; i < strlen(finger); ++i) {
                  Text* k = new Text(gscore);
                  k->setSubtype(TEXT_FINGERING);
                  k->setText(QString(finger[i]));
                  sp->append(k, tr("fingering %1").arg(finger[i]));
                  }
            const char stringnumber[] = "0123456";
            for (unsigned i = 0; i < strlen(stringnumber); ++i) {
                  Text* k = new Text(gscore);
                  k->setSubtype(TEXT_STRING_NUMBER);
                  k->setText(QString(stringnumber[i]));
                  sp->append(k, tr("string number %1").arg(stringnumber[i]));
                  }

            paletteBox->addPalette(tr("Fingering"), sp);

            //-----------------------------------
            //    Noteheads
            //-----------------------------------

            sp = new Palette(1.3);
            sp->setGrid(33, 36);
            sp->setDrawGrid(true);

            NoteHead* nh = new NoteHead(gscore);
            nh->setSym(halfheadSym);
            sp->append(nh, QString("normal"));

            nh = new NoteHead(gscore);
            nh->setSym(halfcrossedheadSym);
            sp->append(nh, QString("crossed"));

            nh = new NoteHead(gscore);
            nh->setSym(halfdiamondheadSym);
            sp->append(nh, QString("diamond"));

            nh = new NoteHead(gscore);
            nh->setSym(halftriangleheadSym);
            sp->append(nh, QString("triangle"));

            nh = new NoteHead(gscore);
            nh->setSym(halfdiamond2headSym);
            sp->append(nh, QString("diamond2"));

            nh = new NoteHead(gscore);
            nh->setSym(halfslashheadSym);
            sp->append(nh, QString("slash"));

            nh = new NoteHead(gscore);
            nh->setSym(xcircledheadSym);
            sp->append(nh, QString("xcircle"));

            paletteBox->addPalette(tr("Note Heads"), sp);

            //-----------------------------------
            //    Tremolo
            //-----------------------------------

            sp = new Palette(1.0);
            sp->setGrid(27, 40);
            sp->setDrawGrid(true);
            const char* tremoloName[] = {
                  "1 through stem", "2 through stem", "3 through stem",
                  "1 between notes", "2 between notes", "3 between notes"
                  };

            for (int i = 0; i < 6; ++i) {
                  Tremolo* tremolo = new Tremolo(gscore);
                  tremolo->setSubtype(i);
                  sp->append(tremolo, tremoloName[i]);
                  }
            paletteBox->addPalette(tr("Tremolo"), sp);

            //-----------------------------------
            //    Repeats
            //-----------------------------------

            sp = new Palette(0.65);
            sp->setGrid(84, 28);
            sp->setDrawGrid(true);

            RepeatMeasure* rm = new RepeatMeasure(gscore);
            sp->append(rm, tr("repeat measure"));

            Marker* mk = new Marker(gscore);
            mk->setMarkerType(MARKER_SEGNO);
            sp->append(mk, tr("Segno"));

            mk = new Marker(gscore);
            mk->setMarkerType(MARKER_CODA);
            sp->append(mk, tr("Coda"));

            mk = new Marker(gscore);
            mk->setMarkerType(MARKER_VARCODA);
            sp->append(mk, tr("VarCoda"));

            mk = new Marker(gscore);
            mk->setMarkerType(MARKER_CODETTA);
            sp->append(mk, tr("Codetta"));

            mk = new Marker(gscore);
            mk->setMarkerType(MARKER_FINE);
            sp->append(mk, tr("Fine"));

            Jump* jp = new Jump(gscore);
            jp->setJumpType(JUMP_DC);
            sp->append(jp, tr("da Capo"));

            jp = new Jump(gscore);
            jp->setJumpType(JUMP_DC_AL_FINE);
            sp->append(jp, tr("da Capo al Fine"));

            jp = new Jump(gscore);
            jp->setJumpType(JUMP_DC_AL_CODA);
            sp->append(jp, tr("da Capo al Coda"));

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

            paletteBox->addPalette(tr("Repeats"), sp);

            //-----------------------------------
            //    breaks
            //-----------------------------------

            sp = new Palette(.7);
            sp->setGrid(42, 36);
            sp->setDrawGrid(true);

            LayoutBreak* lb = new LayoutBreak(gscore);
            lb->setSubtype(LAYOUT_BREAK_LINE);
            sp->append(lb, tr("break line"));

            lb = new LayoutBreak(gscore);
            lb->setSubtype(LAYOUT_BREAK_PAGE);
            sp->append(lb, tr("break page"));

            Spacer* spacer = new Spacer(gscore);
            spacer->setSpace(Spatium(3));
            sp->append(spacer, tr("staff spacer"));

            paletteBox->addPalette(tr("Breaks/Spacer"), sp);

            //-----------------------------------
            //    beam properties
            //-----------------------------------

            sp = new Palette(1.0);
            sp->setGrid(27, 40);
            sp->setDrawGrid(true);
            sp->showStaff(false);

            ik = new Icon(gscore);
            ik->setSubtype(ICON_SBEAM);
            ik->setAction(getAction("beam-start"));
            sp->append(ik, tr("start beam"));

            ik = new Icon(gscore);
            ik->setSubtype(ICON_MBEAM);
            ik->setAction(getAction("beam-mid"));
            sp->append(ik, tr("middle of beam"));

            ik = new Icon(gscore);
            ik->setSubtype(ICON_NBEAM);
            ik->setAction(getAction("no-beam"));
            sp->append(ik, tr("no beam"));

            ik = new Icon(gscore);
            ik->setSubtype(ICON_BEAM32);
            ik->setAction(getAction("beam32"));
            sp->append(ik, tr("start subbeam"));

            ik = new Icon(gscore);
            ik->setSubtype(ICON_AUTOBEAM);
            ik->setAction(getAction("auto-beam"));
            sp->append(ik, tr("auto beam"));

            paletteBox->addPalette(tr("Beam Properties"), sp);


            //-----------------------------------
            //    Symbols
            //-----------------------------------

            sp = new Palette();
            sp->setGrid(42, 45);
            sp->setDrawGrid(true);
            sp->append(wholerestSym);
            sp->append(halfrestSym);
            sp->append(outsidewholerestSym);
            sp->append(outsidehalfrestSym);
            sp->append(longarestSym);
            sp->append(breverestSym);
            sp->append(quartrestSym);
            sp->append(eighthrestSym);
            sp->append(clasquartrestSym);
            sp->append(sixteenthrestSym);
            sp->append(thirtysecondrestSym);
            sp->append(sixtyfourthrestSym);
            sp->append(hundredtwentyeighthrestSym);
            sp->append(zeroSym);
            sp->append(oneSym);
            sp->append(twoSym);
            sp->append(threeSym);
            sp->append(fourSym);
            sp->append(fiveSym);
            sp->append(sixSym);
            sp->append(sevenSym);
            sp->append(eightSym);
            sp->append(nineSym);
            sp->append(sharpSym);
            sp->append(naturalSym);
            sp->append(flatSym);
            sp->append(flatflatSym);
            sp->append(sharpsharpSym);
            sp->append(rightparenSym);
            sp->append(leftparenSym);
            sp->append(dotSym);
            sp->append(brevisheadSym);
            sp->append(wholeheadSym);
            sp->append(halfheadSym);
            sp->append(quartheadSym);
            sp->append(wholediamondheadSym);
            sp->append(halfdiamondheadSym);
            sp->append(diamondheadSym);
            sp->append(wholetriangleheadSym);
            sp->append(halftriangleheadSym);
            sp->append(triangleheadSym);
            sp->append(wholeslashheadSym);
            sp->append(halfslashheadSym);
            sp->append(quartslashheadSym);
            sp->append(wholecrossedheadSym);
            sp->append(halfcrossedheadSym);
            sp->append(crossedheadSym);
            sp->append(xcircledheadSym);

            sp->append(ufermataSym);
            sp->append(dfermataSym);
            sp->append(thumbSym);
            sp->append(sforzatoaccentSym);
            sp->append(staccatoSym);
            sp->append(ustaccatissimoSym);
            sp->append(dstaccatissimoSym);
            sp->append(tenutoSym);
            sp->append(uportatoSym);
            sp->append(dportatoSym);
            sp->append(umarcatoSym);
            sp->append(dmarcatoSym);
            sp->append(ouvertSym);
            sp->append(plusstopSym);
            sp->append(upbowSym);
            sp->append(downbowSym);
            sp->append(reverseturnSym);
            sp->append(turnSym);
            sp->append(trillSym);
            sp->append(upedalheelSym);
            sp->append(dpedalheelSym);
            sp->append(upedaltoeSym);
            sp->append(dpedaltoeSym);
            sp->append(flageoletSym);
            sp->append(segnoSym);
            sp->append(codaSym);
            sp->append(rcommaSym);
            sp->append(lcommaSym);
            sp->append(arpeggioSym);
            sp->append(trillelementSym);
            sp->append(arpeggioarrowdownSym);
            sp->append(arpeggioarrowupSym);
            sp->append(trilelementSym);
            sp->append(prallSym);
            sp->append(mordentSym);
            sp->append(prallprallSym);
            sp->append(prallmordentSym);
            sp->append(upprallSym);
            sp->append(downprallSym);
            sp->append(upmordentSym);
            sp->append(downmordentSym);
            sp->append(lineprallSym);
            sp->append(pralldownSym);
            sp->append(prallupSym);
            sp->append(eighthflagSym);
            sp->append(sixteenthflagSym);
            sp->append(thirtysecondflagSym);
            sp->append(sixtyfourthflagSym);
            sp->append(deighthflagSym);
            sp->append(gracedashSym);
            sp->append(dgracedashSym);
            sp->append(dsixteenthflagSym);
            sp->append(dthirtysecondflagSym);
            sp->append(dsixtyfourthflagSym);
            sp->append(stemSym);
            sp->append(dstemSym);
            sp->append(altoclefSym);
            sp->append(caltoclefSym);
            sp->append(bassclefSym);
            sp->append(cbassclefSym);
            sp->append(trebleclefSym);
            sp->append(ctrebleclefSym);
            sp->append(percussionclefSym);
            sp->append(cpercussionclefSym);
            sp->append(tabclefSym);
            sp->append(ctabclefSym);
            sp->append(fourfourmeterSym);
            sp->append(allabreveSym);
            sp->append(pedalasteriskSym);
            sp->append(pedaldashSym);
            sp->append(pedaldotSym);
            sp->append(pedalPSym);
            sp->append(pedaldSym);
            sp->append(pedaleSym);
            sp->append(pedalPedSym);
            sp->append(accDiscantSym);
            sp->append(accDotSym);
            sp->append(accFreebaseSym);
            sp->append(accStdbaseSym);
            sp->append(accBayanbaseSym);
            sp->append(accSBSym);
            sp->append(accBBSym);
            sp->append(accOldEESym);
            sp->append(accOldEESSym);
            sp->append(wholedoheadSym);
            sp->append(halfdoheadSym);
            sp->append(doheadSym);
            sp->append(wholereheadSym);
            sp->append(halfreheadSym);
            sp->append(reheadSym);
            sp->append(wholemeheadSym);
            sp->append(halfmeheadSym);
            sp->append(meheadSym);
            sp->append(wholefaheadSym);
            sp->append(halffauheadSym);
            sp->append(fauheadSym);
            sp->append(halffadheadSym);
            sp->append(fadheadSym);
            sp->append(wholelaheadSym);
            sp->append(halflaheadSym);
            sp->append(laheadSym);
            sp->append(wholeteheadSym);
            sp->append(halfteheadSym);
            sp->append(letterfSym);
            sp->append(lettermSym);
            sp->append(letterpSym);
            sp->append(letterrSym);
            sp->append(lettersSym);
            sp->append(letterzSym);
            paletteBox->addPalette(tr("Symbols"), sp);
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
      popup->addAction(tr("System Brackets..."), this, SLOT(bracketMenu()));
      popup->addAction(tr("Note Attributes..."), this, SLOT(noteAttributesMenu()));
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
//      text->addAction(getAction("metronome"));

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
            Palette* sp = new Palette();
            clefPalette = new PaletteScrollArea(sp);
            clefPalette->setRestrictHeight(false);
            clefPalette->setWindowTitle(tr("MuseScore: Clefs"));
            sp->setGrid(60, 80);
            sp->showStaff(true);
            for (int i = 0; i < 16; ++i) {
                  Clef* k = new ::Clef(gscore, i);
                  sp->append(k, tr(clefTable[i].name));
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
            Palette* sp = new Palette();
            keyPalette = new PaletteScrollArea(sp);
            keyPalette->setRestrictHeight(false);
            keyPalette->setWindowTitle(tr("MuseScore: Key Signature"));
            sp->setGrid(80, 60);
            sp->showStaff(true);
            for (int i = 0; i < 7; ++i) {
                  KeySig* k = new KeySig(gscore);
                  k->setSubtype(i+1);
                  sp->append(k, keyNames[i*2]);
                  }
            for (int i = -7; i < 0; ++i) {
                  KeySig* k = new KeySig(gscore);
                  k->setSubtype(i & 0xff);
                  sp->append(k, keyNames[(7 + i) * 2 + 1]);
                  }
            KeySig* k = new KeySig(gscore);
            k->setSubtype(0);
            sp->append(k, keyNames[14]);
            k = new KeySig(gscore);
            k->setSubtype(0);
            sp->append(k, keyNames[14]);
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
            Palette* sp = new Palette();
            linePalette = new PaletteScrollArea(sp);
            linePalette->setRestrictHeight(false);
            linePalette->setWindowTitle(tr("MuseScore: Lines"));
            sp->setGrid(100, 30);

            double l = _spatium * 8;

            Hairpin* gabel0 = new Hairpin(gscore);
            gabel0->setSubtype(0);
            gabel0->setLen(l);
            sp->append(gabel0, tr("crescendo"));

            Hairpin* gabel1 = new Hairpin(gscore);
            gabel1->setSubtype(1);
            gabel1->setLen(l);
            sp->append(gabel1, tr("diminuendo"));

            Volta* volta = new Volta(gscore);
            volta->setLen(l);
            volta->setText("1.");
            QList<int> il;
            il.clear();
            il.append(1);
            volta->setEndings(il);
            volta->setSubtype(Volta::VOLTA_CLOSED);

            sp->append(volta, tr("prima volta"));

            volta = new Volta(gscore);
            volta->setLen(l);
            volta->setText("2.");
            il.clear();
            il.append(2);
            volta->setEndings(il);
            volta->setSubtype(Volta::VOLTA_CLOSED);
            sp->append(volta, tr("seconda volta"));

            volta = new Volta(gscore);
            volta->setLen(l);
            volta->setText("3.");
            il.clear();
            il.append(3);
            volta->setEndings(il);
            volta->setSubtype(Volta::VOLTA_CLOSED);
            sp->append(volta, tr("terza volta"));

            volta = new Volta(gscore);
            volta->setLen(l);
            volta->setText("2.");
            il.clear();
            il.append(2);
            volta->setEndings(il);
            volta->setSubtype(Volta::VOLTA_OPEN);
            sp->append(volta, tr("seconda volta"));

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
            sp->append(pedal, tr("pedal"));

            Trill* trill = new Trill(gscore);
            trill->setLen(l);
            sp->append(trill, tr("trill line"));

            TextLine* textLine = new TextLine(gscore);
            textLine->setText("VII");
            textLine->setLen(l);
            sp->append(textLine, tr("text line"));

            TextLine* line = new TextLine(gscore);
            line->setLen(l);
            line->setHasText(false);
            line->setHook(false);
            sp->append(line, tr("line"));
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
            bracketPalette->setWindowTitle(tr("MuseScore: System Brackets"));
            sp->setGrid(40, 80);

            Bracket* b1 = new Bracket(gscore);
            b1->setSubtype(BRACKET_NORMAL);
            Bracket* b2 = new Bracket(gscore);
            b2->setSubtype(BRACKET_AKKOLADE);
            b1->setHeight(_spatium * 7);
            b2->setHeight(_spatium * 7);

            sp->append(b1, "Bracket");
            sp->append(b2, "Akkolade");

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
            noteAttributesPalette = new PaletteScrollArea(sp);
            noteAttributesPalette->setRestrictHeight(false);
            noteAttributesPalette->setWindowTitle(tr("MuseScore: Note Attributes"));
            unsigned nn = NOTE_ATTRIBUTES;
            sp->setGrid(42, 30);

            for (unsigned i = 0; i < nn; ++i) {
                  Articulation* s = new Articulation(gscore);
                  s->setSubtype(i);
                  sp->append(s, s->subtypeName());
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
            accidentalsPalette = new PaletteScrollArea(sp);
            accidentalsPalette->setRestrictHeight(false);
            accidentalsPalette->setWindowTitle(tr("MuseScore: Accidentals"));
            sp->setGrid(60, 60);

            for (int i = 0; i < 16+6+4; ++i) {
                  Accidental* s = new Accidental(gscore);
                  s->setSubtype(i);
                  sp->append(s, s->name());
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
            Palette* sp = new Palette();
            dynamicsPalette = new PaletteScrollArea(sp);
            dynamicsPalette->setRestrictHeight(false);
            dynamicsPalette->setWindowTitle(tr("MuseScore: Dynamics"));
            sp->setGrid(90, 40);

            for (int i = 0; i < 27; ++i) {
                  Dynamic* dynamic = new Dynamic(gscore);
                  dynamic->setSubtype(i + 1);
                  sp->append(dynamic, dynamic->subtypeName());
                  }

            const char* expr[] = {
                  "crescendo", "diminuendo", "dolce", "espressivo",
                  "legato", "leggiero", "marcato", "mero", "molto"
                  };
            for (unsigned int i = 0; i < sizeof(expr) / sizeof(*expr); ++i) {
                  Dynamic* d = new Dynamic(gscore);
                  d->setSubtype(0);
                  d->setText(expr[i]);
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
            barPalette = new PaletteScrollArea(sp, 0);
            barPalette->setRestrictHeight(false);
            barPalette->setWindowTitle(tr("MuseScore: Barlines"));
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
                  sp->append(b, t[i].name);
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
            Palette* sp = new Palette(1.5);
            fingeringPalette = new PaletteScrollArea(sp);
            fingeringPalette->setRestrictHeight(false);
            fingeringPalette->setWindowTitle(tr("MuseScore: Fingering"));
            sp->setGrid(50, 50);

            const char finger[] = "012345pimac";
            Text* k;

            for (unsigned i = 0; i < strlen(finger); ++i) {
                  k = new Text(gscore);
                  k->setSubtype(TEXT_FINGERING);
                  k->setText(QString(finger[i]));
                  sp->append(k, QString("fingering %1").arg(finger[i]));
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
            tempomap->addTempo(tick, bps);
            TempoText* tt = new TempoText(this);
            tt->setTick(tick);
            tt->setTrack(cr->track());
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
            Palette* sp = new Palette();
            layoutBreakPalette = new PaletteScrollArea(sp);
            layoutBreakPalette->setRestrictHeight(false);
            layoutBreakPalette->setWindowTitle(tr("MuseScore: Layout Breaks"));
            sp->setGrid(80, 80);
            LayoutBreak* lb = new LayoutBreak(gscore);
            lb->setSubtype(LAYOUT_BREAK_LINE);
            sp->append(lb, tr("break line"));
            lb = new LayoutBreak(gscore);
            lb->setSubtype(LAYOUT_BREAK_PAGE);
            sp->append(lb, tr("break page"));
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

      PadState* padState = cs->padState();
      Drumset* ds        = padState->drumset;
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
                        chord->setTickLen(division);
                        chord->setDuration(Duration::V_QUARTER);
                        chord->setStemDirection(dir);
                        chord->setTrack(voice);
                        Note* note = new Note(gscore);
                        note->setParent(chord);
                        note->setTrack(voice);
                        note->setPitch(pitch);
                        note->setLine(line);
                        note->setPos(0.0, _spatium * .5 * line);
                        note->setHeadGroup(noteHead);
                        chord->add(note);
                        Stem* stem = new Stem(gscore);
                        stem->setLen(Spatium(up ? -3.0 : 3.0));
                        chord->setStem(stem);
                        stem->setPos(note->stemPos(up));
                        drumPalette->append(chord, drumset->name(pitch));

                        ++i;
                        }
                  }
            }
      if (drumset) {
            int i = 0;
            drumPalette->setSelected(-1);
            for (int pitch = 0; pitch < 128; ++pitch) {
                  if (drumset->isValid(pitch)) {
                        if (pitch == padState->drumNote) {
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
      PadState* padState = cs->padState();
      Drumset* ds        = padState->drumset;
      if (ds == 0)
            return;
      int i = 0;
      for (int pitch = 0; pitch < 128; ++pitch) {
            if (!drumset->isValid(pitch))
                  continue;
            if (i == idx) {
                  padState->drumNote = pitch;
                  padState->voice    = ds->voice(pitch);
//                  printf("drumNote %d voice %d\n", pitch, padState->voice);
                  cs->setPadState();
                  break;
                  }
            ++i;
            }
      }

