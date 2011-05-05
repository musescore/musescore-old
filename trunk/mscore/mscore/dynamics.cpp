//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#include "dynamics.h"
#include "xml.h"
#include "score.h"
#include "measure.h"
#include "system.h"
#include "segment.h"
#include "utils.h"
#include "style.h"

//
// see: http://en.wikipedia.org/wiki/File:Dynamic's_Note_Velocity.svg
//
Dyn dynList[] = {
      // dynamics:
      Dyn( -1,  true,  "other-dynamics"),
      Dyn(  1,  false, "pppppp"),
      Dyn(  5,  false, "ppppp"),
      Dyn( 10,  false, "pppp"),
      Dyn( 16,  false, "ppp"),
      Dyn( 33,  false, "pp"),
      Dyn( 49,  false, "p"),
      Dyn( 64,  false, "mp"),
      Dyn( 80,  false, "mf"),
      Dyn( 96,  false, "f"),
      Dyn(112,  false, "ff"),
      Dyn(126,  false, "fff"),
      Dyn(127,  false, "ffff"),
      Dyn(127,  false, "fffff"),
      Dyn(127,  false, "ffffff"),

      // accents:
      Dyn( 0,   true,  "fp"),
      Dyn( 0,   true,  "sf"),
      Dyn( 0,   true,  "sfz"),
      Dyn( 0,   true,  "sff"),
      Dyn( 0,   true,  "sffz"),
      Dyn( 0,   true,  "sfp"),
      Dyn( 0,   true,  "sfpp"),
      Dyn( 0,   true,  "rfz"),
      Dyn( 0,   true,  "rf"),
      Dyn( 0,   true,  "fz"),
      Dyn( 0,   true,  "m"),
      Dyn( 0,   true,  "r"),
      Dyn( 0,   true,  "s"),
      Dyn( 0,   true,  "z"),
      };

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic(Score* s)
   : Text(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _velocity = -1;
      setTextStyle(TEXT_STYLE_DYNAMICS);
      _dynType  = DYNAMIC_PART;
      }

Dynamic::Dynamic(const Dynamic& d)
   : Text(d)
      {
      setSubtype(subtype());
      _velocity = d._velocity;
      _dynType  = d._dynType;
      }

//---------------------------------------------------------
//   setVelocity
//---------------------------------------------------------

void Dynamic::setVelocity(int v)
      {
      _velocity = v;
      }

//---------------------------------------------------------
//   velocity
//---------------------------------------------------------

int Dynamic::velocity() const
      {
      return _velocity <= 0 ? dynList[subtype()].velocity : _velocity;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Dynamic::write(Xml& xml) const
      {
      xml.stag("Dynamic");
      if (_velocity > 0)
            xml.tag("velocity", _velocity);
      if (_dynType != DYNAMIC_PART)
            xml.tag("dynType", _dynType);
      Text::writeProperties(xml, subtype() == 0);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Dynamic::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "velocity")
                  _velocity = e.text().toInt();
            else if (e.tagName() == "dynType")
                  _dynType = DynamicType(e.text().toInt());
            else if (!Text::readProperties(e))
                  domError(e);
            }
      setSubtype(subtype());
      if (score()->mscVersion() < 118) {
            setTextStyle(TEXT_STYLE_DYNAMICS);
            setStyled(true);
            }
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Dynamic::setSubtype(int idx)
      {
      Element::setSubtype(idx);
      if (idx) {
            doc()->clear();
            QTextCursor cursor(doc());
            cursor.movePosition(QTextCursor::Start);
            QTextCharFormat tf = cursor.charFormat();
            const TextStyle& ts = score()->textStyle(TEXT_STYLE_DYNAMICS);
            double size = ts.size();
            double m = size;
            if (ts.sizeIsSpatiumDependent())
                  m *= (score()->spatium() / (SPATIUM20 * DPI));
            m *= mag();

            QFont font("MScore1");
            font.setPointSizeF(m);
            font.setKerning(true);
            font.setStyleStrategy(QFont::NoFontMerging);
            tf.setFont(font);
            tf.setProperty(QTextFormat::FontKerning, true);
            cursor.setBlockCharFormat(tf);
            cursor.insertText(dynList[idx].tag);
            }
      }

