//=============================================================================
//  MusE
//  Linux Music Score Editor
//  $Id: xml.h,v 1.15 2006/04/05 08:15:12 wschweer Exp $
//
//  Copyright (C) 2004-2007 Werner Schweer and others
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
      int curTick;      // used to optimize output

      Xml(QIODevice*);
      Xml();

      void header();

      void stag(const QString&);
      void etag();

      void tagE(const QString&);
      void tagE(const char* format, ...);
      void ntag(const char* name);
      void netag(const char* name);

      void prop(const Prop& p) { tag(p.name, p.data); }
      void prop(QList<Prop> pl) { foreach(Prop p, pl) prop(p); }

      void tag(const QString& name, QVariant data);
//      void tag(const char* name, double v)         { tag(name, QVariant(v)); }
//      void tag(const char* name, qreal v)          { tag(name, QVariant(double(v))); }
      void tag(const char* name, const char* s)    { tag(name, QVariant(s)); }
      void tag(const char* name, const QString& s) { tag(name, QVariant(s)); }
      void tag(const char* name, const QString&, const QString&);
      void tag(const char* name, const QWidget*);

      void dump(int len, const unsigned char* p);

      static QString xmlString(const QString&);
      };

extern QPointF readPoint(QDomElement);
extern QSizeF readSize(QDomElement);
extern void domError(QDomElement node);
extern void domNotImplemented(QDomElement node);
#endif

