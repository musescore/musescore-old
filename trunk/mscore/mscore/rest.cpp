//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: rest.cpp,v 1.5 2006/03/28 14:58:58 wschweer Exp $
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

#include "rest.h"
#include "score.h"
#include "xml.h"
#include "style.h"
#include "staff.h"
#include "viewer.h"
#include "restproperties.h"
#include "utils.h"
#include "tuplet.h"
#include "sym.h"
#include "icons.h"

//---------------------------------------------------------
//   Rest
//---------------------------------------------------------

Rest::Rest(Score* s)
  : ChordRest(s)
      {
      _beamMode  = BEAM_NO;
      _staffMove = 0;
      _dots      = 0;
      dotline    = -1;
      setOffsetType(OFFSET_SPATIUM);
      _sym       = quartrestSym;
      }

Rest::Rest(Score* s, int tick, int len)
  : ChordRest(s)
      {
      _beamMode  = BEAM_NO;
      _staffMove = 0;
      _dots      = 0;
      dotline    = -1;
      setOffsetType(OFFSET_SPATIUM);
      _sym       = quartrestSym;
      setTick(tick);
      setTickLen(len);
      }

//---------------------------------------------------------
//   Rest::draw
//---------------------------------------------------------

void Rest::draw(QPainter& p) const
      {
      symbols[_sym].draw(p, mag());
      if (_dots) {
            double y = dotline * _spatium * .5;
            for (int i = 1; i <= _dots; ++i) {
                  double x = symbols[_sym].width(mag())
                             + point(score()->style()->dotNoteDistance) * i;
                  symbols[dotSym].draw(p, mag(), x, y);
                  }
            }
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

bool Rest::acceptDrop(Viewer* viewer, const QPointF&, int type, int subtype) const
      {
      if (
         (type == ICON && subtype == ICON_SBEAM)
         || (type == ICON && subtype == ICON_MBEAM)
         || (type == ICON && subtype == ICON_NBEAM)
         || (type == ICON && subtype == ICON_BEAM32)
         || (type == ICON && subtype == ICON_AUTOBEAM)
         || (type == ATTRIBUTE && subtype == UfermataSym)
         || (type == ATTRIBUTE && subtype == DfermataSym)
         ) {
            viewer->setDropTarget(this);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Rest::drop(const QPointF&, const QPointF&, Element* e)
      {
      switch (e->type()) {
            case ATTRIBUTE:
                  if (e->subtype() == UfermataSym || e->subtype() == DfermataSym)
                        score()->addAttribute(this, (NoteAttribute*)e);
                  return 0;
            case ICON:
                  {
                  printf("change beam mode\n");
                  switch(e->subtype()) {
                        case ICON_SBEAM:
                              score()->undoChangeBeamMode(this, BEAM_BEGIN);
                              break;
                        case ICON_MBEAM:
                              score()->undoChangeBeamMode(this, BEAM_MID);
                              break;
                        case ICON_NBEAM:
                              score()->undoChangeBeamMode(this, BEAM_NO);
                              break;
                        case ICON_BEAM32:
                              score()->undoChangeBeamMode(this, BEAM_BEGIN32);
                              break;
                        case ICON_AUTOBEAM:
                              score()->undoChangeBeamMode(this, BEAM_AUTO);
                              break;
                        }
                  }
                  delete e;
                  break;
            default:
                  printf("cannot drop %s\n", e->name());
                  return 0;
            }
      return 0;
      }

//---------------------------------------------------------
//   write Rest
//---------------------------------------------------------

void Rest::write(Xml& xml) const
      {
      xml.stag("Rest");
      ChordRest::writeProperties(xml);
      if (_staffMove)
            xml.tag("move", _staffMove);
      xml.etag();
      }

//---------------------------------------------------------
//   Rest::read
//---------------------------------------------------------

void Rest::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "len")       // obsolete ?!
                  setTickLen(i);
            else if (tag == "move")
                  _staffMove = i;
            else if (!ChordRest::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Rest::add(Element* e)
      {
      if (e->type() != ATTRIBUTE)
            return;
      e->setParent(this);
      e->setTrack(track());
      attributes.push_back((NoteAttribute*)e);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Rest::remove(Element* e)
      {
      if (e->type() != ATTRIBUTE)
            return;
      int idx = attributes.indexOf((NoteAttribute*)e);
      if (idx == -1)
            printf("Rest::remove(): attribute not found\n");
      else
            attributes.removeAt(idx);
      }

//---------------------------------------------------------
//   setTuplet
//---------------------------------------------------------

void Rest::setTuplet(Tuplet* t)
      {
      ChordRest::setTuplet(t);
      setSymbol(t->baseLen());
      }

//---------------------------------------------------------
//   setSymbol
//---------------------------------------------------------

void Rest::setSymbol(int i)
      {
      setYoff(2.0 * mag());
      DurationType type;
      headType(i, &type, &_dots);
      switch(type) {
            case D_LONG:
                  _sym = longarestSym;
                  break;
            case D_BREVE:
                  _sym = breverestSym;
                  break;
            case D_MEASURE:
            case D_WHOLE:
                  _sym = wholerestSym;
                  setYoff(1.0 * mag());
                  break;
            case D_HALF:
                  _sym = halfrestSym;
                  break;
            case D_QUARTER:
                  _sym = quartrestSym;
                  break;
            case D_EIGHT:
                  _sym = eighthrestSym;
                  break;
            case D_16TH:
                  _sym = sixteenthrestSym;
                  break;
            case D_32ND:
                  _sym = thirtysecondrestSym;
                  break;
            case D_64TH:
                  _sym = sixtyfourthrestSym;
                  break;
            case D_128TH:
                  _sym = hundredtwentyeighthrestSym;
                  break;
            case D_256TH:
                  _sym = quartrestSym;    // TODO
                  break;
            }
      }

//---------------------------------------------------------
//   setTickLen
//---------------------------------------------------------

void Rest::setTickLen(int i)
      {
      Element::setTickLen(i);
      setSymbol(tuplet() ? tuplet()->baseLen() : tickLen());
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Rest::layout(ScoreLayout* l)
      {
      layoutAttributes(l);
      Element::layout(l);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Rest::bbox() const
      {
      QRectF b = symbols[_sym].bbox(mag());
      for (ciAttribute i = attributes.begin(); i != attributes.end(); ++i)
            b |= (*i)->bbox().translated((*i)->pos());
      return b;
      }

//---------------------------------------------------------
//   centerX
//---------------------------------------------------------

qreal Rest::centerX() const
      {
      return symbols[_sym].width(mag())*.5;
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
      return symbols[_sym].bbox().y() + symbols[_sym].height();
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Rest::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addSeparator();
      a->setText(tr("Rest"));
      a = popup->addAction(tr("Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Rest::propertyAction(const QString& s)
      {
      if (s == "props") {
            RestProperties vp;
            vp.setSmall(small());
            int rv = vp.exec();
            if (rv) {
                  bool val = vp.small();
                  if (val != small())
                        score()->undoChangeChordRestSize(this, val);
                  }
            }
      else
            Element::propertyAction(s);
      }

