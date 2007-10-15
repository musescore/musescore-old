//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: dynamics.cpp,v 1.23 2006/03/28 14:58:58 wschweer Exp $
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

#include "dynamics.h"
#include "xml.h"
#include "score.h"
#include "measure.h"
#include "system.h"
#include "segment.h"
#include "utils.h"
#include "style.h"
#include "layout.h"

static const QChar pp[]     = { 0x70, 0x70, 0x70, 0x70, 0x70, 0x70};
static const QChar mp[]     = { 0x6d, 0x70 };               // mp
static const QChar mf[]     = { 0x6d, 0x66 };               // mf
static const QChar ff[]     = { 0x66, 0x66, 0x66, 0x66, 0x66, 0x66};
static const QChar fp[]     = { 0x66, 0x70 };               // fp
static const QChar sfz[]    = { 0x73, 0x66, 0x7a };         // sfz
static const QChar sffz[]   = { 0x73, 0x66, 0x66, 0x7a };   // sffz
static const QChar sfp[]    = { 0x73, 0x66, 0x70, 0x70};    // sfpp
static const QChar rfz[]    = { 0x72, 0x66, 0x7a };         // rfz
static const QChar fz[]     = { 0x66, 0x7a };               // fz
static const QChar z[]      = { 0x7a };                     // z

Dyn dynList[] = {
      Dyn(TEXT_STYLE_DYNAMICS,   -1,  "other-dynamics", QString("")),
      Dyn(TEXT_STYLE_DYNAMICS1,   1,  "pppppp", QString(pp, 6)),
      Dyn(TEXT_STYLE_DYNAMICS1,   5,  "ppppp",  QString(pp, 5)),
      Dyn(TEXT_STYLE_DYNAMICS1,  10,  "pppp",   QString(pp, 4)),
      Dyn(TEXT_STYLE_DYNAMICS1,  30,  "ppp",    QString(pp, 3)),
      Dyn(TEXT_STYLE_DYNAMICS1,  50,  "pp",     QString(pp, 2)),
      Dyn(TEXT_STYLE_DYNAMICS1,  60,  "p",      QString(pp, 1)),
      Dyn(TEXT_STYLE_DYNAMICS1,  70,  "mp",     QString(mp,  2)),
      Dyn(TEXT_STYLE_DYNAMICS1,  80,  "mf",     QString(mf,  2)),
      Dyn(TEXT_STYLE_DYNAMICS1,  90,  "f",      QString(ff,  1)),
      Dyn(TEXT_STYLE_DYNAMICS1, 100,  "ff",     QString(ff,  2)),
      Dyn(TEXT_STYLE_DYNAMICS1, 110,  "fff",    QString(ff,  3)),
      Dyn(TEXT_STYLE_DYNAMICS1, 120,  "ffff",   QString(ff,  4)),
      Dyn(TEXT_STYLE_DYNAMICS1, 125,  "fffff",  QString(ff,  5)),
      Dyn(TEXT_STYLE_DYNAMICS1, 127,  "ffffff", QString(ff,  6)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "fp",     QString(fp,  2)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "sf",     QString(sfz, 2)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "sfz",    QString(sfz, 3)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "sffz",   QString(sffz, 4)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "sfp",    QString(sfp, 3)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "sfpp",   QString(sfp, 4)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "rfz",    QString(rfz, 3)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "rf",     QString(rfz, 2)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "fz",     QString(fz,  2)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "m",      QString(mp,  1)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "rfz",    QString(rfz, 1)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "sfz",    QString(sfz, 1)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "z",      QString(z,   1)),
      };

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic(Score* s)
   : Text(s)
      {
      }

Dynamic::Dynamic(Score* s, int st)
   : Text(s)
      {
      setSubtype(st);
      }

Dynamic::Dynamic(Score* s, const QString& t)
   : Text(s)
      {
      setSubtype(0);
      setText(t);
      }

Dynamic::Dynamic(const Dynamic& d)
   : Text(d)
      {
      setSubtype(subtype());
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Dynamic::setSubtype(int idx)
      {
      Element::setSubtype(idx);
      setStyle(dynList[idx].textStyle);
      if (idx != 0)
            setText(dynList[idx].str);
      }

//---------------------------------------------------------
//   endDrag
//    bind to nearest tick position
//---------------------------------------------------------

void Dynamic::endDrag()
      {
#if 1
      int ntick;
      Staff* stf = staff();
      QPointF offset;
      Segment* seg;
      Measure* measure = _score->pos2measure(canvasPos(), &ntick, &stf, 0, &seg, &offset);
      if (measure) {
            offset /= _spatium;
            setTick(ntick);
            setUserOff(offset);
            setStaff(stf);
            if (measure != parent()) {
                  ((Measure*)parent())->remove(this);
                  measure->add(this);
                  }
            }
      else
            printf("Dynamic::endDrag(): measure not found\n");
#endif
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Dynamic::write(Xml& xml) const
      {
      xml.stag("Dynamic");
      if (subtype() == 0)
            xml.tag("data", QVariant(doc->toHtml("utf8")));
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Dynamic::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "data")
                  doc->setHtml(e.text());
            else if (!Element::readProperties(e))
                  domError(e);
            }
      setSubtype(subtype());
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
      setSubtype(0);
      setText(tag);
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString Dynamic::subtypeName() const
      {
      return dynList[subtype()].tag;
      }

