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

#include "breath.h"
#include "sym.h"
#include "system.h"
#include "segment.h"
#include "measure.h"
#include "score.h"
#include "painter.h"

int Breath::symList[Breath::breathSymbols] = {
      rcommaSym,
      lcommaSym,
      caesuraCurvedSym,
      caesuraStraight
      };

//---------------------------------------------------------
//   Breath
//---------------------------------------------------------

Breath::Breath(Score* s)
  : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Breath::layout()
      {
      setbbox(symbols[score()->symIdx()][symList[subtype()]].bbox(magS()));
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

void Breath::draw(Painter* p) const
      {
      symbols[score()->symIdx()][symList[subtype()]].draw(p, magS());
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space Breath::space() const
      {
      return Space(0.0, spatium() * 1.5);
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF Breath::canvasPos() const
      {
      if (parent() == 0)
            return pos();
      qreal xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = segment()->measure()->system();
      qreal yp = y();
      if (system)
            yp += system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }


