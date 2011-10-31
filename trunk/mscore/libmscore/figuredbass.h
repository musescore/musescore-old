//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __FIGUREDBASS_H__
#define __FIGUREDBASS_H__

#include "lyrics.h"

//---------------------------------------------------------
//   FiguredBass
//---------------------------------------------------------

class FiguredBass : public Lyrics {

   public:
      FiguredBass(Score*);
      FiguredBass(const FiguredBass&);
      ~FiguredBass();
      virtual FiguredBass* clone() const { return new FiguredBass(*this); }
      virtual ElementType type() const   { return FIGURED_BASS; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual void layout();
      };

#endif

