//=============================================================================
//  MuseScore
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

#include "hairpin.h"
#include "style.h"
#include "xml.h"
#include "utils.h"
#include "preferences.h"
#include "score.h"
#include "measure.h"
#include "segment.h"
#include "system.h"
#include "undo.h"
#include "staff.h"
#include "painter.h"

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void HairpinSegment::draw(Painter* painter) const
      {
      QPainter& p = *painter->painter();
      double h1 = point(score()->styleS(ST_hairpinHeight)) * .5;
      double h2 = point(score()->styleS(ST_hairpinContHeight)) * .5;

      QPen pen(p.pen());
      pen.setWidthF(point(score()->styleS(ST_hairpinWidth)));
      p.setPen(pen);

      qreal x = pos2().x();
      qreal y = pos2().y();

      if (hairpin()->subtype() == 0) {
            switch(spannerSegmentType()) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_BEGIN:
                        p.drawLine(QLineF(.0, .0, x, y + h1));
                        p.drawLine(QLineF(.0, .0, x, y - h1));
                        break;
                  case SEGMENT_MIDDLE:
                  case SEGMENT_END:
                        p.drawLine(QLineF(.0,  h2, x, y + h1));
                        p.drawLine(QLineF(.0, -h2, x, y - h1));
                        break;
                  }
            }
      else {
            switch(spannerSegmentType()) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_END:
                        p.drawLine(QLineF(.0,  h1, x, y));
                        p.drawLine(QLineF(.0, -h1, x, y));
                        break;
                  case SEGMENT_BEGIN:
                  case SEGMENT_MIDDLE:
                        p.drawLine(QLineF(.0,  h1, x, y + h2));
                        p.drawLine(QLineF(.0, -h1, x, y - h2));
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF HairpinSegment::bbox() const
      {
      double h = point(score()->styleS(ST_hairpinHeight));
      QRectF r(0.0, -h * .5, pos2().x(), h);
      double w = point(score()->styleS(ST_hairpinWidth));
      r.adjust(-w*.5, -w*.5, w, w);
      return r;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool HairpinSegment::genPropertyMenu(QMenu* popup) const
      {
      QAction* a = popup->addSeparator();
      a->setText(tr("Dynamics"));
      if (visible())
            a = popup->addAction(tr("Set Invisible"));
      else
            a = popup->addAction(tr("Set Visible"));
      a->setData("invisible");

      a = popup->addAction(tr("MIDI Properties..."));
      a->setData("dynamics");

      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void HairpinSegment::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "dynamics") {
            Hairpin* hp = hairpin();
            HairpinProperties dp(hp);
            int rv = dp.exec();

            int vo = dp.changeVelo();
            DynamicType dt = dp.dynamicType();
            if (rv && ((vo != hp->veloChange()) || (dt != hp->dynType()))) {
                  score()->undo()->push(new ChangeHairpin(hp, vo, dt));
                  }
            }
      else
            Element::propertyAction(viewer, s);
      }


//---------------------------------------------------------
//   Hairpin
//---------------------------------------------------------

Hairpin::Hairpin(Score* s)
   : SLine(s)
      {
      setLen(spatium() * 7);   // for use in palettes
      _veloChange = 10;
      _dynType    = DYNAMIC_PART;
      _yoffset    = 8.0;
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick() to _tick2
//---------------------------------------------------------

void Hairpin::layout()
      {
      setPos(0.0, spatium() * _yoffset);
      SLine::layout();
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Hairpin::createLineSegment()
      {
      return new HairpinSegment(score());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Hairpin::write(Xml& xml) const
      {
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
      xml.tag("veloChange", _veloChange);
      if (_dynType != DYNAMIC_PART)
            xml.tag("dynType", _dynType);
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Hairpin::read(QDomElement e)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();
      setId(e.attribute("id", "-1").toInt());
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "veloChange")
                  _veloChange = val.toInt();
            else if (e.tagName() == "dynType")
                  _dynType = DynamicType(e.text().toInt());
            else if (!SLine::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   HairpinProperties
//---------------------------------------------------------

HairpinProperties::HairpinProperties(Hairpin* h, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      hairpin = h;
      veloChange->setValue(hairpin->veloChange());
      int tick1 = static_cast<Segment*>(hairpin->startElement())->tick();
      int velo = hairpin->staff()->velocities().velo(tick1);
      beginVelocity->setNum(velo);
      }

//---------------------------------------------------------
//   dynamicType
//---------------------------------------------------------

DynamicType HairpinProperties::dynamicType() const
      {
      if (staffButton->isChecked())
            return DYNAMIC_STAFF;
      if (systemButton->isChecked())
            return DYNAMIC_SYSTEM;
      // default:
      return DYNAMIC_PART;
      }

