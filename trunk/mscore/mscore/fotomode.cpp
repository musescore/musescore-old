//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: scoreview.cpp 3446 2010-09-10 20:18:16Z wschweer $
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

#include "scoreview.h"
#include "mscore.h"
#include "score.h"
#include "lasso.h"
#include "icons.h"
#include "page.h"
#include "preferences.h"

//---------------------------------------------------------
//   FotoScoreViewDragTransition
//---------------------------------------------------------

class FotoScoreViewDragTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            if (me->modifiers() & Qt::ShiftModifier)
                  return false;
            return canvas->fotoScoreViewDragTest(me);
            }
   public:
      FotoScoreViewDragTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   FotoScoreViewDragRectTransition
//---------------------------------------------------------

class FotoScoreViewDragRectTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            if (me->modifiers() & Qt::ShiftModifier)
                  return false;
            return canvas->fotoScoreViewDragRectTest(me);
            }
   public:
      FotoScoreViewDragRectTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   FotoFrameTransition
//---------------------------------------------------------

class FotoFrameTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            return !canvas->fotoRectHit(me->pos());
            }
   public:
      FotoFrameTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            setModifierMask(Qt::ShiftModifier);
            }
      };

//---------------------------------------------------------
//   FotoEditElementDragTransition
//---------------------------------------------------------

class FotoEditElementDragTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            return canvas->fotoEditElementDragTransition(me);
            }
   public:
      FotoEditElementDragTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   FotoDragEditTransition
//---------------------------------------------------------

class FotoDragEditTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->doFotoDragEdit(me);
            }
   public:
      FotoDragEditTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
      };

//---------------------------------------------------------
//   FotoDragTransition
//---------------------------------------------------------

class FotoDragTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->doDragFoto(me);
            }
   public:
      FotoDragTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
      };

//---------------------------------------------------------
//   FotoDragRectTransition
//---------------------------------------------------------

class FotoDragRectTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->doDragFotoRect(me);
            }
   public:
      FotoDragRectTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
      };

//---------------------------------------------------------
//   FotoContextTransition
//---------------------------------------------------------

class FotoContextTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* e) {
            if (!QEventTransition::eventTest(e))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QContextMenuEvent* me = static_cast<QContextMenuEvent*>(we->event());
            return canvas->fotoRectHit(me->pos());
            }
      virtual void onTransition(QEvent* e) {
            QMouseEvent* me = static_cast<QMouseEvent*>(static_cast<QStateMachine::WrappedEvent*>(e)->event());
            canvas->fotoContextPopup(me);
            }
   public:
      FotoContextTransition(ScoreView* c)
         : QEventTransition(c, QEvent::ContextMenu), canvas(c) {}
      };

//---------------------------------------------------------
//   FotoDragDropTransition
//---------------------------------------------------------

class FotoDragDropTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            return canvas->fotoRectHit(me->pos());
            }
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->fotoDragDrop(me);
            }
   public:
      FotoDragDropTransition(ScoreView* c)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c)
            {
            setModifierMask(Qt::ShiftModifier | Qt::ControlModifier);
            }
      };

//---------------------------------------------------------
//   setupFotoMode
//---------------------------------------------------------

