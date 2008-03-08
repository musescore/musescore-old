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

#include <QtCore/QTextStream>
#include <QtCore/QVariant>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtXml/QDomElement>

//---------------------------------------------------------
//   Xml
//---------------------------------------------------------

class Xml : public QTextStream {
      static const int BS = 2048;

      QList<QString> stack;

   public:
      Xml(QIODevice*);
      Xml();

      void putLevel();
      void header();

      void stag(const QString&);
      void etag();

      void tagE(const QString&);
      void tagE(const char* format, ...);
      void ntag(const char* name);
      void netag(const char* name);

      void tag(const QString& name, QVariant data);
      void tag(const char* name, const char* s)    { tag(name, QVariant(s)); }
      void tag(const char* name, const QString& s) { tag(name, QVariant(s)); }

      void dump(int len, const unsigned char* p);

      static QString xmlString(QString);
      };

extern QString docName;
extern void domError(QDomElement node);
#endif

