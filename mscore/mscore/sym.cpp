//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include "globals.h"
#include "style.h"
#include "sym.h"
#include "utils.h"
#include "mscore.h"
#include "score.h"

QVector<Sym> symbols(lastSym);
QMap<const char*, SymCode*> charReplaceMap;

struct SymbolNames {
      int msIndex;
      const char* mname;
      const char* name;
      };

SymbolNames lilypondNames[] = {
      { wholerestSym,         QT_TRANSLATE_NOOP("symbol", "Whole rest"),           "rests.0" },
      { halfrestSym,          QT_TRANSLATE_NOOP("symbol", "Half rest"),            "rests.1" },
      { outsidewholerestSym,  QT_TRANSLATE_NOOP("symbol", "Outside whole rest"),   "rests.0o" },
      { outsidehalfrestSym,   QT_TRANSLATE_NOOP("symbol", "Outside half rest"),    "rests.1o" },
      { rest_M3,              QT_TRANSLATE_NOOP("symbol", "Rest M3"),              "rests.M3" },
      { breverestSym,         QT_TRANSLATE_NOOP("symbol", "Breve rest"),           "rests.M2" },
      { longarestSym,         QT_TRANSLATE_NOOP("symbol", "Longa rest"),           "rests.M1" },
      { rest4Sym,             QT_TRANSLATE_NOOP("symbol", "Quarter rest"),         "rests.2" },
      { clasquartrestSym,     QT_TRANSLATE_NOOP("symbol", "Classical quarter rest"),  "rests.2classical" },
      { rest8Sym,             QT_TRANSLATE_NOOP("symbol", "eighth rest"),          "rests.3" },
      { rest16Sym,            QT_TRANSLATE_NOOP("symbol", "16th rest"),            "rests.4" },
      { rest32Sym,            QT_TRANSLATE_NOOP("symbol", "32th rest"),            "rests.5" },
      { rest64Sym,            QT_TRANSLATE_NOOP("symbol", "64th rest"),            "rests.6" },
      { rest128Sym,           QT_TRANSLATE_NOOP("symbol", "128th rest"),           "rests.7" },

      { sharpSym,             QT_TRANSLATE_NOOP("symbol", "Sharp"),                "accidentals.sharp" },
      { sharpArrowUpSym,      QT_TRANSLATE_NOOP("symbol", "Sharp arrow up"),       "accidentals.sharp.arrowup" },
      { sharpArrowDownSym,    QT_TRANSLATE_NOOP("symbol", "Sharp arrow both"),     "accidentals.sharp.arrowdown" },
      { sharpArrowBothSym,    QT_TRANSLATE_NOOP("symbol", "Sharp arrow both"),     "accidentals.sharp.arrowboth" },
      { sharpslashSym,        QT_TRANSLATE_NOOP("symbol", "Sharp slash"),          "accidentals.sharp.slashslash.stem" },
      { sharpslash2Sym,       QT_TRANSLATE_NOOP("symbol", "Sharp slash2"),         "accidentals.sharp.slashslashslash.stemstem" },
      { sharpslash3Sym,       QT_TRANSLATE_NOOP("symbol", "Sharp slash3"),         "accidentals.sharp.slashslashslash.stem" },
      { sharpslash4Sym,       QT_TRANSLATE_NOOP("symbol", "Sharp slash4"),         "accidentals.sharp.slashslash.stemstemstem" },
      { naturalSym,           QT_TRANSLATE_NOOP("symbol", "Natural"),              "accidentals.natural" },
      { naturalArrowUpSym,    QT_TRANSLATE_NOOP("symbol", "Natural arrow up"),     "accidentals.natural.arrowup" },
      { naturalArrowDownSym,  QT_TRANSLATE_NOOP("symbol", "Natural arrow down"),   "accidentals.natural.arrowdown" },
      { naturalArrowBothSym,  QT_TRANSLATE_NOOP("symbol", "Natural arrow both"),   "accidentals.natural.arrowboth" },
      { flatSym,              QT_TRANSLATE_NOOP("symbol", "Flat"),                 "accidentals.flat" },
      { flatArrowUpSym,       QT_TRANSLATE_NOOP("symbol", "Flat arrow up"),        "accidentals.flat.arrowup" },
      { flatArrowDownSym,     QT_TRANSLATE_NOOP("symbol", "Flat arrow both"),      "accidentals.flat.arrowdown" },
      { flatArrowBothSym,     QT_TRANSLATE_NOOP("symbol", "Flat arrow both"),      "accidentals.flat.arrowboth" },
      { flatslashSym,         QT_TRANSLATE_NOOP("symbol", "Flat slash"),           "accidentals.flat.slash" },
      { flatslash2Sym,        QT_TRANSLATE_NOOP("symbol", "Flat slash2"),          "accidentals.flat.slashslash" },
      { mirroredflat2Sym,     QT_TRANSLATE_NOOP("symbol", "Mirrored flat2"),       "accidentals.mirroredflat.flat" },
      { mirroredflatSym,      QT_TRANSLATE_NOOP("symbol", "Mirrored flat"),        "accidentals.mirroredflat" },
      { mirroredflatslashSym, QT_TRANSLATE_NOOP("symbol", "Mirrored flat slash"),  "accidentals.mirroredflat.backslash" },
      { flatflatSym,          QT_TRANSLATE_NOOP("symbol", "Flat flat"),            "accidentals.flatflat" },
      { flatflatslashSym,     QT_TRANSLATE_NOOP("symbol", "Flat flat slash"),      "accidentals.flatflat.slash" },
      { sharpsharpSym,        QT_TRANSLATE_NOOP("symbol", "Sharp sharp"),          "accidentals.doublesharp" },
      { rightparenSym,        QT_TRANSLATE_NOOP("symbol", "Right parenthesis"),    "accidentals.rightparen" },
      { leftparenSym,         QT_TRANSLATE_NOOP("symbol", "Left parenthesis"),     "accidentals.leftparen" },

      { -1,                   "",                                                  "arrowheads.open.01" },
      { -1,                   "",                                                  "arrowheads.open.0M1" },
      { -1,                   "",                                                  "arrowheads.open.11" },
      { -1,                   "",                                                  "arrowheads.open.1M1" },
      { -1,                   "",                                                  "arrowheads.close.01" },
      { -1,                   "",                                                  "arrowheads.close.0M1" },
      { -1,                   "",                                                  "arrowheads.close.11" },
      { -1,                   "",                                                  "arrowheads.close.1M1" },

      { dotSym,               QT_TRANSLATE_NOOP("symbol", "Dot"),                  "dots.dot" },
      { longaupSym,           QT_TRANSLATE_NOOP("symbol", "Longa up"),             "noteheads.uM2" },
      { longadownSym,         QT_TRANSLATE_NOOP("symbol", "Longa down"),           "noteheads.dM2" },
      { brevisheadSym,        QT_TRANSLATE_NOOP("symbol", "Brevis head"),          "noteheads.sM1" },
      { brevisdoubleheadSym,  QT_TRANSLATE_NOOP("symbol", "Brevis double head"),   "noteheads.sM1double" },
      { wholeheadSym,         QT_TRANSLATE_NOOP("symbol", "Whole head"),           "noteheads.s0" },
      { halfheadSym,          QT_TRANSLATE_NOOP("symbol", "Half head"),            "noteheads.s1" },
      { quartheadSym,         QT_TRANSLATE_NOOP("symbol", "Quarter head"),         "noteheads.s2" },
      { wholediamondheadSym,  QT_TRANSLATE_NOOP("symbol", "Whole diamond head"),   "noteheads.s0diamond" },
      { halfdiamondheadSym,   QT_TRANSLATE_NOOP("symbol", "Half diamond head"),    "noteheads.s1diamond" },
      { diamondheadSym,       QT_TRANSLATE_NOOP("symbol", "Diamond head"),         "noteheads.s2diamond" },
      { s0triangleHeadSym,    QT_TRANSLATE_NOOP("symbol", "Whole triangle head"),  "noteheads.s0triangle" },
      { d1triangleHeadSym,    QT_TRANSLATE_NOOP("symbol", "Down half triangle head"), "noteheads.d1triangle" },
      { u1triangleHeadSym,    QT_TRANSLATE_NOOP("symbol", "Up half triangle head"), "noteheads.u1triangle" },
      { u2triangleHeadSym,    QT_TRANSLATE_NOOP("symbol", "Up quart triangle head"), "noteheads.u2triangle" },
      { d2triangleHeadSym,    QT_TRANSLATE_NOOP("symbol", "Down quart triangle head"), "noteheads.d2triangle" },
      { wholeslashheadSym,    QT_TRANSLATE_NOOP("symbol", "Whole slash head"),     "noteheads.s0slash" },
      { halfslashheadSym,     QT_TRANSLATE_NOOP("symbol", "Half slash head"),      "noteheads.s1slash" },
      { quartslashheadSym,    QT_TRANSLATE_NOOP("symbol", "Quarter slash head"),   "noteheads.s2slash" },
      { wholecrossedheadSym,  QT_TRANSLATE_NOOP("symbol", "Whole cross head"),     "noteheads.s0cross" },
      { halfcrossedheadSym,   QT_TRANSLATE_NOOP("symbol", "Half cross head"),      "noteheads.s1cross" },
      { crossedheadSym,       QT_TRANSLATE_NOOP("symbol", "Cross head"),           "noteheads.s2cross" },
      { xcircledheadSym,      QT_TRANSLATE_NOOP("symbol", "X circle head"),        "noteheads.s2xcircle" },
      { s0doHeadSym,          QT_TRANSLATE_NOOP("symbol", "s0do head"),            "noteheads.s0do" },
      { d1doHeadSym,          QT_TRANSLATE_NOOP("symbol", "d1do head"),            "noteheads.d1do" },
      { u1doHeadSym,          QT_TRANSLATE_NOOP("symbol", "u1do head"),            "noteheads.u1do" },
      { d2doHeadSym,          QT_TRANSLATE_NOOP("symbol", "d2do head"),            "noteheads.d2do" },
      { u2doHeadSym,          QT_TRANSLATE_NOOP("symbol", "u2do head"),            "noteheads.u2do" },
      { s0reHeadSym,          QT_TRANSLATE_NOOP("symbol", "s0re head"),            "noteheads.s0re" },
      { u1reHeadSym,          QT_TRANSLATE_NOOP("symbol", "u1re head"),            "noteheads.u1re" },
      { d1reHeadSym,          QT_TRANSLATE_NOOP("symbol", "d1re head"),            "noteheads.d1re" },
      { u2reHeadSym,          QT_TRANSLATE_NOOP("symbol", "u2re head"),            "noteheads.u2re" },
      { d2reHeadSym,          QT_TRANSLATE_NOOP("symbol", "d2re head"),            "noteheads.d2re" },
      { s0miHeadSym,          QT_TRANSLATE_NOOP("symbol", "s0mi head"),            "noteheads.s0mi" },
      { s1miHeadSym,          QT_TRANSLATE_NOOP("symbol", "s1mi head"),            "noteheads.s1mi" },
      { s2miHeadSym,          QT_TRANSLATE_NOOP("symbol", "s2mi head"),            "noteheads.s2mi" },
      { u0faHeadSym,          QT_TRANSLATE_NOOP("symbol", "u0fa head"),            "noteheads.u0fa" },
      { d0faHeadSym,          QT_TRANSLATE_NOOP("symbol", "d0fa head"),            "noteheads.d0fa" },
      { u1faHeadSym,          QT_TRANSLATE_NOOP("symbol", "u1fa head"),            "noteheads.u1fa" },
      { d1faHeadSym,          QT_TRANSLATE_NOOP("symbol", "d1fa head"),            "noteheads.d1fa" },
      { u2faHeadSym,          QT_TRANSLATE_NOOP("symbol", "u2fa head"),            "noteheads.u2fa" },
      { d2faHeadSym,          QT_TRANSLATE_NOOP("symbol", "d2fa head"),            "noteheads.d2fa" },
      { s0laHeadSym,          QT_TRANSLATE_NOOP("symbol", "s0la head"),            "noteheads.s0la" },
      { s1laHeadSym,          QT_TRANSLATE_NOOP("symbol", "s1la head"),            "noteheads.s1la" },
      { s2laHeadSym,          QT_TRANSLATE_NOOP("symbol", "s2la head"),            "noteheads.s2la" },
      { s0tiHeadSym,          QT_TRANSLATE_NOOP("symbol", "s0ti head"),            "noteheads.s0ti" },
      { u1tiHeadSym,          QT_TRANSLATE_NOOP("symbol", "u1ti head"),            "noteheads.u1ti" },
      { d1tiHeadSym,          QT_TRANSLATE_NOOP("symbol", "d1ti head"),            "noteheads.d1ti" },
      { u2tiHeadSym,          QT_TRANSLATE_NOOP("symbol", "u2ti head"),            "noteheads.u2ti" },
      { d2tiHeadSym,          QT_TRANSLATE_NOOP("symbol", "d2ti head"),            "noteheads.d2ti" },
      { ufermataSym,          QT_TRANSLATE_NOOP("symbol", "ufermata"),             "scripts.ufermata" },
      { dfermataSym,          QT_TRANSLATE_NOOP("symbol", "dfermata"),             "scripts.dfermata" },
      { snappizzicatoSym,     QT_TRANSLATE_NOOP("symbol", "snappizzicato"),        "scripts.snappizzicato" },
      { ushortfermataSym,     QT_TRANSLATE_NOOP("symbol", "ushortfermata"),        "scripts.ushortfermata" },
      { dshortfermataSym,     QT_TRANSLATE_NOOP("symbol", "dshortfermata"),        "scripts.dshortfermata" },
      { ulongfermataSym,      QT_TRANSLATE_NOOP("symbol", "ulongfermata"),         "scripts.ulongfermata" },
      { dlongfermataSym,      QT_TRANSLATE_NOOP("symbol", "dlongfermata"),         "scripts.dlongfermata" },
      { uverylongfermataSym,  QT_TRANSLATE_NOOP("symbol", "uverylongfermata"),     "scripts.uverylongfermata" },
      { dverylongfermataSym,  QT_TRANSLATE_NOOP("symbol", "dverylongfermata"),     "scripts.dverylongfermata" },
      { thumbSym,             QT_TRANSLATE_NOOP("symbol", "thumb"),                "scripts.thumb" },
      { sforzatoaccentSym,    QT_TRANSLATE_NOOP("symbol", "sforza to accent"),     "scripts.sforzato" },
      { esprSym,              QT_TRANSLATE_NOOP("symbol", "espressivo"),           "scripts.espr" },
      { staccatoSym,          QT_TRANSLATE_NOOP("symbol", "staccato"),             "scripts.staccato" },
      { ustaccatissimoSym,    QT_TRANSLATE_NOOP("symbol", "ustaccatissimo"),       "scripts.ustaccatissimo" },
      { dstaccatissimoSym,    QT_TRANSLATE_NOOP("symbol", "dstaccatissimo"),       "scripts.dstaccatissimo" },
      { tenutoSym,            QT_TRANSLATE_NOOP("symbol", "tenuto"),               "scripts.tenuto" },
      { uportatoSym,          QT_TRANSLATE_NOOP("symbol", "uportato"),             "scripts.uportato" },
      { dportatoSym,          QT_TRANSLATE_NOOP("symbol", "dportato"),             "scripts.dportato" },
      { umarcatoSym,          QT_TRANSLATE_NOOP("symbol", "umarcato"),             "scripts.umarcato" },
      { dmarcatoSym,          QT_TRANSLATE_NOOP("symbol", "dmarcato"),             "scripts.dmarcato" },
      { ouvertSym,            QT_TRANSLATE_NOOP("symbol", "Ouvert"),               "scripts.open" },
      { plusstopSym,          QT_TRANSLATE_NOOP("symbol", "Plus stop"),            "scripts.stopped" },
      { upbowSym,             QT_TRANSLATE_NOOP("symbol", "Up bow"),               "scripts.upbow" },
      { downbowSym,           QT_TRANSLATE_NOOP("symbol", "Down bow"),             "scripts.downbow" },
      { reverseturnSym,       QT_TRANSLATE_NOOP("symbol", "Reverse turn"),         "scripts.reverseturn" },
      { turnSym,              QT_TRANSLATE_NOOP("symbol", "Turn"),                 "scripts.turn"        },
      { trillSym,             QT_TRANSLATE_NOOP("symbol", "Trill"),                "scripts.trill"       },
      { upedalheelSym,        QT_TRANSLATE_NOOP("symbol", "upedal heel"),          "scripts.upedalheel"  },
      { dpedalheelSym,        QT_TRANSLATE_NOOP("symbol", "dpedal heel"),          "scripts.dpedalheel"  },
      { upedaltoeSym,         QT_TRANSLATE_NOOP("symbol", "upedal toe"),           "scripts.upedaltoe"   },
      { dpedaltoeSym,         QT_TRANSLATE_NOOP("symbol", "dpedal toe"),           "scripts.dpedaltoe"   },
      { flageoletSym,         QT_TRANSLATE_NOOP("symbol", "Flageolet"),            "scripts.flageolet"   },
      { segnoSym,             QT_TRANSLATE_NOOP("symbol", "Segno"),                "scripts.segno"       },
      { codaSym,              QT_TRANSLATE_NOOP("symbol", "Coda"),                 "scripts.coda"        },
      { varcodaSym,           QT_TRANSLATE_NOOP("symbol", "Varied coda"),          "scripts.varcoda"     },
      { rcommaSym,            QT_TRANSLATE_NOOP("symbol", "rcomma"),               "scripts.rcomma"      },
      { lcommaSym,            QT_TRANSLATE_NOOP("symbol", "lcomma"),               "scripts.lcomma"      },
      { -1,                   "",                                                  "scripts.rvarcomma" },
      { -1,                   "",                                                  "scripts.lvarcomma" },
      { arpeggioSym,          QT_TRANSLATE_NOOP("symbol", "Arpeggio"),             "scripts.arpeggio" },
      { trillelementSym,      QT_TRANSLATE_NOOP("symbol", "Trill element"),        "scripts.trill_element" },
      { arpeggioarrowdownSym, QT_TRANSLATE_NOOP("symbol", "Arpeggio arrow down"),  "scripts.arpeggio.arrow.M1" },
      { arpeggioarrowupSym,   QT_TRANSLATE_NOOP("symbol", "Arpeggio arrow up"),    "scripts.arpeggio.arrow.1" },
      { trilelementSym,       QT_TRANSLATE_NOOP("symbol", "Trill element"),        "scripts.trilelement" },
      { prallSym,             QT_TRANSLATE_NOOP("symbol", "Prall"),                "scripts.prall" },
      { mordentSym,           QT_TRANSLATE_NOOP("symbol", "Mordent"),              "scripts.mordent" },
      { prallprallSym,        QT_TRANSLATE_NOOP("symbol", "Prall prall"),          "scripts.prallprall" },
      { prallmordentSym,      QT_TRANSLATE_NOOP("symbol", "Prall mordent"),        "scripts.prallmordent" },
      { upprallSym,           QT_TRANSLATE_NOOP("symbol", "Up prall"),             "scripts.upprall" },
      { upmordentSym,         QT_TRANSLATE_NOOP("symbol", "Up mordent"),           "scripts.upmordent" },
      { pralldownSym,         QT_TRANSLATE_NOOP("symbol", "Prall down"),           "scripts.pralldown" },
      { downprallSym,         QT_TRANSLATE_NOOP("symbol", "Down prall"),           "scripts.downprall" },
      { downmordentSym,       QT_TRANSLATE_NOOP("symbol", "Down mordent"),         "scripts.downmordent" },
      { prallupSym,           QT_TRANSLATE_NOOP("symbol", "Prall up"),             "scripts.prallup" },
      { lineprallSym,         QT_TRANSLATE_NOOP("symbol", "Line prall"),           "scripts.lineprall" },
      { caesuraCurvedSym,     QT_TRANSLATE_NOOP("symbol", "Caesura curved"),       "scripts.caesura.curved" },
      { caesuraStraight,      QT_TRANSLATE_NOOP("symbol", "Caesura straight"),     "scripts.caesura.straight" },
      { eighthflagSym,        QT_TRANSLATE_NOOP("symbol", "Eighth flag"),          "flags.u3" },
      { sixteenthflagSym,     QT_TRANSLATE_NOOP("symbol", "16th flag"),            "flags.u4" },
      { thirtysecondflagSym,  QT_TRANSLATE_NOOP("symbol", "32nd flag"),            "flags.u5" },
      { sixtyfourthflagSym,   QT_TRANSLATE_NOOP("symbol", "64th flag"),            "flags.u6" },
      { flag128Sym,           QT_TRANSLATE_NOOP("symbol", "128th flag"),           "flags.u7" },
      { deighthflagSym,       QT_TRANSLATE_NOOP("symbol", "deighth flag"),         "flags.d3" },
      { gracedashSym,         QT_TRANSLATE_NOOP("symbol", "Grace dash"),           "flags.ugrace" },
      { dgracedashSym,        QT_TRANSLATE_NOOP("symbol", "dgrace dash"),          "flags.dgrace" },
      { dsixteenthflagSym,    QT_TRANSLATE_NOOP("symbol", "dsixteenth flag"),      "flags.d4" },
      { dthirtysecondflagSym, QT_TRANSLATE_NOOP("symbol", "d32nd flag"),           "flags.d5" },
      { dsixtyfourthflagSym,  QT_TRANSLATE_NOOP("symbol", "d64th flag"),           "flags.d6" },
      { dflag128Sym,          QT_TRANSLATE_NOOP("symbol", "d128th flag"),          "flags.d7" },
      { altoclefSym,          QT_TRANSLATE_NOOP("symbol", "Alto clef"),            "clefs.C" },
      { caltoclefSym,         QT_TRANSLATE_NOOP("symbol", "calto clef"),           "clefs.C_change" },
      { bassclefSym,          QT_TRANSLATE_NOOP("symbol", "Bass clef"),            "clefs.F" },
      { cbassclefSym,         QT_TRANSLATE_NOOP("symbol", "cbass clef"),           "clefs.F_change" },
      { trebleclefSym,        QT_TRANSLATE_NOOP("symbol", "Treble clef"),          "clefs.G" },
      { ctrebleclefSym,       QT_TRANSLATE_NOOP("symbol", "ctreble clef"),         "clefs.G_change" },
      { percussionclefSym,    QT_TRANSLATE_NOOP("symbol", "Percussion clef"),      "clefs.percussion" },
      { cpercussionclefSym,   QT_TRANSLATE_NOOP("symbol", "cpercussion clef"),     "clefs.percussion_change" },
      { tabclefSym,           QT_TRANSLATE_NOOP("symbol", "Tab clef"),             "clefs.tab" },
      { ctabclefSym,          QT_TRANSLATE_NOOP("symbol", "ctab clef"),            "clefs.tab_change" },
      { fourfourmeterSym,     QT_TRANSLATE_NOOP("symbol", "Common meter"),         "timesig.C44" },
      { allabreveSym,         QT_TRANSLATE_NOOP("symbol", "Alla breve"),           "timesig.C22" },
      { pedalasteriskSym,     QT_TRANSLATE_NOOP("symbol", "Pedal asterisk"),       "pedal.*" },
      { pedaldashSym,         QT_TRANSLATE_NOOP("symbol", "Pedal dash"),           "pedal.M" },
      { pedaldotSym,          QT_TRANSLATE_NOOP("symbol", "Pedal dot"),            "pedal.." },
      { pedalPSym,            QT_TRANSLATE_NOOP("symbol", "Pedal P"),              "pedal.P" },
      { pedaldSym,            QT_TRANSLATE_NOOP("symbol", "Pedal d"),              "pedal.d" },
      { pedaleSym,            QT_TRANSLATE_NOOP("symbol", "Pedal e"),              "pedal.e" },
      { pedalPedSym,          QT_TRANSLATE_NOOP("symbol", "Pedal ped"),            "pedal.Ped" },
      { brackettipsRightUp,   QT_TRANSLATE_NOOP("symbol", "Bracket tips up"),      "brackettips.uright"     },
      { brackettipsRightDown, QT_TRANSLATE_NOOP("symbol", "Bracket tips down"),    "brackettips.dright"     },
      { brackettipsLeftUp,    QT_TRANSLATE_NOOP("symbol", "Bracket tips left up"), "brackettips.uleft"      },
      { brackettipsLeftDown,  QT_TRANSLATE_NOOP("symbol", "Bracket tips left down"), "brackettips.dleft"      },
      { accDotSym,            QT_TRANSLATE_NOOP("symbol", "Accordion dot"),              "accordion.accDot"       },
      { accFreebaseSym,       QT_TRANSLATE_NOOP("symbol", "Accordion freebase"),         "accordion.accFreebase"  },
      { accStdbaseSym,        QT_TRANSLATE_NOOP("symbol", "Accordion stdbase"),          "accordion.accStdbase"   },
      { accBayanbaseSym,      QT_TRANSLATE_NOOP("symbol", "Accordion bayanbase"),        "accordion.accBayanbase" },
      { accOldEESym,          QT_TRANSLATE_NOOP("symbol", "Accordion old ee"),           "accordion.accOldEE"     },
      { accDiscantSym,        QT_TRANSLATE_NOOP("symbol", "Accordion discant"),          "accordion.accDiscant"   },
      { -1,                   "",                                                  "left up"               },
      { -1,                   "",                                                  "left down"             },
      { -1,                   "",                                                  "plus"                  },
      { -1,                   "",                                                  "comma"                 },
      { -1,                   "",                                                  "hyphen"                },
      { -1,                   "",                                                  "period"                },
      { zeroSym,              QT_TRANSLATE_NOOP("symbol", "Zero"),                 "zero" },
      { oneSym,               QT_TRANSLATE_NOOP("symbol", "One"),                  "one" },
      { twoSym,               QT_TRANSLATE_NOOP("symbol", "Two"),                  "two" },
      { threeSym,             QT_TRANSLATE_NOOP("symbol", "Three"),                "three" },
      { fourSym,              QT_TRANSLATE_NOOP("symbol", "Four"),                 "four" },
      { fiveSym,              QT_TRANSLATE_NOOP("symbol", "Five"),                 "five" },
      { sixSym,               QT_TRANSLATE_NOOP("symbol", "Six"),                  "six" },
      { sevenSym,             QT_TRANSLATE_NOOP("symbol", "Seven"),                "seven" },
      { eightSym,             QT_TRANSLATE_NOOP("symbol", "Eight"),                "eight" },
      { nineSym,              QT_TRANSLATE_NOOP("symbol", "Nine"),                 "nine" },
      { plusSym,              QT_TRANSLATE_NOOP("symbol", "Plus"),                 "plus" },
      { -1,                   "",                                                  "space" },
      { letterzSym,           QT_TRANSLATE_NOOP("symbol", "z"),                    "z" },
      { letterfSym,           QT_TRANSLATE_NOOP("symbol", "f"),                    "f" },
      { lettersSym,           QT_TRANSLATE_NOOP("symbol", "s"),                    "s" },
      { letterpSym,           QT_TRANSLATE_NOOP("symbol", "p"),                    "p" },
      { lettermSym,           QT_TRANSLATE_NOOP("symbol", "m"),                    "m" },
      { letterrSym,           QT_TRANSLATE_NOOP("symbol", "r"),                    "r" },
      };

