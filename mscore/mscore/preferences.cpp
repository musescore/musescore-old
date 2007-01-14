//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: preferences.cpp,v 1.36 2006/04/06 13:03:11 wschweer Exp $
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

#include "xml.h"
#include "score.h"
#include "mscore.h"
#include "preferences.h"
#include "synti.h"
#include "seq.h"
#include "note.h"
#include "playpanel.h"
#include "pad.h"
#include "icons.h"
#include "shortcutcapturedialog.h"
#include "canvas.h"

//---------------------------------------------------------
//   buttons2stemDir
//    convert checked button to StemDirection
//---------------------------------------------------------

static Direction buttons2stemDir(QRadioButton *up, QRadioButton *down)
      {
      if (up->isChecked()) {
            return UP;
            }
      else if (down->isChecked()) {
            return DOWN;
            }
      else {
            return AUTO;                // default to "auto"
            }
      }

//---------------------------------------------------------
//   stemDir2button
//    convert StemDirection to checked button
//---------------------------------------------------------

static void stemDir2button(Direction dir, QRadioButton *up, QRadioButton *down, QRadioButton *aut)
      {
      if (dir == UP) {
            up->setChecked(true);
            }
      else if (dir == DOWN) {
            down->setChecked(true);
            }
      else {
            aut->setChecked(true);      // default to "auto"
            }
      }

//---------------------------------------------------------
//   Preferences
//---------------------------------------------------------

Preferences preferences;

