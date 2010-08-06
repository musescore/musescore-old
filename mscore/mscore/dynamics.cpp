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
      Dyn( -1,  "other-dynamics"),
      Dyn(  1,  "pppppp"),
      Dyn(  5,  "ppppp"),
      Dyn( 10,  "pppp"),
      Dyn( 16,  "ppp"),
      Dyn( 33,  "pp"),
      Dyn( 49,  "p"),
      Dyn( 64,  "mp"),
      Dyn( 80,  "mf"),
      Dyn( 96,  "f"),
      Dyn(112,  "ff"),
      Dyn(126,  "fff"),
      Dyn(127,  "ffff"),
      Dyn(127,  "fffff"),
      Dyn(127,  "ffffff"),
      Dyn( -1,  "fp"),
      Dyn( -1,  "sf"),
      Dyn( -1,  "sfz"),
      Dyn( -1,  "sff"),
      Dyn( -1,  "sffz"),
      Dyn( -1,  "sfp"),
      Dyn( -1,  "sfpp"),
      Dyn( -1,  "rfz"),
      Dyn( -1,  "rf"),
      Dyn( -1,  "fz"),
      Dyn( -1,  "m"),
      Dyn( -1,  "r"),
      Dyn( -1,  "s"),
      Dyn( -1,  "z"),
      };

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic(Score* s)
   : Text(s)
      {
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
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Dynamic::setSubtype(int idx)
      {
      Element::setSubtype(idx);
      if (idx) {
            Element::setSubtype(idx);
            doc()->clear();
            QTextCursor cursor(doc());
            cursor.movePosition(QTextCursor::Start);
            QTextCharFormat tf = cursor.charFormat();
            TextStyle* ts = score()->textStyle(TEXT_STYLE_DYNAMICS);
            double size = ts->size;
            double m = size;
            if (ts->sizeIsSpatiumDependent)
                  m *= (score()->spatium() / (SPATIUM20 * DPI));
            m *= mag();

            QFont font("MScore1");
            font.setPointSize(m);
            font.setKerning(true);
            tf.setFont(font);
            tf.setProperty(QTextFormat::FontKerning, true);
            // tf.setProperty(QTextFormat::FontLetterSpacing, 100);
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
      a->setText(tr("Dynamics"));
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
            Text* nText = new Text(*this);
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
                  score()->undoChangeDynamic(this, newVelo, int(nt));
                  }
            }
      else
            Element::propertyAction(viewer, s);
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
