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
#include "score.h"
#include "scoreview.h"
#include "painter.h"
#include "mscore.h"

//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

LayoutBreak::LayoutBreak(Score* score)
   : Element(score)
      {
      _pause = score->styleD(ST_SectionPause);
      _startWithLongNames = true;
      _startWithMeasureOne = true;
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
      if (!_startWithLongNames)
            xml.tag("startWithLongNames", _startWithLongNames);
      if (!_startWithMeasureOne)
            xml.tag("startWithMeasureOne", _startWithMeasureOne);
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
            else if (tag == "startWithLongNames")
                  _startWithLongNames = e.text().toInt();
            else if (tag == "startWithMeasureOne")
                  _startWithMeasureOne = e.text().toInt();
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void LayoutBreak::draw(Painter* painter) const
      {
      if (score()->printing() || !score()->showUnprintable())
            return;
      if (selected())
            painter->setPenColor(MScore::selectColor[0]);
      else
            painter->setPenColor(MScore::layoutBreakColor);

      painter->setLineWidth(lw);
      painter->setCapStyle(Qt::RoundCap);
      painter->setJoinStyle(Qt::RoundJoin);
      painter->setNoBrush(true);
      painter->drawPath(path);
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

bool LayoutBreak::acceptDrop(MuseScoreView*, const QPointF&, int type, int st) const
      {
      if (type == LAYOUT_BREAK && st != subtype())
            return true;
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* LayoutBreak::drop(const DropData& data)
      {
      Element* e = data.element;
      score()->undoChangeElement(this, e);
      return e;
      }
