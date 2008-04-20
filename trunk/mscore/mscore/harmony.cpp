//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
//
//  Some Code inspired by "The JAZZ++ Midi Sequencer"
//  Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
//
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

#include "harmony.h"
#include "chordedit.h"
#include "pitchspelling.h"
#include "score.h"
#include "sym.h"
#include "system.h"

const HChord HChord::C0(0,3,6,9);

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

HChord::HChord(const char* s)
      {
      static const char* const scaleNames[2][12] = {
            { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },
            { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" }
            };
      keys = 0;
      QStringList sl = QString(s).split(" ", QString::SkipEmptyParts);
      foreach(QString s, sl) {
            for (int i = 0; i < 12; ++i) {
                  if (s == scaleNames[0][i] || s == scaleNames[1][i]) {
                        operator+=(i);
                        break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

HChord::HChord(int a, int b, int c, int d, int e, int f, int g, int h, int i, int k, int l)
      {
      keys = 0;
      if (a >= 0)
            operator+=(a);
      if (b >= 0)
            operator+=(b);
      if (c >= 0)
            operator+=(c);
      if (d >= 0)
            operator+=(d);
      if (e >= 0)
            operator+=(e);
      if (f >= 0)
            operator+=(f);
      if (g >= 0)
            operator+=(g);
      if (h >= 0)
            operator+=(h);
      if (i >= 0)
            operator+=(i);
      if (k >= 0)
            operator+=(k);
      if (l >= 0)
            operator+=(l);
      }

//---------------------------------------------------------
//   rotate
//    rotate 12 Bits
//---------------------------------------------------------

void HChord::rotate(int semiTones)
      {
      while (semiTones > 0) {
            if (keys & 0x800)
                  keys = ((keys & ~0x800) << 1) + 1;
            else
                  keys <<= 1;
            --semiTones;
            }
      while (semiTones < 0) {
            if (keys & 1)
                  keys = (keys >> 1) | 0x800;
            else
                  keys >>= 1;
            ++semiTones;
            }
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString HChord::name(int tpc)
      {
      QString buf = tpc2name(tpc, false);
      HChord c(*this);

      int key = tpc2pitch(tpc);

      c.rotate(-key);        // transpose to C

      // special cases
      if (c == C0) {
            buf += "o";
            return buf;
            }

      bool seven   = false;
      bool sharp9  = false;
      bool nat11   = false;
      bool sharp11 = false;
      bool nat13   = false;
      bool flat13  = false;

      // minor?
      if (c.contains(3)) {
            if (!c.contains(4))
                  buf += "m";
            else
                  sharp9 = true;
            }

      // 7
      if (c.contains(11)) {
            buf += "Maj7";
            seven = true;
            }
      else if (c.contains(10)) {
            buf += "7";
            seven = true;
            }

      // 4
      if (c.contains(5)) {
            if (!c.contains(4)) {
                  buf += "sus4";
                  }
            else
                  nat11 = true;
            }

      // 5
      if (c.contains(7)) {
            if (c.contains(6))
                  sharp11 = true;
            if (c.contains(8))
                  flat13 = true;
            }
      else {
            if (c.contains(6))
                  buf += "5-";
            if (c.contains(8))
                  buf += "5+";
            }

      // 6
      if (c.contains(9)) {
            if (!seven)
                  buf += "6";
            else
                  nat13 = true;
            }

      // 9
      if (c.contains(1))
            buf += "9-";
      if (c.contains(2))
            buf += "9";
      if (sharp9)
            buf += "9+";

      // 11
      if (nat11)
            buf += "11 ";
      if (sharp11)
            buf += "11+";

      // 13
      if (flat13)
            buf += "13-";
      if (nat13)
            buf += "13";
      return buf;
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void HChord::print() const
      {
      const char* names[] = { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };

      for (int i = 0; i < 12; i++) {
            if (contains(i))
                  printf(" %s", names[i]);
            }
      printf("\n");
      }

//---------------------------------------------------------
//   Chord List
//    list of all known chords
//---------------------------------------------------------

const ChordDescription Harmony::chordList[] = {
            { 0, 0,            0, HChord() },
            { 1, "",           "major", HChord("C E G") },        // major triad
            { 2, "Maj",        0, HChord("C E G") },
            { 3, "5b",         0, HChord("C E Gb") },       // major flat 5 triad
            { 4, "aug",   "augmented", HChord("C E G#") },       // augmented triad
            { 5, "6",     "major-sixth",  HChord("C E G A") },      // sixth
            { 6, "Maj7", "major-seventh", HChord("C E G B") },
            { 7, "Maj9", "major-ninth", HChord("C E G B D") },
            { 8, "Maj9#11",    0, HChord("C E G B D F#") },
            { 9, "Maj13#11",   0, HChord("C E G B D F# A") },
/*10*/
            { 10, "Maj13",  "major-13th", HChord("C E G B D F# A") },
            { 11, 0,            0, HChord() },
            { 12, "+",          0, HChord("C E G#") },       // +5, #5, +, augmented triad
            { 13, "Maj7#5",     0, HChord() },
            { 14, "69",         0, HChord("C D E G A") },
            { 15, "2",          0, HChord("C D E G") },      // major add 2
            { 16, "m",    "minor", HChord("C Eb G") },
            { 17, "maug",       0, HChord("C D# G#") },
            { 18, "mMaj7", "minor-major",   HChord("C Eb G B") },     // minor major 7th = major minor 7th
            { 19, "m7",    "minor-seventh", HChord("C Eb G Bb") },

/*20*/      { 20, "m9",  "minor-ninth", HChord("C Eb G Bb D") },
            { 21, "m11", "minor-11th",  HChord("C Eb G Bb D F") },
            { 22, "m13", "minor-13th",  HChord("C Eb G Bb D F A") },
            { 23, "m6",         "minor-sixth", HChord("C Eb G A") },
            { 24, "m#5",        0, HChord() },
            { 25, "m7#5",       0, HChord() },
            { 26, "m69",        0, HChord() },
            { 27, 0,            0, HChord() },
            { 28, "Maj7Lyd",    0, HChord() },
            { 29, "Maj7b5",     0, HChord() },

/*30*/      { 30, 0,                          0, HChord() },
            { 31, 0,                          0, HChord() },
            { 32, "m7b5",  "half-diminished", HChord("C Eb Gb Bb") },
            { 33, "dim",   "diminished",      HChord("C Eb Gb") },    // dim
            { 34, "m9b5",       0, HChord() },
            { 35, 0,            0, HChord() },
            { 36, 0,            0, HChord() },
            { 37, 0,            0, HChord() },
            { 38, 0,            0, HChord() },
            { 39, 0,            0, HChord() },

/*40*/      { 40, "5",  "power", HChord("C G") },  // power
            { 41, 0,            0, HChord() },
            { 42, 0,            0, HChord() },
            { 43, 0,            0, HChord() },
            { 44, 0,            0, HChord() },
            { 45, 0,            0, HChord() },
            { 46, 0,            0, HChord() },
            { 47, 0,            0, HChord() },
            { 48, 0,            0, HChord() },
            { 49, 0,            0, HChord() },

/*50*/      { 50, 0,            0, HChord() },
            { 51, 0,            0, HChord() },
            { 52, 0,            0, HChord() },
            { 53, 0,            0, HChord() },
            { 54, 0,            0, HChord() },
            { 55, 0,            0, HChord() },
            { 56, "7+",         0, HChord() },
            { 57, "9+",         0, HChord() },
            { 58, "13+",        0, HChord() },
            { 59, "(blues)",    0, HChord() },   // ??

/*60*/      { 60, "7(Blues)",   0, HChord() },   // ??
            { 61, 0,            0, HChord() },
            { 62, 0,            0, HChord() },
            { 63, 0,            0, HChord() },
            { 64, "7",  "dominant",      HChord("C E G Bb") },        // dominant-seventh
            { 65, "13", "dominant-13th", HChord("C E G Bb D F A") },  // dominant 13th
            { 66, "7b13",       0, HChord() },
            { 67, "7#11",       0, HChord() },
            { 68, "13#11",      0, HChord() },
            { 69, "7#11b13",    0, HChord() },

/*70*/      { 70, "9", "dominant-ninth", HChord() },
            { 71, "9b13",       0, HChord() },
            { 72, 0,            0, HChord() },
            { 73, "9#11",       0, HChord() },
            { 74, "13#11",      0, HChord() },
            { 75, "9#11b13",    0, HChord() },
            { 76, "7b9",        0, HChord() },
            { 77, "13b9",       0, HChord() },
            { 78, "7b9b13",     0, HChord() },
            { 79, "7b9#11",     0, HChord() },

/*80*/      { 80, "13b9#11",    0,                HChord() },
            { 81, "7b9#11b13",  0,                HChord() },
            { 82, "7#9",        "dominant add#9", HChord("C E G Bb Eb") },
            { 83, "13#9",       0,                HChord() },
            { 84, "7#9b13",     0, HChord() },
            { 85, "9#11",       0, HChord() },
            { 86, "13#9#11",    0, HChord() },
            { 87, "7#9#11b13",  0, HChord() },
            { 88, "7b5",        0, HChord() },
            { 89, "13b5",       0, HChord() },

/*90*/      { 90, "7b5b13",     0, HChord() },
            { 91, "9b5",        0, HChord() },
            { 92, "9b5b13",     0, HChord() },
            { 93, "7b5b9",      0, HChord() },
            { 94, "13b5b9",     0, HChord() },
            { 95, "7b5b9b13",   0, HChord() },
            { 96, "7b5#9",      0, HChord() },
            { 97, "13b5#9",     0, HChord() },
            { 98, "7b5#9b13",   0, HChord() },
            { 99, "7#5",        "augmented-seventh", HChord() },

/*100*/     { 100, "13#5",       0, HChord() },
            { 101, "7#5#11",     0, HChord() },
            { 102, "13#5#11",    0, HChord() },
            { 103, "9#5",        0, HChord() },
            { 104, "9#5#11",     0, HChord() },
            { 105, "7#5b9",      0, HChord() },
            { 106, "13#5b9",     0, HChord() },
            { 107, "7#5b9#11",   0, HChord() },
            { 108, "13#5b9#11",  0, HChord() },
            { 109, "7#5#9",      0, HChord() },

/*110*/     { 110, "13#5#9#11",  0, HChord() },
            { 111, "7#5#9#11",   0, HChord() },
            { 112, "13#5#9#11",  0, HChord() },
            { 113, "7alt",       0, HChord() },
            { 114, 0,            0, HChord() },
            { 115, 0,            0, HChord() },
            { 116, 0,            0, HChord() },
            { 117, 0,            0, HChord() },
            { 118, 0,            0, HChord() },
            { 119, 0,            0, HChord() },

/*120*/     { 120, 0,           0, HChord() },
            { 121, 0,           0, HChord() },
            { 122, 0,           0, HChord() },
            { 123, 0,           0, HChord() },
            { 124, 0,           0, HChord() },
            { 125, 0,            0, HChord() },
            { 126, 0,            0, HChord() },
            { 127, 0,            0, HChord() },
            { 128, "7sus",       0, HChord() },
            { 129, "13sus",      0, HChord() },

/*130*/     { 130, "7susb13",    0, HChord() },  //?? not checked
            { 131, "7sus#11",    0, HChord() },
            { 132, "13sus#11",   0, HChord() },
            { 133, "7sus#11b13", 0, HChord() },
            { 134, "9sus",       0, HChord() },   // 11 - 9sus
            { 135, "9susb13",    0, HChord() },
            { 136, "9sus#11",    0, HChord() },
            { 137, "13sus#11",   0, HChord() },
            { 138, "9sus#11b13", 0, HChord() },
            { 139, 0,            0, HChord() },

/*140*/     { 140, "7susb9",     0, HChord() },
            { 141, "13susb9",    0, HChord() },
            { 142, "7susb9b13",  0, HChord() },
            { 143, "7susb9#11",  0, HChord() },
            { 144, "13susb9#11", 0, HChord() },
            { 145, "7susb9#11b13",0, HChord() },
            { 146, "7sus#9",      0, HChord() },
            { 147, "13sus#9",     0, HChord() },
            { 148, "7sus#9b13",   0, HChord() },
            { 149, "9sus#11",     0, HChord() },

/*150*/     { 150, "13sus#9#11",   0, HChord() },
            { 151, "7sus#9#11b13", 0, HChord() },
            { 152, "7susb5",       0, HChord() },
            { 153, "13susb5",      0, HChord() },
            { 154, "7susb5b13",    0, HChord() },
            { 155, "9susb5",       0, HChord() },
            { 156, "9susb5b13",    0, HChord() },
            { 157, "7susb5b9",     0, HChord() },
            { 158, "13susb5b9",    0, HChord() },
            { 159, "7susb5b9b13",  0, HChord() },

/*160*/     { 160, "7susb5#9",     0, HChord() },
            { 161, "13susb5#9",    0, HChord() },
            { 162, "7susb5#9b13",  0, HChord() },
            { 163, "7sus#5",       0, HChord() },
            { 164, "13sus#5",      0, HChord() },
            { 165, "7sus#5#11",    0, HChord() },
            { 166, "13sus#5#11",   0, HChord() },
            { 167, "9sus#5",       0, HChord() },
            { 168, "9sus#5#11",    0, HChord() },
            { 169, "7sus#5b9",     0, HChord() },

/*170*/     { 170, "13sus#5b9",    0, HChord() },
            { 171, "7sus#5b9#11",  0, HChord() },
            { 172, "13sus#5b9#11", 0, HChord() },
            { 173, "7sus#5#9",     0, HChord() },
            { 174, "13sus#5#9#11", 0, HChord() },
            { 175, "7sus#5#9#11",  0, HChord() },
            { 176, "13sus#5#9#11", 0, HChord() },
            { 177, "4",            0, HChord() },
            { 178, 0,              0, HChord() },
            { 179, 0,              0, HChord() },

/*180*/     { 180, 0,              0, HChord() },
            { 181, 0,              0, HChord() },
            { 182, 0,              0, HChord() },
            { 183, 0,              0, HChord() },
            { 184, "sus",          0, HChord("C F G") },

            { 185, "dim7",   "diminished-seventh", HChord() },  // mscore ext.
            { 186, "sus2",   "suspended-second", HChord("C D G") },      // suspended 2nd chord
            { 187, "mb3b13", "neapolitan", HChord() },
            { 188, "#13",    "italian", HChord() },
            { 189, "#11#13",  "french", HChord() },

/*190*/     { 190, "add#13",   "german",           HChord() },
            { 191, "6/add9",   "maj69",            HChord() },
            { 192, "sus4",     "suspended-fourth", HChord("C F G") },        // sus4
            { 193, "11",       "dominant-11th",    HChord("C E G Bb D F") }, // dominant 11th / 9sus
            { 194, "Maj11",    "major-11th",       HChord("C E G B D F") },  // major 11th
            { 195, "Tristan",  "tristan",          HChord("C F# A# D") },    // Tristan
      };

//---------------------------------------------------------
//   getExtensionName
//---------------------------------------------------------

const char* Harmony::getExtensionName(int i)
      {
      if (i >= int(sizeof(chordList)/sizeof(*chordList)))
            return 0;
      return chordList[i].name;
      }

//---------------------------------------------------------
//   harmonyName
//---------------------------------------------------------

// convert degrees to semitones

static int degreeTable[] = {
      // 1  2  3  4  5  6   7
      // C  D  E  F  G  A   B
         0, 2, 4, 5, 7, 9, 11
      };

QString Harmony::harmonyName() const
      {
//      printf("Harmony::harmonyName(this=%p)", this);
      bool germanNames = score()->style()->useGermanNoteNames;
      QString s = harmonyName(germanNames, rootTpc(), extension(), baseTpc(), &_degreeList);
//      printf("Harmony::harmonyName: <%s>\n", qPrintable(s));
      return s;
      }

QString Harmony::harmonyName(bool germanNames, int root, int extension, int base, const QList<HDegree>* degreeList)
      {
/*
      printf("Harmony::harmonyName(root=%d extension=%d base=%d list=%p", root, extension, base, degreeList);
      if (degreeList)
            printf(" size=%d)", degreeList->size());
      else
            printf(")");
*/
      HChord hc;
      QString s;

      if (extension >= 0 && extension < int(sizeof(chordList)/sizeof(*chordList)))
            hc = chordList[extension].chord;
//      printf(" HChord.name=%s", qPrintable(hc.name(0, 0)));
//      printf("\n");

      if (degreeList && !degreeList->isEmpty()) {
/*
            // print the chord without the degree modification(s)
            for (int i = 0; i < 12; i++)
                  if (hc.contains(i))
                        printf(" %s", qPrintable(tpc2name(i+1, germanNames)));
            printf("\n");
*/
            // factor in the degrees
            for (int i = 0; i < degreeList->size(); i++) {
                  HDegree d = (*degreeList)[i];
                  int dv = degreeTable[(d.value() - 1) % 7] + d.alter();

                  if (d.type() == ADD) {
//                        printf("hc: ");
//                        hc.print();
                        hc += dv;
//                        printf(" add degree value=%d alter=%d, dv=%d\n",
//                           d.value(), d.alter(), dv);
//                        hc.print();
//                        printf("hc: ");
                        }
                  else if (d.type() == ALTER) {
//                        printf(" alter degree value=%d alter=%d\n", d.value(), d.alter());
                        if (hc.contains(degreeTable[(d.value() - 1) % 7])) {
                              hc -= (degreeTable[(d.value() - 1) % 7]);
                              hc += dv;
                              }
                        else
                              printf("chord does not contain degree %d\n", degreeTable[(d.value() - 1) % 7]);
                        }
                  else if (d.type() == SUBTRACT) {
//                        printf(" subtract degree value=%d alter=%d\n", d.value(), d.alter());
                        if (hc.contains(degreeTable[(d.value() - 1) % 7])) {
                              hc -= (degreeTable[(d.value() - 1) % 7]);
                              }
                        else
                              printf("chord does not contain degree %d\n", degreeTable[(d.value() - 1) % 7]);
                        }
                  else printf("degree type %d not supported\n", d.type());
                  }
/*
            // print the chord with the degree modification(s)
            for (int i = 0; i < 12; i++)
                  if (hc.contains(i))
                        printf(" %s", qPrintable(tpc2name(i+1, germanNames)));
            printf(" HChord.name=%s", qPrintable(hc.name(0, 0)));
            printf("\n");
*/
            // try to find the chord in chordList
            int newExtension = 0;
            for (int i = 1; i < int(sizeof(chordList)/sizeof(*chordList)); i++) {
                  if (chordList[i].chord == hc && chordList[i].name != 0) {
                        printf(" found in table as %s\n", chordList[i].name);
                        newExtension = i;
                        break;
                        }
                  }

            // now determine the chord name
            if (newExtension)
                  s = tpc2name(root, germanNames) + getExtensionName(newExtension);
            else
                  // not in table, fallback to using HChord.name()
                  s = hc.name(root);
            } // end if (degreeList ...
      else {
            s = tpc2name(root, germanNames);
            if (extension)
                  s += getExtensionName(extension);
      }

      if (base != INVALID_TPC) {
            s += "/";
            s += tpc2name(base, germanNames);
            }

//      printf(" %s\n", qPrintable(s));
      return s;
      }

//---------------------------------------------------------
//   resolveDegreeList
//    try to detect chord number and to eliminate degree
//    list
//---------------------------------------------------------

void Harmony::resolveDegreeList()
      {
      if (_degreeList.isEmpty())
            return;

      HChord hc = chordList[_extension].chord;

      // factor in the degrees
      for (int i = 0; i < _degreeList.size(); i++) {
            HDegree d = _degreeList[i];
            int dv = degreeTable[(d.value() - 1) % 7] + d.alter();

            if (d.type() == ADD) {
                  hc += dv;
                  }
            else if (d.type() == ALTER) {
                  if (hc.contains(degreeTable[(d.value() - 1) % 7])) {
                        hc -= (degreeTable[(d.value() - 1) % 7]);
                        hc += dv;
                        }
                  else
                        printf("chord does not contain degree %d\n", degreeTable[(d.value() - 1) % 7]);
                  }
            else if (d.type() == SUBTRACT) {
                  if (hc.contains(degreeTable[(d.value() - 1) % 7])) {
                        hc -= (degreeTable[(d.value() - 1) % 7]);
                        }
                  else
                        printf("chord does not contain degree %d\n", degreeTable[(d.value() - 1) % 7]);
                  }
            else
                  printf("degree type %d not supported\n", d.type());
            }


      // try to find the chord in chordList
      for (int i = 1; i < int(sizeof(chordList)/sizeof(*chordList)); i++) {
            if (chordList[i].chord == hc && chordList[i].name != 0) {
                  printf("ResolveDegreeList: found in table as %s\n", chordList[i].name);
                  _extension = i;
                  _degreeList.clear();
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   Harmony
//---------------------------------------------------------

Harmony::Harmony(Score* score)
   : Text(score)
      {
      Text::setSubtype(TEXT_CHORD);
      _rootTpc   = INVALID_TPC;
      _extension = 0;
      _baseTpc   = INVALID_TPC;
      }

//---------------------------------------------------------
//   ~Harmony
//---------------------------------------------------------

Harmony::~Harmony()
      {
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Harmony::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addSeparator();
      a = popup->addAction(tr("Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Harmony::propertyAction(const QString& s)
      {
      if (s == "props") {
            ChordEdit ce(score());
            ce.setRoot(rootTpc());
            ce.setBase(baseTpc());
            ce.setExtension(extension());
            for (int i = 0; i < numberOfDegrees(); i++)
                  ce.addDegree(degree(i));
            int rv = ce.exec();
            if (rv) {
                  setRootTpc(ce.root());
                  setBaseTpc(ce.base());
                  setExtension(ce.extension());
                  clearDegrees();
                  for (int i = 0; i < ce.numberOfDegrees(); i++)
                        addDegree(ce.degree(i));
                  setText(harmonyName());
                  }
            }
      else
            Element::propertyAction(s);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Harmony::write(Xml& xml) const
      {
      xml.stag("Harmony");
      if (_rootTpc != INVALID_TPC) {
            xml.tag("root", _rootTpc);
            if (_extension)
                  xml.tag("extension", _extension);
            if (_baseTpc != INVALID_TPC)
                  xml.tag("base", _baseTpc);
            for (int i = 0; i < _degreeList.size(); i++) {
                  HDegree hd = _degreeList.value(i);
                  int tp = hd.type();
                  if (tp == ADD || tp == ALTER || tp == SUBTRACT) {
                        xml.stag("degree");
                        xml.tag("degree-value", hd.value());
                        xml.tag("degree-alter", hd.alter());
                        switch (tp) {
                              case ADD: xml.tag("degree-type", "add");
                                    break;
                              case ALTER: xml.tag("degree-type", "alter");
                                    break;
                              case SUBTRACT: xml.tag("degree-type", "subtract");
                                    break;
                              default:
                                    break;
                              }
                        xml.etag();
                        }
                  }
            }
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Harmony::read(QDomElement e)
      {
      // convert table to tpc values
      static const int table[] = {
            14, 9, 16, 11, 18, 13, 8, 15, 10, 17, 12, 19
            };
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            int i = e.text().toInt();
            if (tag == "base") {
                  if (score()->mscVersion() >= 106)
                        setBaseTpc(i);
                  else
                        setBaseTpc(table[i-1]);    // obsolete
                  }
            else if (tag == "extension")
                  setExtension(i);
            else if (tag == "root") {
                  if (score()->mscVersion() >= 106)
                        setRootTpc(i);
                  else
                        setRootTpc(table[i-1]);    // obsolete
                  }
            else if (tag == "degree") {
                  int degreeValue = 0;
                  int degreeAlter = 0;
                  QString degreeType = "";
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "degree-value") {
                              degreeValue = ee.text().toInt();
                              }
                        else if (tag == "degree-alter") {
                              degreeAlter = ee.text().toInt();
                              }
                        else if (tag == "degree-type") {
                              degreeType = ee.text();
                              }
                        else
                              domError(ee);
                        }
                  if (degreeValue <= 0 || degreeValue > 13
                      || degreeAlter < -2 || degreeAlter > 2
                      || (degreeType != "add" && degreeType != "alter" && degreeType != "subtract")) {
                        printf("incorrect degree: degreeValue=%d degreeAlter=%d degreeType=%s\n",
                               degreeValue, degreeAlter, qPrintable(degreeType));
                        }
                  else {
                        if (degreeType == "add")
                              addDegree(HDegree(degreeValue, degreeAlter, ADD));
                        else if (degreeType == "alter")
                              addDegree(HDegree(degreeValue, degreeAlter, ALTER));
                        else if (degreeType == "subtract")
                              addDegree(HDegree(degreeValue, degreeAlter, SUBTRACT));
                        }
                  }
            else if (!Text::readProperties(e))
                  domError(e);
            }
      buildText();
      }

//---------------------------------------------------------
//   buildText
//    construct Chord Symbol
//---------------------------------------------------------

void Harmony::buildText()
      {
      static const QChar majorSym(0x25b3);
      static const QChar minor(0x2d);
      static const QChar augmented(0x2b);
      static const QChar diminished(0xb0);
      static const QChar halfDiminished(0xf8);

      if (_rootTpc == INVALID_TPC)
            return;

      clear();

      QString txt(harmonyName());
      const char* s = strdup(txt.toAscii().data());

//    printf("Harmony <%s>\n", s);

      QTextCursor cursor(doc());
      cursor.setPosition(0);
      QTextCharFormat f = cursor.charFormat();

      if (*s == 0)
            return;

      cursor.insertText(QString(*s++));
      if (s == 0)
            return;

      QTextCharFormat sf(f);
#if 0
      double extraMag = 1.0;
      double mag = _spatium * extraMag / (SPATIUM20 * DPI);
      QFont font("MScore1");
      font.setPointSizeF(20.0 * mag);
      sf.setFont(font);
#endif
      sf.setVerticalAlignment(QTextCharFormat::AlignSuperScript);

      if ((*s == '#') || (*s == 'b')) {
#if 1
            cursor.insertText(QString(*s), f);
#else
            cursor.setCharFormat(sf);
            if (*s == '#')
                  cursor.insertText(QString(symbols[sharpSym].code()));
            else
                  cursor.insertText(QString(symbols[flatSym].code()));
#endif
            ++s;
            }
      bool useSymbols = score()->style()->chordNamesUseSymbols;

      if (*s == 0)
            return;
      if (*s == 'm') {
            cursor.insertText(QString("m"), f);
            ++s;
            }
      else if (useSymbols && (strncmp(s, "Maj7", 4) == 0)) {
            cursor.insertText(QString(majorSym), sf);
            s += 4;
            }
      else if (strncmp(s, "Maj", 3) == 0) {
            cursor.insertText(QString("maj"), f);
            s += 3;
            }
      else if (strncmp(s, "aug", 3) == 0) {
            cursor.insertText(QString("aug"), f);
            s += 3;
            }
      else if (strncmp(s, "sus", 3) == 0) {
            cursor.insertText(QString("sus"), f);
            s += 3;
            }
      else if (strncmp(s, "dim", 3) == 0) {
            cursor.insertText(QString("dim"), f);
            s += 3;
            }

      const char* ss = s;
      while (*ss) {
            if (*ss == '/')
                  break;
            ++ss;
            }
      const char* slash = *ss == '/' ? ss+1 : 0;
      if (ss - s > 0) {
            while (s < ss)
                  cursor.insertText(QString(*s++), sf);
            }
      if (slash) {
            cursor.insertText(QString('/'), f);
            cursor.insertText(QString(slash), f);
            }
      }

//---------------------------------------------------------
//   convertRoot
//    convert something like "C#" into tpc 21
//---------------------------------------------------------

static int convertRoot(const QString& s, bool germanNames)
      {
      int n = s.size();
      if (n < 1)
            return INVALID_TPC;
      int alter = 0;
      if (n > 1) {
            if (s[1].toLower().toAscii() == 'b')
                  alter = -1;
            else if (s[1] == '#')
                  alter = 1;
            }
      int r;
      if (germanNames) {
            switch(s[0].toLower().toAscii()) {
                  case 'c':   r = 0; break;
                  case 'd':   r = 1; break;
                  case 'e':   r = 2; break;
                  case 'f':   r = 3; break;
                  case 'g':   r = 4; break;
                  case 'a':   r = 5; break;
                  case 'h':   r = 6; break;
                  case 'b':
                        if (alter)
                              return INVALID_TPC;
                        r = 6;
                        alter = -1;
                        break;
                  default:
                        return INVALID_TPC;
                  }
            static const int spellings[] = {
               // bb  b   -   #  ##
                  0,  7, 14, 21, 28,  // C
                  2,  9, 16, 23, 30,  // D
                  4, 11, 18, 25, 32,  // E
                 -1,  6, 13, 20, 27,  // F
                  1,  8, 15, 22, 29,  // G
                  3, 10, 17, 24, 31,  // A
                  5, 12, 19, 26, 33,  // B
                  };
            r = spellings[r * 5 + alter + 2];
            }
      else {
            switch(s[0].toLower().toAscii()) {
                  case 'c':   r = 0; break;
                  case 'd':   r = 1; break;
                  case 'e':   r = 2; break;
                  case 'f':   r = 3; break;
                  case 'g':   r = 4; break;
                  case 'a':   r = 5; break;
                  case 'b':   r = 6; break;
                  default:    return INVALID_TPC;
                  }
            static const int spellings[] = {
               // bb  b   -   #  ##
                  0,  7, 14, 21, 28,  // C
                  2,  9, 16, 23, 30,  // D
                  4, 11, 18, 25, 32,  // E
                 -1,  6, 13, 20, 27,  // F
                  1,  8, 15, 22, 29,  // G
                  3, 10, 17, 24, 31,  // A
                  5, 12, 19, 26, 33,  // B
                  };
            r = spellings[r * 5 + alter + 2];
            }
      return r;
      }

//---------------------------------------------------------
//   parseHarmony
//    return extension number
//---------------------------------------------------------

int Harmony::parseHarmony(const QString& ss, int* root, int* base)
      {
      QString s = ss.simplified();
      int n = s.size();
      if (n < 1) {
            printf("parseHarmony failed <%s>\n", qPrintable(ss));
            return -1;
            }
      bool germanNames = score()->style()->useGermanNoteNames;
      int r = convertRoot(s, germanNames);
      if (r == INVALID_TPC) {
            printf("1:parseHarmony failed <%s>\n", qPrintable(ss));
            return -1;
            }
      *root = r;
      int idx = ((n > 1) && ((s[1] == 'b') || (s[1] == '#'))) ? 2 : 1;
      *base = INVALID_TPC;
      int slash = s.indexOf('/');
      if (slash != -1) {
            QString bs = s.mid(slash+1);
            s     = s.mid(idx, slash - idx);
            *base = convertRoot(bs, germanNames);
            }
      else
            s = s.mid(idx);
      s = s.toLower();
      for (unsigned i = 1; i < sizeof(chordList)/sizeof(*chordList); ++i) {
            if (QString(chordList[i].name).toLower() == s)
                  return i;
            }
      printf("2:parseHarmony failed <%s>\n", qPrintable(ss));
      return -1;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Harmony::endEdit()
      {
      Text::endEdit();

      int r, b;
      int e = parseHarmony(getText(), &r, &b);
      if (e > 0) {
            setRootTpc(r);
            setBaseTpc(b);
            setExtension(e);
            buildText();
            }
      else {
            // syntax error, leave text as is
            setRootTpc(INVALID_TPC);
            setBaseTpc(INVALID_TPC);
            setExtension(0);
            }
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Harmony::dragAnchor() const
      {
      QPointF cp = canvasPos();
      QPointF anchor = cp - (userOff() * _spatium);
      return QLineF(cp, anchor);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Harmony::layout(ScoreLayout* l)
      {
      Text::layout(l);
      double y = track() != -1 ? measure()->system()->staff(track() / VOICES)->y() : 0.0;
      double x = time().isValid() ? measure()->tick2pos(tick()) : 0.0;
      setPos(ipos() + QPointF(x, y));
      }
//---------------------------------------------------------
//   xmlName
//---------------------------------------------------------

const char* Harmony::xmlName() const
      {
      return chordList[_extension].xml;
      }

//---------------------------------------------------------
//   fromXml
//---------------------------------------------------------

int Harmony::fromXml(const QString& s)
      {
      int extension = 0;
      QString lowerCaseKind = s.toLower();
      for (unsigned i = 1; i < sizeof(chordList)/sizeof(*chordList); ++i) {
            if (lowerCaseKind == chordList[i].xml) {
                  extension = i;
                  break;
                  }
            }
      return extension;
      }


