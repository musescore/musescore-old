//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "repeat.h"
#include "layout.h"
#include "sym.h"
#include "score.h"
#include "jumpproperties.h"
#include "markerproperties.h"

//---------------------------------------------------------
//   JumpProperties
//---------------------------------------------------------

JumpProperties::JumpProperties(Jump* jp, QWidget* parent)
   : QDialog(parent)
      {
      jump = jp;
      setupUi(this);
      jumpTo->setText(jump->jumpTo());
      playUntil->setText(jump->playUntil());
      continueAt->setText(jump->continueAt());
      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void JumpProperties::saveValues()
      {
      jump->setJumpTo(jumpTo->text());
      jump->setPlayUntil(playUntil->text());
      jump->setContinueAt(continueAt->text());
      }

//---------------------------------------------------------
//   MarkerProperties
//---------------------------------------------------------

MarkerProperties::MarkerProperties(Marker* mk, QWidget* parent)
   : QDialog(parent)
      {
      marker = mk;
      setupUi(this);
      label->setText(marker->label());
      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void MarkerProperties::saveValues()
      {
      marker->setLabel(label->text());
      }

//---------------------------------------------------------
//   RepeatMeasure
//---------------------------------------------------------

RepeatMeasure::RepeatMeasure(Score* score)
   : Element(score)
      {
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void RepeatMeasure::draw(QPainter& p) const
      {
      p.setBrush(p.pen().color());
      p.drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void RepeatMeasure::layout(ScoreLayout* layout)
      {
      double sp  = layout->spatium();

      double w   = sp * 2.0;
      double h   = sp * 2.0;
      double lw  = sp * .30;  // line width
      double r   = sp * .15;  // dot radius

      path       = QPainterPath();

      path.moveTo(w - lw, 0.0);
      path.lineTo(w,  0.0);
      path.lineTo(lw,  h);
      path.lineTo(0.0, h);
      path.closeSubpath();
      path.addEllipse(QRectF(w * .25 - r, h * .25 - r, r * 2.0, r * 2.0 ));
      path.addEllipse(QRectF(w * .75 - r, h * .75 - r, r * 2.0, r * 2.0 ));

      setbbox(path.boundingRect());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void RepeatMeasure::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      xml.etag();
      }


//---------------------------------------------------------
//   read
//---------------------------------------------------------

void RepeatMeasure::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   Marker
//---------------------------------------------------------

Marker::Marker(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_REPEAT);
      }

//---------------------------------------------------------
//   setMarkerType
//---------------------------------------------------------

void Marker::setMarkerType(int t)
      {
      switch(t) {
            case MARKER_SEGNO:
                  setHtml(symToHtml(symbols[segnoSym]));
                  setLabel("segno");
                  setRXoff(0.0);                  // move to start of measure
                  break;

            case MARKER_CODA:
                  setHtml(symToHtml(symbols[codaSym]));
                  setLabel("coda");
                  setRXoff(0.0);                  // move to start of measure
                  break;

            case MARKER_VARCODA:
                  setHtml(symToHtml(symbols[varcodaSym]));
                  setLabel("varcoda");
                  setRXoff(0.0);                  // move to start of measure
                  break;

            case MARKER_CODETTA:
                  setHtml(symToHtml(symbols[codaSym], symbols[codaSym]));
                  setLabel("codetta");
                  setRXoff(0.0);                  // move to start of measure
                  break;

            case MARKER_FINE:
                  setText("Fine");
                  setLabel("fine");
                  setRXoff(100.0);                  // move to end of measure
                  break;

            case MARKER_USER:
                  break;

            default:
                  printf("unknown marker type %d\n", t);
                  break;
            }
      }

//---------------------------------------------------------
//   markerType
//---------------------------------------------------------

int Marker::markerType() const
      {
      if (_label == "segno")
            return MARKER_SEGNO;
      else if (_label == "coda")
            return MARKER_CODA;
      else if (_label == "varcoda")
            return MARKER_VARCODA;
      else if (_label == "codetta")
            return MARKER_CODETTA;
      else if (_label == "fine")
            return MARKER_FINE;
      else
            return MARKER_USER;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Marker::read(QDomElement e)
      {
      setAlign(0);
      setAnchor(ANCHOR_PARENT);
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "label")
                  _label = e.text();
            else if (!Text::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Marker::write(Xml& xml) const
      {
      xml.stag(name());
      Text::writeProperties(xml);
      xml.tag("label", _label);
      xml.etag();
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Marker::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Marker::propertyAction(const QString& s)
      {
      if (s == "props") {
            MarkerProperties rp(this);
            rp.exec();
            }
      else
            Element::propertyAction(s);
      }

//---------------------------------------------------------
//   Jump
//---------------------------------------------------------

Jump::Jump(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_REPEAT);
      }

//---------------------------------------------------------
//   setJumpType
//---------------------------------------------------------

void Jump::setJumpType(int t)
      {
      switch(t) {
            case JUMP_DC:
                  setText("D.C.");
                  setJumpTo("start");
                  setPlayUntil("end");
                  break;

            case JUMP_DC_AL_FINE:
                  setText("D.C. al Fine");
                  setJumpTo("start");
                  setPlayUntil("fine");
                  break;

            case JUMP_DC_AL_CODA:
                  setText("D.C. al Coda");
                  setJumpTo("start");
                  setPlayUntil("coda");
                  setContinueAt("coda");
                  break;

            case JUMP_DS_AL_CODA:
                  setText("D.S. al Coda");
                  setJumpTo("segno");
                  setPlayUntil("coda");
                  setContinueAt("coda");
                  break;

            case JUMP_DS_AL_FINE:
                  setText("D.S. al Fine");
                  setJumpTo("segno");
                  setPlayUntil("fine");
                  break;

            case JUMP_USER:
                  break;

            default:
                  printf("unknown jump type\n");
                  break;
            }
      }

//---------------------------------------------------------
//   jumpType
//---------------------------------------------------------

int Jump::jumpType() const
      {
      if (_jumpTo == "start" && _playUntil == "end" && _continueAt == "")
            return JUMP_DC;
      else if (_jumpTo == "start" && _playUntil == "fine" && _continueAt == "")
            return JUMP_DC_AL_FINE;
      else if (_jumpTo == "start" && _playUntil == "coda" && _continueAt == "coda")
            return JUMP_DC_AL_CODA;
      else if (_jumpTo == "segno" && _playUntil == "coda" && _continueAt == "coda")
            return JUMP_DS_AL_CODA;
      else if (_jumpTo == "segno" && _playUntil == "fine" && _continueAt == "")
            return JUMP_DS_AL_FINE;
      return JUMP_USER;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Jump::read(QDomElement e)
      {
      setAlign(0);
      setAnchor(ANCHOR_PARENT);
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "jumpTo")
                  _jumpTo = e.text();
            else if (tag == "playUntil")
                  _playUntil = e.text();
            else if (tag == "continueAt")
                  _continueAt = e.text();
            else if (!Text::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Jump::write(Xml& xml) const
      {
      xml.stag(name());
      Text::writeProperties(xml);
      xml.tag("jumpTo", _jumpTo);
      xml.tag("playUntil", _playUntil);
      xml.tag("continueAt", _continueAt);
      xml.etag();
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Jump::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Jump::propertyAction(const QString& s)
      {
      if (s == "props") {
            JumpProperties rp(this);
            rp.exec();
            }
      else
            Element::propertyAction(s);
      }