//---------------------------------------------------------
//   toSubtype
//---------------------------------------------------------

void Dynamic::setSubtype(const QString& tag)
      {
      int n = sizeof(dynList)/sizeof(*dynList);
      for (int i = 0; i < n; ++i) {
            if (dynList[i].tag == tag) {
                  setSubtype(i);
                  return;
                  }
            }
      Element::setSubtype(0);
      setText(tag);
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString Dynamic::subtypeName() const
      {
      return dynList[subtype()].tag;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Dynamic::startEdit(ScoreView* v, const QPointF& p)
      {
      setSubtype(0);
      Text::startEdit(v, p);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Dynamic::endEdit()
      {
      Text::endEdit();
      }

//---------------------------------------------------------
//   resetType
//---------------------------------------------------------

void Dynamic::resetType()
      {
      QString tag = getText();
      int n = sizeof(dynList)/sizeof(*dynList);
      for (int i = 0; i < n; ++i) {
            if (dynList[i].tag == tag) {
                  setSubtype(i);
                  return;
                  }
            }
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Dynamic::toDefault()
      {
      QString tag = getText();
      int n = sizeof(dynList)/sizeof(*dynList);
      int idx = 0;
      for (int i = 0; i < n; ++i) {
            if (dynList[i].tag == tag) {
                  idx = i;
                  break;
                  }
            }
      Text::toDefault();
      setTextStyle(TEXT_STYLE_DYNAMICS);
      setSubtype(idx);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Dynamic::layout()
      {
      setSubtype(subtype());  // re-apply style
      Text::layout();
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Dynamic::genPropertyMenu(QMenu* popup) const
      {
      QAction* a = popup->addSeparator();
//      a->setText(tr("Dynamics"));
      if (visible())
            a = popup->addAction(tr("Set Invisible"));
      else
            a = popup->addAction(tr("Set Visible"));
      a->setData("invisible");
      a = popup->addAction(tr("MIDI Properties..."));
      a->setData("dynamics");
      a = popup->addAction(tr("Text Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Dynamic::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "props") {
            Dynamic* nText = new Dynamic(*this);
            TextProperties tp(nText, 0);
            int rv = tp.exec();
            if (rv)
                  score()->undoChangeElement(this, nText);
            else
                  delete nText;
            }
      else if (s == "dynamics") {
            int oldVelo    = _velocity;
            DynamicType ot = _dynType;
            DynamicProperties dp(this);
            int rv = dp.exec();
            if (rv) {
                  int newVelo    = _velocity;
                  DynamicType nt = _dynType;
                  _velocity      = oldVelo;
                  _dynType       = ot;
                  score()->undoChangeDynamic(this, newVelo, nt);
                  }
            }
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Dynamic::dragAnchor() const
      {
      double xp = 0.0;
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      double yp = measure()->system()->staffY(staffIdx());
      QPointF p(xp, yp);
      return QLineF(p, canvasPos());
      }

//---------------------------------------------------------
//   DynamicProperties
//---------------------------------------------------------

DynamicProperties::DynamicProperties(Dynamic* d, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      dynamic = d;
      velocity->setValue(dynamic->velocity());
      switch(dynamic->dynType()) {
            case DYNAMIC_STAFF:     staffButton->setChecked(true); break;
            case DYNAMIC_PART:      partButton->setChecked(true); break;
            case DYNAMIC_SYSTEM:    systemButton->setChecked(true); break;
            }
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void DynamicProperties::accept()
      {
      dynamic->setVelocity(velocity->value());
      if (staffButton->isChecked())
            dynamic->setDynType(DYNAMIC_STAFF);
      else if (partButton->isChecked())
            dynamic->setDynType(DYNAMIC_PART);
      else if (systemButton->isChecked())
            dynamic->setDynType(DYNAMIC_SYSTEM);
      QDialog::accept();
      }
