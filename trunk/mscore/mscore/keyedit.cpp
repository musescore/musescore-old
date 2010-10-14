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
#include "palette.h"
#include "keyedit.h"
#include "keysig.h"
#include "score.h"
#include "accidental.h"
#include "keycanvas.h"
#include "clef.h"

extern bool useFactorySettings;
extern Palette* newAccidentalsPalette();
extern Palette* newKeySigPalette();

//---------------------------------------------------------
//   KeyCanvas
//---------------------------------------------------------

KeyCanvas::KeyCanvas(QWidget* parent)
   : QFrame(parent)
      {
      setAcceptDrops(true);
      extraMag   = 2.0;
      qreal mag  = PALETTE_SPATIUM * extraMag / gscore->spatium();
      _matrix    = QTransform(mag, 0.0, 0.0, mag, 0.0, 0.0);
      imatrix    = _matrix.inverted();
      dragElement = 0;
      setFocusPolicy(Qt::StrongFocus);
      QAction* a = new QAction("delete", this);
      a->setShortcut(Qt::Key_Delete);
      addAction(a);
      clef = new Clef(gscore);
      clef->setClefType(CLEF_G);
      connect(a, SIGNAL(triggered()), SLOT(deleteElement()));
      }

//---------------------------------------------------------
//   delete
//---------------------------------------------------------