Preferences::Preferences()
      {
      selectColor[0] = Qt::blue;
      selectColor[1] = Qt::green;
      selectColor[2] = Qt::yellow;
      selectColor[3] = Qt::magenta;
      dropColor      = Qt::red;

      // set fallback defaults:

      cursorBlink = false;
      fgUseColor  = false;
      bgUseColor  = true;
      fgWallpaper = ":/data/paper2.png";
      enableMidiInput = true;
      playNotes       = true;
      bgColor.setRgb(0x19, 0x6c, 0xb9);
      fgColor.setRgb(50, 50, 50);
      soundFont = "";
      lPort     = "";
      rPort     = "";
      stemDir[0] = AUTO;
      stemDir[1] = AUTO;
      stemDir[2] = AUTO;
      stemDir[3] = AUTO;
      showNavigator = true;
      showPlayPanel = false;
      showPad       = false;
      padPos       = QPoint(100, 100);
      playPanelPos = QPoint(100, 300);
      useAlsaAudio = true;
      useJackAudio = true;
      alsaDevice   = "default";
      // alsaDevice   = "hw:0";
      alsaSampleRate = 48000;
      alsaPeriodSize = 1024;
      alsaFragments  = 3;
      layoutBreakColor = Qt::green;
      antialiasedDrawing = true;
      };

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Preferences::write()
      {
      QFile f(QString("%1/.mscore").arg(QDir::homePath()));
      if (!f.open(QIODevice::WriteOnly)) {
            if (debugMode) {
                  printf("Preferences: open(%s) failed: %s\n",
                     f.fileName().toLatin1().data(), strerror(errno));
                  }
            return;
            }
      Xml xml(&f);
      xml.header("museScore");
      xml.stag("museScore version=\"1.0\"");
      xml.stag("Preferences");
      xml.tag("cursorBlink", cursorBlink);
      if (fgUseColor)
            xml.tag("fgColor", fgColor.name());
      else
            xml.tag("fgWallpaper", fgWallpaper);
      if (bgUseColor)
            xml.tag("bgColor", bgColor.name());
      else
            xml.tag("bgWallpaper", bgWallpaper);
      xml.tag("selectColor1", selectColor[0].name());
      xml.tag("selectColor2", selectColor[1].name());
      xml.tag("selectColor3", selectColor[2].name());
      xml.tag("selectColor4", selectColor[3].name());
      xml.tag("enableMidiInput", enableMidiInput);
      xml.tag("playNotes", playNotes);
      xml.tag("lPort", lPort);
      xml.tag("rPort", rPort);
      if (!soundFont.isEmpty())
            xml.tag("soundFont", soundFont);
      xml.tag("stemDirection1", stemDir[0]);
      xml.tag("stemDirection2", stemDir[1]);
      xml.tag("stemDirection3", stemDir[2]);
      xml.tag("stemDirection4", stemDir[3]);
      xml.tag("showNavigator", showNavigator);

      xml.tag("showPlayPanel", showPlayPanel);
      xml.tagE("PlayPanelPos x=\"%d\" y=\"%d\"",
         playPanelPos.x(), playPanelPos.y());
      xml.tag("showPad", showPad);
      xml.tagE("KeyPadPos x=\"%d\" y=\"%d\"",
         padPos.x(), padPos.y());
      xml.tag("showNavigator", showNavigator);
      xml.tag("useAlsaAudio", useAlsaAudio);
      xml.tag("useJackAudio", useJackAudio);
      xml.tag("alsaDevice", alsaDevice);
      xml.tag("alsaSampleRate", alsaSampleRate);
      xml.tag("alsaPeriodSize", alsaPeriodSize);
      xml.tag("alsaFragments", alsaFragments);
      xml.tag("layoutBreakColor", layoutBreakColor);
      xml.tag("antialiasedDrawing", antialiasedDrawing);
      writeShortcuts(xml);
      xml.etag("Preferences");
      xml.etag("museScore");
      f.close();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Preferences::read()
      {
      QFile qf(QString("%1/.mscore").arg(QDir::homePath()));
      if (!qf.open(QIODevice::ReadOnly)) {
            if (debugMode) {
                  printf("Preferences: open(%s) failed: %s\n",
                     qf.fileName().toLatin1().data(), strerror(errno));
                  }
            return;
            }

      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(&qf, false, &err, &line, &column)) {
            printf("error reading file %s at line %d column %d: %s\n",
               qf.fileName().toLatin1().data(), line, column, err.toLatin1().data());
            return;
            }
      for (QDomNode node = doc.documentElement(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull() || e.tagName() != "museScore")
                  continue;

            for (QDomNode nnode = node.firstChild(); !nnode.isNull(); nnode = nnode.nextSibling()) {
                  QDomElement e = nnode.toElement();
                  if (e.isNull() || e.tagName() != "Preferences")
                        continue;

                  for (QDomNode nnnode = nnode.firstChild(); !nnnode.isNull(); nnnode = nnnode.nextSibling()) {
                        QDomElement e = nnnode.toElement();
                        if (e.isNull())
                              continue;
                        QString tag(e.tagName());
                        QString val(e.text());
                        int i = val.toInt();
                        if (tag == "cursorBlink")
                              cursorBlink = i;
                        else if (tag == "fgColor") {
                              fgColor.setNamedColor(val);
                              fgUseColor = true;
                              }
                        else if (tag == "bgColor") {
                              bgColor.setNamedColor(val);
                              bgUseColor = true;
                              }
                        else if (tag == "fgWallpaper") {
                              fgWallpaper = val;
                              fgUseColor = false;
                              }
                        else if (tag == "bgWallpaper") {
                              bgWallpaper = val;
                              bgUseColor = false;
                              }
                        else if (tag == "selectColor")      // obsolete
                              selectColor[0].setNamedColor(val);
                        else if (tag == "selectColor1")
                              selectColor[0].setNamedColor(val);
                        else if (tag == "selectColor2")
                              selectColor[1].setNamedColor(val);
                        else if (tag == "selectColor3")
                              selectColor[2].setNamedColor(val);
                        else if (tag == "selectColor4")
                              selectColor[3].setNamedColor(val);
                        else if (tag == "enableMidiInput")
                              enableMidiInput = i;
                        else if (tag == "playNotes")
                              playNotes = i;
                        else if (tag == "soundFont")
                              soundFont = val;
                        else if (tag == "rPort")
                              rPort = val;
                        else if (tag == "lPort")
                              lPort = val;
                        else if (tag == "stemDirection1")
                              stemDir[0] = Direction(i);
                        else if (tag == "stemDirection2")
                              stemDir[1] = Direction(i);
                        else if (tag == "stemDirection3")
                              stemDir[2] = Direction(i);
                        else if (tag == "stemDirection4")
                              stemDir[3] = Direction(i);
                        else if (tag == "showNavigator")
                              showNavigator = i;
                        else  if (tag == "showPlayPanel")
                              showPlayPanel = i;
                        else if (tag == "showPad")
                              showPad = i;
                        else if (tag == "PlayPanelPos") {
                              playPanelPos.setX(e.attribute("x").toInt());
                              playPanelPos.setY(e.attribute("y").toInt());
                              }
                        else if (tag == "KeyPadPos") {
                              padPos.setX(e.attribute("x").toInt());
                              padPos.setY(e.attribute("y").toInt());
                              }
                        else if (tag == "useAlsaAudio")
                              useAlsaAudio = i;
                        else if (tag == "useJackAudio")
                              useJackAudio = i;
                        else if (tag == "alsaDevice")
                              alsaDevice = val;
                        else if (tag == "alsaSampleRate")
                              alsaSampleRate = i;
                        else if (tag == "alsaPeriodSize")
                              alsaPeriodSize = i;
                        else if (tag == "alsaFragments")
                              alsaFragments = i;
                        else if (tag == "layoutBreakColor")
                              layoutBreakColor.setNamedColor(val);
                        else if (tag == "antialiasedDrawing")
                              antialiasedDrawing = i;
                        else if (tag == "Shortcuts")
                              readShortcuts(nnnode);
                        else
                              printf("Mscore:Preferences: unknown tag %s\n",
                                 tag.toLatin1().data());
                        }
                  }
            }
      qf.close();
      }

