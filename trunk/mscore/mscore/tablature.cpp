//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "tablature.h"

#define TAB_DEFAULT_DUR_YOFFS	(-1.75)
#define TAB_DEFAULT_LINE_SP		(1.5)

static int guitarStrings[6] = { 40, 45, 50, 55, 59, 64 };

Tablature guitarTablature(13, 6, guitarStrings);

//---------------------------------------------------------
//   Tablature
//---------------------------------------------------------


Tablature::Tablature()
	  {
	  // set reasonable defaults
	  _frets = 0;
	  setDurationFontName("FreeSans");
	  setDurationFontSize(10.0);
	  setDurationFontY(0.0);
	  _fretFontName		= "FreeSans";
	  _fretFontSize		= 9.0;
	  _fretY			= 0.0;
	  _linesThrough		= false;
	  setOnStrings(true);
	  _showClef			= true;
	  _showStems		= true;
	  _showTimeSig		= false;
//	  _stringSpacing	= 1.5;
	  _useNumbers		= true;

	  _metricsValid = false;
	  _charBoxH = _charBoxY = _fretYOffset = _refDPI = _refSpatium = 0.0;
  }


Tablature::Tablature(int numFrets, int numStrings, int strings[])
      {
      _frets = numFrets;
      for (int i = 0; i < numStrings; ++i)
            stringTable.append(strings[i]);
	  // set reasonable defaults
	  setDurationFontName("FreeSans");
	  setDurationFontSize(10.0);
	  setDurationFontY(0.0);
	  _fretFontName		= "FreeSans";
	  _fretFontSize		= 9.0;
	  _fretY			= 0.0;
	  _linesThrough		= false;
	  setOnStrings(true);
	  _showClef			= true;
	  _showStems		= true;
	  _showTimeSig		= false;
//	  _stringSpacing	= 1.5;
	  _useNumbers		= true;

	  _metricsValid = false;
	  _charBoxH = _charBoxY = _fretYOffset = _refDPI = _refSpatium = 0.0;
  }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tablature::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            int v = e.text().toInt();
			double	val = e.text().toDouble();
            if (tag == "frets")
                  _frets = v;
            else if (tag == "string")
                  stringTable.append(v);
			else if(tag == "durationFontName")
				setDurationFontName(e.text());
			else if(tag == "durationFontSize")
				setDurationFontSize(val);
			else if(tag == "durationFontY")
				setDurationFontY(val);
			else if(tag == "fretFontName")
				setFretFontName(e.text());
			else if(tag == "fretFontSize")
				setFretFontSize(val);
			else if(tag == "fretY")
				setFretY(val);
			else if(tag == "linesThrough")
				setLinesThrough(v != 0);
			else if(tag == "onStrings")
				setOnStrings(v != 0);
			else if(tag == "showClef")
				setShowClef(v != 0);
			else if(tag == "showStems")
				setShowStems(v != 0);
			else if(tag == "showTimeSig")
				setShowTimeSig(v != 0);
//			else if(tag == "stringSpacing")
//				setStringSpacing(val);
			else if(tag == "useNumbers")
				setUseNumbers(v != 0);
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tablature::write(Xml& xml) const
      {
      xml.stag("Tablature");
      xml.tag("frets", _frets);
      foreach(int pitch, stringTable)
            xml.tag("string", pitch);
	  xml.tag("durationFontName", _durationFontName);
	  xml.tag("durationFontSize", _durationFontSize);
	  xml.tag("durationFontY", _durationFontY);
	  xml.tag("fretFontName", _fretFontName);
	  xml.tag("fretFontSize", _fretFontSize);
	  xml.tag("fretY", _fretY);
	  xml.tag("linesThrough", _linesThrough);
	  xml.tag("onStrings", _onStrings);
	  xml.tag("showClef", _showClef);
	  xml.tag("showStems", _showStems);
	  xml.tag("showTimeSig", _showTimeSig);
//	  xml.tag("stringSpacing", _stringSpacing);
	  xml.tag("useNumbers", _useNumbers);
	  xml.etag();
      }

//---------------------------------------------------------
//   convertPitch
//---------------------------------------------------------

