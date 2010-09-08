//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2009 Werner Schweer and others
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

#include "mscore.h"
#include "preferences.h"
#include "voiceselector.h"

//---------------------------------------------------------
//   VoiceButton
//---------------------------------------------------------

VoiceButton::VoiceButton(int v, QWidget* parent)
   : QToolButton(parent)
      {
      voice = v;
      setToolButtonStyle(Qt::ToolButtonIconOnly);
      setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void VoiceButton::paintEvent(QPaintEvent* e)
      {
      QPainter p(this);
      QColor c(preferences.selectColor[voice]);
      p.fillRect(e->rect(), isChecked() ? c.light(170) : c.light(100));
      p.setPen(QPen(palette().color(QPalette::Normal, QPalette::Text)));
      if (isChecked())
            p.drawRect(0, 0, width()-1, height()-1);
      QFont f = font();
      f.setPixelSize(height() * 3 / 4);
      p.setFont(f);
      p.drawText(rect(), Qt::AlignCenter, QString("%1").arg(voice+1));
      }

//---------------------------------------------------------
//   VoiceSelector
//---------------------------------------------------------

VoiceSelector::VoiceSelector(QWidget* parent)
   : QWidget(parent)
      {
      QGridLayout* vwl = new QGridLayout;
      vwl->setSpacing(0);
      vwl->setContentsMargins(0, 0, 0, 0);

      QStringList sl2;
      sl2 << "voice-1" << "voice-3" << "voice-2" << "voice-4";
      int v[4] = { 0, 2, 1, 3 };
      QActionGroup* vag = new QActionGroup(this);
      vag->setExclusive(true);
      int i = 0;
      foreach(const QString& s, sl2) {
            QAction* a = getAction(s.toLatin1().data());
            a->setCheckable(true);
            vag->addAction(a);
            VoiceButton* tb = new VoiceButton(v[i]);
            tb->setDefaultAction(a);
            vwl->addWidget(tb, i/2, i%2, 1, 1);
            ++i;
            }
      setLayout(vwl);
//      setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
      connect(vag, SIGNAL(triggered(QAction*)), this, SIGNAL(triggered(QAction*)));
      }

