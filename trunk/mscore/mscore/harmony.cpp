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
            buf += "j7";
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


#if 0
        ======================unknown==================
      "7susb13",
      "7sus#11",
      "13sus#11",
      "7sus#11b13",
      "9sus",
      "9susb13",
      "9sus#11",
      "13sus#11",
      "9sus#11b13",
#endif

//---------------------------------------------------------
//   extensionNames
//    chord extension names
//---------------------------------------------------------

struct ChordNames {
      const char* name;
      HChord chord;           // C based chord
      };

static const ChordNames extensionNames[] = {
            { 0,            HChord() },
            { "",           HChord("C E G") },        // major triad
            { "Maj",        HChord("C E G") },
            { "5b",         HChord("C E Gb") },       // major flat 5 triad
            { "aug",        HChord("C E G#") },       // augmented triad
            { "6",          HChord("C E G A") },      // sixth
            { "Maj7",       HChord("C E G B") },
            { "Maj9",       HChord("C E G B D") },
            { "Maj9#11",    HChord("C E G B D F#") },
            { "Maj13#11",   HChord("C E G B D F# A") },
/*10*/
            { "Maj13",      HChord("C E G B D F# A") },
            { 0,            HChord() },
            { "+",          HChord("C E G#") },       // xx augmented triad
            { "Maj7#5",     HChord() },
            { "69",         HChord("C D E G A") },
            { "2",          HChord("C D E G") },      // major add 2
            { "m",          HChord("C Eb G") },
            { "maug",       HChord("C D# G#") },
            { "mMaj7",      HChord("C Eb G B") },     // minor major 7th = major minor 7th
            { "m7",         HChord("C Eb G Bb") },

/*20*/      { "m9",         HChord("C Eb G Bb D") },
            { "m11",        HChord("C Eb G Bb D F") },
            { "m13",        HChord("C Eb G Bb D F A") },
            { "m6",         HChord("C Eb G A") },
            { "m#5",        HChord() },
            { "m7#5",       HChord() },
            { "m69",        HChord() },
            { 0,            HChord() },
            { "Maj7Lyd",    HChord() },
            { "Maj7b5",     HChord() },

/*30*/      { 0,            HChord() },
            { 0,            HChord() },
            { "m7b5",       HChord() },
            { "dim",        HChord("C Eb Gb A") },    // dim7
            { "m9b5",       HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },

/*40*/      { "5",          HChord("C G") },  // power
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },

/*50*/      { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { "7+",         HChord() },
            { "9+",         HChord() },
            { "13+",        HChord() },
            { "(blues)",    HChord() },   // ??

/*60*/      { "7(Blues)",   HChord() },   // ??
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { "7",          HChord() },
            { "13",         HChord("C E G Bb D F A") },  // dominant 13th
            { "7b13",       HChord() },
            { "7#11",       HChord() },
            { "13#11",      HChord() },
            { "7#11b13",    HChord() },

/*70*/      { "9",          HChord() },
            { "9b13",       HChord() },
            { 0,            HChord() },
            { "9#11",       HChord() },
            { "13#11",      HChord() },
            { "9#11b13",    HChord() },
            { "7b9",        HChord() },
            { "13b9",       HChord() },
            { "7b9b13",     HChord() },
            { "7b9#11",     HChord() },

/*80*/      { "13b9#11",    HChord() },
            { "7b9#11b13",  HChord() },
            { "7#9",        HChord() },
            { "13#9",       HChord() },
            { "7#9b13",     HChord() },
            { "9#11",       HChord() },
            { "13#9#11",    HChord() },
            { "7#9#11b13",  HChord() },
            { "7b5",        HChord() },
            { "13b5",       HChord() },

/*90*/      { "7b5b13",     HChord() },
            { "9b5",        HChord() },
            { "9b5b13",     HChord() },
            { "7b5b9",      HChord() },
            { "13b5b9",     HChord() },
            { "7b5b9b13",   HChord() },
            { "7b5#9",      HChord() },
            { "13b5#9",     HChord() },
            { "7b5#9b13",   HChord() },
            { "7#5",        HChord() },

/*100*/     { "13#5",       HChord() },
            { "7#5#11",     HChord() },
            { "13#5#11",    HChord() },
            { "9#5",        HChord() },
            { "9#5#11",     HChord() },
            { "7#5b9",      HChord() },
            { "13#5b9",     HChord() },
            { "7#5b9#11",   HChord() },
            { "13#5b9#11",  HChord() },
            { "7#5#9",      HChord() },

/*110*/     { "13#5#9#11",  HChord() },
            { "7#5#9#11",   HChord() },
            { "13#5#9#11",  HChord() },
            { "7alt",       HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },

/*120*/     { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { "7sus",       HChord() },
            { "13sus",      HChord() },

/*130*/     { "7susb13",    HChord() },
            { "7sus#11",    HChord() },
            { "13sus#11",   HChord() },
            { "7sus#11b13", HChord() },
            { "9sus",       HChord() },   // 11 - 9sus
            { "9susb13",    HChord() },
            { "9sus#11",    HChord() },
            { "13sus#11",   HChord() },
            { "9sus#11b13", HChord() },
            { 0,            HChord() },

/*140*/     { "7susb9",     HChord() },
            { "13susb9",    HChord() },
            { "7susb9b13",  HChord() },
            { "7susb9#11",  HChord() },
            { "13susb9#11", HChord() },
            { "7susb9#11b13",HChord() },
            { "7sus#9",      HChord() },
            { "13sus#9",     HChord() },
            { "7sus#9b13",   HChord() },
            { "9sus#11",     HChord() },

/*150*/     { "13sus#9#11",   HChord() },
            { "7sus#9#11b13", HChord() },
            { "7susb5",       HChord() },
            { "13susb5",      HChord() },
            { "7susb5b13",    HChord() },
            { "9susb5",       HChord() },
            { "9susb5b13",    HChord() },
            { "7susb5b9",     HChord() },
            { "13susb5b9",    HChord() },
            { "7susb5b9b13",  HChord() },

/*160*/     { "7susb5#9",     HChord() },
            { "13susb5#9",    HChord() },
            { "7susb5#9b13",  HChord() },
            { "7sus#5",       HChord() },
            { "13sus#5",      HChord() },
            { "7sus#5#11",    HChord() },
            { "13sus#5#11",   HChord() },
            { "9sus#5",       HChord() },
            { "9sus#5#11",    HChord() },
            { "7sus#5b9",     HChord() },

/*170*/     { "13sus#5b9",    HChord() },
            { "7sus#5b9#11",  HChord() },
            { "13sus#5b9#11", HChord() },
            { "7sus#5#9",     HChord() },
            { "13sus#5#9#11", HChord() },
            { "7sus#5#9#11",  HChord() },
            { "13sus#5#9#11", HChord() },
            { "4",            HChord() },
            { 0,              HChord() },
            { 0,              HChord() },

/*180*/     { 0,              HChord() },
            { 0,              HChord() },
            { 0,              HChord() },
            { 0,              HChord() },
            { "sus",          HChord("C F G") },

            { "dim7",         HChord() },  // mscore ext.
            { "sus2",         HChord("C D G") },      // suspended 2nd chord
            { "mb3b13",       HChord() },  // neapolitan
            { "#13",          HChord() },  // italian
            { "#11#13",       HChord() },  // french

/*190*/     { "add#13",       HChord() },  // german
            { "6/add9",       HChord() },
            { "sus4",         HChord("C F G") },        // sus
            { "11",           HChord("C E G Bb D F") }, // dominant 11th / 9sus
            { "Maj11",        HChord("C E G B D F") },  // major 11th
            { "Tristan",      HChord("C F# A# D") },    // Tristan
      };

//---------------------------------------------------------
//   getExtensionName
//---------------------------------------------------------

const char* Harmony::getExtensionName(int i)
      {
      if (i >= int(sizeof(extensionNames)/sizeof(*extensionNames)))
            return 0;
      return extensionNames[i].name;
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
      printf("Harmony::harmonyName: <%s>\n", qPrintable(s));
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

      if (extension >= 0 && extension < int(sizeof(extensionNames)/sizeof(*extensionNames)))
            hc = extensionNames[extension].chord;
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
                  if (d.type() == ADD) {
//                        printf(" add degree value=%d alter=%d\n", d.value(), d.alter());
                        hc += (degreeTable[(d.value() - 1) % 7] + d.alter());
                        }
                  else if (d.type() == ALTER) {
//                        printf(" alter degree value=%d alter=%d\n", d.value(), d.alter());
                        if (hc.contains(degreeTable[(d.value() - 1) % 7])) {
                              hc -= (degreeTable[(d.value() - 1) % 7]);
                              hc += (degreeTable[(d.value() - 1) % 7] + d.alter());
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
            // try to find the chord in extensionNames
            int newExtension = 0;
            for (int i = 1; i < int(sizeof(extensionNames)/sizeof(*extensionNames)); i++)
                  if (extensionNames[i].chord == hc && extensionNames[i].name != 0) {
//                        printf(" found in table as %s\n", extensionNames[i].name);
                        newExtension = i;
                        break;
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
      for (unsigned i = 1; i < sizeof(extensionNames)/sizeof(*extensionNames); ++i) {
            if (QString(extensionNames[i].name).toLower() == s)
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