SymCode pSymbols[] = {
      SymCode(0xe10e, 1),    //natural
      SymCode(0xe10c, 1),    // sharp
      SymCode(0xe10d, 1),    // flat
      SymCode(0xe104, 1),    // note2_Sym
      SymCode(0xe105, 1),    // note4_Sym
      SymCode(0xe106, 1),    // note8_Sym
      SymCode(0xe107, 1),    // note16_Sym
      SymCode(0xe108, 1),    // note32_Sym
      SymCode(0xe109, 1),    // note64_Sym
      SymCode(0xe10a, 1),    // dot
      SymCode(0xe10b, 1),    // dotdot
      SymCode(0xe167, 1),    // coda
      SymCode(0xe168, 1),    // varcoda
      SymCode(0xe169, 1),    // segno
      SymCode(0, 0),
      SymCode(0xa9,   -1, "(C)", SYMBOL_COPYRIGHT),
      SymCode(0x00c0, -1),
      SymCode(0x00c1, -1),
      SymCode(0x00c2, -1),
      SymCode(0x00c3, -1),
      SymCode(0x00c4, -1),
      SymCode(0x00c5, -1),
      SymCode(0x00c6, -1),
      SymCode(0x00c7, -1),
      SymCode(0x00c8, -1),
      SymCode(0x00c9, -1),
      SymCode(0x00ca, -1),
      SymCode(0x00cb, -1),
      SymCode(0x00cc, -1),
      SymCode(0x00cd, -1),
      SymCode(0x00ce, -1),
      SymCode(0x00cf, -1),

      SymCode(0x00d0, -1),
      SymCode(0x00d1, -1),
      SymCode(0x00d2, -1),
      SymCode(0x00d3, -1),
      SymCode(0x00d4, -1),
      SymCode(0x00d5, -1),
      SymCode(0x00d6, -1),
      SymCode(0x00d7, -1),
      SymCode(0x00d8, -1),
      SymCode(0x00d9, -1),
      SymCode(0x00da, -1),
      SymCode(0x00db, -1),
      SymCode(0x00dc, -1),
      SymCode(0x00dd, -1),
      SymCode(0x00de, -1),
      SymCode(0x00df, -1),

      SymCode(0x00e0, -1),
      SymCode(0x00e1, -1),
      SymCode(0x00e2, -1),
      SymCode(0x00e3, -1),
      SymCode(0x00e4, -1),
      SymCode(0x00e5, -1),
      SymCode(0x00e6, -1),
      SymCode(0x00e7, -1),
      SymCode(0x00e8, -1),
      SymCode(0x00e9, -1),
      SymCode(0x00ea, -1),
      SymCode(0x00eb, -1),
      SymCode(0x00ec, -1),
      SymCode(0x00ed, -1),
      SymCode(0x00ee, -1),
      SymCode(0x00ef, -1),

      SymCode(0x00f0, -1),
      SymCode(0x00f1, -1),
      SymCode(0x00f2, -1),
      SymCode(0x00f3, -1),
      SymCode(0x00f4, -1),
      SymCode(0x00f5, -1),
      SymCode(0x00f6, -1),
      SymCode(0x00f7, -1),
      SymCode(0x00f8, -1),
      SymCode(0x00f9, -1),
      SymCode(0x00fa, -1),
      SymCode(0x00fb, -1),
      SymCode(0x00fc, -1),
      SymCode(0x00fd, -1),
      SymCode(0x00fe, -1),
      SymCode(0x00ff, -1),

      SymCode(0x00BC, -1, "1/4", SYMBOL_FRACTION),
      SymCode(0x00BD, -1, "1/2", SYMBOL_FRACTION),
      SymCode(0x00BE, -1, "3/4", SYMBOL_FRACTION),
      SymCode(0x2153, -1, "1/3", SYMBOL_FRACTION),
      SymCode(0x2154, -1, "2/3", SYMBOL_FRACTION),
      SymCode(0x2155, -1, "1/5", SYMBOL_FRACTION),
      SymCode(0x2156, -1, "2/5", SYMBOL_FRACTION),
      SymCode(0x2157, -1, "3/5", SYMBOL_FRACTION),
      SymCode(0x2158, -1, "4/5", SYMBOL_FRACTION),
      SymCode(0x2159, -1, "1/6", SYMBOL_FRACTION),
      SymCode(0x215A, -1, "5/6", SYMBOL_FRACTION),
      SymCode(0x215B, -1, "1/8", SYMBOL_FRACTION),
      SymCode(0x215C, -1, "3/8", SYMBOL_FRACTION),
      SymCode(0x215D, -1, "5/8", SYMBOL_FRACTION),
      SymCode(0x215E, -1, "7/8", SYMBOL_FRACTION),

      // SymCode(0x203F, -1),    // curved ligature to connect two syllables
      SymCode(0x35c, -1),    // curved ligature to connect two syllables
      SymCode(0x361, -1),    // curved ligature (top)

      SymCode(-1, -1)    // indicates end
      };

