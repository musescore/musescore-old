//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
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

#include "score.h"
#include "stafftext.h"

//---------------------------------------------------------
//   StaffText
//---------------------------------------------------------

StaffText::StaffText(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_STAFF);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffText::write(Xml& xml) const
      {
      xml.stag("StaffText");
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffText::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (!Text::readProperties(e))
                  domError(e);
            }
      cursorPos = 0;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool StaffText::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void StaffText::propertyAction(const QString& s)
      {
      if (s == "props") {
            StaffTextProperties rp(this);
            rp.exec();
            }
      else
            Element::propertyAction(s);
      }

//---------------------------------------------------------
//   StaffTextProperties
//---------------------------------------------------------

StaffTextProperties::StaffTextProperties(StaffText* st, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      staffText = st;
      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void StaffTextProperties::saveValues()
      {
#if 0
      Score* score    = tempoText->score();
      double newTempo = tempo->value() / 60.0;
      if (newTempo == tempoText->tempo())
            return;
      int tick        = tempoText->tick();
      TempoList* tl   = score->tempomap;

      iTEvent o = tl->find(tick);
      if (o == tl->end()) {
            printf("TempoProperties: cannot find tempo at %d\n", tick);
            return;
            }
      TEvent n(newTempo);
      score->undoChangeTempo(tick, o->second, n);
      TempoText* ntt = new TempoText(*tempoText);
      ntt->setTempo(newTempo);
      score->undoChangeElement(tempoText, ntt);
#endif
      }

