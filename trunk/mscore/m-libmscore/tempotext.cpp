//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: tempotext.cpp 3311 2010-07-24 18:09:23Z wschweer $
//
//  Copyright (C) 2008 Werner Schweer and others
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

#include "score.h"
#include "tempotext.h"
#include "m-al/tempo.h"
#include "system.h"
#include "measure.h"
#include "m-al/xml.h"

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

TempoText::TempoText(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_TEMPO);
      setTextStyle(TEXT_STYLE_TEMPO);
      _tempo = 2.0;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TempoText::read(XmlReader* r)
      {
      while (r->readElement()) {
            if (r->readReal("tempo", &_tempo))
                  ;
            else if (!Text::readProperties(r))
                  r->unknown();
            }
      }

