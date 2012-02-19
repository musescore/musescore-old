//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "libmscore/score.h"
#include "libmscore/hairpin.h"
#include "mtest.h"
#include "testutils.h"

extern Score* score;

//---------------------------------------------------------
//   testHairpin
//---------------------------------------------------------

bool testHairpin(MTest*)
      {
      bool passed = true;

      Hairpin* hp = new Hairpin(score);

   // subtype
      printf("  -subtype\n");
      hp->setSubtype(1);
      Hairpin* hp2 = static_cast<Hairpin*>(writeReadElement(hp));
      TEST(hp2->subtype() == 1);
      delete hp2;

      hp->setSubtype(0);
      hp2 = static_cast<Hairpin*>(writeReadElement(hp));
      TEST(hp2->subtype() == 0);
      delete hp2;

      return passed;
      }

