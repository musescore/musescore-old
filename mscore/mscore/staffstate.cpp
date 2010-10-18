//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#include "staffstate.h"
#include "preferences.h"
#include "score.h"
#include "scoreview.h"

//---------------------------------------------------------
//   StaffState
//---------------------------------------------------------

StaffState::StaffState(Score* score)
   : Element(score)
      {
      _reloff.rx() = 100.0;
      setXoff(-1.0);
      setYoff(-2.0);
      setOffsetType(OFFSET_SPATIUM);
      setAlign(ALIGN_RIGHT | ALIGN_BOTTOM);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffState::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffState::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            // QString tag(e.tagName());
            if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffState::draw(QPainter& p, ScoreView*) const
      {
      if (score()->printing())
            return;
      QPen pen;
      if (selected())
            pen.setColor(preferences.selectColor[0]);
      else
            pen.setColor(preferences.layoutBreakColor);

      pen.setWidthF(lw);
      pen.setCapStyle(Qt::RoundCap);
      pen.setJoinStyle(Qt::RoundJoin);
      p.setPen(pen);
      p.setBrush(Qt::NoBrush);
      p.drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffState::layout()
      {
      double _spatium = spatium();
      path      = QPainterPath();
      lw        = _spatium * 0.3;
      double h  = _spatium * 4;
      double w  = _spatium * 2.5;
      double w1 = w * .6;

      switch(subtype()) {
            case STAFF_STATE_INSTRUMENT:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            case STAFF_STATE_TYPE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            case STAFF_STATE_VISIBLE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            case STAFF_STATE_INVISIBLE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            default:
                  printf("unknown layout break symbol\n");
                  break;
            }
      QRectF bb(0, 0, w, h);
      bb.adjust(-lw, -lw, lw, lw);
      setbbox(bb);
      ElementLayout::layout(this);      // alignment & offset
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void StaffState::setSubtype(const QString& s)
      {
      if (s == "instrument")
            setSubtype(STAFF_STATE_INSTRUMENT);
      else if (s == "type")
            setSubtype(STAFF_STATE_TYPE);
      else if (s == "visible")
            setSubtype(STAFF_STATE_VISIBLE);
      else if (s == "invisible")
            setSubtype(STAFF_STATE_INVISIBLE);
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString StaffState::subtypeName() const
      {
      switch(subtype()) {
            case STAFF_STATE_INSTRUMENT:
                  return "instrument";
            case STAFF_STATE_TYPE:
                  return "type";
            case STAFF_STATE_VISIBLE:
                  return "visible";
            case STAFF_STATE_INVISIBLE:
                  return "invisible";
            default:
                  return "??";
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool StaffState::acceptDrop(ScoreView*, const QPointF&, int type, int st) const
      {
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* StaffState::drop(ScoreView*, const QPointF& /*p1*/, const QPointF& /*p2*/, Element* e)
      {
      score()->undoChangeElement(this, e);
      return e;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool StaffState::genPropertyMenu(QMenu* popup) const
      {
#if 0
      if (subtype() == LAYOUT_BREAK_SECTION) {
            QAction* a;
            a = popup->addAction(tr("Section Break Properties..."));
            a->setData("props");
            return true;
            }
#endif
      return false;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void StaffState::propertyAction(ScoreView* viewer, const QString& s)
      {
#if 0
      if (subtype() != LAYOUT_BREAK_SECTION) {
            Element::propertyAction(viewer, s);
            return;
            }
      if (s == "props") {
            SectionBreakProperties sbp(this, 0);
            if (sbp.exec()) {
                  if (pause() != sbp.pause()) {
                        StaffState* nlb = new StaffState(*this);
                        nlb->setParent(parent());
                        nlb->setPause(sbp.pause());
                        score()->undoChangeElement(this, nlb);
                        }
                  }
            }
      else
            Element::propertyAction(viewer, s);
#endif
      }