//---------------------------------------------------------
//   preferences
//---------------------------------------------------------

void MuseScore::startPreferenceDialog()
      {
      if (!preferenceDialog) {
            preferenceDialog = new PreferenceDialog(this);
            connect(preferenceDialog, SIGNAL(preferencesChanged()),
               SLOT(preferencesChanged()));
            }
      preferenceDialog->show();
      }

//---------------------------------------------------------
//   PreferenceDialog
//---------------------------------------------------------

PreferenceDialog::PreferenceDialog(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);

      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
//      connect(buttonCancel, SIGNAL(pressed()), SLOT(cancel()));

      cursorBlink->setChecked(preferences.cursorBlink);

      QButtonGroup* fgButtons = new QButtonGroup(this);
      fgButtons->setExclusive(true);
      fgButtons->addButton(fgColorButton);
      fgButtons->addButton(fgWallpaperButton);
      connect(fgColorButton, SIGNAL(toggled(bool)), SLOT(fgClicked(bool)));

      QButtonGroup* bgButtons = new QButtonGroup(this);
      bgButtons->setExclusive(true);
      bgButtons->addButton(bgColorButton);
      bgButtons->addButton(bgWallpaperButton);
      connect(bgColorButton, SIGNAL(toggled(bool)), SLOT(bgClicked(bool)));

      fgWallpaper->setText(preferences.fgWallpaper);
      bgWallpaper->setText(preferences.bgWallpaper);

      QPalette p(fgColorLabel->palette());
      p.setColor(QPalette::Background, preferences.fgColor);
      fgColorLabel->setPalette(p);
      p.setColor(QPalette::Background, preferences.bgColor);
      bgColorLabel->setPalette(p);

      p.setColor(QPalette::Background, preferences.selectColor[0]);
      selectColorLabel1->setPalette(p);
      p.setColor(QPalette::Background, preferences.selectColor[1]);
      selectColorLabel2->setPalette(p);
      p.setColor(QPalette::Background, preferences.selectColor[2]);
      selectColorLabel3->setPalette(p);
      p.setColor(QPalette::Background, preferences.selectColor[3]);
      selectColorLabel4->setPalette(p);

      bgColorButton->setChecked(preferences.bgUseColor);
      bgWallpaperButton->setChecked(!preferences.bgUseColor);
      fgColorButton->setChecked(preferences.fgUseColor);
      fgWallpaperButton->setChecked(!preferences.fgUseColor);

      enableMidiInput->setChecked(preferences.enableMidiInput);
      playNotes->setChecked(preferences.playNotes);

      if (!preferences.soundFont.isEmpty())
            soundFont->setText(preferences.soundFont);
      else {
            const char* p = getenv("DEFAULT_SOUNDFONT");
            soundFont->setText(QString(p ? p : ""));
            }

      if (seq->isRunning()) {
            std::list<QString> sl = seq->inputPorts();
            int idx = 0;
            for (std::list<QString>::iterator i = sl.begin(); i != sl.end(); ++i, ++idx) {
                  jackRPort->addItem(*i);
                  jackLPort->addItem(*i);
                  if (preferences.rPort == *i)
                        jackRPort->setCurrentIndex(idx);
                  if (preferences.lPort == *i)
                        jackLPort->setCurrentIndex(idx);
                  }
            }
      else {
            jackRPort->setEnabled(false);
            jackLPort->setEnabled(false);
            }

      stemDir2button(preferences.stemDir[0], upRadioButton1, downRadioButton1, autoRadioButton1);
      stemDir2button(preferences.stemDir[1], upRadioButton2, downRadioButton2, autoRadioButton2);
      stemDir2button(preferences.stemDir[2], upRadioButton3, downRadioButton3, autoRadioButton3);
      stemDir2button(preferences.stemDir[3], upRadioButton4, downRadioButton4, autoRadioButton4);

      navigatorShow->setChecked(preferences.showNavigator);
      keyPadShow->setChecked(preferences.showPad);
      playPanelShow->setChecked(preferences.showPlayPanel);
      keyPadX->setValue(preferences.padPos.x());
      keyPadY->setValue(preferences.padPos.y());
      playPanelX->setValue(preferences.playPanelPos.x());
      playPanelY->setValue(preferences.playPanelPos.y());

      alsaDriver->setChecked(preferences.useAlsaAudio);
      jackDriver->setChecked(preferences.useJackAudio);
      alsaDevice->setText(preferences.alsaDevice);

      int index = alsaSampleRate->findText(QString("%1").arg(preferences.alsaSampleRate));
      alsaSampleRate->setCurrentIndex(index);
      index = alsaPeriodSize->findText(QString("%1").arg(preferences.alsaPeriodSize));
      alsaPeriodSize->setCurrentIndex(index);

      alsaFragments->setValue(preferences.alsaFragments);
      drawAntialiased->setChecked(preferences.antialiasedDrawing);

      //
      // initialize local shortcut table
      //    we need a deep copy to be able to rewind all
      //    changes on "Abort"
      //
      foreach(Shortcut* s, shortcuts) {
            Shortcut* ns = new Shortcut(*s);
            ns->action = 0;
            ns->icon   = 0;
            localShortcuts[s->xml] = ns;
            }
      updateSCListView();

      connect(fgColorSelect,      SIGNAL(clicked()), SLOT(selectFgColor()));
      connect(bgColorSelect,      SIGNAL(clicked()), SLOT(selectBgColor()));
      connect(selectColorSelect1, SIGNAL(clicked()), SLOT(selectSelectColor1()));
      connect(selectColorSelect2, SIGNAL(clicked()), SLOT(selectSelectColor2()));
      connect(selectColorSelect3, SIGNAL(clicked()), SLOT(selectSelectColor3()));
      connect(selectColorSelect4, SIGNAL(clicked()), SLOT(selectSelectColor4()));
      connect(fgWallpaperSelect,  SIGNAL(clicked()), SLOT(selectFgWallpaper()));
      connect(bgWallpaperSelect,  SIGNAL(clicked()), SLOT(selectBgWallpaper()));
      connect(sfButton, SIGNAL(clicked()), SLOT(selectSoundFont()));
      sfChanged = false;

      connect(playPanelCur, SIGNAL(clicked()), SLOT(playPanelCurClicked()));
      connect(keyPadCur, SIGNAL(clicked()), SLOT(padCurClicked()));

