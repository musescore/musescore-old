//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
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
#include "harmonyedit.h"
#include "harmonycanvas.h"
#include "palette.h"
#include "accidental.h"
#include "score.h"
#include "icons.h"
#include "pitchspelling.h"

extern bool useFactorySettings;

//---------------------------------------------------------
//   ChordStyleEditor
//---------------------------------------------------------

ChordStyleEditor::ChordStyleEditor(QWidget* parent)
   : QWidget(parent, Qt::Dialog | Qt::Window)
      {
      setupUi(this);
      setWindowTitle(tr("MuseScore: Chord Style Editor"));
      fileButton->setIcon(*icons[fileOpen_ICON]);
      chordList = 0;

      // create symbol palette
      QLayout* l = new QVBoxLayout();
      l->setContentsMargins(0, 0, 0, 0);
      paletteFrame->setLayout(l);
      sp1 = new Palette();
      PaletteScrollArea* accPalette = new PaletteScrollArea(sp1);
      QSizePolicy policy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
      accPalette->setSizePolicy(policy1);
      accPalette->setRestrictHeight(false);

      l->addWidget(accPalette);
      sp1->setGrid(33, 36);

#if 0
      for (int i = 16; i < 26+9; ++i) {
            Accidental* s = new Accidental(gscore);
            s->setSubtype(i);
            sp1->append(s, qApp->translate("Accidental", s->subTypeName()));
            }
#endif

#if 0
      if (!useFactorySettings) {
            QFile f(dataPath + "/" + "keysigs.xml");
            if (f.exists() && sp->read(&f))
                  return;
            }
      //
      // create default palette
      //
      for (int i = 0; i < 7; ++i) {
            KeySig* k = new KeySig(gscore);
            k->setSubtype(i+1);
            sp->append(k, qApp->translate("MuseScore", keyNames[i*2]));
            }
      for (int i = -7; i < 0; ++i) {
            KeySig* k = new KeySig(gscore);
            k->setSubtype(i & 0xff);
            sp->append(k, qApp->translate("MuseScore", keyNames[(7 + i) * 2 + 1]));
            }
      KeySig* k = new KeySig(gscore);
      k->setSubtype(0);
      sp->append(k, qApp->translate("MuseScore", keyNames[14]));
#endif
      connect(fileButton, SIGNAL(clicked()), SLOT(fileButtonClicked()));
      connect(saveButton, SIGNAL(clicked()), SLOT(saveButtonClicked()));
      connect(harmonyList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
         SLOT(harmonyChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
      }

//---------------------------------------------------------
//   editChordStyle
//---------------------------------------------------------

void MuseScore::editChordStyle()
      {
      if (chordStyleEditor == 0) {
            chordStyleEditor = new ChordStyleEditor(0);
            chordStyleEditor->restore();
            }

      chordStyleEditor->show();
      chordStyleEditor->raise();
      }

//---------------------------------------------------------
//   fileButtonClicked
//---------------------------------------------------------

void ChordStyleEditor::fileButtonClicked()
      {
      QString path = QString("%1styles/").arg(mscoreGlobalShare);
      QString fn = QFileDialog::getOpenFileName(
         0, QWidget::tr("MuseScore: Load Chord Description File"),
         path,
            QWidget::tr("MuseScore Chord Description (*.xml);;"
            "All Files (*)"
            )
         );
      if (fn.isEmpty())
            return;
      loadChordDescriptionFile(fn);
      }

//---------------------------------------------------------
//   saveButtonClicked
//---------------------------------------------------------

void ChordStyleEditor::saveButtonClicked()
      {
      if (!chordList)
            return;
      ChordDescription* d = canvas->getChordDescription();
      updateChordDescription(d);

      QString path = QString("%1styles/").arg(mscoreGlobalShare);
      QString fn = QFileDialog::getSaveFileName(
         0, QWidget::tr("MuseScore: Save Chord Description File"),
         path,
            QWidget::tr("MuseScore Chord Description (*.xml);;"
            "All Files (*)"
            )
         );
      if (fn.isEmpty())
            return;
      chordList->write(fn);
      }

//---------------------------------------------------------
//   loadChordDescriptionFile
//---------------------------------------------------------

void ChordStyleEditor::loadChordDescriptionFile(const QString& s)
      {
      ChordList* cl = new ChordList;
      if (!cl->read("chords.xml")) {
            printf("cannot read <chords.xml>\n");
            return;
            }
      if (!cl->read(s)) {
            printf("cannot read <%s>\n", qPrintable(s));
            return;
            }
      descriptionFile->setText(s);  // set text in text entry widget
      harmonyList->clear();
      for (iChordDescription i = cl->begin(); i != cl->end(); ++i) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            ChordDescription* d = i.value();
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(d));
            item->setText(0, QString("%1").arg(d->id));
            item->setText(1, QString("%1").arg(d->name));
            harmonyList->addTopLevelItem(item);
            }
      if (chordList)
            delete chordList;
      chordList = cl;
      canvas->setChordDescription(0, 0);
      }

