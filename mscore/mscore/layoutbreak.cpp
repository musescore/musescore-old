//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "layoutbreak.h"
#include "preferences.h"
#include "score.h"
#include "scoreview.h"
#include "painter.h"

//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

LayoutBreak::LayoutBreak(Score* score)
   : Element(score)
      {
      _pause = score->styleD(ST_SectionPause);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void LayoutBreak::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      if (score()->styleD(ST_SectionPause) != _pause)
            xml.tag("pause", _pause);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void LayoutBreak::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "pause")
                  _pause = e.text().toDouble();
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void LayoutBreak::draw(Painter* painter) const
      {
      QPainter& p = *painter->painter();
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

void LayoutBreak::layout()
      {
      double _spatium = spatium();
      path      = QPainterPath();
      lw        = _spatium * 0.3;
      double h  = _spatium * 4;
      double w  = _spatium * 2.5;
      double w1 = w * .6;

      switch(subtype()) {
            case LAYOUT_BREAK_LINE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);

                  path.moveTo(w * .8, w * .7);
                  path.lineTo(w * .8, w);
                  path.lineTo(w * .2, w);

                  path.moveTo(w * .4, w * .8);
                  path.lineTo(w * .2, w);
                  path.lineTo(w * .4, w * 1.2);
                  break;

            case LAYOUT_BREAK_PAGE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h-w1);
                  path.lineTo(w1, h-w1);
                  path.lineTo(w1, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  path.moveTo(w, h-w1);
                  path.lineTo(w1, h);
                  break;

            case LAYOUT_BREAK_SECTION:
                  path.lineTo(w, 0.0);
                  path.lineTo(w,  h);
                  path.lineTo(0.0,  h);
                  path.moveTo(w-_spatium * .8,  0.0);
                  path.lineTo(w-_spatium * .8,  h);
                  break;

            default:
                  printf("unknown layout break symbol\n");
                  break;
            }
      QRectF bb(0, 0, w, h);
      bb.adjust(-lw, -lw, lw, lw);
      setbbox(bb);

      if (parent()) {
            setPos(-_spatium - w + parent()->width(), -2 * _spatium - h);
            }
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void LayoutBreak::setSubtype(const QString& s)
      {
      if (s == "line")
            setSubtype(LAYOUT_BREAK_LINE);
      else if (s == "page")
            setSubtype(LAYOUT_BREAK_PAGE);
      else
            setSubtype(LAYOUT_BREAK_SECTION);
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString LayoutBreak::subtypeName() const
      {
      switch(subtype()) {
            case LAYOUT_BREAK_LINE:
                  return "line";
            case LAYOUT_BREAK_PAGE:
                  return "page";
            case LAYOUT_BREAK_SECTION:
                  return "section";
            default:
                  return "??";
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool LayoutBreak::acceptDrop(ScoreView*, const QPointF&, int type, int st) const
      {
      if (type == LAYOUT_BREAK && st != subtype())
            return true;
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* LayoutBreak::drop(ScoreView*, const QPointF& /*p1*/, const QPointF& /*p2*/, Element* e)
      {
      score()->undoChangeElement(this, e);
      return e;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool LayoutBreak::genPropertyMenu(QMenu* popup) const
      {
      if (subtype() == LAYOUT_BREAK_SECTION) {
            QAction* a;
            a = popup->addAction(tr("Section Break Properties..."));
            a->setData("props");
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void LayoutBreak::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (subtype() != LAYOUT_BREAK_SECTION) {
            Element::propertyAction(viewer, s);
            return;
            }
      if (s == "props") {
            SectionBreakProperties sbp(this, 0);
            if (sbp.exec()) {
                  if (pause() != sbp.pause()) {
                        LayoutBreak* nlb = new LayoutBreak(*this);
                        nlb->setParent(parent());
                        nlb->setPause(sbp.pause());
                        score()->undoChangeElement(this, nlb);
                        }
                  }
            }
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   SectionBreakProperties
//---------------------------------------------------------

SectionBreakProperties::SectionBreakProperties(LayoutBreak* lb, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      _pause->setValue(lb->pause());
      }

//---------------------------------------------------------
//   pause
//---------------------------------------------------------

double SectionBreakProperties::pause() const
      {
      return _pause->value();
      }