//      connect(shortcutList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(defineShortcutClicked()));
      connect(shortcutList, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(defineShortcutClicked()));
      connect(resetShortcut, SIGNAL(clicked()), SLOT(resetShortcutClicked()));
      connect(clearShortcut, SIGNAL(clicked()), SLOT(clearShortcutClicked()));
      connect(defineShortcut, SIGNAL(clicked()), SLOT(defineShortcutClicked()));
      }

//---------------------------------------------------------
//   updateSCListView
//---------------------------------------------------------

void PreferenceDialog::updateSCListView()
      {
      shortcutList->clear();
      foreach (Shortcut* s, localShortcuts) {
            if (s) {
                  QTreeWidgetItem* newItem = new QTreeWidgetItem;
                  newItem->setText(0, tr(s->descr));
                  newItem->setIcon(0, *s->icon);
                  QKeySequence seq = s->key;
                  newItem->setText(1, s->key.toString(QKeySequence::NativeText));
                  newItem->setData(0, Qt::UserRole, s->xml);
                  shortcutList->addTopLevelItem(newItem);
                  // does not work:
                  // QBrush brush = newItem->background(1);
                  // brush.setColor(brush.color().dark(200));
                  // newItem->setBackground(0, brush);
                  }
            }
      shortcutList->resizeColumnToContents(0);
      }