void ScoreView::setupFotoMode()
      {
      QState* s = states[FOTOMODE];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      s->addTransition(new CommandTransition("escape", states[NORMAL]));    // ->normal
      s->addTransition(new CommandTransition("fotomode", states[NORMAL]));  // ->normal
      s->addTransition(new ScoreViewDragTransition(this, states[DRAG]));    // ->stateDrag

      QState* f1 = new QState(s);
      f1->setObjectName("foto-normal");
      connect(f1, SIGNAL(entered()), SLOT(enterState()));
      connect(f1, SIGNAL(exited()), SLOT(exitState()));

      //
      // in the f2 state the canvas is dragged
      //
      QState* f2 = new QState(s);
      f2->setObjectName("foto-drag");
      connect(f2, SIGNAL(entered()), SLOT(enterState()));
      connect(f2, SIGNAL(exited()), SLOT(exitState()));

      //
      // state f3 draws the foto rectangle
      //
      QState* f3 = new QState(s);
      f3->setObjectName("foto-frame");
      connect(f3, SIGNAL(entered()), SLOT(enterState()));
      connect(f3, SIGNAL(exited()), SLOT(exitState()));

      //
      // f4 drags the foto rectangle grips
      //
      QState* f4 = new QState(s);
      f4->setObjectName("foto-drag-edit");
      connect(f4, SIGNAL(entered()), SLOT(enterState()));
      connect(f4, SIGNAL(exited()), SLOT(exitState()));

      //
      // f5 drags the foto rectangle itself
      //
      QState* f5 = new QState(s);
      f5->setObjectName("foto-drag-rect");
      connect(f5, SIGNAL(entered()), SLOT(enterState()));
      connect(f5, SIGNAL(exited()), SLOT(exitState()));

      f1->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      f1->addTransition(new FotoScoreViewDragTransition(this,   f2));   // ->stateDrag
      f1->addTransition(new FotoFrameTransition(this,           f3));   // ->stateLasso
      f1->addTransition(new FotoEditElementDragTransition(this, f4));   // ->editDrag
      f1->addTransition(new FotoScoreViewDragRectTransition(this, f5)); // ->foto-drag-rect
      f1->addTransition(new FotoContextTransition(this));               // context menu
      f1->addTransition(new FotoDragDropTransition(this));

      // drag canvas in foto state
      f2->assignProperty(this, "cursor", QCursor(Qt::SizeAllCursor));
      QEventTransition* cl = new QEventTransition(this, QEvent::MouseButtonRelease);
      cl->setTargetState(f1);
      f2->addTransition(cl);
      f2->addTransition(new DragTransition(this));

      // drag foto frame
      f3->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      cl = new QEventTransition(this, QEvent::MouseButtonRelease);
      cl->setTargetState(f1);
      f3->addTransition(cl);
      f3->addTransition(new class FotoDragTransition(this));
      connect(f3, SIGNAL(entered()), SLOT(startFotoDrag()));
      connect(f3, SIGNAL(exited()),  SLOT(endFotoDrag()));

      // foto drag edit state
      f4->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      cl = new QEventTransition(this, QEvent::MouseButtonRelease);
      cl->setTargetState(f1);
      f4->addTransition(cl);
      f4->addTransition(new FotoDragEditTransition(this));
      connect(f4, SIGNAL(exited()), SLOT(endFotoDragEdit()));

      f5->assignProperty(this, "cursor", QCursor(Qt::SizeAllCursor));
      cl = new QEventTransition(this, QEvent::MouseButtonRelease);
      cl->setTargetState(f1);
      f5->addTransition(cl);
      f5->addTransition(new class FotoDragRectTransition(this));

      s->setInitialState(f1);
      s->addTransition(new ScoreViewDragTransition(this, f2));

      connect(s, SIGNAL(entered()), mscore, SLOT(setFotomode()));
      connect(s, SIGNAL(entered()), SLOT(startFotomode()));
      connect(s, SIGNAL(exited()),  SLOT(stopFotomode()));
      }

//---------------------------------------------------------
//   startFotomode
//---------------------------------------------------------

void ScoreView::startFotomode()
      {
      editObject = _foto;
      _foto->startEdit(this, QPointF());
      qreal w = 8.0 / _matrix.m11();
      qreal h = 8.0 / _matrix.m22();
      QRectF r(-w*.5, -h*.5, w, h);
      _foto->setRect(QRectF(-10, -10, 0, 0));
      for (int i = 0; i < MAX_GRIPS; ++i)
            grip[i] = r;
      curGrip = 0;
      updateGrips();
      _score->addRefresh(_foto->abbox());
      _score->end();
      }

//---------------------------------------------------------
//   stopFotomode
//---------------------------------------------------------

void ScoreView::stopFotomode()
      {
      QAction* a = getAction("fotomode");
      a->setChecked(false);

      _score->addRefresh(_foto->abbox());
      editObject = 0;
      grips      = 0;

      _foto->endEdit();
      _score->end();
      }

//---------------------------------------------------------
//   startFotoDrag
//---------------------------------------------------------

void ScoreView::startFotoDrag()
      {
      _score->addRefresh(_foto->abbox());
      _score->end();
      grips = 0;
      }

//---------------------------------------------------------
//   doDragFoto
//    drag canvas in foto mode
//---------------------------------------------------------

void ScoreView::doDragFoto(QMouseEvent* ev)
      {
      QPointF p = toLogical(ev->pos());
      QRectF r;
      r.setCoords(startMove.x(), startMove.y(), p.x(), p.y());
      _foto->setRect(r.normalized());

      QRectF rr(_foto->rect());
      r = _matrix.mapRect(rr);
      QSize sz(r.size().toSize());
      mscore->statusBar()->showMessage(QString("%1 x %2").arg(sz.width()).arg(sz.height()), 3000);

      _score->addRefresh(_foto->abbox());
      _score->end();
      }

