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

const HChord HChord::C0(0,3,6,9);

//---------------------------------------------------------
//   scale_names
//---------------------------------------------------------

const char* const HChord::scaleNames[2][12] = {
      { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },
      { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" }
      };

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

HChord::HChord(const char* s)
      {
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

QString HChord::name(int key, bool flat)
      {
      QString buf = scaleName(key, flat);
      HChord c(*this);
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
            { "+",          HChord("C E G#") },       // augmented triad
            { "Maj7#5",     HChord() },
            { "69",         HChord("C D E G A") },
            { "2",          HChord("C D E G") },      // major add 2
            { "m",          HChord("C Eb G") },
            { "maug",       HChord("C D# G#") },
            { "mMaj7",      HChord() },
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

/*40*/      { "5",          HChord() },
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
            { "(blues)",    HChord() },

/*60*/      { "7(Blues)",   HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { 0,            HChord() },
            { "7",          HChord() },
            { "13",         HChord() },
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
            { "9sus",       HChord() },
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
            { "sus4",         HChord("C F G") },      // sus
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
//   rootName
//---------------------------------------------------------

QString Harmony::rootName(int root)
      {
      static char* rootTable[] = {
            "C",   "Db", "D",   "Eb",  "E",   "F",   "Gb", "G",
            "Ab", "A",   "Bb", "B"
            };
      return QString(rootTable[root-1]);
      }

//---------------------------------------------------------
//   harmonyName
//---------------------------------------------------------

QString Harmony::harmonyName(int root, int extension, int base)
      {
      QString s(rootName(root));
      if (extension)
            s += getExtensionName(extension);
      if (base) {
            s += "/";
            s += rootName(base);
            }
      return s;
      }

//---------------------------------------------------------
//   Harmony
//---------------------------------------------------------

Harmony::Harmony(Score* score)
   : Text(score)
      {
      Text::setSubtype(TEXT_CHORD);
      _root      = 0;
      _extension = 0;
      _base      = 0;
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
//      a->setText(tr("Chord name"));
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
            ChordEdit ce;
            ce.setRoot(root());
            ce.setBase(base());
            ce.setExtension(extension());
            int rv = ce.exec();
            if (rv) {
                  setRoot(ce.root());
                  setBase(ce.base());
                  setExtension(ce.extension());
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
      xml.tag("root", _root);
      if (_extension)
            xml.tag("extension", _extension);
      if (_base)
            xml.tag("base", _base);
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Harmony::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "base")
                  setBase(e.text().toInt());
            else if (tag == "extension")
                  setExtension(e.text().toInt());
            else if (tag == "root")
                  setRoot(e.text().toInt());
            else if (!Text::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   buildText
//    construct Chord Symbol
//---------------------------------------------------------

void Harmony::buildText()
      {
      setText(harmonyName());
      }

//---------------------------------------------------------
//   convertRoot
//---------------------------------------------------------

int convertRoot(const QString& s, int* alter)
      {
      int n = s.size();
      if (n < 1)
            return -1;
      *alter = 0;
      if (n > 1) {
            if (s[1].toLower().toAscii() == 'b')
                  *alter = -1;
            }
      int r = 0;
      switch(s[0].toLower().toAscii()) {
            case 'c':
                  r = 1;
                  break;
            case 'd':
                  r = 3;
                  break;
            case 'e':
                  r = 5;
                  break;
            case 'f':
                  r = 6;
                  break;
            case 'g':
                  r = 8;
                  break;
            case 'a':
                  r = 10;
                  break;
            case 'b':
                  r = 12;
                  break;
            default:
                  return -1;
            }
      r -= *alter;
      if (r < 1)
            r = 12;
      return r;
      }

//---------------------------------------------------------
//   parseHarmony
//---------------------------------------------------------

int Harmony::parseHarmony(const QString& ss, int* root, int* base)
      {
      QString s = ss.simplified();
      int n = s.size();
      if (n < 1)
            return -1;
      int alter;
      int r = convertRoot(s, &alter);
      if (r == -1)
            return -1;
      *root   = r;
      int idx = alter ? 2 : 1;
      *base = 0;
      int slash = s.indexOf('/');
      if (slash != -1) {
            s     = s.mid(idx, slash - idx);
            *base = convertRoot(s.mid(slash + 1), &alter);
            }
      else
            s = s.mid(idx);
      for (unsigned i = 1; i < sizeof(extensionNames)/sizeof(*extensionNames); ++i) {
            if (extensionNames[i].name == s)
                  return i;
            }
      return 1;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Harmony::endEdit()
      {
      Text::endEdit();

      QString s = getText();
      int r, b;
      int e = parseHarmony(getText(), &r, &b);
      if (e != -1) {
            setRoot(r);
            setBase(b);
            setExtension(e);
            buildText();
            }
      else {
            setRoot(0);             // unknown
            setBase(0);
            setExtension(1);
            // leave text as entered
            }
      }

