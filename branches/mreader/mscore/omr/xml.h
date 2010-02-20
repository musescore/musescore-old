//=============================================================================
//  MusE
//  Linux Music Score Editor
//  $Id: xml.h 1840 2009-05-20 11:57:51Z wschweer $
//
//  Copyright (C) 2004-2009 Werner Schweer and others
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

#ifndef __AL_XML_H__
#define __AL_XML_H__

//---------------------------------------------------------
//   Property
//---------------------------------------------------------

class Prop {
   public:
      const char* name;
      QVariant data;
      Prop() {}
      Prop(const char* n, const QVariant& d) : name(n), data(d) {}
      };

//---------------------------------------------------------
//   Xml
//---------------------------------------------------------

class Xml : public QTextStream {
      static const int BS = 2048;

      QList<QString> stack;
      void putLevel();

   public:
      Xml(QIODevice*);
      Xml();

      void header();

      void stag(const QString&);
      void etag();

      void tagE(const QString&);
      void tagE(const char* format, ...);
      void ntag(const char* name);
      void netag(const char* name);

      void prop(const Prop& p)  { tag(p.name, p.data); }
      void prop(QList<Prop> pl) { foreach(Prop p, pl) prop(p); }

      void tag(const QString& name, QVariant data);
      void tag(const char* name, const char* s)    { tag(name, QVariant(s)); }
      void tag(const char* name, const QString& s) { tag(name, QVariant(s)); }
      void tag(const char* name, const QWidget*);

      void writeHtml(const QString& s);
      void dump(int len, const unsigned char* p);

      static QString xmlString(const QString&);
      static void htmlToString(QDomElement, int level, QString*);
      static QString htmlToString(QDomElement);
      };

extern QString docName;

#endif

