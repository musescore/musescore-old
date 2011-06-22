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
#include "al/xml.h"
#include "al/fraction.h"

using AL::Prop;
using AL::docName;
using AL::readPoint;
using AL::readRectF;
using AL::readSize;
using AL::readColor;
using AL::domError;
using AL::domNotImplemented;

//---------------------------------------------------------
//   Xml
//---------------------------------------------------------

class Xml : public AL::Xml {

   public:
      int curTick;            // used to optimize output
      int curTrack;
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

      void sTag(const char* name, Spatium sp) { AL::Xml::tag(name, QVariant(sp.val())); }
      void pTag(const char* name, Placement);
      void fTag(const char* name, const Fraction&);
      void valueTypeTag(const char* name, ValueType t);
      };

extern Placement readPlacement(QDomElement);
extern ValueType readValueType(QDomElement);
extern Fraction  readFraction(QDomElement);
#endif

