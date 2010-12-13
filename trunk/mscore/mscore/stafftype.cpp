//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#include "stafftype.h"
#include "staff.h"
//#include "score.h"
#include "xml.h"

QList<StaffType*> staffTypes;

//---------------------------------------------------------
//   initStaffTypes
//---------------------------------------------------------

void initStaffTypes()
      {
      StaffType* st = new StaffTypePitched("Pitched 5 lines");
      st->setLines(5);
      st->setLineDistance(Spatium(1.0));
      st->setGenClef(true);
      st->setGenKeysig(true);
      st->setSlashStyle(false);
      st->setShowBarlines(true);
      st->setShowLedgerLines(true);
      staffTypes.append(st);

      st = new StaffTypeTablature("Tab");
      st->setLines(6);
      st->setLineDistance(Spatium(1.5));
      st->setGenClef(true);
      st->setGenKeysig(false);
      st->setSlashStyle(false);
      st->setShowBarlines(true);
      st->setShowLedgerLines(false);
      staffTypes.append(st);

      st = new StaffTypePercussion("Percussion 5 lines");
      st->setLines(5);
      st->setLineDistance(Spatium(1.0));
      st->setGenClef(true);
      st->setGenKeysig(false);
      st->setSlashStyle(false);
      st->setShowBarlines(true);
      st->setShowLedgerLines(true);
      staffTypes.append(st);
      }

//---------------------------------------------------------
//   StaffType
//---------------------------------------------------------

StaffType::StaffType()
      {
      _modified = false;
      }

