//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
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

static char* extensionNames[] = {
            0,                0,                "Maj",            "5b",             "aug",
            "6",              "Maj7",           "Maj9",           "Maj9#11",        "Maj13#11",
/*10*/      "Maj13",          0,                "+",              "Maj7#5",         "69",
            "2",              "m",              "maug",           "mMaj7",          "m7",
/*20*/      "m9",             "m11",            "m13",            "m6",             "m#5",
            "m7#5",           "m69",            0,                "Maj7Lyd",        "Maj7b5",
/*30*/      0,                0,                "m7b5",           "dim",            "m9b5",
            0,                0,                0,                0,                0,
/*40*/      "5",              0,                0,                0,                0,
            0,                0,                0,                0,                0,
/*50*/      0,                0,                0,                0,                0,
            0,                "7+",             "9+",             "13+",            "(blues)",
/*60*/      "7(Blues)",       0,                0,                0,                "7",
            "13",             "7b13",           "7#11",           "13#11",          "7#11b13",
/*70*/      "9",              "9b13",           0,                "9#11",           "13#11",
            "9#11b13",        "7b9",            "13b9",           "7b9b13",         "7b9#11",
            "13b9#11",        "7b9#11b13",      "7#9",            "13#9",           "7#9b13",
            "9#11",           "13#9#11",        "7#9#11b13",      "7b5",            "13b5",
            "7b5b13",         "9b5",            "9b5b13",         "7b5b9",          "13b5b9",
            "7b5b9b13",       "7b5#9",          "13b5#9",         "7b5#9b13",       "7#5",

/*100*/     "13#5",           "7#5#11",         "13#5#11",        "9#5",            "9#5#11",
            "7#5b9",          "13#5b9",         "7#5b9#11",       "13#5b9#11",      "7#5#9",
            "13#5#9#11",      "7#5#9#11",       "13#5#9#11",      "7alt",           0,
            0,                0,                0,                0,                0,
            0,                0,                0,                0,                0,
            0,                0,                0,                "7sus",           "13sus",
/*130*/     "7susb13",        "7sus#11",        "13sus#11",       "7sus#11b13",     "9sus",
            "9susb13",        "9sus#11",        "13sus#11",       "9sus#11b13",     0,
/*140*/     "7susb9",         "13susb9",        "7susb9b13",      "7susb9#11",      "13susb9#11",
            "7susb9#11b13",   "7sus#9",         "13sus#9",        "7sus#9b13",      "9sus#11",
            "13sus#9#11",     "7sus#9#11b13",   "7susb5",         "13susb5",        "7susb5b13",
            "9susb5",         "9susb5b13",      "7susb5b9",       "13susb5b9",      "7susb5b9b13",
            "7susb5#9",       "13susb5#9",      "7susb5#9b13",    "7sus#5",         "13sus#5",
            "7sus#5#11",      "13sus#5#11",     "9sus#5",         "9sus#5#11",      "7sus#5b9",
            "13sus#5b9",      "7sus#5b9#11",    "13sus#5b9#11",   "7sus#5#9",       "13sus#5#9#11",
            "7sus#5#9#11",    "13sus#5#9#11",   "4",              0,                0,
            0,                0,                0,                0,                "sus"
      };

//---------------------------------------------------------
//   getExtensionName
//---------------------------------------------------------

const char* Harmony::getExtensionName(int i)
      {
      if (i >= int(sizeof(extensionNames)/sizeof(*extensionNames)))
            return 0;
      return extensionNames[i];
      }

//---------------------------------------------------------
//   getName
//---------------------------------------------------------

QString Harmony::harmonyName(int root, int extension, int base)
      {
      static char* rootTable[] = {
            "C",   "Db", "D",   "Eb",  "E",   "F",   "Gb", "G",
            "Ab", "A",   "Bb", "B"
            };
      QString s(rootTable[root]);
      if (extension)
            s += getExtensionName(extension);
      if (base) {
            s += "/";
            s += rootTable[base-1];
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
      _root = 0;
      _extension = 0;
      _base = 0;
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

