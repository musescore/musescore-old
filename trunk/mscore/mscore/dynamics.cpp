//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: dynamics.cpp,v 1.23 2006/03/28 14:58:58 wschweer Exp $
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
#include "layout.h"

Dyn dynList[] = {
      Dyn( -1,  "other-dynamics"),
      Dyn(  1,  "pppppp"),
      Dyn(  5,  "ppppp"),
      Dyn( 10,  "pppp"),
      Dyn( 30,  "ppp"),
      Dyn( 50,  "pp"),
      Dyn( 60,  "p"),
      Dyn( 70,  "mp"),
      Dyn( 80,  "mf"),
      Dyn( 90,  "f"),
      Dyn(100,  "ff"),
      Dyn(110,  "fff"),
      Dyn(120,  "ffff"),
      Dyn(125,  "fffff"),
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
      setTextStyle(TEXT_STYLE_DYNAMICS);
      }

Dynamic::Dynamic(const Dynamic& d)
   : Text(d)
      {
      setSubtype(subtype());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Dynamic::write(Xml& xml) const
      {
      xml.stag("Dynamic");
      xml.tag("velocity", _velocity);
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
            double m = size * DPI / PPI;
            if (ts->sizeIsSpatiumDependent)
                  m *= (::_spatium / (SPATIUM20 * DPI));
            m *= mag();

            QFont font("MScore1");
            font.setPixelSize(lrint(m));
            tf.setFont(font);
            cursor.setBlockCharFormat(tf);
            cursor.insertText(dynList[idx].tag);
            _velocity = dynList[idx].velocity;
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

bool Dynamic::startEdit(Viewer* v, const QPointF& p)
      {
      setSubtype(0);
      return Text::startEdit(v, p);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Dynamic::endEdit()
      {
      Text::endEdit();
//      setSubtype(0);
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

void Dynamic::layout(ScoreLayout* sl)
      {
      setSubtype(subtype());  // re-apply style
      Text::layout(sl);
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
      a = popup->addAction(tr("Midi Properties..."));
      a->setData("dynamics");
      a = popup->addAction(tr("Text Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Dynamic::propertyAction(const QString& s)
      {
      if (s == "props") {
            Text* nText = new Text(*this);
            TextProperties tp(nText, 0);
            int rv = tp.exec();
            if (rv) {
                  score()->undoChangeElement(this, nText);
                  }
            else
                  delete nText;
            }
      else if (s == "dynamics") {
            int oldVelo = _velocity;
            DynamicProperties dp(this);
            int rv = dp.exec();
            if (rv) {
                  int newVelo = _velocity;
                  _velocity = oldVelo;
                  score()->undoChangeVelocity(this, newVelo);
                  }
            }
      else
            Element::propertyAction(s);
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
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void DynamicProperties::accept()
      {
      dynamic->setVelocity(velocity->value());
      QDialog::accept();
      }



