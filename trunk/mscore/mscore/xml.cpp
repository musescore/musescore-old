//=============================================================================
//  MusE
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include "xml.h"
#include "globals.h"
#include "config.h"

//---------------------------------------------------------
//   Xml
//---------------------------------------------------------

Xml::Xml()
   : AL::Xml()
      {
      curTick       = 0;
      curTrack      = -1;
      trackDiff     = 0;
      noSlurs       = false;
      clipboardmode = false;
      tupletId      = 1;
      beamId        = 1;
      }

Xml::Xml(QIODevice* device)
   : AL::Xml(device)
      {
      curTick       = 0;
      curTrack      = -1;
      trackDiff     = 0;
      noSlurs       = false;
      clipboardmode = false;
      tupletId      = 1;
      beamId        = 1;
      }

//---------------------------------------------------------
//   pTag
//---------------------------------------------------------

void Xml::pTag(const char* name, Placement place)
      {
      const char* tags[] = {
            "auto", "above", "below", "left"
            };
      tag(name, tags[int(place)]);
      }

//---------------------------------------------------------
//   readPlacement
//---------------------------------------------------------

Placement readPlacement(QDomElement e)
      {
      QString s(e.text());
      if (s == "auto")
            return PLACE_AUTO;
      if (s == "above")
            return PLACE_ABOVE;
      if (s == "below")
            return PLACE_BELOW;
      if (s == "left")
            return PLACE_LEFT;
      printf("unknown placement value <%s>\n", qPrintable(s));
      return PLACE_AUTO;
      }