//---------------------------------------------------------
//   endFotoDrag
//---------------------------------------------------------

void ScoreView::endFotoDrag()
      {
      qreal w = 8.0 / _matrix.m11();
      qreal h = 8.0 / _matrix.m22();
      QRectF r(-w*.5, -h*.5, w, h);
      for (int i = 0; i < 8; ++i)
            grip[i] = r;
      editObject = _foto;
      updateGrips();
      _score->setUpdateAll();
      _score->end();
      }

//---------------------------------------------------------
//   doFotoDragEdit
//---------------------------------------------------------

void ScoreView::doFotoDragEdit(QMouseEvent* ev)
      {
      QPointF p     = toLogical(ev->pos());
      QPointF delta = p - startMove;
      _score->setLayoutAll(false);
      score()->addRefresh(_foto->abbox());
      _foto->editDrag(curGrip, delta);
      updateGrips();
      startMove = p;
      _score->end();
      }

//---------------------------------------------------------
//   endFotoDragEdit
//---------------------------------------------------------

void ScoreView::endFotoDragEdit()
      {
      }

//---------------------------------------------------------
//   fotoEditElementDragTransition
//---------------------------------------------------------

bool ScoreView::fotoEditElementDragTransition(QMouseEvent* ev)
      {
      startMove = imatrix.map(QPointF(ev->pos()));
      int i;
      for (i = 0; i < grips; ++i) {
            if (grip[i].contains(startMove)) {
                  curGrip = i;
                  switch(curGrip) {
                        case 0:
                        case 2:
                              setCursor(Qt::SizeFDiagCursor);
                              break;
                        case 1:
                        case 3:
                              setCursor(Qt::SizeBDiagCursor);
                              break;
                        case 4:
                        case 6:
                              setCursor(Qt::SizeVerCursor);
                              break;
                        case 5:
                        case 7:
                              setCursor(Qt::SizeHorCursor);
                              break;
                        }
                  updateGrips();
                  score()->end();
                  break;
                  }
            }
      return i != grips;
      }

//---------------------------------------------------------
//   fotoScoreViewDragTest
//---------------------------------------------------------

bool ScoreView::fotoScoreViewDragTest(QMouseEvent* me)
      {
      QPointF p(imatrix.map(QPointF(me->pos())));
      if (_foto->rect().contains(p))
            return false;
      for (int i = 0; i < grips; ++i) {
            if (grip[i].contains(p))
                  return false;
            }
      startMove = p;
      return true;
      }

//---------------------------------------------------------
//   fotoScoreViewDragRectTest
//---------------------------------------------------------

bool ScoreView::fotoScoreViewDragRectTest(QMouseEvent* me)
      {
      QPointF p(toLogical(me->pos()));
      if (!_foto->rect().contains(p))
            return false;
      for (int i = 0; i < grips; ++i) {
            if (grip[i].contains(p))
                  return false;
            }
      startMove = p;
      return true;
      }

//---------------------------------------------------------
//   doDragFotoRect
//---------------------------------------------------------

void ScoreView::doDragFotoRect(QMouseEvent* ev)
      {
      QPointF p(toLogical(ev->pos()));
      QPointF delta = p - startMove;
      _score->setLayoutAll(false);
      score()->addRefresh(_foto->abbox());
      _foto->setRect(_foto->rect().translated(delta));
      score()->addRefresh(_foto->abbox());
      startMove = p;
      updateGrips();
      _score->end();
      }

//---------------------------------------------------------
//   fotoContextPopup
//---------------------------------------------------------

void ScoreView::fotoContextPopup(QMouseEvent* ev)
      {
      QPoint pos(ev->globalPos());
      QMenu* popup = new QMenu(this);
      popup->setSeparatorsCollapsible(false);
      QAction* a = popup->addSeparator();
      a->setText(tr("Foto-Mode"));

      a = getAction("copy");
      a->setEnabled(true);
      popup->addAction(a);
      popup->addSeparator();
      a = new QAction(*icons[fileSave_ICON], tr("Save As (print mode)..."), this);
      a->setData("print");
      popup->addAction(a);
      a = new QAction(*icons[fileSave_ICON], tr("Save As (screenshot mode)..."), this);
      a->setData("screenshot");
      popup->addAction(a);

      a = popup->exec(pos);
      if (a == 0)
            return;
      QString cmd(a->data().toString());
      if (cmd == "print")
            saveFotoAs(true, _foto->rect());
      else if (cmd == "screenshot")
            saveFotoAs(false, _foto->rect());
      else if (cmd == "copy") {
            QMimeData* mimeData = new QMimeData;

            // oowriter wants transparent==false
            bool transparent = false; // preferences.pngTransparent;
            double convDpi   = DPI; // preferences.pngResolution;
            double mag       = convDpi / DPI;

            QRectF r(_foto->abbox());
            int w = lrint(r.width()  * mag);
            int h = lrint(r.height() * mag);
            QImage::Format f;
            f = QImage::Format_ARGB32_Premultiplied;
            QImage printer(w, h, f);
            printer.setDotsPerMeterX(lrint(DPMM * 1000.0));
            printer.setDotsPerMeterY(lrint(DPMM * 1000.0));
            printer.fill(transparent ? 0 : 0xffffffff);
            QPainter p(&printer);
            paintRect(true, p, r, mag);
            p.end();
            mimeData->setImageData(printer);
            QApplication::clipboard()->setMimeData(mimeData);
            }
      }

