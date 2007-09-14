//=============================================================================
//  Awl
//  Audio Widget Library
//  $Id:$
//
//  Copyright (C) 2002-2007 by Werner Schweer and others
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

#ifndef __AWLCOLORLABEL_H__
#define __AWLCOLORLABEL_H__

namespace Awl {

      //---------------------------------------------------------
      //   ColorLabel
      //---------------------------------------------------------

      class ColorLabel : public QFrame {
            Q_OBJECT
            QColor _color;

            virtual void paintEvent(QPaintEvent*);

         public:
            ColorLabel(QWidget* parent = 0) : QFrame (parent) {
                  _color = Qt::blue;
                  }
            void setColor(const QColor& c) { _color = c; update(); }
            QColor color() const           { return _color; }
            virtual QSize sizeHint() const { return QSize(50, 20); }
            };

      }  // namespace Awl
#endif

