//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: icons.cpp,v 1.16 2006/03/02 17:08:34 wschweer Exp $
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

#include "globals.h"
#include "icons.h"
#include "score.h"
#include "painter.h"
#include "style.h"
#include "preferences.h"

#include "data/flip.xpm"

QIcon noteIcon;
QIcon note2Icon;
QIcon note4Icon;
QIcon note8Icon;
QIcon note16Icon;
QIcon note32Icon;
QIcon note64Icon;

QIcon naturalIcon;
QIcon sharpIcon;
QIcon sharpsharpIcon;
QIcon flatIcon;
QIcon flatflatIcon;
QIcon quartrestIcon;
QIcon dotIcon;
QIcon sforzatoaccentIcon;
QIcon staccatoIcon;
QIcon tenutoIcon;
QIcon plusIcon;
QIcon flipIcon;
QIcon voiceIcons[VOICES];

//---------------------------------------------------------
//   notePixmap
//---------------------------------------------------------

static QPixmap notePixmap(int ticks, const QColor& fg, int width, int height)
      {
      qreal ospatium = _spatium;
      _spatium = 6;

      TextStyle* s = &textStyles[TEXT_STYLE_SYMBOL1];
      QFont f(s->family);
      f.setPixelSize(lrint(20.0 / 5 * _spatium));

      QPixmap pm(width, height);

      Painter painter(&pm);
      painter.setFont(f);

      pm.fill(Qt::white);

      QPen pen(fg);
      qreal stemWidth = point(style->stemWidth);
      pen.setWidthF(stemWidth);
      painter.setPen(pen);

      QChar code;
      switch(ticks) {
            case division*4:
                  code = 0xe11b;
                  break;
            case division*2:
                  code = 0xe11c;
                  break;
            default:
                  code = 0xe11d;
                  break;
            }
      QFontMetricsF fm(f);
      QRectF bb(fm.boundingRect(code));

      qreal w   = bb.width();
      qreal h   = bb.height();
      qreal x   = 2;
      qreal y   = height - h - bb.y();

      painter.drawText(QPointF(x, y), QString(code));

      if (ticks < division*4) {
            x += w - stemWidth * .5;;
            QPointF p(x, 0.0);
            painter.drawLine(QLineF(x, y-1, x, 0));
            y = 0;
            switch (ticks) {
                  case division/16:
                        painter.drawText(p, QString(0xe182));
                        break;
                  case division/8:
                        painter.drawText(p, QString(0xe181));
                        break;
                  case division/4:
                        painter.drawText(p, QString(0xe180));
                        break;
                  case division/2:
                        painter.drawText(p, QString(0xe17f));
                        break;
                  }
            }
      painter.end();
      pm.setMask(pm.createHeuristicMask());
      _spatium = ospatium;
      return pm;
      }

//---------------------------------------------------------
//   noteIconSet
//---------------------------------------------------------

static QIcon noteIconSet(int ticks)
      {
      return QIcon(notePixmap(ticks, Qt::black, ICON_WIDTH, ICON_HEIGHT));
      }

//---------------------------------------------------------
//   symPixmap
//---------------------------------------------------------

static QPixmap symPixmap(const Sym& sym, int width, int height)
      {
      TextStyle* s = &textStyles[TEXT_STYLE_SYMBOL1];
      QFont f(s->family, 14);

      QPixmap pm(width, height);
      Painter painter(&pm);
      painter.setFont(f);

      pm.fill(Qt::white);

      QFontMetricsF fm(f);
      QRectF bb(fm.boundingRect(sym.code()));
      qreal x = (width - bb.width()) / 2.0;
      qreal y = (height  - bb.height()) / 2.0;

      painter.drawText(QPointF(x - bb.x(), y - bb.y()), QString(sym.code()));

      painter.end();
      pm.setMask(pm.createMaskFromColor(Qt::white));
      return pm;
      }

//---------------------------------------------------------
//   symIcon
//---------------------------------------------------------

static QIcon symIcon(const Sym& sym)
      {
      return QIcon(symPixmap(sym, ICON_WIDTH, ICON_HEIGHT));
      }

//---------------------------------------------------------
//   genIcons
//    create some icon sets
//---------------------------------------------------------

void genIcons()
      {
      noteIcon           = noteIconSet(division * 4);
      note2Icon          = noteIconSet(division * 2);
      note4Icon          = noteIconSet(division);
      note8Icon          = noteIconSet(division/2);
      note16Icon         = noteIconSet(division/4);
      note32Icon         = noteIconSet(division/8);
      note64Icon         = noteIconSet(division/16);

      naturalIcon        = symIcon(naturalSym);
      sharpIcon          = symIcon(sharpSym);
      sharpsharpIcon     = symIcon(sharpsharpSym);
      flatIcon           = symIcon(flatSym);
      flatflatIcon       = symIcon(flatflatSym);
      quartrestIcon      = symIcon(quartrestSym);
      dotIcon            = symIcon(dotSym);
      sforzatoaccentIcon = symIcon(sforzatoaccentSym);
      staccatoIcon       = symIcon(staccatoSym);
      tenutoIcon         = symIcon(tenutoSym);
      plusIcon           = symIcon(plusSym);

      char* vtext[VOICES] = { "1","2","3","4" };
      for (int i = 0; i < VOICES; ++i) {
            QPixmap image(ICON_WIDTH, ICON_HEIGHT);
            QColor c(preferences.selectColor[i].light(180));
            image.fill(c);
            QPainter painter(&image);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(QPen(Qt::black));
            painter.drawText(QRect(0, 0, ICON_WIDTH, ICON_HEIGHT), Qt::AlignCenter, vtext[i]);
            painter.end();
            voiceIcons[i].addPixmap(image);

            painter.begin(&image);
            c = QColor(preferences.selectColor[i].light(140));
            painter.fillRect(0, 0, ICON_WIDTH, ICON_HEIGHT, c);
            painter.setPen(QPen(Qt::black));
            painter.drawText(QRect(0, 0, ICON_WIDTH, ICON_HEIGHT), Qt::AlignCenter, vtext[i]);
            painter.end();
            voiceIcons[i].addPixmap(image, QIcon::Normal, QIcon::On);
            }
      flipIcon.addPixmap(flip_xpm);
      }