//---------------------------------------------------------
//   fotoRectHit
//---------------------------------------------------------

bool ScoreView::fotoRectHit(const QPoint& pos)
      {
      QPointF p = toLogical(pos);
      for (int i = 0; i < grips; ++i) {
            if (grip[i].contains(p))
                  return false;
            }
      startMove = p;
      return _foto->rect().contains(p);
      }

//---------------------------------------------------------
//   saveFotoAs
//    return true on success
//---------------------------------------------------------

bool ScoreView::saveFotoAs(bool printMode, const QRectF& r)
      {
      QStringList fl;
      fl.append(tr("PNG Bitmap Graphic (*.png)"));
      fl.append(tr("PDF File (*.pdf)"));
      fl.append(tr("Encapsulated PostScript File (*.eps)"));
      fl.append(tr("Scalable Vector Graphic (*.svg)"));

      QString saveDialogTitle = tr("MuseScore: Save As");

      QSettings settings;
      if (mscore->lastSaveDirectory.isEmpty())
            mscore->lastSaveDirectory = settings.value("lastSaveDirectory", ".").toString();
      QString saveDirectory = mscore->lastSaveDirectory;

      QString selectedFilter;
      QString fn = QFileDialog::getSaveFileName(
               0,
               saveDialogTitle,
               "",
               fl.join(";;"),
               &selectedFilter
               );
      if (fn.isEmpty())
            return false;

      QFileInfo fi(fn);
      mscore->lastSaveDirectory = fi.absolutePath();

      QString ext;
      if (selectedFilter.isEmpty())
            ext = fi.suffix();
      else {
            int idx = fl.indexOf(selectedFilter);
            if (idx != -1) {
                  static const char* extensions[] = { "png", "pdf", "eps", "svg" };
                  ext = extensions[idx];
                  }
            }
      if (ext.isEmpty()) {
            QMessageBox::critical(mscore, tr("MuseScore: Save As"), tr("cannot determine file type"));
            return false;
            }
      if (fi.suffix() != ext)
            fn += "." + ext;

      bool transparent = preferences.pngTransparent;
      double convDpi   = DPI; // preferences.pngResolution;
      double mag       = convDpi / DPI;

      int w = lrint(r.width()  * mag);
      int h = lrint(r.height() * mag);

      if (ext == "pdf") {
            QPrinter printer(QPrinter::HighResolution);
            printer.setPaperSize(QSizeF(w/DPI, h/DPI) , QPrinter::Inch);
            printer.setCreator("MuseScore Version: " VERSION);
            printer.setFullPage(true);
            printer.setColorMode(QPrinter::Color);
            printer.setDocName(fn);
            printer.setOutputFileName(fn);
            printer.setOutputFormat(QPrinter::PdfFormat);
            QPainter p(&printer);
            paintRect(printMode, p, r, mag);
            }
      else if (ext == "eps") {
            QPrinter printer(QPrinter::HighResolution);
            printer.setPaperSize(QSizeF(w/DPI, h/DPI) , QPrinter::Inch);
            printer.setCreator("MuseScore Version: " VERSION);
            printer.setFullPage(true);
            printer.setColorMode(QPrinter::Color);
            printer.setDocName(fn);
            printer.setOutputFileName(fn);
            printer.setOutputFormat(QPrinter::PostScriptFormat);
            QPainter p(&printer);
            paintRect(printMode, p, r, mag);
            }
      else if (ext == "svg") {
            QSvgGenerator printer;
            printer.setResolution(int(DPI));
            printer.setFileName(fn);
            printer.setSize(QSize(w, h));
            printer.setViewBox(QRect(0, 0, w, h));
            printer.setDescription("created with MuseScore " VERSION);
            QPainter p(&printer);
            paintRect(printMode, p, r, mag);
            }
      else if (ext == "png") {
            QImage::Format f = QImage::Format_ARGB32_Premultiplied;
            QImage printer(w, h, f);
            printer.setDotsPerMeterX(lrint(DPMM * 1000.0));
            printer.setDotsPerMeterY(lrint(DPMM * 1000.0));
            printer.fill(transparent ? 0 : 0xffffffff);

            QPainter p(&printer);
            paintRect(printMode, p, r, mag);
            printer.save(fn, "png");
            }
      else
            printf("unknown extension <%s>\n", qPrintable(ext));
      return true;
      }