//---------------------------------------------------------
//   fontId2Font
//---------------------------------------------------------

QFont fontId2font(int fontId)
      {
      QFont _font;
      //
      // font is rendered with a physical resolution of PDPI
      //    and logical resolution of DPI
      //
      // rastral size is 20pt = 20/72 inch
      //
      int size = lrint(20.0 * DPI / PPI);
      if (fontId == 0) {
            _font.setFamily("MScore");
            _font.setStyleStrategy(QFont::NoFontMerging);
            }
      else if (fontId == 1) {
            _font.setFamily("MScore1");
            }
      else if (fontId == 2) {
            _font.setFamily("Times New Roman");
            _font.setStyleStrategy(QFont::NoFontMerging);
            size = lrint(8 * DPI / PPI);
            }
      else {
            printf("illegal font id %d\n", fontId);
            abort();
            }
      _font.setPixelSize(size);
      return _font;
      }

//---------------------------------------------------------
//   Sym
//---------------------------------------------------------

Sym::Sym(const char* name, int c, int fid, double ax, double ay)
   : _code(c), fontId(fid), _name(name), _font(fontId2font(fontId)), _attach(ax * DPI/PPI, ay * DPI/PPI)
      {
      QFontMetricsF fm(_font);
      if (!fm.inFont(_code))
            printf("Sym: character 0x%x(%d) <%s> are not in font <%s>\n", _code.unicode(),c, _name, qPrintable(_font.family()));
      w     = fm.width(_code);
      _bbox = fm.boundingRect(_code);
      }

