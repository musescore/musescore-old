//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __HOOK_H__
#define __HOOK_H__

#include "symbol.h"

//---------------------------------------------------------
//   Hook
//---------------------------------------------------------

class Hook : public Symbol {
      int _subtype;

   public:
      Hook(Score*);
      virtual Hook* clone() const      { return new Hook(*this); }
      virtual ElementType type() const { return HOOK; }
      void setSubtype(int v);
      int subtype() const { return _subtype; }
      virtual void layout();
      };

#endif