//---------------------------------------------------------
//   paintRect
//---------------------------------------------------------

void ScoreView::paintRect(bool printMode, QPainter& p, const QRectF& r, double mag)
      {
      double x = r.x();
      double y = r.y();

      p.translate(-x, -y);
      p.scale(mag, mag);

      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);

      score()->setPrinting(printMode);
      paint1(printMode, r, p);
      score()->setPrinting(false);
      p.end();
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void ScoreView::paint1(bool printMode, const QRectF& fr, QPainter& p)
      {
      foreach (Page* page, _score->pages()) {
            QRectF pr(page->abbox());
            if (pr.right() < fr.left())
                  continue;
            if (pr.left() > fr.right())
                  break;
            QList<const Element*> ell = page->items(fr);
            qStableSort(ell.begin(), ell.end(), elementLessThan);
            drawElements(p, ell);
            }
      }

//---------------------------------------------------------
//   fotoDragDrop
//---------------------------------------------------------

#if 0
void ScoreView::fotoDragDrop(QMouseEvent*)
      {
      QDrag* drag = new QDrag(this);
      QMimeData* mimeData = new QMimeData;

      // oowriter wants transparent==false
      bool transparent = false; // preferences.pngTransparent;
      double convDpi   = DPI; // preferences.pngResolution;
      double mag       = convDpi / DPI;

      QRectF r(_foto->abbox());
      int w = lrint(r.width()  * mag);
      int h = lrint(r.height() * mag);
      QImage::Format f;
      f = QImage::Format_ARGB32_Premultiplied;
      QImage printer(w, h, f);
      printer.setDotsPerMeterX(lrint(DPMM * 1000.0));
      printer.setDotsPerMeterY(lrint(DPMM * 1000.0));
      printer.fill(transparent ? 0 : 0xffffffff);
      QPainter p(&printer);
      paintRect(true, p, r, mag);
      p.end();
#if 0
      QByteArray ba;
      QBuffer ob(&ba);
      ob.open(QIODevice::WriteOnly);
      printer.save(&ob, "PNG");
      mimeData->setData("image/png", ba);
#else
      // this is understood by oowriter
      mimeData->setImageData(printer);
#endif
      drag->setMimeData(mimeData);
      drag->start(Qt::CopyAction);
      }
#endif

void ScoreView::fotoDragDrop(QMouseEvent*)
      {
      bool printMode   = true;
      double convDpi   = DPI; // preferences.pngResolution;
      double mag       = convDpi / DPI;
      QRectF r(_foto->abbox());
      int w            = lrint(r.width()  * mag);
      int h            = lrint(r.height() * mag);

      QTemporaryFile tf(QDir::tempPath() + QString("/imgXXXXXX.eps"));
      tf.setAutoRemove(false);
      tf.open();
      tf.close();
      printf("Temp File <%s>\n", qPrintable(tf.fileName()));

//      QString fn = "/home/ws/mops.eps";
      QString fn = tf.fileName();

      QPrinter printer(QPrinter::HighResolution);
      printer.setPaperSize(QSizeF(w/DPI, h/DPI) , QPrinter::Inch);
      printer.setCreator("MuseScore Version: " VERSION);
      printer.setFullPage(true);
      printer.setColorMode(QPrinter::Color);
      printer.setDocName(fn);
      printer.setOutputFileName(fn);
      printer.setOutputFormat(QPrinter::PostScriptFormat);
      QPainter p(&printer);
      paintRect(printMode, p, r, mag);

      QDrag* drag = new QDrag(this);
      QMimeData* mimeData = new QMimeData;
      QUrl url(QString("file://") + fn);
      QList<QUrl> ul;
      ul.append(url);
      mimeData->setUrls(ul);

      drag->setMimeData(mimeData);
      drag->start(Qt::CopyAction);
      }
