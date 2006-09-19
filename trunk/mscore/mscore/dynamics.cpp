//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: dynamics.cpp,v 1.23 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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
      Dyn(DynPPPPPP,   TEXT_STYLE_SYMBOL1,  "pppppp", QString(pp, 6)),
      Dyn(DynPPPPP,    TEXT_STYLE_SYMBOL1,  "ppppp",  QString(pp, 5)),
      Dyn(DynPPPP,     TEXT_STYLE_SYMBOL1,  "pppp",   QString(pp, 4)),
      Dyn(DynPPP,      TEXT_STYLE_SYMBOL1,  "ppp",    QString(pp, 3)),
      Dyn(DynPP,       TEXT_STYLE_SYMBOL1,  "pp",     QString(pp, 2)),
      Dyn(DynP,        TEXT_STYLE_SYMBOL1,  "p",      QString(pp, 1)),
      Dyn(DynMP,       TEXT_STYLE_SYMBOL1,  "mp",     QString(mp,  2)),
      Dyn(DynMF,       TEXT_STYLE_SYMBOL1,  "mf",     QString(mf,  2)),
      Dyn(DynF,        TEXT_STYLE_SYMBOL1,  "f",      QString(ff,  1)),
      Dyn(DynFF,       TEXT_STYLE_SYMBOL1,  "ff",     QString(ff,  2)),
      Dyn(DynFFF,      TEXT_STYLE_SYMBOL1,  "fff",    QString(ff,  3)),
      Dyn(DynFFFF,     TEXT_STYLE_SYMBOL1,  "ffff",   QString(ff,  4)),
      Dyn(DynFFFFF,    TEXT_STYLE_SYMBOL1,  "fffff",  QString(ff,  5)),
      Dyn(DynFFFFFF,   TEXT_STYLE_SYMBOL1,  "ffffff", QString(ff,  6)),
      Dyn(DynFP,       TEXT_STYLE_SYMBOL1,  "fp",     QString(fp,  2)),
      Dyn(DynSF,       TEXT_STYLE_SYMBOL1,  "sf",     QString(sfz, 2)),
      Dyn(DynSFZ,      TEXT_STYLE_SYMBOL1,  "sfz",    QString(sfz, 3)),
      Dyn(DynSFFZ,     TEXT_STYLE_SYMBOL1,  "sffz",   QString(sffz, 4)),
      Dyn(DynSFP,      TEXT_STYLE_SYMBOL1,  "sfp",    QString(sfp, 3)),
      Dyn(DynSFPP,     TEXT_STYLE_SYMBOL1,  "sfpp",   QString(sfp, 4)),
      Dyn(DynRFZ,      TEXT_STYLE_SYMBOL1,  "rfz",    QString(rfz, 3)),
      Dyn(DynRF,       TEXT_STYLE_SYMBOL1,  "rf",     QString(rfz, 2)),
      Dyn(DynFZ,       TEXT_STYLE_SYMBOL1,  "fz",     QString(fz,  2)),
      Dyn(DynM,        TEXT_STYLE_SYMBOL1,  "m",      QString(mp,  1)),
      Dyn(DynRFZ,      TEXT_STYLE_SYMBOL1,  "rfz",    QString(rfz, 1)),
      Dyn(DynSFZ,      TEXT_STYLE_SYMBOL1,  "sfz",    QString(sfz, 1)),
      Dyn(DynZ,        TEXT_STYLE_SYMBOL1,  "z",      QString(z,   1)),
      Dyn(DynOTHER,    TEXT_STYLE_DYNAMICS, "other-dynamics", QString("")),
      };

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic(Score* s)
   : TextElement(s, TEXT_STYLE_SYMBOL1)
      {
      setVal(DynMF);
      }

Dynamic::Dynamic(Score* s, DynVal v)
   : TextElement(s, TEXT_STYLE_SYMBOL1)
      {
      setVal(v);
      }

Dynamic::Dynamic(Score* s, const QString& t)
   : TextElement(s, TEXT_STYLE_DYNAMICS)
      {
      setVal(DynOTHER);
      setText(t);
      }

//---------------------------------------------------------
//   setVal
//---------------------------------------------------------

void Dynamic::setVal(DynVal idx)
      {
      val = idx;
      setStyle(dynList[val].textStyle);
      setText(dynList[val].str);
      }

//---------------------------------------------------------
//   endDrag
//    bind to nearest tick position
//---------------------------------------------------------

void Dynamic::endDrag()
      {
      int ntick;
      Staff* stf = staff();
      QPointF offset;
      Segment* seg;
      Measure* measure = _score->pos2measure(aref(), &ntick, &stf, 0, &seg, &offset);
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
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Dynamic::write(Xml& xml) const
      {
      xml.stag("Dynamic");
      if (val == DynOTHER)
            ;     // TODO xml.tag("other-dynamics", text());
      else
            xml.tagE(dynList[val].tag);
      Element::writeProperties(xml);
      xml.etag("Dynamic");
      }

//---------------------------------------------------------
//   tag2val
//---------------------------------------------------------

DynVal Dynamic::tag2val(const QString& tag)
      {
      int n = sizeof(dynList)/sizeof(*dynList);
      for (int i = 0; i < n; ++i) {
            if (dynList[i].tag == tag) {
                  return DynVal(dynList[i].val);
                  }
            }
      return DynINVALID;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Dynamic::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            DynVal v = tag2val(tag);
            if (v != DynINVALID) {
                  setVal(v);
                  if (v == DynOTHER) {
                        setText(e.text());
                        printf("dyn-other <%s>\n", e.text().toLatin1().data());
                        }
                  }
            else if (Element::readProperties(node))
                  ;
            else
                  domError(node);
            }
      }

