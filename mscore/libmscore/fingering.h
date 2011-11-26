//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __FINGERING_H__
#define __FINGERING_H__

#include "text.h"

class Note;

//---------------------------------------------------------
//   Fingering
//---------------------------------------------------------

class Fingering : public Text {

   public:
      Fingering(Score* s);
      virtual Fingering* clone() const { return new Fingering(*this); }
      virtual ElementType type() const { return FINGERING; }

      Note* note() const { return (Note*)parent(); }

      virtual void layout();
      virtual void write(Xml&) const;
      virtual void read(QDomElement);
      virtual void toDefault();
      virtual QString subtypeName() const { return QString(); }
      };

#endif