StaffType::StaffType(const QString& s)
      {
      _name = s;
      _modified = false;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffType::write(Xml& xml, int idx) const
      {
      xml.stag(QString("StaffType idx=\"%1\" group=\"%2\"").arg(idx).arg(groupName()));
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void StaffType::writeProperties(Xml& xml) const
      {
      xml.tag("name", name());
      if (lines() != 5)
            xml.tag("lines", lines());
      if (lineDistance().val() != 1.0)
            xml.tag("lineDistance", lineDistance().val());
      if (!genClef())
            xml.tag("clef", genClef());
      if (!genKeysig())
            xml.tag("keysig", genKeysig());
      if (slashStyle())
            xml.tag("slashStyle", slashStyle());
      if (!showBarlines())
            xml.tag("barlines", showBarlines());
      if (!showLedgerLines())
            xml.tag("ledgerlines", showLedgerLines());
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffType::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if(!readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool StaffType::readProperties(QDomElement e)
      {
      QString tag(e.tagName());
      int v = e.text().toInt();
      if (tag == "name")
            setName(e.text());
      else if (tag == "lines")
            setLines(v);
      else if (tag == "lineDistance")
            setLineDistance(Spatium(e.text().toDouble()));
      else if (tag == "clef")
            setGenClef(v);
      else if (tag == "keysig")
            setGenKeysig(v);
      else if (tag == "slashStyle")
            setSlashStyle(v);
      else if (tag == "barlines")
            setShowBarlines(v);
      else if (tag == "ledgerlines")
            setShowLedgerLines(v);
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   StaffTypeTablature
//---------------------------------------------------------

#define TAB_DEFAULT_DUR_YOFFS	(-1.75)
#define TAB_DEFAULT_LINE_SP	(1.5)

void StaffTypeTablature::init()
      {
      // set reasonable defaults for inherited members
      setLines(6);
      setLineDistance(Spatium(TAB_DEFAULT_LINE_SP));
      setGenClef(true);
      setGenKeysig(false);
      setSlashStyle(false);
      setShowBarlines(true);
      setShowLedgerLines(false);
      // for specific members
      setDurationFontName("FreeSans");
      setDurationFontSize(10.0);
      setDurationFontUserY(0.0);
      setFretFontName("FreeSans");
      setFretFontSize(9.0);
      setFretFontUserY(0.0);
      setGenDurations(false);
      setLinesThrough(false);
      setOnLines(true);
      setUseNumbers(true);
      // internal
      _metricsValid = false;
      _charBoxH = _charBoxY = _fretYOffset = _refDPI = 0.0;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffTypeTablature::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            int v = e.text().toInt();
            double val = e.text().toDouble();
            if(tag == "durations")
                  setGenDurations(v != 0);
            else if(tag == "durationFontName")
                  setDurationFontName(e.text());
            else if(tag == "durationFontSize")
                  setDurationFontSize(val);
            else if(tag == "durationFontY")
                  setDurationFontUserY(val);
            else if(tag == "fretFontName")
                  setFretFontName(e.text());
            else if(tag == "fretFontSize")
                  setFretFontSize(val);
            else if(tag == "fretFontY")
                  setFretFontUserY(val);
            else if(tag == "linesThrough")
                  setLinesThrough(v != 0);
            else if(tag == "onLines")
                  setOnLines(v != 0);
            else if(tag == "timesig")
                  setGenTimesig(v != 0);
            else if(tag == "useNumbers")
                  setUseNumbers(v != 0);
            else
                  if(!StaffType::readProperties(e))
                        domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffTypeTablature::write(Xml& xml, int idx) const
      {
      xml.stag(QString("StaffType idx=\"%1\" group=\"%2\"").arg(idx).arg(groupName()));
      StaffType::writeProperties(xml);
      xml.tag("durations",        _genDurations);
      xml.tag("durationFontName", _durationFontName);
      xml.tag("durationFontSize", _durationFontSize);
      xml.tag("durationFontY",    _durationFontUserY);
      xml.tag("fretFontName",     _fretFontName);
      xml.tag("fretFontSize",     _fretFontSize);
      xml.tag("fretFontY",        _fretFontUserY);
      xml.tag("linesThrough",     _linesThrough);
      xml.tag("onLines",          _onLines);
      xml.tag("timesig",          _genTimesig);
      xml.tag("useNumbers",       _useNumbers);
      xml.etag();
      }

//---------------------------------------------------------
//   setOnLines
//---------------------------------------------------------

void StaffTypeTablature::setOnLines(bool val)
{
      _onLines = val;
      _metricsValid = false;
      _durationYOffset = TAB_DEFAULT_DUR_YOFFS - (_onLines ? 0.0 : lineDistance().val()/2.0);
}

//---------------------------------------------------------
//   setMetrics
//    checks whether the internally computed metrics are is still valid and re-computes them, if not
//---------------------------------------------------------

static QString	g_strNumbers("0123456789");
static QString	g_strLetters("abcdefghiklmnopq");

void StaffTypeTablature::setMetrics()
{
      if(_metricsValid && _refDPI == DPI)
            return;

      QFontMetricsF fm(fretFont());
      QRectF bb(fm.tightBoundingRect(_useNumbers ? g_strNumbers : g_strLetters));
      if(_useNumbers) {
            // for numbers: move down by the whole part above (negative) the base line ( -bb.y() )
            // then up by half the whole height ( -bb.height()/2 )
            _fretYOffset = -(bb.y() + bb.height()/2.0);
            }
      else {
            // for letters: centre on the x height, by moving down half of the part above the base line in bx
            QRectF bx( fm.tightBoundingRect("a") );
            _fretYOffset = -bx.y() / 2.0;
            }
      // if on string, we are done; if between strings, raise by half space
      if(!_onLines)
            _fretYOffset -= lineDistance().val()*DPI*SPATIUM20 / 2.0;

      // from _fretYOffset, compute _charBoxH and _charBoxY
      _charBoxH = bb.height();
      _charBoxY = bb.y()  + _fretYOffset;

      // keep track of the conditions under which metrics have been computed
      _metricsValid = true;
      _refDPI = DPI;
}

//---------------------------------------------------------
//   TabDurationSymbol
//---------------------------------------------------------

TabDurationSymbol::TabDurationSymbol(Score* s)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      setGenerated(true);
      _tab  = 0;
      _text = QString();
      }

static QChar g_cDurationChars[] = { 0xE0FF, 0xE100, 0xE101, 0xE102, 0xE103, 0xE104,
//                                   Longa  Brevis   Whole   Half   Quarter  1/8
                                    0xE105, 0xE106, 0xE107, 0xE108, 0xE109, 0xE10B, ' ', ' '};
//                                   1\16    1\32    1\64    1\128   1\256   dot

TabDurationSymbol::TabDurationSymbol(Score* s, StaffTypeTablature * tab, Duration::DurationType type, int dots)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      setGenerated(true);
      _tab  = tab;
      // text string is a main symbol plus as many dots as required by chord duration
      _text = QString(g_cDurationChars[type]);
      for(int count=0; count < dots; count++)
            _text.append(g_cDurationChars[11]);
      }

TabDurationSymbol::TabDurationSymbol(const TabDurationSymbol& e)
   : Element(e)
      {
      _tab = e._tab;
      _text = e._text;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TabDurationSymbol::draw(QPainter& p, ScoreView*) const
      {
      if(!_tab)
            return;
      double mag = magS();
      double imag = 1.0 / mag;

      p.scale(mag, mag);
      p.setFont(_tab->durationFont());
      p.drawText(0.0, _tab->durationFontYOffset() * spatium(), _text);
      p.scale(imag, imag);
      }