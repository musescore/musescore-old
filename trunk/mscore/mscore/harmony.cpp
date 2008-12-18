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

QHash<int, const ChordDescription*> Harmony::chordHash;

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
      static const HChord C0(0,3,6,9);
      static const HChord C1(0,3);

      QString buf = tpc2name(tpc, false);
      HChord c(*this);

      int key = tpc2pitch(tpc);

      c.rotate(-key);        // transpose to C

      // special cases
      if (c == C0) {
            buf += "dim";
            return buf;
            }
      if (c == C1) {
            buf += "no5";
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
                  buf += "b5";
            if (c.contains(8))
                  buf += "#5";
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
            buf += "b9";
      if (c.contains(2))
            buf += "9";
      if (sharp9)
            buf += "#9";

      // 11
      if (nat11)
            buf += "11 ";
      if (sharp11)
            buf += "#11";

      // 13
      if (flat13)
            buf += "b13";
      if (nat13) {
            if (c.contains(1) || c.contains(2) || sharp9 || nat11 || sharp11)
                  buf += "13";
            else
                  buf += "add13";
            }
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
//   add
//---------------------------------------------------------

void HChord::add(const QList<HDegree>& degreeList)
      {
// printf("HChord::add   ");print();
      // convert degrees to semitones
      static const int degreeTable[] = {
            // 1  2  3  4  5  6   7
            // C  D  E  F  G  A   B
               0, 2, 4, 5, 7, 9, 11
            };
      // factor in the degrees
      foreach(const HDegree d, degreeList) {
            int dv  = degreeTable[(d.value() - 1) % 7] + d.alter();
            int dv1 = degreeTable[(d.value() - 1) % 7];

            if (d.value() == 7 && d.alter() == 0) {
                  // DEBUG: seventh degree is Bb, not B
                  //        except Maj   (TODO)
                  dv -= 1;
                  }

            if (d.type() == ADD)
                  *this += dv;
            else if (d.type() == ALTER) {
                  if (contains(dv1)) {
                        *this -= dv1;
                        *this += dv;
                        }
                  else {
//                        printf("ALTER: chord does not contain degree %d(%d):",
//                           d.value(), d.alter());
//                        print();
                        *this += dv;      // DEBUG: default to add
                        }
                  }
            else if (d.type() == SUBTRACT) {
                  if (contains(dv1))
                        *this -= dv1;
                  else {
                        printf("SUB: chord does not contain degree %d(%d):",
                           d.value(), d.alter());
                        print();
                        }
                  }
            else
                  printf("degree type %d not supported\n", d.type());

// printf("  HCHord::added  "); print();
            }
      }

//---------------------------------------------------------
//   Chord List
//    list of all known chords
//    sus4 == sus
//    sus2 == 2
//---------------------------------------------------------

const ChordDescription Harmony::chordList[] = {
            { 1, "",                "major", 0,                                          HChord("C E G") },        // major triad
            { 2, "Maj",             0, 0,                                                HChord("C E G") },
            { 3, "5b",              0, 0,                                                HChord("C E Gb") },       // major flat 5 triad
            { 4, "aug",             "augmented", 0,                                      HChord("C E G#") },       // augmented triad
            { 5, "6",               "major-sixth", 0,                                    HChord("C E G A") },      // sixth
            { 6, "Maj7",            "major-seventh", 0,                                  HChord("C E G B") },
            { 7, "Maj9",            "major-ninth", 0,                                    HChord("C E G B D") },
            { 8, "Maj9#11",         "major-ninth", "add#11",                             HChord("C E G B D F#") },
            { 9, "Maj13#11",        "major-13th", "add#11",                              HChord("C E G B D F# A") },
/*10*/
            { 10, "Maj13",          "major-13th", 0,                                     HChord("C E G B D F# A") },
            { 11, "Maj9(no 3)",     "major-ninth", "sub3",                               HChord("C G B D") },
            { 12, "+",              "augmented", 0,                                      HChord("C E G#") },       // +5, #5, +, augmented triad
            { 13, "Maj7#5",         "major-seventh", "alt#5",                            HChord("C E G# B") },
            { 14, "69",             "maj69", 0,                                          HChord("C D E G A") },
            { 15, "2",              "major", "add2",                                     HChord("C D E G") },
            { 16, "m",              "minor", 0,                                          HChord("C Eb G") },
            { 17, "maug",           "minor", "alt#5",                                    HChord("C Eb G#") },
            { 18, "mMaj7",          "minor-major", 0,                                    HChord("C Eb G B") },     // minor major 7th = major minor 7th
            { 19, "m7",             "minor-seventh", 0,                                  HChord("C Eb G Bb") },

/*20*/      { 20, "m9",             "minor-ninth", 0,                                    HChord("C Eb G Bb D") },
            { 21, "m11",            "minor-11th", 0,                                     HChord("C Eb G Bb D F") },
            { 22, "m13",            "minor-13th", 0,                                     HChord("C Eb G Bb D F A") },
            { 23, "m6",             "minor-sixth", 0,                                    HChord("C Eb G A") },
            { 24, "m#5",            "minor alt#5", 0,                                    HChord("C Eb G#") },
            { 25, "m7#5",           "minor-seventh", "alt#5",                            HChord("C Eb G# Bb") },
            { 26, "m69",            "minor-sixth", "add9",                               HChord("C D Eb G A") },
            { 27, "Lyd",            0, 0,                                                  HChord() },
//??          { 28, "Maj7Lyd",        "major-seventh", "altb5",                            HChord("C D E G A") },
            { 28, "Maj7Lyd",        0, 0,                                                HChord("C D E G A") },
            { 29, "Maj7b5",         "major-seventh", "altb5",                            HChord("C E Gb B") },

            { 32, "m7b5",           "half-diminished", 0,                                HChord("C Eb Gb Bb") },
            { 33, "dim",            "diminished", 0,                                     HChord("C Eb Gb") },
            { 34, "m9b5",           "minor-ninth", "altb5",                              HChord("C Eb Gb Bb D") },

/*40*/      { 40, "5",              "power", 0,                                          HChord("C G") },

            { 56, "7+",             "augmented-seventh", 0,                              HChord("C E Ab") },        // augmented 7th
            { 57, "9+",             "augmented-ninth", 0,                                HChord("C E Ab Bb D") },   // augmented 9th
            { 58, "13+",            "dominant-13th", "alt#5",                            HChord("C E G# Bb D A") },
            { 59, "(blues)",        0, 0,                                                HChord() },   // ??

/*60*/      { 60, "7(Blues)",       0, 0,                                                HChord("C E G Bb") },
            { 64, "7",              "dominant-seventh", 0,                               HChord("C E G Bb") },        // dominant-seventh
            { 64, "7",              "dominant", 0,                                       HChord("C E G Bb") },        // dominant-seventh
            { 65, "13",             "dominant-13th", 0,                                  HChord("C E G Bb D F A") },  // dominant 13th
            { 66, "7b13",           "dominant", "addb13",                                HChord("C E G Ab Bb") },
            { 67, "7#11",           "dominant", "add#11",                                HChord("C E F# G Bb") },
            { 68, "13#11",          "dominant-13th", "add#11",                           HChord("C E F# G Bb D F A") },

            { 69, "7#11b13",        "dominant-seventh", "add#11 addb13",                 HChord("C E G Bb F# Ab") },
/*70*/      { 70, "9",              "dominant-ninth", 0,                                 HChord("C E G Bb D") },
            { 72, "9b13",           "dominant-ninth", "addb13",                          HChord("C E G Bb D Ab") },  // same as 71
            { 73, "9#11",           "dominant-ninth", "add#11",                          HChord("C E G Bb D F#") },
            { 74, "13#11",          "dominant-13th", "alt#11",                           HChord("C E G Bb D F# A") },
            { 75, "9#11b13",        "dominant-ninth", "add#11 addb13",                   HChord("C E G Bb D F# Ab") },
            { 76, "7b9",            "dominant", "addb9",                                 HChord("C E G Bb Db") },

//          { 77, "13b9",           "dominant-13th", "altb9",                            HChord("C E G Bb D Fb A") },
            { 77, "13b9",           "dominant-13th", "altb9",                            HChord("C E G Bb Db F A") },

            { 78, "7b9b13",         "dominant-seventh", "addb9 addb13",                  HChord("C E G Bb Db Ab") },
            { 79, "7b9#11",         "dominant-seventh", "addb9 alt#11",                  HChord("C E G Bb Db F#") },

/*80*/      { 80, "13b9#11",        "dominant-13th", "altb9 alt#11",                     HChord("C E G Bb Db F# A") },
            { 81, "7b9#11b13",      "dominant-seventh", "add#11 addb13",                 HChord("C E G Bb Db F# Ab") },
            { 82, "7#9",            "dominant", "add#9",                                 HChord("C E G Bb Eb") },
            { 83, "13#9",           "dominant-13th", "alt#9",                            HChord("C E G Bb D# F A") },
            { 84, "7#9b13",         "dominant-seventh", "add#9 addb13",                  HChord("C E G Bb D# Ab") },
            { 85, "9#11",           "dominant-ninth", "add#11",                          HChord("C E G Bb D F#") },
            { 86, "13#9#11",        "dominant-13th", "alt#9 alt#11",                     HChord("C E G Bb D# F# A") },
            { 87, "7#9#11b13",      "dominant-seventh", "add#9 add#11 addb13",           HChord("C E G Bb D# F# Ab") },
            { 88, "7b5",            "dominant", "altb5",                                 HChord("C E Gb Bb") },
            { 89, "13b5",           "dominant-13th", "altb5",                            HChord("C E Gb Bb D F A") },

/*90*/      { 90, "7b5b13",         "dominant-seventh", "altb5 addb13",                  HChord("C E Gb Bb A") },
            { 91, "9b5",            "dominant-ninth", "altb5",                           HChord("C E Gb Bb D") },
            { 92, "9b5b13",         "dominant-ninth", "altb5 addb13",                    HChord("C E Gb Bb D Ab") },
            { 93, "7b5b9",          "dominant-seventh", "altb5 addb9",                   HChord("C E Gb Bb Db") },
            { 94, "13b5b9",         "dominant-13th", "altb5 addb9",                      HChord("C E Gb Bb Db F A") },
            { 95, "7b5b9b13",       "dominant-seventh", "altb5 addb9 addb13",            HChord("C E Gb Bb Db Ab") },
            { 96, "7b5#9",          "dominant-seventh", "altb5 add#9",                   HChord("C E Gb Bb D#") },
            { 97, "13b5#9",         "dominant-13th", "altb5 alt#9",                      HChord("C E Gb Bb D# F A") },
            { 98, "7b5#9b13",       "dominant-seventh", "altb5 add#9 addb13",            HChord("C E Gb Bb D# Ab") },
            { 99, "7#5",            "augmented-seventh", 0,                              HChord("C E Ab Bb") },

/*100*/     { 100, "13#5",          "dominant-13th", "alt#5",                            HChord("C E G# Bb D F A") },
            { 101, "7#5#11",        "dominant-seventh", "alt#5 add#11",                  HChord("C E G# Bb D#") },
            { 102, "13#5#11",       "dominant-13th", "alt#5 alt#11",                     HChord("C E G# Bb D F# A") },
            { 103, "9#5",           "dominant-ninth", "alt#5",                           HChord("C E G# Bb D") },
            { 104, "9#5#11",        "dominant-ninth", "alt#5 add#11",                    HChord("C E G# Bb D F#") },
            { 105, "7#5b9",         "dominant-seventh", "alt#5 addb9",                   HChord("C E G# Bb Db") },
            { 106, "13#5b9",        "dominant-13th", "alt#5 altb9",                      HChord("C E G# Bb Db F A") },
            { 107, "7#5b9#11",      "dominant-seventh", "alt#5 addb9 add#11",            HChord("C E G# Bb Db F#") },
            { 108, "13#5b9#11",     "dominant-13th", "alt#5 altb9 alt#11",               HChord("C E G# Bb Db F# A") },
            { 109, "7#5#9",         "dominant-seventh", "alt#5 add#9",                   HChord("C E G# Bb D#") },

/*110*/     { 110, "13#5#9#11",     "dominant-13th", "alt#5 alt#9 alt#11",               HChord("C E G# Bb D# F# A") },
            { 111, "7#5#9#11",      "dominant-seventh", "alt#5 add#9 add#11",            HChord("C E G# Bb D# F#") },
            { 112, "13#5#9#11",     "dominant-13th", "alt#5 alt#9 alt#11",               HChord("C E G# Bb D# F# A") }, // same as 110
            { 113, "7alt",          "dominant-seventh", "alt#5 add#9 add#11",            HChord("C E G# Bb D# F#") },

            { 128, "7sus",          "suspended-fourth", "add7",                          HChord("C F G Bb")        },
            { 129, "13sus",         "suspended-fourth", "add7 add13",                    HChord("C F G Bb D A") },

            { 130, "7susb13",       "suspended-fourth", "add7 addb13",                   HChord("C F G Bb Ab") },
            { 131, "7sus#11",       "suspended-fourth", "add7 add#11",                   HChord("C F G Bb F") },
            { 132, "13sus#11",      "suspended-fourth", "add#11 add13",                  HChord("C F G Bb D F# A") },
            { 133, "7sus#11b13",    "suspended-fourth", "add7 add#11 addb13",            HChord("C F G Bb F# Ab") },
            { 134, "9sus",          "suspended-fourth", "add7 add9",                     HChord("C F G Bb D") },   // 11 - 9sus
            { 135, "9susb13",       "suspended-fourth", "add7 add9 addb13",              HChord("C F G Bb D Ab") },
            { 136, "9sus#11",       "suspended-fourth", "add7 add9 add#11",              HChord("C F G Bb D F#") },
            { 137, "13sus#11",      "suspended-fourth", "add7 add9 add#11 add13",        HChord("C F G Bb D F# A") },
            { 138, "13sus#11",      "suspended-fourth", "add7 add9 add#11 add13",        HChord("C F G Bb D F# A") },   // as 137
            { 139, "9sus#11b13",    "suspended-fourth", "add7 add9 add#11 addb13",       HChord("C F G Bb D F# Ab") },

/*140*/     { 140, "7susb9",        "suspended-fourth", "add7 addb9",                    HChord("C F G Bb Db") },
            { 141, "13susb9",       "suspended-fourth", "add7 addb9 add11 add13",        HChord("C F G Bb Db F A") },
            { 142, "7susb9b13",     "suspended-fourth", "add7 addb9 addb13",             HChord("C F G Bb Db Ab") },
            { 143, "7susb9#11",     "suspended-fourth", "add7 addb9 add#11",             HChord("C F G Bb Db F#") },
            { 144, "13susb9#11",    "suspended-fourth", "add7 addb9 add#11 add13",       HChord("C F G Bb Dd F# A") },
            { 145, "7susb9#11b13",  "suspended-fourth", "add7 addb9 add#11 addb13",      HChord("C F G Bb Db F# Ab") },
            { 146, "7sus#9",        "suspended-fourth", "add7 add#9",                    HChord("C F G Bb Eb") },
            { 147, "13sus#9",       "suspended-fourth", "add7 add#9 add13",              HChord("C F G Bb D# A") },
            { 148, "7sus#9b13",     "suspended-fourth", "add7 add#9 addb13",             HChord("C F G Bb D# Ab") },
            { 149, "9sus#11",       "suspended-fourth", "add7 add9 add#11",              HChord("C F G Bb D F#") },

/*150*/     { 150, "13sus#9#11",    "suspended-fourth", "add7 add#9 add#11 add13",       HChord("C F G Bb D# F# A") },
            { 151, "7sus#9#11b13",  "suspended-fourth", "add7 add#9 add#11 addb13",      HChord("C F G Bb D# F# Ab") },
            { 148, "7sus#9b13",     "suspended-fourth", "add7 add#9 addb13",             HChord("C F G Bb D# Ab") },
            { 152, "7susb5",        "suspended-fourth", "add7 altb5" ,                   HChord("C F Gb Bb")       },
            { 153, "13susb5",       "suspended-fourth", "alt5 add7 add9 add13",          HChord("C F Gb Bb D A") },
            { 154, "7susb5b13",     "suspended-fourth", "add7 altb5 addb13",             HChord("C F Gb Bb Ab")    },
            { 155, "9susb5",        "suspended-fourth", "alt5 add7 add9",                HChord("C F Gb Bb D") },
            { 156, "9susb5b13",     "suspended-fourth", "alt5 add7 add9 add13",          HChord("C F Gb Bb D Ab") },
            { 157, "7susb5b9",      "suspended-fourth", "add7 altb5 addb9",              HChord("C F Gb Bb Db")    },
            { 158, "13susb5b9",     "suspended-fourth", "alt5 add7 altb9 add13",         HChord("C F Gb Bb D A") },
            { 159, "7susb5b9b13",   "suspended-fourth", "add7 altb5 addb9 addb13",       HChord("C F Gb Bb Db Ab") },

/*160*/     { 160, "7susb5#9",      "suspended-fourth", "add7 altb5 add#9",              HChord("C F Gb Bb Eb")    },
            { 161, "13susb5#9",     "suspended-fourth", "altb5 add7 add#9 add13",        HChord("C F Gb Bb D# A") },
            { 162, "7susb5#9b13",   "suspended-fourth", "add7 altb5 add#9 addb13",       HChord("C F Gb Bb Eb Ab") },
            { 163, "7sus#5",        "suspended-fourth", "add7 alt#5",                    HChord("C F Ab Bb")       },
            { 164, "13sus#5",       "suspended-fourth", "alt#5 add7 add9 add13",         HChord("C F G# Bb D A") },
            { 165, "7sus#5#11",     "suspended-fourth", "add7 alt#5 add#11",             HChord("C F Ab Bb")       },
            { 166, "13sus#5#11",    "suspended-fourth", "alt#5 add7 add9 add#11 add13",  HChord("C F G# Bb D F# A") },
            { 167, "9sus#5",        "suspended-fourth", "alt#5 add7 add9",               HChord("C F G# Bb D") },
            { 168, "9sus#5#11",     "suspended-fourth", "alt#5 add7 add9 add#11",        HChord("C F G# Bb D F#") },
            { 169, "7sus#5b9",      "suspended-fourth", "add7 alt#5 addb9",              HChord("C F Ab Bb Db")    },

/*170*/     { 170, "13sus#5b9",     "suspended-fourth", "alt#5 add7 add9 add13",         HChord("C F G# Bb Db A") },
            { 171, "7sus#5b9#11",   "suspended-fourth", "add7 alt#5 addb9 add#11",       HChord("C F Ab Bb Db Gb") },
            { 172, "13sus#5b9#11",  "suspended-fourth", "alt#5 add7 add9 add#11 add13",  HChord("C F G# Bb Db F# A") },
            { 173, "7sus#5#9",      "suspended-fourth", "add7 alt#5 add#9",              HChord("C F Ab Bb Eb")    },
            { 174, "13sus#5#9#11",  "suspended-fourth", "alt#5 add7 add#9 add#11 add13", HChord("C F G# Bb D# F# A") },
            { 175, "7sus#5#9#11",   "suspended-fourth", "add7 alt#5 add#9 add#11",       HChord("C F Ab Bb Eb Gb") },
            { 176, "13sus#5#9#11",  "suspended-fourth", "alt#5 add7 add#9 add#11 add13", HChord("C F G# Bb D# F# A") }, //same as 174
            { 177, "4",             "major", "add4",                                     HChord("C E F G") },

            { 184, "sus",           "suspended-fourth", 0,                               HChord("C F G") },  // sus4

            // the following ids are not in "BandInABox"
            { 185, "dim7",          "diminished-seventh", 0,                             HChord("C Eb Gb Bbb") },  // mscore ext.
            { 186, "sus2",          "suspended-second", 0,                               HChord("C D G") },      // suspended 2nd chord
            { 187, "maddb13",       "minor", "addb13",                                   HChord("C Eb Ab") },
            { 188, "#13",           "major", "add#13",                                   HChord("C E G A#") },
            // { 189, "#11#13",     "major", "add#11 add#13",                            HChord("C E G F# A#") },
            { 189, "add#11#13",     "major", "add#11 add#13",                            HChord("C E G F# A#") },

/*190*/     { 190, "add#13",        "major", "add#13",                                   HChord("C E G A#") },
            { 191, "6add9",         "maj69", 0,                                          HChord("C E G A D") },
            { 192, "sus4",          "suspended-fourth", 0,                               HChord("C F G") },        // sus4
            { 193, "11",            "dominant-11th", 0,                                  HChord("C E G Bb D F") }, // dominant 11th / 9sus
            { 194, "Maj11",         "major-11th", 0,                                     HChord("C E G B D F") },  // major 11th
            { 195, "Tristan",       "tristan", 0,                                        HChord("C F# A# D") },    // Tristan

            { 64,  "7",             "dominant", 0,                                       HChord("C E G Bb") },     // dominant-seventh
            { 128, "7sus4",         "suspended-fourth", "add7",                          HChord("C F G Bb")   },

//            { 146, "7sus#9",        "suspended-fourth", "add7 add#9",                  HChord("C F G Bb Eb") },
            { 196, "m7add11",       "minor-seventh", "add11",                            HChord("C Eb F G Bb") },
            { 197, "Maj7add13",     "major-seventh", "add13",                            HChord("C E G A B") },
            { 198, "madd9",         "minor", "add9",                                     HChord("C D Eb G") },
            { 18,  "mMaj7",         "major-minor", 0,                                    HChord("C Eb G B") },     // minor major 7th = major minor 7th
            { 199, "m9Maj7",        "major-minor", "add9",                               HChord("C Eb G B D") },
/* 40 */    { 200, "5",             "pedal", "add5",                                     HChord("C G") },
            { 201, "m11b5",         "minor-11th", "altb5",                               HChord("C Eb Gb Bb D F") },
            { 202, "dim7add#7",     "diminished-seventh", "add#7",                       HChord() }, // HChord("C Eb Gb Bbb") },
            { 203, "#59",           "augmented-seventh", "add9",                         HChord("C E Ab D") },
            { 204, "omit5",         "major", "sub5",                                     HChord("C E") },
            { 205, "aug7",          "augmented-seventh", 0,                              HChord("C E Ab") },        // augmented 7th
            { 206, "aug9",          "augmented-ninth", 0,                                HChord("C E Ab Bb D") },   // augmented 9th
            { 207, "aug13",         "dominant-13th", "alt#5",                            HChord("C E G# Bb D A") },
      };

//---------------------------------------------------------
//   chordListSize
//---------------------------------------------------------

unsigned int Harmony::chordListSize()
      {
      return sizeof(chordList)/sizeof(*chordList);
      }

//---------------------------------------------------------
//   initHarmony
//    init chordHash hash table
//---------------------------------------------------------

void Harmony::initHarmony()
      {
      for (unsigned i = 0; i < sizeof(chordList)/sizeof(*chordList); ++i) {
            if (chordList[i].name) {
                  int id = chordList[i].id;
                  chordHash[id] = &chordList[i];
                  }
            }
      }

//---------------------------------------------------------
//   harmonyName
//---------------------------------------------------------

QString Harmony::harmonyName() const
      {
      bool germanNames = score()->style()->useGermanNoteNames;

      HChord hc = _descr ? _descr->chord : HChord();
      QString s;

      if (!_degreeList.isEmpty()) {
            hc.add(_degreeList);
            // try to find the chord in chordList
            const ChordDescription* newExtension = 0;
            for (int i = 1; i < int(sizeof(chordList)/sizeof(*chordList)); i++) {
                  if (chordList[i].chord == hc && chordList[i].name != 0) {
                        newExtension = &chordList[i];
                        break;
                        }
                  }
            // now determine the chord name
            if (newExtension)
                  s = tpc2name(_rootTpc, germanNames) + newExtension->name;
            else
                  // not in table, fallback to using HChord.name()
                  s = hc.name(_rootTpc);
            s += " ";
            } // end if (degreeList ...
      else {
            s = tpc2name(_rootTpc, germanNames);
            if (_descr) {
                  s += " ";
                  s += _descr->name;
                  }
            }
      if (_baseTpc != INVALID_TPC) {
            s += "/";
            s += tpc2name(_baseTpc, germanNames);
            }
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

      HChord hc = _descr ? _descr->chord : HChord();

      hc.add(_degreeList);

printf("resolveDegreeList: <%s> <%s-%s>: ", _descr->name, _descr->xmlKind, _descr->xmlDegrees);
hc.print();
_descr->chord.print();

      // try to find the chord in chordList
      for (int i = 1; i < int(sizeof(chordList)/sizeof(*chordList)); i++) {
            if ((chordList[i].chord == hc) && chordList[i].name != 0) {
printf("ResolveDegreeList: found in table as %s\n", chordList[i].name);
                  _descr = &chordList[i];
                  _degreeList.clear();
                  return;
                  }
            }
printf("ResolveDegreeList: not found in table\n");
      }

//---------------------------------------------------------
//   Harmony
//---------------------------------------------------------

Harmony::Harmony(Score* score)
   : Text(score)
      {
      Text::setSubtype(TEXT_CHORD);
      _rootTpc   = INVALID_TPC;
      _baseTpc   = INVALID_TPC;
      _descr     = 0;
      }

Harmony::Harmony(const Harmony& h)
   : Text(h)
      {
      _rootTpc    = h._rootTpc;
      _baseTpc    = h._baseTpc;
      _descr      = h._descr;
      _degreeList = h._degreeList;
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
      a = popup->addAction(tr("Harmony Properties..."));
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
            ce.setHarmony(this);
            int rv = ce.exec();
            if (rv) {
                  const Harmony* h = ce.harmony();
                  setRootTpc(h->rootTpc());
                  setBaseTpc(h->baseTpc());
                  setDescr(h->descr());
                  clearDegrees();
                  for (int i = 0; i < h->numberOfDegrees(); i++)
                        addDegree(h->degree(i));
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
            if (_descr)
                  xml.tag("extension", _descr->id);
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
                              case ADD:
                                    xml.tag("degree-type", "add");
                                    break;
                              case ALTER:
                                    xml.tag("degree-type", "alter");
                                    break;
                              case SUBTRACT:
                                    xml.tag("degree-type", "subtract");
                                    break;
                              default:
                                    break;
                              }
                        xml.etag();
                        }
                  }
            Element::writeProperties(xml);
            }
      else
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
                  setChordId(i);
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
      static const QChar sharp(0xe10c);
      static const QChar flat(0xe10d);

      if (_rootTpc == INVALID_TPC)
            return;

      clear();

      QString txt(harmonyName());
      const char* s = strdup(txt.toAscii().data());

