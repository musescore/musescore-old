//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: rest.cpp,v 1.5 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include "rest.h"
#include "score.h"
#include "xml.h"
#include "style.h"
#include "staff.h"

//---------------------------------------------------------
//   Rest
//---------------------------------------------------------

Rest::Rest(Score* s)
  : ChordRest(s)
      {
      _beamMode = BEAM_NO;
      _move     = 0;
      }

Rest::Rest(Score* s, int tick, int len)
  : ChordRest(s)
      {
      setTick(tick);
      setTickLen(len);
      _beamMode = BEAM_NO;
      _move     = 0;
      }

//---------------------------------------------------------
//   Rest::draw
//---------------------------------------------------------

void Rest::draw1(Painter& p)
      {
      symbols[_sym].draw(p);
      for (ciAttribute l = attributes.begin(); l != attributes.end(); ++l)
            (*l)->draw(p);
      }

//---------------------------------------------------------
//   setSym
//---------------------------------------------------------

void Rest::setSym(int s)
      {
      _sym = s;
      bboxUpdate();
      }

//---------------------------------------------------------
//   setLen
//---------------------------------------------------------

void Rest::setTickLen(int i)
      {
      Element::setTickLen(i);
      // if rest spans measure:
#if 0 // TODO: whole rest symbol
      Measure* m = measure();
      int ticksMeasure = m->score()->sigmap->ticksMeasure(tick());
      if (ticksMeasure == tickLen()) {
            setSym(wholerestSym);
            return;
            }
#endif
      if (i <= division/32)
            setSym(hundredtwentyeighthrestSym);
      else if (i <= division/16)
            setSym(sixtyfourthrestSym);
      else if (i <= division/8)
            setSym(thirtysecondrestSym);
      else if (i <= division/4)
            setSym(sixteenthrestSym);
      else if (i <= division/2)
            setSym(eighthrestSym);
      else if (i <= division)
            setSym(quartrestSym);
      else if (i <= division*2)
            setSym(halfrestSym);
      else if (i <= division*4)
            setSym(wholerestSym);
      else
            setSym(wholerestSym);
            // printf("Rest::setTickLen: unknown %d %d\n", i, division*4);
      }

void Rest::dump() const
      {
      printf("Rest tick %d  len %d\n", tick(), tickLen());
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF Rest::drag(const QPointF& s)
      {
      QRectF r(abbox());
      setUserOff(QPointF(0, s.y()) / _spatium);  // only move vertical
      return abbox() | r;
      }

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void Rest::endDrag()
      {
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

void Rest::space(double& min, double& extra) const
      {
      min   = width();
      extra = 0.0;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Rest::acceptDrop(const QPointF&, int type, int subtype) const
      {
      return (type == ATTRIBUTE && (subtype == UfermataSym || subtype == DfermataSym));
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

void Rest::drop(const QPointF&, int t, int st)
      {
      if (!(t == ATTRIBUTE && (st == UfermataSym || st == DfermataSym)))
            return;
      NoteAttribute* atr = new NoteAttribute(score());
      atr->setSubtype(st);
      score()->addAttribute(this, atr);
      }

//---------------------------------------------------------
//   write Rest
//---------------------------------------------------------

void Rest::write(Xml& xml) const
      {
      xml.stag("Rest");
      ChordRest::writeProperties(xml);
      if (_move)
            xml.tag("move", _move);
      xml.etag("Rest");
      }

//---------------------------------------------------------
//   Rest::read
//---------------------------------------------------------

void Rest::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "len")
                  setTickLen(i);
            else if (tag == "move")
                  _move = i;
            else if (ChordRest::readProperties(node))
                  ;
            else
                  domError(node);
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Rest::add(Element* e)
      {
      if (e->type() != ATTRIBUTE)
            return;
      e->setVoice(voice());
      e->setParent(this);
      e->setStaff(staff());
      attributes.push_back((NoteAttribute*)e);
      bboxUpdate();
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Rest::remove(Element* e)
      {
      if (e->type() != ATTRIBUTE)
            return;
      iAttribute l = attributes.find((NoteAttribute*)e);
      if (l == attributes.end())
            printf("Rest::remove(): attribute not found\n");
      else
            attributes.erase(l);
      bboxUpdate();
      }

//---------------------------------------------------------
//   findSelectableElement
//    p is Measure relative
//---------------------------------------------------------

Element* Rest::findSelectableElement(QPointF p) const
      {
      p -= pos();
      for (ciAttribute ia = attributes.begin(); ia != attributes.end(); ++ia) {
            if ((*ia)->contains(p))
                  return *ia;
            }
      return 0;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Rest::layout()
      {
      layoutAttributes();
      bboxUpdate();
      }

//---------------------------------------------------------
//   bboxUpdate
//---------------------------------------------------------

void Rest::bboxUpdate()
      {
      setbbox(symbols[_sym].bbox());
      for (ciAttribute i = attributes.begin(); i != attributes.end(); ++i)
            orBbox((*i)->bbox().translated((*i)->pos()));
      }

//---------------------------------------------------------
//   centerX
//---------------------------------------------------------

qreal Rest::centerX() const
      {
      return symbols[_sym].bbox().width()/2.0;
      }

//---------------------------------------------------------
//   upPos
//---------------------------------------------------------

qreal Rest::upPos() const
      {
      return symbols[_sym].bbox().y();
      }

//---------------------------------------------------------
//   downPos
//---------------------------------------------------------

qreal Rest::downPos() const
      {
      return symbols[_sym].bbox().y() + symbols[_sym].bbox().height();
      }


