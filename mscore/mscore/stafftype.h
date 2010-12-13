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

#ifndef __STAFFTYPE_H__
#define __STAFFTYPE_H__

#include "element.h"
#include "spatium.h"
#include "globals.h"
#include "durationtype.h"

//class Instrument;
class Staff;
class Xml;

//---------------------------------------------------------
//   StaffType
//---------------------------------------------------------

class StaffType {

   protected:
      bool _modified;         // if true, this StaffType belongs to Score(),
                              // otherwise it is a global build in
      QString _name;
      uchar _lines;
      Spatium _lineDistance;
      bool _genClef;          // create clef at beginning of system
      bool _genKeysig;        // create key signature at beginning of system
      bool _slashStyle;       // do not show stems
      bool _showBarlines;
      bool _showLedgerLines;

   public:
      StaffType();
      StaffType(const QString& s);
      QString name() const                     { return _name;            }
      void setName(const QString& val)         { _name = val;             }
      virtual StaffGroup group() const = 0;
      virtual StaffType* clone() const = 0;
      virtual const char* groupName() const = 0;
      void setLines(int val)                   { _lines = val;            }
      int lines() const                        { return _lines;           }
      void setLineDistance(const Spatium& val) { _lineDistance = val;     }
      Spatium lineDistance() const             { return _lineDistance;    }
      void setGenClef(bool val)                { _genClef = val;          }
      bool genClef() const                     { return _genClef;         }
      void setGenKeysig(bool val)              { _genKeysig = val;        }
      bool genKeysig() const                   { return _genKeysig;       }
      void setSlashStyle(bool val)             { _slashStyle = val;       }
      bool slashStyle() const                  { return _slashStyle;      }
      void setShowBarlines(bool val)           { _showBarlines = val;     }
      bool showBarlines() const                { return _showBarlines;    }
      void setShowLedgerLines(bool val)        { _showLedgerLines = val;  }
      bool showLedgerLines() const             { return _showLedgerLines; }
      bool modified() const                    { return _modified;        }
      void setModified(bool val)               { _modified = val;         }
      virtual void write(Xml& xml, int) const;
      void writeProperties(Xml& xml) const;
      virtual void read(QDomElement);
      bool readProperties(QDomElement e);
      };

// first three staff types in staffTypes[] are build in:

enum {
      PITCHED_STAFF_TYPE, TAB_STAFF_TYPE, PERCUSSION_STAFF_TYPE
      };

//---------------------------------------------------------
//   StaffTypePitched
//---------------------------------------------------------

class StaffTypePitched : public StaffType {

   public:
      StaffTypePitched() : StaffType() {}
      StaffTypePitched(const QString& s) : StaffType(s) {}
      virtual StaffGroup group() const        { return PITCHED_STAFF; }
      virtual StaffTypePitched* clone() const { return new StaffTypePitched(*this); }
      virtual const char* groupName() const   { return "pitched"; }
      };

//---------------------------------------------------------
//   StaffTypeTablature
//---------------------------------------------------------

class StaffTypeTablature : public StaffType {

   protected:
      // configurable properties
//      Instrument* _instrument;            // to access the underlying string data
      QString     _durationFontName;      // the name of the font used for duration symbols
      double      _durationFontSize;      // the size (in points) for the duration symbol font
      double      _durationFontUserY;     // the vertical offset (spatium units) for the duration symb. font
                                          // user configurable
      QString     _fretFontName;          // the name of the font used for fret marks
      double      _fretFontSize;          // the size (in points) for the fret marks font
      double      _fretFontUserY;         // additional vert. offset of fret marks with respect to
                                          // the string line (spatium unit); user configurable
      bool        _genDurations;          // whether duration symbols are drawn or not
      bool        _genTimesig;            // whether time signature is shown or not
      bool        _linesThrough;          // whether lines for strings and stems may pass through fret marks or not
      bool        _onLines;               // whether fret marks are drawn on the string lines or between them
      bool        _useNumbers;            // true: use numbers ('0' - ...) for frets | false: use letters ('a' - ...)

      // internally managed variables
      double      _durationBoxH, _durationBoxY; // the height and the y rect.coord. (relative to staff top line)
                                          // of a box bounding all duration symbols (raster units) internally computed:
                                          // depends upon _onString and the metrics of the duration font
      QFont       _durationFont;          // font used to draw dur. symbols; cached for efficiency
      double      _durationYOffset;       // the vertical offset to draw duration symbols with respect to the
                                          // string lines (raster units); internally computed: depends upon _onString
      bool        _durationMetricsValid;  // whether duration font metrics are valid or not
      double      _fretBoxH, _fretBoxY;   // the height and the y rect.coord. (relative to staff line)
                                          // of a box bounding all fret characters (raster units) internally computed:
                                          // depends upon _onString, _useNumbers and the metrics of the fret font
      QFont       _fretFont;              // font used to draw fret marks; cached for efficiency
      double      _fretYOffset;           // the vertical offset to draw fret marks with respect to the string lines;
                                          // (raster units); internally computed: depends upon _onString, _useNumbers
                                          // and the metrics of the fret font
      bool        _fretMetricsValid;      // whether fret font metrics are valid or not
      qreal       _refDPI;                // reference value used to last compute metrics and to see if they are still valid

