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

#ifndef __TABLATURE_H__
#define __TABLATURE_H__

#include "text.h"
#include "xml.h"

//---------------------------------------------------------
//   Tablature
//---------------------------------------------------------

class Tablature {
      QList<int> stringTable;
      int _frets;
	  QString	_durationFontName;		// the name of the font used for duration symbols
	  double	_durationFontSize;		// the size (in points) for the duration symbol font
	  double	_durationFontY;			// the vertical offset (in sp. units) for the duration symb. font
	  QString	_fretFontName;			// the name of the font used for fret marks
	  double	_fretFontSize;			// the size (in points) for the fret marks font
	  double	_fretY;					// additional vert. offset of fret marks with respect to the string (sp. unit)
										// user configurable
	  bool		_linesThrough;			// whether lines for strings and stems may pass through fret marks or not
	  bool		_onStrings;				// whether fret marks are drawn on the strings or between them
	  bool		_showClef;				// whether clef (i.e. "TAB") is drawn ir not
	  bool		_showStems;				// whether note stems (and beams) are drawn or not
	  bool		_showTimeSig;			// whether time signatures are drawn or not
//	  double	_stringSpacing;			// the distance between string lines (in spatium unit)
	  bool		_useNumbers;			// true: use numbers ('0' - ...) for frets | false: use letters ('a' - ...)

	  double	_charBoxH, _charBoxY;	// the height and the y rect.coord. of a box bounding all fret characters
										// internally computed: depends upon _onString and _useNumbers and the
										// metrics of the fret font (sp. units)
	  TextStyle _durationTextStyle;		// a pre-computed text style to be used for duration symbols
	  double	_fretYOffset;			// the vertical offset to draw fret marks with the respect to the string
										// internally computed: depends upon _onString and _useNumbers and the
										// metrics of the fret font (sp. units)
	  bool		_metricsValid;			// whether metrics are valid or not
	  qreal		_refDPI, _refSpatium;	// reference values used to last compute metrics and to see if they are still valid

   public:
	  Tablature();
      Tablature(int numFrets, int numStrings, int strings[]);
      bool convertPitch(int pitch, int* string, int* fret) const;
      int fret(int pitch, int string) const;
      int getPitch(int string, int fret) const;
      int strings() const { return stringTable.size(); }
      int frets() const   { return _frets; }
      void read(QDomElement);
      void write(Xml&) const;
	  // properties getters (some getters may require to update the metrics)
			double	charBoxH(double spatium)	{ setMetrics(spatium); return _charBoxH; }
			double	charBoxY(double spatium)	{ setMetrics(spatium); return _charBoxY + _fretY; }
	  const	QString	durationFontName() const	{ return _durationFontName; }
			double	durarionFontSize() const	{ return _durationFontSize; }
			double	durationFontY() const		{ return _durationFontY; }
			Text *	durationTextElement(QString text);
	  const	TextStyle* durationTextStyle() const	{ return &_durationTextStyle; }
	  const	QString	fretFontName() const		{ return _fretFontName; }
			double	fretFontSize() const		{ return _fretFontSize; }
			double	fretY(double spatium)		{ setMetrics(spatium); return _fretYOffset + _fretY; }
			bool	linesThrough() const		{ return _linesThrough; }
			bool	onStrings() const			{ return _onStrings; }
			bool	showClef() const			{ return _showClef; }
			bool	showStems()					{ return _showStems; }
			bool	showTimeSig() const			{ return _showTimeSig; }
//			double	stringSpacing() const		{ return _stringSpacing; }
			bool	useNumbers() const			{ return _useNumbers; }
	  // properties setters (setting some props invalidates metrics)
			void	setDurationFontName(QString name);
			void	setDurationFontSize(double val);
			void	setDurationFontY(double val);
			void	setFretFontName(QString name){ _fretFontName = name; _metricsValid = false; }
			void	setFretFontSize(double val)	{ _fretFontSize = val; _metricsValid = false; }
			void	setFretY(double val)		{ _fretY = val; }
			void	setLinesThrough(bool val)	{ _linesThrough = val; }
			void	setOnStrings(bool val);
			void	setShowClef(bool val)		{ _showClef = val; }
			void	setShowStems(bool val)		{ _showStems = val; }
			void	setShowTimeSig(bool val)	{ _showTimeSig = val; }
//			void	setStringSpacing(double val){ _stringSpacing = val; }
			void	setUseNumbers(bool val)		{ _useNumbers = val; _metricsValid = false; }

  protected:
	  void		setMetrics(double spatium);
	  };

extern Tablature guitarTablature;
#endif

