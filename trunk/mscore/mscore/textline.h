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
      void collectElements(QList<const Element*>& el) const;
      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void layout(ScoreLayout*);
      TextC* text() const { return _text; }

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(const QString&);
      };

//---------------------------------------------------------
//   TextLine
//    brackets
//---------------------------------------------------------

class TextLine : public SLine {
      Spatium _hookHeight;
      Spatium _lineWidth;
      Qt::PenStyle _lineStyle;
      bool _hookUp;
      bool _hook;
      QColor _lineColor;
      bool _hasText;
      int _mxmlOff2;

   protected:
      TextBase* _text;
      friend class TextLineSegment;

   public:
      TextLine(Score* s);
      TextLine(const TextLine&);
      ~TextLine();
      virtual TextLine* clone() const      { return new TextLine(*this); }
      virtual ElementType type() const     { return TEXTLINE; }
      virtual void layout(ScoreLayout*);
      virtual LineSegment* createLineSegment();
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);

      void setText(const QString& s)       { _text->setText(s, 0);    }
      QString text() const                 { return _text->getText(); }
      void setHtml(const QString& s)       { _text->setHtml(s);       }
      QString getHtml() const              { return _text->getHtml(); }
      TextBase** textBasePtr()             { return &_text;           }
      TextBase*  textBase()                { return _text;            }
      void setTextBase(TextBase* b)        { _text = b;               }
      Spatium hookHeight() const           { return _hookHeight;      }
      void setHookHeight(const Spatium& v) { _hookHeight = v;         }
      Spatium lineWidth() const            { return _lineWidth;       }
      void setLineWidth(const Spatium& v)  { _lineWidth = v;          }
      Qt::PenStyle lineStyle() const       { return _lineStyle;       }
      void setLineStyle(Qt::PenStyle v)    { _lineStyle = v;          }
      bool hookUp() const                  { return _hookUp;          }
      void setHookUp(bool v)               { _hookUp = v;             }
      QColor lineColor() const             { return _lineColor;       }
      void setLineColor(const QColor& c)   { _lineColor = c;          }
      bool hook() const                    { return _hook;            }
      void setHook(bool v)                 { _hook = v;               }
      bool hasText() const                 { return _hasText;         }
      void setHasText(bool v)              { _hasText = v;            }
      int mxmlOff2() const                 { return _mxmlOff2;        }
      void setMxmlOff2(int o)              { _mxmlOff2 = o;           }
      };

//---------------------------------------------------------
//   LineProperties
//---------------------------------------------------------

class LineProperties : public QDialog, public Ui::LinePropertiesDialog {
      Q_OBJECT

      TextLine* tl;

   public slots:
      virtual void accept();

   public:
      LineProperties(TextLine*, QWidget* parent = 0);
      };

#endif