//---------------------------------------------------------
//   updateCurrentChordDescription
//---------------------------------------------------------

void ChordStyleEditor::updateChordDescription(ChordDescription* d)
      {
      double extraMag     = 3.0;
      double _spatium     = 2.0 * PALETTE_SPATIUM / extraMag;
      double mag          = PALETTE_SPATIUM * extraMag / _spatium;
      d->renderList.clear();
      QList<TextSegment*> tl = canvas->getTextList();
      int idx = 0;
      double x  = 0, y = 0;
      foreach(const TextSegment* ts, tl) {
            ++idx;
            if (idx == 1) {     // dont save base
                  x = ts->x + ts->width();
                  y = ts->y;
                  continue;
                  }
            RenderAction ra(RenderAction::RENDER_MOVE);
            ra.movex = (ts->x - x) / mag;
            ra.movey = (ts->y - y) / mag;
            d->renderList.append(ra);

            ra.type  = RenderAction::RENDER_SET;
            ra.text  = ts->text;
            d->renderList.append(ra);
            x = ts->x + ts->width();
            y = ts->y;
            }
      }

//---------------------------------------------------------
//   harmonyChanged
//---------------------------------------------------------

void ChordStyleEditor::harmonyChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
      {
      if (previous) {
            ChordDescription* d = static_cast<ChordDescription*>(previous->data(0, Qt::UserRole).value<void*>());
            updateChordDescription(d);
            }
      if (current) {
            ChordDescription* d = static_cast<ChordDescription*>(current->data(0, Qt::UserRole).value<void*>());
            canvas->setChordDescription(d, chordList);
            }
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void ChordStyleEditor::save()
      {
      QSettings settings;
      settings.beginGroup("ChordStyleEditor");
      settings.setValue("splitter1", splitter1->saveState());
      settings.setValue("splitter2", splitter2->saveState());
//      settings.setValue("list", harmonyList->saveState());
      settings.setValue("col1", harmonyList->columnWidth(0));
      }

//---------------------------------------------------------
//   restore
//---------------------------------------------------------

void ChordStyleEditor::restore()
      {
      if (!useFactorySettings) {
            QSettings settings;
            settings.beginGroup("ChordStyleEditor");
            splitter1->restoreState(settings.value("splitter1").toByteArray());
            splitter2->restoreState(settings.value("splitter2").toByteArray());
            harmonyList->setColumnWidth(0, settings.value("col1", 30).toInt());
            }
      }

//---------------------------------------------------------
//   HarmonyCanvas
//---------------------------------------------------------

HarmonyCanvas::HarmonyCanvas(QWidget* parent)
   : QFrame(parent)
      {
      setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
      setAcceptDrops(true);
      setFocusPolicy(Qt::StrongFocus);
      extraMag = 3.0;
      chordDescription = 0;
      chordList = 0;
      moveElement = 0;
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void HarmonyCanvas::paintEvent(QPaintEvent* event)
      {
      QFrame::paintEvent(event);
      if (!chordDescription)
            return;

      double spatium = 2.0 * PALETTE_SPATIUM / extraMag;
      gscore->setSpatium(spatium);

      QPainter p(this);

      p.setRenderHint(QPainter::Antialiasing, true);
      qreal wh = double(height());
      qreal ww = double(width());

      qreal mag  = PALETTE_SPATIUM * extraMag / spatium;
      _matrix    = QTransform(mag, 0.0, 0.0, mag, ww*.1, wh*.8);
      imatrix    = _matrix.inverted();

      p.setWorldTransform(_matrix);
      QRectF f = imatrix.mapRect(QRectF(0.0, 0.0, ww, wh));

      p.setPen(QPen(Qt::darkGray));
      p.drawLine(f.x(), 0.0, f.width(), 0.0);
      p.drawLine(0.0, f.y(), 0.0, f.height());

      foreach(const TextSegment* ts, textList) {
            p.setFont(ts->font);
            QPen pen(ts->select ? Qt::blue : palette().color(QPalette::Text));
            p.setPen(pen);
            p.drawText(ts->x, ts->y, ts->text);
            }
      }

//---------------------------------------------------------
//   render
//---------------------------------------------------------

void HarmonyCanvas::render(const QList<RenderAction>& renderList, double& x, double& y, int tpc)
      {
      QStack<QPointF> stack;
      int fontIdx = 0;
      double _spatium = 2.0 * PALETTE_SPATIUM / extraMag;
      qreal mag  = PALETTE_SPATIUM * extraMag / _spatium;

      QList<QFont> fontList;              // temp values used in render()
      const TextStyle* st = &gscore->textStyle(TEXT_STYLE_HARMONY);

      foreach(ChordFont cf, chordList->fonts) {
            if (cf.family.isEmpty() || cf.family == "default")
                  fontList.append(st->fontPx(_spatium * cf.mag));
            else {
                  QFont ff(st->fontPx(_spatium * cf.mag));
                  ff.setFamily(cf.family);
                  fontList.append(ff);
                  }
            }
      if (fontList.isEmpty())
            fontList.append(st->fontPx(_spatium));

      foreach(const RenderAction& a, renderList) {
            if (a.type == RenderAction::RENDER_SET) {
                  TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                  ChordSymbol cs = chordList->symbol(a.text);
                  if (cs.isValid()) {
                        ts->font = fontList[cs.fontIdx];
                        ts->setText(QString(cs.code));
                        }
                  else
                        ts->setText(a.text);
                  textList.append(ts);
                  x += ts->width();
                  }
            else if (a.type == RenderAction::RENDER_MOVE) {
                  x += a.movex * mag;
                  y += a.movey * mag;
                  }
            else if (a.type == RenderAction::RENDER_PUSH)
                  stack.push(QPointF(x,y));
            else if (a.type == RenderAction::RENDER_POP) {
                  if (!stack.isEmpty()) {
                        QPointF pt = stack.pop();
                        x = pt.x();
                        y = pt.y();
                        }
                  else
                        printf("RenderAction::RENDER_POP: stack empty\n");
                  }
            else if (a.type == RenderAction::RENDER_NOTE) {
                  bool germanNames = gscore->styleB(ST_useGermanNoteNames);
                  QChar c;
                  int acc;
                  tpc2name(tpc, germanNames, &c, &acc);
                  TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                  ChordSymbol cs = chordList->symbol(QString(c));
                  if (cs.isValid()) {
                        ts->font = fontList[cs.fontIdx];
                        ts->setText(QString(cs.code));
                        }
                  else
                        ts->setText(QString(c));
                  textList.append(ts);
                  x += ts->width();
                  }
            else if (a.type == RenderAction::RENDER_ACCIDENTAL) {
                  QChar c;
                  int acc;
                  tpc2name(tpc, false, &c, &acc);
                  if (acc) {
                        TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                        QString s;
                        if (acc == -1)
                              s = "b";
                        else if (acc == 1)
                              s = "#";
                        ChordSymbol cs = chordList->symbol(s);
                        if (cs.isValid()) {
                              ts->font = fontList[cs.fontIdx];
                              ts->setText(QString(cs.code));
                              }
                        else
                              ts->setText(s);
                        textList.append(ts);
                        x += ts->width();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void HarmonyCanvas::mousePressEvent(QMouseEvent* event)
      {
      startMove = imatrix.map(QPointF(event->pos()));
      moveElement = 0;
      foreach(TextSegment* ts, textList) {
            QRectF r = ts->boundingRect().translated(ts->x, ts->y);
            ts->select = r.contains(startMove);
            if (ts->select)
                  moveElement = ts;
            }
      update();
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void HarmonyCanvas::mouseMoveEvent(QMouseEvent* event)
      {
      if (moveElement == 0)
            return;
      QPointF p = imatrix.map(QPointF(event->pos()));
      QPointF delta = p - startMove;
      moveElement->x += delta.x();
      moveElement->y += delta.y();
      startMove = p;
      update();
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void HarmonyCanvas::mouseReleaseEvent(QMouseEvent*)
      {
      }

//---------------------------------------------------------
//   setChordDescription
//---------------------------------------------------------

void HarmonyCanvas::setChordDescription(ChordDescription* sd, ChordList* sl)
      {
      chordDescription = sd;
      chordList = sl;

      foreach(TextSegment* s, textList)
            delete s;
      textList.clear();

      if (chordList) {
            int tpc = 14;
            double x = 0.0, y = 0.0;
            render(chordList->renderListRoot, x, y, 14);
            render(chordDescription->renderList, x, y, tpc);
            }
      moveElement = 0;
      update();
      }