void KeyCanvas::deleteElement()
      {
      foreach(Accidental* a, accidentals) {
            if (a->selected()) {
                  accidentals.removeOne(a);
                  delete a;
                  update();
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void KeyCanvas::clear()
      {
      foreach(Accidental* a, accidentals)
            delete a;
      accidentals.clear();
      update();
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void KeyCanvas::paintEvent(QPaintEvent*)
      {
      double spatium = 2.0 * PALETTE_SPATIUM / (PDPI/DPI * extraMag);
      gscore->setSpatium(spatium);
      gscore->setPaintDevice(this);

      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, true);
      qreal wh = double(height());
      qreal ww = double(width());
      double y = wh * .5 - 2 * PALETTE_SPATIUM * extraMag;

      qreal mag  = PALETTE_SPATIUM * extraMag / gscore->spatium();
      _matrix    = QTransform(mag, 0.0, 0.0, mag, 0.0, y);
      imatrix    = _matrix.inverted();

      qreal x = 3;
      qreal w = ww - 6;

      p.setWorldTransform(_matrix);

      QRectF r = imatrix.mapRect(QRectF(x, y, w, wh));

      QPen pen(palette().brush(QPalette::Normal, QPalette::Text).color());
      pen.setWidthF(defaultStyle->valueS(ST_staffLineWidth).val() * spatium);
      p.setPen(pen);

      for (int i = 0; i < 5; ++i) {
            qreal yy = r.y() + i * spatium;
            p.drawLine(QLineF(r.x(), yy, r.x() + r.width(), yy));
            }
      if (dragElement) {
            p.save();
            p.translate(dragElement->canvasPos());
            dragElement->draw(p, 0);
            p.restore();
            }
      foreach(Accidental* a, accidentals) {
            p.save();
            p.translate(a->canvasPos());
            p.setPen(QPen(a->curColor()));
            a->draw(p, 0);
            p.restore();
            }
      clef->setPos(0.0, 0.0);
      clef->layout();
      p.translate(clef->canvasPos());
      clef->draw(p, 0);
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void KeyCanvas::mousePressEvent(QMouseEvent* event)
      {
      startMove = imatrix.map(QPointF(event->pos() - base));
      moveElement = 0;
      foreach(Accidental* a, accidentals) {
            QRectF r = a->abbox();
            if (r.contains(startMove)) {
                  a->setSelected(true);
                  moveElement = a;
                  }
            else
                  a->setSelected(false);
            }
      update();
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void KeyCanvas::mouseMoveEvent(QMouseEvent* event)
      {
      if (moveElement == 0)
            return;
      QPointF p = imatrix.map(QPointF(event->pos()));
      QPointF delta = p - startMove;
      moveElement->movePos(delta);
      startMove = p;
      update();
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void KeyCanvas::mouseReleaseEvent(QMouseEvent*)
      {
      if (moveElement == 0)
            return;
      snap(moveElement);
      update();
      }

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void KeyCanvas::dragEnterEvent(QDragEnterEvent* event)
      {
      const QMimeData* data = event->mimeData();
      if (data->hasFormat(mimeSymbolFormat)) {
            QByteArray a = data->data(mimeSymbolFormat);

            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(a, &err, &line, &column)) {
                  printf("error reading drag data at %d/%d: %s\n<%s>\n",
                     line, column, err.toLatin1().data(), a.data());
                  return;
                  }
            docName = "--";
            QDomElement e = doc.documentElement();

            QPointF dragOffset;
            ElementType type = Element::readType(e, &dragOffset);
            if (type != ACCIDENTAL)
                  return;

            event->acceptProposedAction();
            dragElement = static_cast<Accidental*>(Element::create(type, gscore));
            dragElement->setParent(0);
            dragElement->read(e);
            dragElement->layout();
            }
      else {
            if (debugMode) {
                  printf("KeyCanvas::dragEnterEvent: formats:\n");
                  foreach(const QString& s, event->mimeData()->formats())
                        printf("   %s\n", qPrintable(s));
                  }
            }
      }

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void KeyCanvas::dragMoveEvent(QDragMoveEvent* event)
      {
      if (dragElement) {
            event->acceptProposedAction();
            QPointF pos(imatrix.map(QPointF(event->pos())));
            dragElement->setPos(pos);
            update();
            }
      }

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void KeyCanvas::dropEvent(QDropEvent*)
      {
      foreach(Accidental* a, accidentals)
            a->setSelected(false);
      dragElement->setSelected(true);
      accidentals.append(dragElement);
      snap(dragElement);
      dragElement = 0;
      update();
      }

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

void KeyCanvas::snap(Accidental* a)
      {
      double y = a->ipos().y();
      double spatium2 = PALETTE_SPATIUM / (PDPI/DPI * extraMag);
      int line = int(y / spatium2);
      y = line * spatium2;
      a->rypos() = y;
      }

//---------------------------------------------------------
//   KeyEditor
//---------------------------------------------------------

KeyEditor::KeyEditor(QWidget* parent)
   : QWidget(parent, Qt::Dialog | Qt::Window)
      {
      setupUi(this);
      setWindowTitle(tr("MuseScore: Key Signatures"));

      // create key signature palette

      QLayout* l = new QVBoxLayout();
      l->setContentsMargins(0, 0, 0, 0);
      frame->setLayout(l);

      sp = newKeySigPalette();
      sp->setReadOnly(false);

      PaletteScrollArea* keyPalette = new PaletteScrollArea(sp);
      QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      keyPalette->setSizePolicy(policy);
      keyPalette->setRestrictHeight(false);

      l->addWidget(keyPalette);

      // create accidental palette

      l = new QVBoxLayout();
      l->setContentsMargins(0, 0, 0, 0);
      frame_3->setLayout(l);
      sp1 = newAccidentalsPalette();
      PaletteScrollArea* accPalette = new PaletteScrollArea(sp1);
      QSizePolicy policy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
      accPalette->setSizePolicy(policy1);
      accPalette->setRestrictHeight(false);

      l->addWidget(accPalette);

      connect(addButton, SIGNAL(clicked()), SLOT(addClicked()));
      connect(clearButton, SIGNAL(clicked()), SLOT(clearClicked()));

      if (!useFactorySettings) {
            QFile f(dataPath + "/" + "keysigs.xml");
            if (f.exists() && sp->read(&f))
                  return;
            }
      }

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void KeyEditor::addClicked()
      {
      QList<KeySym*> symbols;

      double extraMag = 2.0;
      const QList<Accidental*> al = canvas->getAccidentals();
      double spatium = 2.0 * PALETTE_SPATIUM / (PDPI/DPI * extraMag);
      double xoff = 10000000.0;
      foreach(Accidental* a, al) {
            QPointF pos = a->ipos();
            if (pos.x() < xoff)
                  xoff = pos.x();
            }
      foreach(Accidental* a, al) {
            KeySym* s = new KeySym;
            s->sym = a->symbol();
            QPointF pos = a->ipos();
            pos.rx() -= xoff;
            s->spos = pos / spatium;
            symbols.append(s);
            }

      KeySig* ks = new KeySig(gscore);
      ks->setCustom(symbols);
      sp->append(ks, "custom");
      _dirty = true;
      }

//---------------------------------------------------------
//   clearClicked
//---------------------------------------------------------

void KeyEditor::clearClicked()
      {
      canvas->clear();
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void KeyEditor::save()
      {
      QDir dir;
      dir.mkpath(dataPath);
      sp->write(dataPath + "/" + "keysigs.xml");
      }

//---------------------------------------------------------
//   showKeyEditor
//---------------------------------------------------------

void MuseScore::showKeyEditor()
      {
      if (keyEditor == 0) {
            keyEditor = new KeyEditor(0);
            }
      keyEditor->show();
      keyEditor->raise();
      }

