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

#include "stem.h"
#include "staff.h"
#include "chord.h"
#include "score.h"
#include "stafftype.h"
#include "hook.h"
#include "tremolo.h"

// TEMPORARY HACK!!
#include "sym.h"
// END OF HACK

//---------------------------------------------------------
//   Stem
//    Notenhals
//---------------------------------------------------------

Stem::Stem(Score* s)
   : Element(s)
      {
      _len     = 0.0;
      _userLen = 0.0;
      setFlags(ELEMENT_SELECTABLE);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Stem::layout()
      {
      qreal lw = point(score()->styleS(ST_stemWidth));
      qreal l  = stemLen();
      setbbox(QRectF(-lw * .5, 0, lw, l).normalized());
      }

//---------------------------------------------------------
//   setLen
//---------------------------------------------------------

void Stem::setLen(qreal v)
      {
      _len = v;
      layout();
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Stem::spatiumChanged(qreal oldValue, qreal newValue)
      {
      _userLen = (_userLen / oldValue) * newValue;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Stem::draw(QPainter* painter) const
      {
      bool useTab = false;
      Staff* st = staff();
      if (st && st->useTablature()) {     // stems used in palette do not have a staff
            if (st->staffType()->slashStyle())
                  return;
            useTab = true;
            }
      qreal lw = point(score()->styleS(ST_stemWidth));
      painter->setPen(QPen(curColor(), lw, Qt::SolidLine, Qt::RoundCap));
      painter->drawLine(QLineF(0.0, 0.0, 0.0, stemLen()));

      // NOT THE BEST PLACE FOR THIS?
      // with tablatures, dots are not drawn near 'notes', but near stems
      if (useTab) {
            int nDots = chord()->dots();
            if (nDots > 0)
                  symbols[score()->symIdx()][dotSym].draw(painter, magS(), QPointF(spatium(), stemLen()), nDots);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Stem::write(Xml& xml) const
      {
      xml.stag("Stem");
      Element::writeProperties(xml);
      if (_userLen != 0.0)
            xml.tag("userLen", _userLen / spatium());
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Stem::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "userLen")
                  _userLen = e.text().toDouble() * spatium();
            else if (e.tagName() == "subtype")        // obsolete
                  ;
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Stem::updateGrips(int* grips, QRectF* grip) const
      {
      *grips   = 1;
      QPointF p(0.0, stemLen());
      grip[0].translate(pagePos() + p);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Stem::editDrag(const EditData& ed)
      {
      _userLen += ed.delta.y();
      Chord* c = static_cast<Chord*>(parent());
      if (c->hook())
            c->hook()->move(0.0, ed.delta.y());
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Stem::toDefault()
      {
      _userLen = 0.0;         // TODO: make undoable
      Element::toDefault();
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Stem::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      if ((e->type() == TREMOLO) && (static_cast<Tremolo*>(e)->subtype() <= TREMOLO_R64)) {
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Stem::drop(const DropData& data)
      {
      Element* e = data.element;
      Chord* ch = chord();
      switch(e->type()) {
            case TREMOLO:
                  e->setParent(ch);
                  score()->setLayout(ch->measure());
                  score()->undoAddElement(e);
                  break;
            default:
                  delete e;
                  break;
            }
      return 0;
      }


