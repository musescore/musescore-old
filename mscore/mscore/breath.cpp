//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "breath.h"
#include "sym.h"
#include "system.h"
#include "segment.h"
#include "measure.h"

int Breath::symList[Breath::breathSymbols] = {
      rcommaSym, lcommaSym, rcommaSym, lcommaSym
      };

//---------------------------------------------------------
//   Breath
//---------------------------------------------------------

Breath::Breath(Score* s)
  : Element(s)
      {
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Breath::layout()
      {
      _bbox = symbols[symList[subtype()]].bbox(mag());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Breath::write(Xml& xml) const
      {
      xml.stag("Breath");
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Breath::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Breath::draw(QPainter& p) const
      {
      symbols[symList[subtype()]].draw(p, mag());
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

void Breath::space(double& min, double& extra) const
      {
      min   = spatium() * 1.5;
      extra = 0.0;
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF Breath::canvasPos() const
      {
      if (parent() == 0)
            return pos();
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = segment()->measure()->system();
      double yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }


