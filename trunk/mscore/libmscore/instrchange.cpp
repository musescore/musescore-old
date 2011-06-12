//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: stafftext.cpp 3199 2010-06-19 19:41:16Z wschweer $
//
//  Copyright (C) 2008-2010 Werner Schweer and others
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

#include "instrchange.h"
#include "preferences.h"
#include "score.h"
#include "scoreview.h"
#include "instrtemplate.h"
#include "segment.h"
#include "staff.h"
#include "part.h"
#include "seq.h"
#include "undo.h"

//---------------------------------------------------------
//   InstrumentChange
//---------------------------------------------------------

InstrumentChange::InstrumentChange(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_INSTRUMENT_CHANGE);
      setTextStyle(TEXT_STYLE_INSTRUMENT_CHANGE);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void InstrumentChange::write(Xml& xml) const
      {
      xml.stag("InstrumentChange");
      _instrument.write(xml);
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void InstrumentChange::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "Instrument")
                  _instrument.read(e);
            else if (!Text::readProperties(e))
                  domError(e);
            }
      }

#if 0
//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool InstrumentChange::genPropertyMenu(QMenu* popup) const
      {
      Text::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Change Instrument..."));
      a->setData("sprops");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void InstrumentChange::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "sprops") {
            SelectInstrument si(_instrument, 0);
            if (si.exec()) {
                  const InstrumentTemplate* it = si.instrTemplate();
                  if (it) {
                        _instrument = Instrument::fromTemplate(it);
                        score()->undo()->push(new ChangeInstrument(this, _instrument));
                        }
                  else
                        printf("no template selected?\n");
                  }
            }
      else
            Text::propertyAction(viewer, s);
      }
#endif