Sym::Sym(const char* name, int c, int fid, const QPointF& a, const QRectF& b)
   : _code(c), fontId(fid), _name(name), _font(fontId2font(fontId))
      {
      double s = DPI/PPI;
      _bbox.setRect(b.x() * s, b.y() * s, b.width() * s, b.height() * s);
      _attach = a * s;
      w = _bbox.width();
      }

//---------------------------------------------------------
//   findSymbol
//---------------------------------------------------------

const Sym* findSymbol(QChar code, int fontId)
      {
      foreach(const Sym& s, symbols) {
            if (s.code() == code && s.getFontId() == fontId)
                  return &s;
            }
      return 0;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

const QRectF Sym::bbox(double mag) const
      {
      return QRectF(_bbox.x() * mag, _bbox.y() * mag, _bbox.width() * mag, _bbox.height() * mag);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter, double mag, qreal x, qreal y) const
      {
      double imag = 1.0 / mag;
      painter.scale(mag, mag);
      painter.setFont(_font);
      painter.drawText(x * imag, y * imag, QString(_code));
      painter.scale(imag, imag);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter, double mag, qreal x, qreal y, int n) const
      {
      double imag = 1.0 / mag;
      painter.scale(mag, mag);
      painter.setFont(_font);
      painter.drawText(x * imag, y * imag, QString(n, _code));
      painter.scale(imag, imag);
      }

//---------------------------------------------------------
//   symToHtml
//    transform symbol into html code suitable
//    for QDocument->setHtml()
//---------------------------------------------------------

QString symToHtml(const Sym& s, int leftMargin)
      {
      double size    = s.font().pixelSize() * 72.0 / DPI;
      QString family = s.font().family();
      return QString(
      "<data>"
        "<html>"
          "<head>"
            "<meta name=\"qrichtext\" content=\"1\" >"
            "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\" />"
            "<style type=\"text/css\">"
              "p, li { white-space: pre-wrap; }"
              "</style>"
            "</head>"
          "<body style=\" font-family:'%1'; font-size:%2pt;\">"
            "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:%3px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                "&#%4;"
              "</p>"
            "</body>"
          "</html>"
      "</data>").arg(family).arg(size).arg(leftMargin).arg(s.code().unicode());
      }

QString symToHtml(const Sym& s1, const Sym& s2, int leftMargin)
      {
      QFont f        = s1.font();
      double size    = s1.font().pixelSize() * 72.0 / DPI;
      QString family = f.family();

      return QString(
      "<data>"
        "<html>"
          "<head>"
            "<meta name=\"qrichtext\" content=\"1\" >"
            "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\" />"
            "<style type=\"text/css\">"
              "p, li { white-space: pre-wrap; }"
              "</style>"
            "</head>"
          "<body style=\" font-family:'%1'; font-size:%2pt;\">"
            "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:%3px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                "&#%4;&#%5;"
              "</p>"
            "</body>"
          "</html>"
      "</data>").arg(family).arg(size).arg(leftMargin).arg(s1.code().unicode()).arg(s2.code().unicode());
      }

//---------------------------------------------------------
//   initSymbols
//---------------------------------------------------------

void initSymbols()
      {
      symbols[clefEightSym] = Sym(QT_TRANSLATE_NOOP("symbol", "Clef eight"),                 0x38, 2);
      symbols[clefOneSym]   = Sym(QT_TRANSLATE_NOOP("symbol", "Clef one"),                   0x31, 2);
      symbols[clefFiveSym]  = Sym(QT_TRANSLATE_NOOP("symbol", "Clef five"),                  0x35, 2);
      symbols[letterfSym]   = Sym(QT_TRANSLATE_NOOP("symbol", "f"),                          0x66, 1);
      symbols[lettermSym]   = Sym(QT_TRANSLATE_NOOP("symbol", "m"),                          0x6d, 1);
      symbols[letterpSym]   = Sym(QT_TRANSLATE_NOOP("symbol", "p"),                          0x70, 1);
      symbols[letterrSym]   = Sym(QT_TRANSLATE_NOOP("symbol", "r"),                          0x72, 1);
      symbols[lettersSym]   = Sym(QT_TRANSLATE_NOOP("symbol", "s"),                          0x73, 1);
      symbols[letterzSym]   = Sym(QT_TRANSLATE_NOOP("symbol", "z"),                          0x7a, 1);
      // used for GUI:
      symbols[note2Sym]     = Sym(QT_TRANSLATE_NOOP("symbol", "Note half"),   0xe104, 1);
      symbols[note4Sym]     = Sym(QT_TRANSLATE_NOOP("symbol", "Note quarter"),   0xe105, 1);
      symbols[note8Sym]     = Sym(QT_TRANSLATE_NOOP("symbol", "Note eighth"),   0xe106, 1);
      symbols[note16Sym]    = Sym(QT_TRANSLATE_NOOP("symbol", "Note 16th"),  0xe107, 1);
      symbols[note32Sym]    = Sym(QT_TRANSLATE_NOOP("symbol", "Note 32nd"),  0xe108, 1);
      symbols[note64Sym]    = Sym(QT_TRANSLATE_NOOP("symbol", "Note 64th"),  0xe109, 1);
      symbols[dotdotSym]    = Sym(QT_TRANSLATE_NOOP("symbol", "Dot dot"),    0xe10b, 1);


      QHash<QString, int> lnhash;
      for (unsigned int i = 0; i < sizeof(lilypondNames)/sizeof(*lilypondNames); ++i)
            lnhash[QString(lilypondNames[i].name)] = lilypondNames[i].msIndex;

      QFile f(":/data/symbols.xml");
      if (!f.open(QFile::ReadOnly)) {
            printf("cannot open symbols file\n");
            exit(-1);
            }
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(&f, false, &err, &line, &column)) {
            QString error;
            error.sprintf("error reading style file %s at line %d column %d: %s\n",
               f.fileName().toLatin1().data(), line, column, err.toLatin1().data());
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: Load font symbols failed:"),
               error,
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            return;
            }
      f.close();
      docName = f.fileName();
      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull();  ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "Glyph") {
                              QString name;
                              int code = -1;
                              QPointF p;
                              QRectF b;
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull();  eee = eee.nextSiblingElement()) {
                                    QString tag(eee.tagName());
                                    QString val(eee.text());
                                    if (tag == "name")
                                          name = val;
                                    else if (tag == "code") {
                                          bool ok;
                                          code = val.mid(2).toInt(&ok, 16);
                                          if (!ok)
                                                printf("cannot read code\n");
                                          }
                                    else if (tag == "attach")
                                          p = readPoint(eee);
                                    else if (tag == "bbox")
                                          b = readRectF(eee);
                                    else
                                          domError(eee);
                                    }
                              if (code == -1)
                                    printf("no code for glyph <%s>\n", qPrintable(name));
                              int idx = lnhash[name];
                              if (idx > 0)
                                    symbols[idx] = Sym(strdup(qPrintable(name)), code, 0, p, b);
                              else if (idx == 0)
                                    printf("symbol <%s> not found\n", qPrintable(name));
                              }
                        else
                              domError(ee);
                        }
                  }
            else
                  domError(e);
            }

      for (unsigned int i = 0; i < sizeof(lilypondNames)/sizeof(*lilypondNames); ++i) {
            int idx = lilypondNames[i].msIndex;
            if (idx != -1)
                  symbols[idx].setName(lilypondNames[i].mname);
            }
      if (charReplaceMap.isEmpty()) {
            for (unsigned i = 0; pSymbols[i].code != -1; ++i) {
                  if (pSymbols[i].code == 0 || pSymbols[i].text == 0)
                        continue;
                  charReplaceMap.insert(pSymbols[i].text, &pSymbols[i]);
                  }
            }
      }

