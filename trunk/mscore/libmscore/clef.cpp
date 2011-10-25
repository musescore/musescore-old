//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 \file
 Implementation of classes Clef (partial) and ClefList (complete).
*/

#include "clef.h"
#include "xml.h"
#include "sym.h"
#include "symbol.h"
#include "score.h"
#include "staff.h"
#include "segment.h"
#include "stafftype.h"
#include "painter.h"

#define TR(a)  QT_TRANSLATE_NOOP("clefTable", a)

const ClefInfo clefTable[] = {
//   MusicXml           octchng  pitchoffset
// tag  xmlName     line   yoffs     |-lines for sharps--||--lines for flats--|   name
{ "G",    "G",         2,  0,   0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Treble clef"),            PITCHED_STAFF },
{ "G8va", "G",         2,  1,   7, 52, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Treble clef 8va"),        PITCHED_STAFF },
{ "G15ma","G",         2,  2,  14, 59, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Treble clef 15ma"),       PITCHED_STAFF },
{ "G8vb", "G",         2, -1,  -7, 38, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Treble clef 8vb"),        PITCHED_STAFF },
{ "F",    "F",         4,  0, -12, 33, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("Bass clef"),              PITCHED_STAFF },
{ "F8vb", "F",         4, -1, -19, 26, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("Bass clef 8vb"),          PITCHED_STAFF },
{ "F15mb","F",         4, -2, -26, 19, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("Bass clef 15mb"),         PITCHED_STAFF },
{ "F3",   "F",         3,  0, -10, 35, { 4, 0, 3,-1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 }, TR("Baritone clef (F clef)"), PITCHED_STAFF },
{ "F5",   "F",         5,  0, -14, 31, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Subbass clef"),           PITCHED_STAFF },
{ "C1",   "C",         1,  0,  -2, 43, { 5, 1, 4, 0, 3,-1, 2, 2,-1, 3, 0, 4, 1, 5 }, TR("Soprano clef"),           PITCHED_STAFF }, // CLEF_C1
{ "C2",   "C",         2,  0,  -4, 41, { 3, 6, 2, 5, 1, 4, 0, 0, 4, 1, 5, 2, 6, 3 }, TR("Mezzo-soprano clef"),     PITCHED_STAFF }, // CLEF_C2
{ "C3",   "C",         3,  0,  -6, 39, { 1, 4, 0, 3, 6, 2, 5, 5, 2, 6, 3, 7, 4, 8 }, TR("Alto clef"),              PITCHED_STAFF }, // CLEF_C3
{ "C4",   "C",         4,  0,  -8, 37, { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 }, TR("Tenor clef"),             PITCHED_STAFF }, // CLEF_C4
{ "TAB",  "TAB",       5,  0,   0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Tablature"),              TAB_STAFF     },
{ "PERC", "percussion",2,  0,   0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Percussion"),             PERCUSSION_STAFF },
{ "C5",   "C",         5,  0, -10, 35, { 4, 0, 3,-1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 }, TR("Baritone clef (C clef)"), PITCHED_STAFF }, // CLEF_C5
{ "G1",   "G",         1,  0,   2, 47, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("French violin clef"),     PITCHED_STAFF }, // CLEF_G4
{ "F8va", "F",         4,  1,  -5, 40, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("Bass clef 8va"),          PITCHED_STAFF }, // CLEF_F_8VA
{ "F15ma","F",         4,  2,   2, 47, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("Bass clef 15ma"),         PITCHED_STAFF }, // CLEF_F_15MA
{ "PERC2","percussion",2,  0,   0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Percussion"),             PERCUSSION_STAFF }, // CLEF_PERC2 placeholder
{ "TAB2", "TAB",       5,  0,   0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Tablature2"),             TAB_STAFF     },
      };
#undef TR

//---------------------------------------------------------
//   Clef
//---------------------------------------------------------

Clef::Clef(Score* s)
  : Element(s)
      {
      setFlags(ELEMENT_SELECTABLE | ELEMENT_ON_STAFF);

      _showCourtesyClef = true;
      _small            = false;
      _clefTypes._concertClef     = CLEF_INVALID;
      _clefTypes._transposingClef = CLEF_INVALID;
      }

Clef::Clef(const Clef& c)
   : Element(c)
      {
      setFlags(ELEMENT_SELECTABLE | ELEMENT_ON_STAFF);

      _showCourtesyClef = c._showCourtesyClef;
      _small            = c._small;
      _clefTypes        = c._clefTypes;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Clef::addElement(Element* e, qreal x, qreal y)
      {
      e->layout();
      e->setPos(x, y);
      e->setParent(this);
      elements.push_back(e);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Clef::layout()
      {
      qreal smag     = _small ? score()->style(ST_smallClefMag).toDouble() : 1.0;
      qreal _spatium = spatium();
      qreal msp      = _spatium * smag;
      qreal yoff     = 0.0;
      elements.clear();

      ClefType st    = clefType();
      int lines      = 5;
      qreal lineDist = 1.0;
      if (staff() && staff()->staffType()) {
            StaffType* staffType = staff()->staffType();

            if (staffType->group() == TAB_STAFF) {
                  if (!staffType->genClef())
                        return;
                  if (clefTable[st].staffGroup != TAB_STAFF)
                        st = ClefType(score()->styleI(ST_tabClef));
	            }
            lines = staffType->lines();
            lineDist = staffType->lineDistance().val();
	      }
      Symbol* symbol = new Symbol(score());

      switch (st) {
            case CLEF_G:
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0;
                  break;
            case CLEF_G1:
                  {
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0;
                  Symbol* number = new Symbol(score());
                  number->setMag(smag);
                  number->setSym(clefEightSym);
                  addElement(number, 1.0 * msp, -5.0 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_G2:
                  {
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0;
                  Symbol* number = new Symbol(score());
                  symbol->setMag(smag);
                  number->setSym(clefOneSym);
                  addElement(number, .6 * msp, -5.0 * msp + yoff * _spatium);
                  number = new Symbol(score());
                  number->setSym(clefFiveSym);
                  addElement(number, 1.4 * msp, -5.0 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_G3:
                  {
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0;
                  Symbol* number = new Symbol(score());
                  symbol->setMag(smag);
                  number->setSym(clefEightSym);
                  addElement(number, 1.0 * msp, 4.0 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_F:
                  symbol->setSym(bassclefSym);
                  yoff = 1.0;
                  break;
            case CLEF_F8:
                  {
                  symbol->setSym(bassclefSym);
                  yoff = 1.0;
                  Symbol* number = new Symbol(score());
                  symbol->setMag(smag);
                  number->setSym(clefEightSym);
                  addElement(number, .0, 4.5 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_F15:
                  {
                  symbol->setSym(bassclefSym);
                  yoff = 1.0;
                  Symbol* number = new Symbol(score());
                  symbol->setMag(smag);
                  number->setSym(clefOneSym);
                  addElement(number, .0, 4.5 * msp + yoff * _spatium);
                  number = new Symbol(score());
                  number->setSym(clefFiveSym);
                  addElement(number, .8 * msp, 4.5 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_F_B:                            // baritone clef
                  symbol->setSym(bassclefSym);
                  yoff = 2.0;
                  break;
            case CLEF_F_C:                            // subbass clef
                  symbol->setSym(bassclefSym);
                  yoff = 0.0;
                  break;
            case CLEF_C1:
                  symbol->setSym(altoclefSym);
                  yoff = 4.0;
                  break;
            case CLEF_C2:
                  symbol->setSym(altoclefSym);
                  yoff = 3.0;
                  break;
            case CLEF_C3:
                  symbol->setSym(altoclefSym);
                  yoff = 2.0;
                  break;
            case CLEF_C4:
                  symbol->setSym(altoclefSym);
                  yoff = 1.0;
                  break;
            case CLEF_C5:
                  symbol->setSym(altoclefSym);
                  yoff = 0.0;
                  break;
            case CLEF_TAB:
                  symbol->setSym(tabclefSym);
                  // on tablature, position clef at half the number of spaces * line distance
                  yoff = lineDist * (lines - 1) * .5;
                  break;
            case CLEF_TAB2:
                  symbol->setSym(tabclef2Sym);
                  // on tablature, position clef at half the number of spaces * line distance
                  yoff = lineDist * (lines - 1) * .5;
                  break;
            case CLEF_PERC:
            case CLEF_PERC2:
                  symbol->setSym(percussionclefSym);
                  yoff = lineDist * (lines - 1) * 0.5;
                  break;
            case CLEF_G4:
                  symbol->setSym(trebleclefSym);
                  yoff = 4.0;
                  break;
            case CLEF_F_8VA:
                  {
                  symbol->setSym(bassclefSym);
                  yoff = 1.0;
                  Symbol* number = new Symbol(score());
                  number->setMag(smag);
                  number->setSym(clefEightSym);
                  addElement(number, .5 * msp, -1.5 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_F_15MA:
                  {
                  symbol->setSym(bassclefSym);
                  yoff = 1.0;
                  Symbol* number = new Symbol(score());
                  symbol->setMag(smag);
                  number->setSym(clefOneSym);
                  addElement(number, .0 * msp, -1.5 * msp + yoff * _spatium);
                  number = new Symbol(score());
                  number->setSym(clefFiveSym);
                  addElement(number, .8 * msp, -1.5 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_INVALID:
            case CLEF_MAX:
                  return;
            }
      symbol->setMag(smag * mag());
      symbol->layout();
      addElement(symbol, .0, yoff * _spatium);
      setbbox(QRectF());
      for (iElement i = elements.begin(); i != elements.end(); ++i) {
            Element* e = *i;
            addbbox(e->bbox().translated(e->pos()));
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Clef::draw(Painter* painter) const
      {
      if (staff() && staff()->useTablature() && !staff()->staffType()->genClef())
	      return;
      foreach(Element* e, elements) {
            QPointF pt(e->pos());
            painter->translate(pt);
            e->draw(painter);
            painter->translate(-pt);
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Clef::acceptDrop(MuseScoreView*, const QPointF&, int type, int) const
      {
      return type == CLEF;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Clef::drop(const DropData& data)
      {
      Element* e = data.element;
      Clef* c = 0;
      if (e->type() == CLEF) {
            Clef* clef = static_cast<Clef*>(e);
            ClefType stype  = clef->clefType();
qDebug("drop clef %d -> %d, track %d\n", int(clefType()), int(stype), track());
            if (clefType() != stype) {
                  score()->undoChangeClef(staff(), segment(), stype);
                  c = this;
                  }
            }
      delete e;
      return c;
      }

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void Clef::setSmall(bool val)
      {
      if (val != _small)
            _small = val;
      layout();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Clef::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "subtype")
                  setClefType(clefType(val));
            else if (tag == "concertClefType")
                  _clefTypes._concertClef = Clef::clefType(val);
            else if (tag == "transposingClefType")
                  _clefTypes._transposingClef = Clef::clefType(val);
            else if (!Element::readProperties(e))
                  domError(e);
            }
      if (score()->mscVersion() < 113)
            setUserOff(QPointF());
      if (clefType() == CLEF_INVALID)
            setClefType(CLEF_G);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Clef::write(Xml& xml) const
      {
      xml.stag(name());
      xml.tag("concertClefType",     clefTable[_clefTypes._concertClef].tag);
      xml.tag("transposingClefType", clefTable[_clefTypes._transposingClef].tag);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int Clef::tick() const
      {
      return segment() ? segment()->tick() : 0;
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString Clef::subtypeName() const
      {
      return QString(clefTable[int(clefType())].tag);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Clef::setSubtype(const QString& s)
      {
      ClefType ct = clefType(s);
      if (ct == CLEF_INVALID) {
            qDebug("Clef::setSubtype: unknown: <%s>\n", qPrintable(s));
            ct = CLEF_G;
            }
      setClefType(ct);
      }

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefType Clef::clefType(const QString& s)
      {
      ClefType ct = CLEF_G;
      bool ok;
      int i = s.toInt(&ok);
      if (ok) {
            //
            // convert obsolete old coding
            //
            switch (i) {
                  default:
                  case  0: ct = CLEF_G; break;
                  case  1: ct = CLEF_G1; break;
                  case  2: ct = CLEF_G2; break;
                  case  3: ct = CLEF_G3; break;
                  case  4: ct = CLEF_F; break;
                  case  5: ct = CLEF_F8; break;
                  case  6: ct = CLEF_F15; break;
                  case  7: ct = CLEF_F_B; break;
                  case  8: ct = CLEF_F_C; break;
                  case  9: ct = CLEF_C1; break;
                  case 10: ct = CLEF_C2; break;
                  case 11: ct = CLEF_C3; break;
                  case 12: ct = CLEF_C4; break;
                  case 13: ct = CLEF_TAB; break;
                  case 14: ct = CLEF_PERC; break;
                  case 15: ct = CLEF_C5; break;
                  case 16: ct = CLEF_G4; break;
                  case 17: ct = CLEF_F_8VA; break;
                  case 18: ct = CLEF_F_15MA; break;
                  case 19: ct = CLEF_PERC2; break;
                  case 20: ct = CLEF_TAB2; break;
                  }
            }
      else {
            for (unsigned i = 0; i < sizeof(clefTable)/sizeof(*clefTable); ++i) {
                  if (clefTable[i].tag == s) {
                        ct = ClefType(i);
                        break;
                        }
                  }
            }
      return ct;
      }

//---------------------------------------------------------
//   setClefType
//---------------------------------------------------------

void Clef::setClefType(ClefType i)
      {
      if (score()->concertPitch()) {
            _clefTypes._concertClef = i;
            if (_clefTypes._transposingClef == CLEF_INVALID)
                  _clefTypes._transposingClef = i;

            }
      else {
            _clefTypes._transposingClef = i;
            if (_clefTypes._concertClef == CLEF_INVALID)
                  _clefTypes._concertClef = i;
            }
      }

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefType Clef::clefType() const
      {
      if (score()->concertPitch())
            return _clefTypes._concertClef;
      else
            return _clefTypes._transposingClef;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Clef::getProperty(int propertyId) const
      {
      switch(propertyId) {
            case P_SHOW_COURTESY: return _showCourtesyClef;
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void Clef::setProperty(int propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_SHOW_COURTESY: _showCourtesyClef = v.toBool(); break;
            default:
                  Element::setProperty(propertyId, v);
            }
      }


