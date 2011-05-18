//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: trill.cpp 3229 2010-06-27 14:55:28Z wschweer $
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

#include "fingering.h"
#include "score.h"
#include "undo.h"

//---------------------------------------------------------
//   Fingering
//---------------------------------------------------------

Fingering::Fingering(Score* s)
  : Text(s)
      {
      setTextStyle(TEXT_STYLE_FINGERING);
      setUseSelectionColor(true);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Fingering::layout()
      {
      Text::layout();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Fingering::write(Xml& xml) const
      {
      xml.stag(QString("%1").arg(name()));
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Fingering::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (!Text::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Fingering::toDefault()
      {
      QPointF o(userOff());
      score()->layoutFingering(this);
      QPointF no(userOff());
      setUserOff(o);
      score()->undo()->push(new ChangeUserOffset(this, no));
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Fingering::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addSeparator();

      QMenu* menuLayer = new QMenu(tr("Layer"));
      for (int i = 0; i < 32; ++i) {
            QString tag = score()->layerTags()[i];
            if (!tag.isEmpty()) {
                  a = menuLayer->addAction(tag);
                  a->setData(QString("layer-%1").arg(i));
                  a->setCheckable(true);
                  a->setChecked(layer() & (1 << i));
                  }
            }
      popup->addMenu(menuLayer);
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Fingering::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s.startsWith("layer-")) {
            int n = s.mid(6).toInt();
            uint mask = 1 << n;
            foreach(Element* e, score()->selection().elements()) {
                  if (e->type() != FINGERING)
                        continue;
                  e->setLayer(mask);
                  }
            }
      else
            Element::propertyAction(viewer, s);
      }


