//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
//  Copyright (C) 2003 Mathias Lundgren (lunar_shuttle@users.sourceforge.net)
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

#include "shortcutcapturedialog.h"
#include "musescore.h"

//---------------------------------------------------------
//   ShortcutCaptureDialog
//---------------------------------------------------------

ShortcutCaptureDialog::ShortcutCaptureDialog(Shortcut* _s, QMap<QString, Shortcut*> ls, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      localShortcuts = ls;
      s = _s;

      oshrtLabel->setText(s->key.toString(QKeySequence::NativeText));
      connect(clearButton, SIGNAL(clicked()), SLOT(clearClicked()));
      clearClicked();
      grabKeyboard();
      }

//---------------------------------------------------------
//   ShortcutCaptureDialog
//---------------------------------------------------------

ShortcutCaptureDialog::~ShortcutCaptureDialog()
      {
      releaseKeyboard();
      }

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void ShortcutCaptureDialog::keyPressEvent(QKeyEvent* e)
      {
      if (key.count() >= 4)
            return;
      int k = e->key();
      if (k == 0 || k == Qt::Key_Shift || k == Qt::Key_Control ||
         k == Qt::Key_Meta || k == Qt::Key_Alt || k == Qt::Key_AltGr
         || k == Qt::Key_CapsLock || k == Qt::Key_NumLock
         || k == Qt::Key_ScrollLock || k == Qt::Key_unknown)
            return;

      k += e->modifiers();
      switch(key.count()) {
            case 0: key = QKeySequence(k); break;
            case 1: key = QKeySequence(key[0], k); break;
            case 2: key = QKeySequence(key[0], key[1], k); break;
            case 3: key = QKeySequence(key[0], key[1], key[2], k); break;
            default:
                  qDebug("internal error: bad key count\n");
                  break;
            }

      // Check against conflicting shortcuts
      bool conflict = false;
      QString msgString;

      foreach (Shortcut* ss, localShortcuts) {
            if ((s != ss) && (ss->key == key)) {
                  msgString = tr("Shortcut conflicts with ") + ss->descr;
                  conflict = true;
                  break;
                  }
            }
      messageLabel->setText(msgString);
      okButton->setEnabled(conflict == false);
      nshrtLabel->setText(key.toString(QKeySequence::NativeText));

qDebug("capture key 0x%x  modifiers 0x%x virt 0x%x scan 0x%x <%s><%s>\n",
      k,
      int(e->modifiers()),
      int(e->nativeVirtualKey()),
      int(e->nativeScanCode()),
      qPrintable(key.toString(QKeySequence::NativeText)),
      qPrintable(key.toString(QKeySequence::PortableText))
      );

      }

//---------------------------------------------------------
//   clearClicked
//---------------------------------------------------------

void ShortcutCaptureDialog::clearClicked()
      {
      nshrtLabel->setText(tr("Undefined"));
      key = 0;
      }