      void init();                        // init to reasonable defaults

   public:
      StaffTypeTablature() : StaffType() { init(); }
      StaffTypeTablature(const QString& s) : StaffType(s) { init(); }
      virtual StaffGroup group() const          { return TAB_STAFF; }
      virtual StaffTypeTablature* clone() const { return new StaffTypeTablature(*this); }
      virtual const char* groupName() const     { return "tablature"; }
      virtual void read(QDomElement e);
      virtual void write(Xml& xml, int) const;

      // properties getters (some getters require updated metrics)
      double  durationBoxH()              { if(!_genDurations && !_slashStyle) return 0.0;
                                            setDurationMetrics(); return _durationBoxH; }
      double  durationBoxY()              { if(!_genDurations && !_slashStyle) return 0.0;
                                            setDurationMetrics(); return _durationBoxY + _durationFontUserY; }
const QFont&  durationFont()              { return _durationFont;     }
const	QString durationFontName() const    { return _durationFontName; }
      double  durationFontSize() const    { return _durationFontSize; }
      double  durationFontUserY() const   { return _durationFontUserY;}
      double  durationFontYOffset()       { setDurationMetrics(); return _durationYOffset + _durationFontUserY; }
      double  fretBoxH()                  { setFretMetrics(); return _fretBoxH; }
      double  fretBoxY()                  { setFretMetrics(); return _fretBoxY + _fretFontUserY; }
const QFont&  fretFont()                  { return _fretFont;         }
const QString fretFontName() const        { return _fretFontName;     }
      double  fretFontSize() const        { return _fretFontSize;     }
      double  fretFontUserY() const       { return _fretFontUserY;    }
      double  fretFontYOffset()           { setFretMetrics(); return _fretYOffset + _fretFontUserY; }
      bool    genDurations() const        { return _genDurations;     }
      bool    genTimesig() const          { return _genTimesig;       }
      bool    linesThrough() const        { return _linesThrough;     }
      bool    onLines() const             { return _onLines;          }
      bool    useNumbers() const          { return _useNumbers;       }
      // properties setters (setting some props invalidates metrics)
      void    setDurationFontName(QString name) { _durationFontName = name;
                                                  _durationFont.setFamily(name);
                                                  _durationMetricsValid = false; }
      void    setDurationFontSize(double val)   { _durationFontSize = val;
                                                  _durationFont.setPointSizeF(val);
                                                  _durationMetricsValid = false; }
      void    setDurationFontUserY(double val)  { _durationFontUserY = val; }
      void    setFretFontName(QString name)     { _fretFontName = name;
                                                  _fretFont.setFamily(name);
                                                  _fretMetricsValid = false; }
      void    setFretFontSize(double val)       { _fretFontSize = val;
                                                  _fretFont.setPointSizeF(val);
                                                  _fretMetricsValid = false; }
      void    setFretFontUserY(double val){ _fretFontUserY = val;     }
      void    setGenDurations(bool val)   { _genDurations = val;      }
      void    setGenTimesig(bool val)     { _genTimesig = val;        }
      void    setLinesThrough(bool val)   { _linesThrough = val;      }
      void    setOnLines(bool val);
      void    setUseNumbers(bool val)     { _useNumbers = val; _fretMetricsValid = false; }

protected:
      void    setDurationMetrics();
      void    setFretMetrics();
      };

//---------------------------------------------------------
//   StaffTypePercussion
//---------------------------------------------------------

class StaffTypePercussion : public StaffType {

   public:
      StaffTypePercussion() : StaffType() {}
      StaffTypePercussion(const QString& s) : StaffType(s) {}
      virtual StaffGroup group() const           { return PERCUSSION_STAFF; }
      virtual StaffTypePercussion* clone() const { return new StaffTypePercussion(*this); }
      virtual const char* groupName() const      { return "percussion"; }
      };

extern void initStaffTypes();
extern QList<StaffType*> staffTypes;

class ScoreView;

//---------------------------------------------------------
//   TabDurationSymbol
//    Element used to draw duration symbols above tablatures
//---------------------------------------------------------

class TabDurationSymbol : public Element {

      StaffTypeTablature *    _tab;
      QString                 _text;

public:
      TabDurationSymbol(Score* s);
      TabDurationSymbol(Score* s, StaffTypeTablature * tab, Duration::DurationType type, int dots);
      TabDurationSymbol(const TabDurationSymbol&);
      virtual TabDurationSymbol* clone() const  { return new TabDurationSymbol(*this); }
      virtual void draw(QPainter&, ScoreView*) const;
      virtual bool isEditable() const           { return false; }
      virtual ElementType type() const          { return TAB_DURATION_SYMBOL; }

//      virtual QPointF canvasPos() const;  ///< position in canvas coordinates
//      virtual void write(Xml& xml) const;
//      virtual void read(QDomElement);
//      void layout();

      void setTablature(StaffTypeTablature * tab)     { _tab = tab; }
      void setText(QString s)                         { _text = s; }
      };

#endif