bool Tablature::convertPitch(int pitch, int* string, int* fret) const
      {
      int strings = stringTable.size();

      for (int i = 0; i < strings; ++i) {
            int min = stringTable[i];
            int max;
            if (i + 1 == strings)
                  max = min + _frets;
            else
                  max = stringTable[i+1] - 1;

            if (pitch >= min && pitch <= max) {
                  *string = strings - i - 1;
                  *fret   = pitch - min;
                  return true;
                  }
            }
      *string = 0;
      *fret   = 0;
      return false;
      }

//---------------------------------------------------------
//   getPitch
//---------------------------------------------------------

int Tablature::getPitch(int string, int fret) const
      {
      int strings = stringTable.size();
      return stringTable[strings - string - 1] + fret;
      }

//---------------------------------------------------------
//   fret
//    return fret for given pitch and string
//    return -1 if not possible
//---------------------------------------------------------

int Tablature::fret(int pitch, int string) const
      {
      int strings = stringTable.size();

      if (string < 0 || string >= strings)
            return -1;
      int fret = pitch - stringTable[strings - string - 1];
      if (fret < 0 || fret >= _frets)
            return -1;
      return fret;
      }

//---------------------------------------------------------
//   duration font properties
//---------------------------------------------------------

void Tablature::setDurationFontName(QString name)
{
	_durationTextStyle.setFamily(name);
	_durationFontName = name;
}
void Tablature::setDurationFontSize(double val)
{
	_durationTextStyle.setSize(val);
	_durationFontSize = val;
}
void Tablature::setDurationFontY(double val)
{
	_durationTextStyle.setYoff(TAB_DEFAULT_DUR_YOFFS - (_onStrings ? 0.0 : TAB_DEFAULT_LINE_SP/2.0) + val);
	_durationFontY = val;
}
void Tablature::setOnStrings(bool val)
{
	_onStrings = val;
	_metricsValid = false;
	setDurationFontY(durationFontY());
}

//---------------------------------------------------------
//   durationTextElement
//    return a new Text element already set to the proper duration style. The caller only needs to set
//		the required text and dispose of the element once done.
//---------------------------------------------------------

Text * Tablature::durationTextElement(QString text)
{
	return 0;
}

//---------------------------------------------------------
//   setMetrics
//    checks whether the internally computed metrics are is still valid and re-computes them, if not
//---------------------------------------------------------

static QString	g_strNumbers("0123456789");
static QString	g_strLetters("abcdefghiklmnopq");

void Tablature::setMetrics(double spatium)
{//	int				ascent;
	int				size;

	if(_metricsValid && _refDPI == DPI && _refSpatium == spatium)
		return;

	QFont f(_fretFontName);
	size = lrint(_fretFontSize * DPI / PPI);
	f.setPixelSize(size);
	QFontMetricsF fm(f);
	QRectF bb(fm.tightBoundingRect(_useNumbers ? g_strNumbers : g_strLetters));
//	QRectF bx(fm.tightBoundingRect("a");
//	ascent = fm.ascent();
//	// with numbers, the text is aligned at half the ascent
//	// with letters, it is aligned at half the x height
//	int	xHeight = fm.xHeight();
//	_fretYOffset = (_useNumbers ? ascent : xHeight) / 2.0 / spatium;
	if(_useNumbers)
	{	// for numbers: move down by the whole part above (negative) the base line ( -bb.y() )
		// then up by half the whole height ( -bb.height()/2 )
//		QRectF bb( fm.tightBoundingRect(g_strNumbers) );
		_fretYOffset = -(bb.y() + bb.height()/2.0);
	}
	else
	{	// for letters: centre on the x height, by moving down half of the part above the base line in bx
		QRectF bx( fm.tightBoundingRect("a") );
		_fretYOffset = -bx.y() / 2.0;
	}
	_fretYOffset /= spatium;
	// if on string, we are done; if between strings, raise by half space
	if(!_onStrings)
		_fretYOffset -= 1.5 / 2.0;

	// from _fretYOffset, compute _carBoxH and _charBoxY
	_charBoxH = bb.height() / spatium;
	_charBoxY = bb.y() / spatium + _fretYOffset;

	// keep track of the conditions under which metrics have been computed
	_metricsValid = true;
	_refDPI = DPI;
	_refSpatium = spatium;
}