//---------------------------------------------------------
//   resetShortcutClicked
//    reset all shortcuts to buildin defaults
//---------------------------------------------------------

void PreferenceDialog::resetShortcutClicked()
      {
      }

//---------------------------------------------------------
//   clearShortcutClicked
//---------------------------------------------------------

void PreferenceDialog::clearShortcutClicked()
      {
      QTreeWidgetItem* active = shortcutList->currentItem();
      if (active) {
            Shortcut* s = localShortcuts[active->data(0, Qt::UserRole).toString()];
            s->key = 0;
            active->setText(1, s->key.toString(QKeySequence::NativeText));
            }
      }

//---------------------------------------------------------
//   defineShortcutClicked
//---------------------------------------------------------

void PreferenceDialog::defineShortcutClicked()
      {
      QTreeWidgetItem* active = shortcutList->currentItem();
      Shortcut* s = localShortcuts[active->data(0, Qt::UserRole).toString()];
      ShortcutCaptureDialog sc(s, this);
      if (sc.exec()) {
            s->key = sc.getKey();
            active->setText(1, s->key.toString(QKeySequence::NativeText));
            shortcutsChanged = true;
            }
//      clearButton->setEnabled(true);
      }

//---------------------------------------------------------
//   selectFgColor
//---------------------------------------------------------

void PreferenceDialog::selectFgColor()
      {
      QPalette p(fgColorLabel->palette());
      QColor c = QColorDialog::getColor(p.color(QPalette::Background), this);
      if (c.isValid()) {
            p.setColor(QPalette::Background, c);
            fgColorLabel->setPalette(p);
            }
      }

//---------------------------------------------------------
//   selectBgColor
//---------------------------------------------------------

void PreferenceDialog::selectBgColor()
      {
      QPalette p(bgColorLabel->palette());
      QColor c = QColorDialog::getColor(p.color(QPalette::Background), this);
      if (c.isValid()) {
            p.setColor(QPalette::Background, c);
            bgColorLabel->setPalette(p);
            }
      }

//---------------------------------------------------------
//   selectSelectColor1
//---------------------------------------------------------

void PreferenceDialog::selectSelectColor1()
      {
      QPalette p(selectColorLabel1->palette());
      QColor c = QColorDialog::getColor(p.color(QPalette::Background), this);
      if (c.isValid()) {
            p.setColor(QPalette::Background, c);
            selectColorLabel1->setPalette(p);
            }
      }

//---------------------------------------------------------
//   selectSelectColor2
//---------------------------------------------------------

void PreferenceDialog::selectSelectColor2()
      {
      QPalette p(selectColorLabel2->palette());
      QColor c = QColorDialog::getColor(p.color(QPalette::Background), this);
      if (c.isValid()) {
            p.setColor(QPalette::Background, c);
            selectColorLabel2->setPalette(p);
            }
      }

//---------------------------------------------------------
//   selectSelectColor3
//---------------------------------------------------------

void PreferenceDialog::selectSelectColor3()
      {
      QPalette p(selectColorLabel3->palette());
      QColor c = QColorDialog::getColor(p.color(QPalette::Background), this);
      if (c.isValid()) {
            p.setColor(QPalette::Background, c);
            selectColorLabel3->setPalette(p);
            }
      }

//---------------------------------------------------------
//   selectSelectColor4
//---------------------------------------------------------

void PreferenceDialog::selectSelectColor4()
      {
      QPalette p(selectColorLabel4->palette());
      QColor c = QColorDialog::getColor(p.color(QPalette::Background), this);
      if (c.isValid()) {
            p.setColor(QPalette::Background, c);
            selectColorLabel4->setPalette(p);
            }
      }

