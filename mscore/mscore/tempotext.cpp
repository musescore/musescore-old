//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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
#include "tempotext.h"
#include "al/tempo.h"
#include "system.h"

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

TempoText::TempoText(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_TEMPO);
      setTextStyle(TEXT_STYLE_TEMPO);
      _tempo = 2.0;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TempoText::write(Xml& xml) const
      {
      xml.stag("Tempo");
      xml.tag("tempo", _tempo);
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TempoText::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "tempo"){
                  double tpo = e.text().toDouble();
                  setTempo(tpo);
                  //Don't need to add to tempo since tempo map is read at the beginning of the file.
            }
            else if (!Text::readProperties(e))
                  domError(e);
            }
      cursorPos = 0;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool TempoText::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Tempo Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void TempoText::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "props") {
            TempoProperties rp(this);
            rp.exec();
            }
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   TempoProperties
//---------------------------------------------------------

TempoProperties::TempoProperties(TempoText* tt, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      tempoText = tt;
      tempo->setValue(tempoText->tempo() * 60.0);
      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void TempoProperties::saveValues()
      {
      Score* score    = tempoText->score();
      double newTempo = tempo->value() / 60.0;
      if (newTempo == tempoText->tempo())
            return;
      int tick        = tempoText->tick();
      AL::TempoMap* tl   = score->tempomap();

      AL::iTEvent o = tl->find(tick);
      if (o == tl->end()) {
            printf("TempoProperties: cannot find tempo at %d\n", tick);
            return;
            }
      AL::TEvent n(newTempo);
      score->undoChangeTempo(tick, o->second, n);
      TempoText* ntt = new TempoText(*tempoText);
      ntt->setTempo(newTempo);
      score->undoChangeElement(tempoText, ntt);
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

#if 0
QLineF TempoText::dragAnchor() const
      {
      Measure* m      = measure();
      System*  system = m->system();
      double yp       = system->y(); // system->staff(staffIdx())->y() + system->y();
      double xp       = m->tick2pos(tick()) + m->canvasPos().x();
      QPointF cp      = canvasPos()  + QPointF(0.0, baseLine());
      return QLineF(cp, QPointF(xp, yp));
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TempoText::layout(ScoreLayout* l)
      {
      Measure* m = measure();
      if (m == 0 || tick() == -1)
            return;
      Text::layout(l);
      double y   = 0;  // track() != -1 ? m->system()->staff(track() / VOICES)->y() : 0.0;
      double x   = m->tick2pos(tick());
      setPos(ipos() + QPointF(x, y));
      }
#endif
