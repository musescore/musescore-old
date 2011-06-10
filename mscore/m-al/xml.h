//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
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

#ifndef __XML_H__
#define __XML_H__

#include <QtCore/QString>
#include "mxml/xmlreader.h"
#include "m-libmscore/globals.h"

class Color;
class QPointF;
class QSizeF;
class Fraction;

extern qreal matof(const char*);

//---------------------------------------------------------
//   MString8
//---------------------------------------------------------

class MString8 {
      const xmlChar* _s;

   public:
      MString8()                         { _s = 0; }
      MString8(const xmlChar* s) : _s(s) {}
      const xmlChar* s() const           { return _s; }
      QString string() const { return QString::fromUtf8((const char*)_s); }

      void operator=(const char* s) { _s = (const xmlChar*) s; }
      };

//---------------------------------------------------------
//   XmlReader
//---------------------------------------------------------

class XmlReader : public XmlTextReader {
      MString8 _tag;

   public:
      XmlReader(const char* p, int n) : XmlTextReader(p, n, 0, 0, 0) {}

      bool readElement(const char*);
      bool readElement();
      bool readAttribute();

      void skipElement(const xmlChar*);

      bool readInt(const char* e, int*);
      bool readChar(const char* e, char*);
      int readInt(const char* e);
      bool readBool(const char* e, bool*);
      bool readReal(const char* e, qreal*);
      qreal readReal(const char* e);
      bool readString(const char* tag, QString*);
      bool readColor(const char* tag, Color*);
      bool readPoint(const char* tag, QPointF*);
      bool readFraction(const char* tag, Fraction*);
      bool readPlacement(const char* tag, Placement*);
      bool readSize(const char* t, QSizeF* val);

      int line() const            { return currentNode() ? currentNode()->line : 0; }
      QString stringValue() const { return QString::fromUtf8((const char*)value()); }
      int intValue() const        { return atoi((const char*)value()); }
      qreal realValue() const     { return matof((const char*)value()); }
      void unknown();
      void error(const char*) const {}
      const MString8& tag() const   { return _tag; }
      };

// extern bool isTag(const xmlChar* s, const char* tag);
extern bool operator==(const MString8, const char*);
extern bool operator!=(const MString8, const char*);

#endif