//---------------------------------------------------------
//   selectFgWallpaper
//---------------------------------------------------------

void PreferenceDialog::selectFgWallpaper()
      {
      QString s = QFileDialog::getOpenFileName(
         this,                            // parent
         tr("Choose Notepaper"),          // caption
         fgWallpaper->text(),             // dir
         tr("Images (*.jpg *.gif *.png)") // filter
         );
      if (!s.isNull())
            fgWallpaper->setText(s);
      }

//---------------------------------------------------------
//   selectBgWallpaper
//---------------------------------------------------------

void PreferenceDialog::selectBgWallpaper()
      {
      QString s = QFileDialog::getOpenFileName(
         this,
         tr("Choose Background Wallpaper"),
         bgWallpaper->text(),
         tr("Images (*.jpg *.gif *.png)")
         );
      if (!s.isNull())
            bgWallpaper->setText(s);
      }

//---------------------------------------------------------
//   selectSoundFont
//---------------------------------------------------------

void PreferenceDialog::selectSoundFont()
      {
      QString s = QFileDialog::getOpenFileName(
         this,
         tr("Choose Synthesizer Sound Font"),
         soundFont->text(),
         tr("Sound Fonds (*.sf2 *.SF2);;All (*)")
         );
      if (!s.isNull()) {
            sfChanged = soundFont->text() != s;
            soundFont->setText(s);
            }
      }

//---------------------------------------------------------
//   fgClicked
//---------------------------------------------------------

void PreferenceDialog::fgClicked(bool id)
      {
      fgColorLabel->setEnabled(id);
      fgColorSelect->setEnabled(id);
      fgWallpaper->setEnabled(!id);
      fgWallpaperSelect->setEnabled(!id);
      }

//---------------------------------------------------------
//   bgClicked
//---------------------------------------------------------

void PreferenceDialog::bgClicked(bool id)
      {
      bgColorLabel->setEnabled(id);
      bgColorSelect->setEnabled(id);
      bgWallpaper->setEnabled(!id);
      bgWallpaperSelect->setEnabled(!id);
      }

