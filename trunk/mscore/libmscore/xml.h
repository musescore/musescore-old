//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2004-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __XML_H__
#define __XML_H__

#include "mscore.h"
#include "spatium.h"
#include "fraction.h"

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
      int curTick;            // used to optimize output
      int curTrack;
      int tickDiff;
      int trackDiff;          // saved track is curTrack-trackDiff

      bool clipboardmode;     // used to modify write() behaviour
      bool excerptmode;       // true when writing a part
      bool writeOmr;          // false if writing into *.msc file

      int tupletId;
      int beamId;
      int spannerId;
      int slurId;

      Xml(QIODevice* dev);
      Xml();

      void sTag(const char* name, Spatium sp) { Xml::tag(name, QVariant(sp.val())); }
      void pTag(const char* name, Placement);
      void fTag(const char* name, const Fraction&);
      void valueTypeTag(const char* name, ValueType t);

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
      static void htmlToString(const QDomElement&, int level, QString*);
      static QString htmlToString(const QDomElement&);
      };

extern Placement readPlacement(const QDomElement&);
extern ValueType readValueType(const QDomElement&);
extern Fraction  readFraction(const QDomElement&);
extern QString docName;
extern QPointF readPoint(const QDomElement&);
extern QSizeF readSize(const QDomElement&);
extern QRectF readRectF(const QDomElement&);
extern QColor readColor(const QDomElement&);
extern void domError(const QDomElement&);
extern void domNotImplemented(const QDomElement&);

#endif

