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
#include "repeatproperties.h"

struct RepeatDict {
      int type;
      const char* name;
      };

static RepeatDict rdict[] = {
      { RepeatSegno,          "segno"           },
      { RepeatCoda,           "coda"            },
      { RepeatVarcoda,        "varcoda"         },
      { RepeatCodetta,        "codetta"         },
      { RepeatDacapo,         "daCapo"          },
      { RepeatDacapoAlFine,   "daCapoAlFine"    },
      { RepeatDacapoAlCoda,   "daCapoAlCoda"    },
      { RepeatDalSegno,       "dalSegno"        },
      { RepeatDalSegnoAlFine, "dalSegnoAlFine"  },
      { RepeatDalSegnoAlCoda, "dalSegnoAlCoda"  },
      { RepeatAlSegno,        "alSegno"         },
      { RepeatFine,           "fine"            },
      };

QMap<QString, int> Repeat::mapSI;
QMap<int, QString> Repeat::mapIS;
bool Repeat::initialized = false;

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
//   Repeat
//---------------------------------------------------------

Repeat::Repeat(Score* s)
   : Element(s)
      {
      if (!initialized) {
            for (unsigned int i = 0; i < sizeof(rdict)/sizeof(*rdict); ++i) {
                  mapSI[rdict[i].name] = rdict[i].type;
                  mapIS[rdict[i].type] = rdict[i].name;
                  }
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Repeat::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Repeat::write(Xml& xml) const
      {
      xml.stag("Repeat");
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString Repeat::subtypeName() const
      {
      return mapIS[subtype()];
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Repeat::setSubtype(const QString& s)
      {
      Element::setSubtype(mapSI[s]);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Repeat::draw(QPainter& p) const
      {
      TextStyle* ts = score()->textStyle(TEXT_STYLE_REPEAT);
      QFont font = ts->font();
      p.setFont(ts->font());

      switch(subtype()) {
            case RepeatSegno:
                  symbols[segnoSym].draw(p);
                  break;
            case RepeatCoda:
                  symbols[codaSym].draw(p);
                  break;
            case RepeatVarcoda:
                  symbols[varcodaSym].draw(p);
                  break;
            case RepeatCodetta:
                  symbols[codaSym].draw(p, 0.0, 0.0, 2);
                  break;
            case RepeatDacapo:
                  p.drawText(0, 0, "D.C.");
                  break;
            case RepeatDacapoAlFine:
                  p.drawText(0, 0, "D.C. al fine");
                  break;
            case RepeatDacapoAlCoda:
                  p.drawText(0, 0, "D.C. al coda");
                  break;
            case RepeatDalSegno:
                  p.drawText(0, 0, "D.S.");
                  break;
            case RepeatDalSegnoAlFine:
                  p.drawText(0, 0, "D.S. al fine");
                  break;
            case RepeatDalSegnoAlCoda:
                  p.drawText(0, 0, "D.S. al coda");
                  break;
            case RepeatAlSegno:
                  p.drawText(0, 0, "al segno");
                  break;
            case RepeatFine:
                  p.drawText(0, 0, "fine");
                  break;
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Repeat::bbox() const
      {
      TextStyle* ts = score()->textStyle(TEXT_STYLE_REPEAT);

      QRectF bb;
      switch(subtype()) {
            case RepeatSegno:
                  bb = symbols[segnoSym].bbox();
                  break;
            case RepeatCoda:
                  bb = symbols[codaSym].bbox();
                  break;
            case RepeatVarcoda:
                  bb = symbols[varcodaSym].bbox();
                  break;
            case RepeatCodetta:
                  bb = symbols[segnoSym].bbox();
                  bb |= bb.translated(symbols[segnoSym].width(), 0.0);
                  break;
            case RepeatDacapo:
                  bb = ts->bbox("D.C.");
                  break;
            case RepeatDacapoAlFine:
                  bb = ts->bbox("D.C. al fine");
                  break;
            case RepeatDacapoAlCoda:
                  bb = ts->bbox("D.C. al coda");
                  break;
            case RepeatDalSegno:
                  bb = ts->bbox("D.S.");
                  break;
            case RepeatDalSegnoAlFine:
                  bb = ts->bbox("D.S. al fine");
                  break;
            case RepeatDalSegnoAlCoda:
                  bb = ts->bbox("D.S. al coda");
                  break;
            case RepeatAlSegno:
                  bb = ts->bbox("al segno");
                  break;
            case RepeatFine:
                  bb = ts->bbox("fine");
                  break;
            }
      return bb;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Repeat::genPropertyMenu(QMenu* popup) const
      {
      QAction* a = popup->addAction(tr("Properties..."));
      a->setData("props");
printf("genPropertyMEnu\n");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Repeat::propertyAction(const QString& s)
      {
      if (s == "props") {
printf("propertyAction\n");
            RepeatProperties rp;
            int rv = rp.exec();
            if (rv) {
                  printf("OK\n");
                  }
            }
      }