//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void PreferenceDialog::buttonBoxClicked(QAbstractButton* button)
      {
      switch(buttonBox->standardButton(button)) {
            case QDialogButtonBox::Apply:
                  apply();
                  break;
            case QDialogButtonBox::Ok:
                  apply();
            case QDialogButtonBox::Cancel:
            default:
                  hide();
                  break;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void PreferenceDialog::apply()
      {
      preferences.selectColor[0] = selectColorLabel1->palette().color(QPalette::Background);
      preferences.selectColor[1] = selectColorLabel2->palette().color(QPalette::Background);
      preferences.selectColor[2] = selectColorLabel3->palette().color(QPalette::Background);
      preferences.selectColor[3] = selectColorLabel4->palette().color(QPalette::Background);

      preferences.cursorBlink = cursorBlink->isChecked();
      preferences.fgWallpaper = fgWallpaper->text();
      preferences.bgWallpaper = bgWallpaper->text();

      preferences.fgColor = fgColorLabel->palette().color(QPalette::Background);
      preferences.bgColor = bgColorLabel->palette().color(QPalette::Background);

      preferences.bgUseColor  = bgColorButton->isChecked();
      preferences.fgUseColor  = fgColorButton->isChecked();
      preferences.enableMidiInput = enableMidiInput->isChecked();
      preferences.playNotes   = playNotes->isChecked();
      preferences.soundFont   = soundFont->text();
      if (preferences.lPort != jackLPort->currentText()
         || preferences.rPort != jackRPort->currentText()) {
            // TODO: change ports
            preferences.lPort       = jackLPort->currentText();
            preferences.rPort       = jackRPort->currentText();
            }
      preferences.stemDir[0] = buttons2stemDir(upRadioButton1, downRadioButton1);
      preferences.stemDir[1] = buttons2stemDir(upRadioButton2, downRadioButton2);
      preferences.stemDir[2] = buttons2stemDir(upRadioButton3, downRadioButton3);
      preferences.stemDir[3] = buttons2stemDir(upRadioButton4, downRadioButton4);

      preferences.showNavigator  = navigatorShow->isChecked();
      preferences.showPad        = keyPadShow->isChecked();
      preferences.showPlayPanel  = playPanelShow->isChecked();
      preferences.padPos         = QPoint(keyPadX->value(), keyPadY->value());
      preferences.playPanelPos   = QPoint(playPanelX->value(), playPanelY->value());

      preferences.useAlsaAudio   = alsaDriver->isChecked();
      preferences.useJackAudio   = jackDriver->isChecked();
      preferences.alsaDevice     = alsaDevice->text();
      preferences.alsaSampleRate = alsaSampleRate->currentText().toInt();
      preferences.alsaPeriodSize = alsaPeriodSize->currentText().toInt();
      preferences.alsaFragments  = alsaFragments->value();
      preferences.antialiasedDrawing = drawAntialiased->isChecked();

      if (shortcutsChanged) {
            shortcutsChanged = false;
            foreach(Shortcut* s, localShortcuts) {
                  Shortcut* os = shortcuts[s->xml];
                  if (os->key != s->key) {
                        os->key = s->key;
                        if (os->action)
                              os->action->setShortcut(s->key);
                        }
                  }
            }
      emit preferencesChanged();
      preferences.write();
      if (sfChanged) {
            if (!seq->isRunning()) {
                  // try to start sequencer
                  seq->init();
                  }
            if (seq->isRunning()) {
                  sfChanged = false;
                  seq->loadSoundFont(preferences.soundFont.toLatin1().data());
                  }
            }
      }

//---------------------------------------------------------
//   playPanelCurClicked
//---------------------------------------------------------

void PreferenceDialog::playPanelCurClicked()
      {
      PlayPanel* w = mscore->getPlayPanel();
      if (w == 0)
            return;
      QPoint s(w->pos());
      playPanelX->setValue(s.x());
      playPanelY->setValue(s.y());
      }

//---------------------------------------------------------
//   padCurClicked
//---------------------------------------------------------

void PreferenceDialog::padCurClicked()
      {
      Pad* w = mscore->getKeyPad();
      if (w == 0)
            return;
      QPoint s(w->pos());
      keyPadX->setValue(s.x());
      keyPadY->setValue(s.y());
      }

//---------------------------------------------------------
//   writeShortcuts
//---------------------------------------------------------

void writeShortcuts(Xml& xml)
      {
      xml.stag("Shortcuts");
      foreach(Shortcut* s, shortcuts) {
            //
            // save only if different from default
            //
            for (unsigned i = 0;; ++i) {
                  if (MuseScore::sc[i].xml == s->xml) {
                        if (MuseScore::sc[i].key != s->key)
                              xml.tag(s->xml, s->key.toString(QKeySequence::PortableText));
                        break;
                        }
                  }
            }
      xml.etag("Shortcuts");
      }

//---------------------------------------------------------
//   readShortcuts
//---------------------------------------------------------

void readShortcuts(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            Shortcut* s = shortcuts.value(e.tagName());
            if (s) {
                  s->key = QKeySequence::fromString(e.text(), QKeySequence::PortableText);
                  }
            else
                  printf("MuseScore:readShortCuts: unknown tag <%s>\n", e.tagName().toLatin1().data());
            }
      }

//---------------------------------------------------------
//   getAction
//    returns action for shortcut
//---------------------------------------------------------

QAction* getAction(const char* id)
      {
      Shortcut* s = shortcuts.value(id);
      if (s == 0) {
            printf("interanl error: shortcut <%s> not found\n", id);
            return 0;
            }
      if (s->action == 0) {
            QAction* a = new QAction(s->xml, mscore); // ->getCanvas());
            s->action = a;
            a->setData(s->xml);
            a->setShortcut(s->key);
            a->setShortcutContext(s->context);
            if (!s->help.isEmpty()) {
                  a->setToolTip(s->help);
                  a->setWhatsThis(s->help);
                  }
            else {
                  a->setToolTip(s->descr);
                  a->setWhatsThis(s->descr);
                  }
            if (!s->text.isEmpty())
                  a->setText(s->text);
            if (s->icon)
                  a->setIcon(*s->icon);
            }
      return s->action;
      }
