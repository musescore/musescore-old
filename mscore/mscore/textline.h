//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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

#ifndef __TEXTLINE_H__
#define __TEXTLINE_H__

#include "line.h"
#include "text.h"
#include "ui_lineproperties.h"

class TextLine;

//---------------------------------------------------------
//   TextLineSegment
//---------------------------------------------------------

class TextLineSegment : public LineSegment {
      TextC* _text;

   protected:

   public:
      TextLineSegment(Score* s);
      TextLineSegment(const TextLineSegment&);
      virtual ElementType type() const       { return TEXTLINE_SEGMENT; }
      virtual TextLineSegment* clone() const { return new TextLineSegment(*this); }
      TextLine* textLine() const             { return (TextLine*)parent(); }
      virtual void draw(QPainter&) const;
      virtual QRectF bbox() const;

      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void layout(ScoreLayout*);
      virtual void setSelected(bool f);

      TextC* text() const { return _text; }

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(const QString&);
      };

//---------------------------------------------------------
//   TextLine
//    brackets
//---------------------------------------------------------

class TextLine : public SLine {
      Spatium _lineWidth;
      QColor _lineColor;
      Qt::PenStyle _lineStyle;

      bool _beginHook, _endHook;
      Spatium _beginHookHeight, _endHookHeight;

      bool _hasBeginText, _hasContinueText;

      int _beginSymbol, _continueSymbol, _endSymbol;  // -1: no symbol
      QPointF _beginSymbolOffset, _continueSymbolOffset, _endSymbolOffset;

      int _mxmlOff2;

   protected:
      TextBase* _beginText;
      TextBase* _continueText;
      friend class TextLineSegment;

   public:
      TextLine(Score* s);
      TextLine(const TextLine&);
      ~TextLine() {}

      virtual TextLine* clone() const      { return new TextLine(*this); }
      virtual ElementType type() const     { return TEXTLINE; }
      virtual LineSegment* createLineSegment();
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);

      bool hasBeginText() const               { return _hasBeginText;         }
      void setHasBeginText(bool v)            { _hasBeginText = v;            }
      bool hasContinueText() const            { return _hasContinueText;      }
      void setHasContinueText(bool v)         { _hasContinueText = v;         }
      void setBeginText(const QString& s)     { _beginText->setText(s, 0);    }
      QString beginText() const               { return _beginText->getText(); }
      bool beginHook() const                  { return _beginHook;            }
      bool endHook() const                    { return _endHook;              }
      void setBeginHook(bool v)               { _beginHook = v;               }
      void setEndHook(bool v)                 { _endHook = v;                 }
      TextBase* beginTextBase() const         { return _beginText;            }
      TextBase* continueTextBase() const      { return _continueText;         }
      TextBase** beginTextBasePtr()           { return &_beginText;           }
      TextBase** continueTextBasePtr()        { return &_continueText;        }
      void setBeginSymbol(int v)              { _beginSymbol = v;             }
      void setContinueSymbol(int v)           { _continueSymbol = v;          }
      void setEndSymbol(int v)                { _endSymbol = v;               }
      void setBeginHookHeight(Spatium v)      { _beginHookHeight = v;         }
      void setEndHookHeight(Spatium v)        { _endHookHeight = v;           }
      Spatium beginHookHeight() const         { return _beginHookHeight;      }
      Spatium endHookHeight() const           { return _endHookHeight;        }

      Spatium lineWidth() const               { return _lineWidth;            }
      QColor lineColor() const                { return _lineColor;            }
      Qt::PenStyle lineStyle() const          { return _lineStyle;            }
      void setLineWidth(const Spatium& v)     { _lineWidth = v;               }
      void setLineColor(const QColor& v)      { _lineColor = v;               }
      void setLineStyle(Qt::PenStyle v)       { _lineStyle = v;               }
      int beginSymbol() const                 { return _beginSymbol;          }
      int continueSymbol() const              { return _continueSymbol;       }
      int endSymbol() const                   { return _endSymbol;            }
      QPointF beginSymbolOffset() const       { return _beginSymbolOffset;    }
      QPointF continueSymbolOffset() const    { return _continueSymbolOffset; }
      QPointF endSymbolOffset() const         { return _endSymbolOffset;      }
      void setBeginSymbolOffset(QPointF v)    { _beginSymbolOffset = v;       }
      void setContinueSymbolOffset(QPointF v) { _continueSymbolOffset = v;    }
      void setEndSymbolOffset(QPointF v)      { _endSymbolOffset = v;         }
      void setMxmlOff2(int v)                 { _mxmlOff2 = v;                }
      int mxmlOff2() const                    { return _mxmlOff2;             }
      };

//---------------------------------------------------------
//   LineProperties
//---------------------------------------------------------

class LineProperties : public QDialog, public Ui::LinePropertiesDialog {
      Q_OBJECT

      TextLine* tl;

   public slots:
      virtual void accept();
      void beginTextClicked();
      void continueTextClicked();
      void beginTextToggled(bool);
      void beginSymbolToggled(bool);
      void continueTextToggled(bool);
      void continueSymbolToggled(bool);

   public:
      LineProperties(TextLine*, QWidget* parent = 0);
      };

#endif