//    printf("Harmony:buildText(): Harmony <%s>\n", s);

      QTextCursor cursor(doc());
      cursor.setPosition(0);
      QTextCharFormat f = cursor.charFormat();
      QTextCharFormat noteSymbolFormat(f);
      noteSymbolFormat.setFont(QFont("MScore1"));

      QTextCharFormat snoteSymbolFormat(noteSymbolFormat);
      snoteSymbolFormat.setVerticalAlignment(QTextCharFormat::AlignSuperScript);

      if (*s == 0)
            return;

      cursor.insertText(QString(*s++));
      if (s == 0)
            return;

      bool useSymbols = score()->style()->chordNamesUseSymbols;

      if ((*s == '#') || (*s == 'b')) {
            if (useSymbols)
                  cursor.insertText(QString(*s), f);
            else {
                  if (*s == '#')
                        cursor.insertText(QString(sharp), noteSymbolFormat);
                  else
                        cursor.insertText(QString(flat), noteSymbolFormat);
                  }
            ++s;
            }
      if (*s == ' ')
            ++s;

      QTextCharFormat sf(f);
      sf.setVerticalAlignment(QTextCharFormat::AlignSuperScript);

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
            while (s < ss) {
                  if ((*s == '#') || (*s == 'b')) {
                        if (useSymbols)
                              cursor.insertText(QString(*s), sf);
                        else {
                              if (*s == '#')
                                    cursor.insertText(QString(sharp), snoteSymbolFormat);
                              else
                                    cursor.insertText(QString(flat), snoteSymbolFormat);
                              }
                        ++s;
                        }
                  else
                        cursor.insertText(QString(*s++), sf);
                  }
            }
      if (slash) {
            cursor.insertText(QString('/'), f);
            cursor.insertText(QString(*slash++), f);

            if ((*slash == '#') || (*slash == 'b')) {
                  if (useSymbols)
                        cursor.insertText(QString(*slash), f);
                  else {
                        if (*slash == '#')
                              cursor.insertText(QString(sharp), noteSymbolFormat);
                        else
                              cursor.insertText(QString(flat), noteSymbolFormat);
                        }
                  ++slash;
                  }
            if (*slash)
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
//    return ChordDescription
//---------------------------------------------------------

const ChordDescription* Harmony::parseHarmony(const QString& ss, int* root, int* base)
      {
      QString s = ss.simplified();
      int n = s.size();
      if (n < 1) {
            printf("harmony is empty %d\n", tick());
            return 0;
            }
      bool germanNames = score()->style()->useGermanNoteNames;
      int r = convertRoot(s, germanNames);
      if (r == INVALID_TPC) {
            printf("1:parseHarmony failed <%s>\n", qPrintable(ss));
            return 0;
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
      for (unsigned i = 0; i < sizeof(chordList)/sizeof(*chordList); ++i) {
            if (QString(chordList[i].name).toLower() == s)
                  return &chordList[i];
            }
      printf("2:parseHarmony failed <%s><%s>\n", qPrintable(ss), qPrintable(s));
      return 0;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Harmony::endEdit()
      {
      Text::endEdit();

      int r, b;
      const ChordDescription* e = parseHarmony(getText(), &r, &b);
      if (e) {
            setRootTpc(r);
            setBaseTpc(b);
            setDescr(e);
            buildText();
            }
      else {
            // syntax error, leave text as is
            setRootTpc(INVALID_TPC);
            setBaseTpc(INVALID_TPC);
            setDescr(0);
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
      double x = (tick() != -1) ? measure()->tick2pos(tick()) : 0.0;
      setPos(ipos() + QPointF(x, y));
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString HDegree::text() const
      {
      if (_type == UNDEF)
            return QString();
      QString degree;
      switch(_type) {
            case UNDEF: break;
            case ADD:         degree += "add"; break;
            case ALTER:       degree += "alt"; break;
            case SUBTRACT:    degree += "sub"; break;
            }
      switch(_alter) {
            case -1:          degree += "b"; break;
            case 1:           degree += "#"; break;
            default:          break;
            }
      return degree + QString("%1").arg(_value);
      }

//---------------------------------------------------------
//   fromXml
//    lookup harmony in harmony data base
//    using musicXml "kind" string and degree list
//---------------------------------------------------------

const ChordDescription* Harmony::fromXml(const QString& kind,  const QList<HDegree>& dl)
      {
      QString degrees;

      foreach(const HDegree& d, dl) {
            if (!degrees.isEmpty())
                  degrees += " ";
            degrees += d.text();
            }
      QString lowerCaseKind = kind.toLower();
      for (unsigned i = 0; i < sizeof(chordList)/sizeof(*chordList); ++i) {
            const ChordDescription& cd = chordList[i];
            const char* k = cd.xmlKind;
            const char* d = cd.xmlDegrees;
            if (
               (lowerCaseKind == k)
               &&
               (((d == 0) && degrees.isEmpty()) || (d == degrees))
               ) {
                  printf("harmony found in db: %s %s -> %d\n", qPrintable(kind), qPrintable(degrees), cd.id);
                  return &cd;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   fromXml
//    lookup harmony in harmony data base
//    using musicXml "kind" string and degree list
//---------------------------------------------------------

const ChordDescription* Harmony::fromXml(const QString& kind)
      {
      QString lowerCaseKind = kind.toLower();
      for (unsigned i = 0; i < sizeof(chordList)/sizeof(*chordList); ++i) {
            const ChordDescription& cd = chordList[i];
            if (lowerCaseKind == cd.xmlKind)
                  return &cd;
            }
      return 0;
      }

//---------------------------------------------------------
//   setChordId
//---------------------------------------------------------

void Harmony::setChordId(int id)
      {
      _descr = chordHash[id];
      }

//---------------------------------------------------------
//   harmonyEndEdit
//---------------------------------------------------------

void Score::harmonyEndEdit()
      {
      Harmony* harmony = static_cast<Harmony*>(editObject);
      Harmony* origH   = static_cast<Harmony*>(origEditObject);

      if (harmony->isEmpty() && origH->isEmpty()) {
            Measure* measure = (Measure*)(harmony->parent());
printf("remove empty harmony %d\n", harmony->tick());
            measure->remove(harmony);
            }
      }


