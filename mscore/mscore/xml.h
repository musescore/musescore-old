//=============================================================================
//  MusE
//  Linux Music Score Editor
//  $Id: xml.h,v 1.15 2006/04/05 08:15:12 wschweer Exp $
//
//  Copyright (C) 2004-2006 Werner Schweer (ws@seh.de)
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

      void tagE(const char* format, ...);
      void ntag(const char* name);
      void netag(const char* name);

      void tag(const char* name, int);
      void tag(const char* name, QChar);
      void tag(const char* name, double);
      void tag(const char* name, float);
      void tag(const char* name, const char*);
      void tag(const char* name, const QString&);
      void tag(const char* name, const char* attribute, const QString&);
      void tag(const char* name, const QColor&);
      void tag(const char* name, const QWidget*);
      void tag(const char* name, const QRect&);
      void tag(const char* name, const QPointF&);

      static QString xmlString(const QString&);
      };

extern QPointF readPoint(QDomNode);
extern void domError(QDomNode node);
#endif

