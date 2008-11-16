//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: palette.cpp,v 1.23 2006/03/06 21:08:55 wschweer Exp $
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

#include "palette.h"
#include "mscore.h"
#include "element.h"
#include "style.h"
#include "spatium.h"
#include "globals.h"
#include "sym.h"
#include "symbol.h"
#include "layout.h"
#include "score.h"
#include "image.h"
#include "xml.h"
#include "canvas.h"
#include "note.h"
#include "chord.h"

//---------------------------------------------------------
//   Palette
//---------------------------------------------------------

/**
 Create Symbol palette with \a r rows and \a c columns
*/

Palette::Palette(QWidget* parent)
   : QWidget(parent)
      {
      extraMag      = 1.0;
      staff         = false;
      currentIdx    = -1;
      selectedIdx   = -1;
      _yOffset      = 0;
      hgrid         = 50;
      vgrid         = 60;
      _drawGrid     = false;
      _selectable   = false;
      _readOnly     = true;
      setMouseTracking(true);
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
//      setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
      }

Palette::Palette(qreal mag)
      {
      extraMag      = mag;
      staff         = false;
      currentIdx    = -1;
      selectedIdx   = -1;
      _yOffset      = 0;
      hgrid         = 50;
      vgrid         = 60;
      _drawGrid     = false;
      _selectable   = false;
      _readOnly     = true;
      setMouseTracking(true);
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
//      setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
      }

Palette::~Palette()
      {
      foreach(PaletteCell* cell, cells)
            delete cell;
      }

//---------------------------------------------------------
//   setGrid
//---------------------------------------------------------

/**
 Set size of one element in palette
*/

void Palette::setGrid(int hh, int vv)
      {
      hgrid = hh;
      vgrid = vv;
      }

//---------------------------------------------------------
//   contentsMousePressEvent
//---------------------------------------------------------

void Palette::mousePressEvent(QMouseEvent* ev)
      {
      dragStartPosition = ev->pos();
      if (_selectable) {
            int i = idx(ev->pos());
            if (i != selectedIdx) {
                  update(idxRect(i) | idxRect(selectedIdx));
                  selectedIdx = i;
                  emit boxClicked(i);
                  }
            }
      }

//---------------------------------------------------------
//   applyDrop
//---------------------------------------------------------

static void applyDrop(Score* score, Viewer* viewer, Element* target, Element* e)
      {
      QPointF pt;
      if (target->acceptDrop(viewer, pt, e->type(), e->subtype())) {
            Element* ne = e->clone();
            ne = target->drop(pt, pt, ne);
            if (ne)
                  score->select(ne, SELECT_SINGLE, 0);
            score->canvas()->setDropTarget(0);     // acceptDrop sets dropTarget
            }
      }

//---------------------------------------------------------
//   mouseDoubleClickEvent
//---------------------------------------------------------

void Palette::mouseDoubleClickEvent(QMouseEvent* ev)
      {
      int i               = idx(ev->pos());
      Score* score        = mscore->currentScore();
      Selection* sel      = score->selection();

      if (sel->state() == SEL_NONE)
            return;

      QList<Element*>* el = sel->elements();
      QMimeData* mimeData = new QMimeData;
      Element* element    = cells[i]->element;
      Viewer* viewer      = score->canvas();
      mimeData->setData(mimeSymbolFormat, element->mimeData(QPointF()));

      score->startCmd();
      if (sel->state() == SEL_SINGLE || sel->state() == SEL_MULT) {
            foreach(Element* e, *el)
                  applyDrop(score, viewer, e, element);
            }
      else if (sel->state() == SEL_STAFF || sel->state() == SEL_SYSTEM) {
            int track1 = sel->staffStart * VOICES;
            int track2 = sel->staffEnd * VOICES;
            for (Segment* s = sel->startSegment(); s && s != sel->endSegment(); s = s->next1()) {
                  for (int track = track1; track < track2; ++track) {
                        Element* e = s->element(track);
                        if (e == 0)
                              continue;
                        if (e->type() == CHORD) {
                              Chord* chord = static_cast<Chord*>(e);
                              NoteList* nl = chord->noteList();
                              for (iNote i = nl->begin(); i != nl->end(); ++i)
                                    applyDrop(score, viewer, i->second, element);

                              }
                        else
                              applyDrop(score, viewer, e, element);
                        }
                  }
            }
      else
            printf("unknown selection state\n");
      score->endCmd();
      }

//---------------------------------------------------------
//   idx
//---------------------------------------------------------

int Palette::idx(const QPoint& p) const
      {
      int x = p.x();
      int y = p.y();

      int row = y / vgrid;
      int col = x / hgrid;

      int nc = columns();
      if (col > nc)
            return -1;

      int idx = row * nc + col;
      if (idx >= cells.size())
            return -1;
      return idx;
      }

//---------------------------------------------------------
//   idxRect
//---------------------------------------------------------

QRect Palette::idxRect(int i)
      {
      if (i == -1)
            return QRect();
      if (columns() == 0)
            return QRect();
      int cc = i % columns();
      int cr = i / columns();
      return QRect(cc * hgrid, cr * vgrid, hgrid, vgrid);
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void Palette::mouseMoveEvent(QMouseEvent* ev)
      {
      if ((currentIdx != -1) && (ev->buttons() & Qt::LeftButton)
         && (ev->pos() - dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
            QDrag* drag = new QDrag(this);
            QMimeData* mimeData = new QMimeData;
            if (cells[currentIdx]) {
                  Element* el = cells[currentIdx]->element;
                  QRect ir = idxRect(currentIdx);
                  QPoint sp = dragStartPosition - ir.topLeft();

                  qreal mag = PALETTE_SPATIUM * extraMag / _spatium;
// printf("pos %d %d   %f %f  mag %f\n", sp.x(), sp.y(), el->pos().x(), el->pos().y(), mag);
                  QPointF spos = QPointF(sp) / mag;
                  QPointF rpos(spos - el->pos());
                  rpos /= mag;

                  mimeData->setData(mimeSymbolFormat, el->mimeData(rpos));
                  drag->setMimeData(mimeData);

                  int srcIdx = currentIdx;
                  emit startDragElement(el);
                  if (_readOnly) {
                        drag->start(Qt::CopyAction);
                        }
                  else {
                        Qt::DropAction action = drag->start(Qt::CopyAction | Qt::MoveAction);
                        if (action == Qt::MoveAction) {
                              cells[srcIdx] = 0;
                              }
                        }
                  }
            }
      else {
            QRect r;
            if (currentIdx != -1)
                  r = idxRect(currentIdx);
            currentIdx = idx(ev->pos());
            if (currentIdx != -1) {
                  if (cells[currentIdx] == 0)
                        currentIdx = -1;
                  else
                        r |= idxRect(currentIdx);
                  }
            update(r);
            }
      }

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void Palette::leaveEvent(QEvent*)
      {
      if (currentIdx != -1) {
            QRect r = idxRect(currentIdx);
            currentIdx = -1;
            update(r);
            }
      }

//---------------------------------------------------------
//   addObject
//---------------------------------------------------------

void Palette::addObject(int idx, Element* s, const QString& name)
      {
      PaletteCell* cell = new PaletteCell;

      if (idx < cells.size()) {
            if (cells[idx])
                  delete cells[idx];
            }
      else {
            for (int i = cells.size(); i <= idx; ++i)
                  cells.append(0);
            }
      cells[idx]     = cell;
      cell->element  = s;
      cell->name     = name;
      update();
      if (s && s->type() == ICON) {
            Icon* icon = static_cast<Icon*>(s);
            connect(icon->action(), SIGNAL(toggled(bool)), SLOT(actionToggled(bool)));
            }
      }

//---------------------------------------------------------
//   addObject
//---------------------------------------------------------

void Palette::addObject(int idx, int symIdx)
      {
      Symbol* s = new Symbol(0);
      s->setSym(symIdx);
      addObject(idx, s, ::symbols[symIdx].name());
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void Palette::paintEvent(QPaintEvent*)
      {
      qreal mag = PALETTE_SPATIUM * extraMag / _spatium;
      ScoreLayout layout(gscore);
      layout.setSpatium(_spatium);
      layout.setPaintDevice(this);

      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, true);

      //
      // draw grid
      //

      if (_drawGrid) {
            p.setPen(Qt::gray);
            for (int row = 1; row < rows(); ++row)
                  p.drawLine(0, row*vgrid, columns()*hgrid, row*vgrid);
            for (int column = 1; column < columns(); ++column)
                  p.drawLine(hgrid*column, 0, hgrid*column, rows()*vgrid);
            }

      qreal dy = lrint(2 * PALETTE_SPATIUM * extraMag);

      //
      // draw symbols
      //

      QPen pen(QColor(Qt::black));
      pen.setWidthF(defaultStyle.staffLineWidth.val() * PALETTE_SPATIUM * extraMag);

      int c = columns();
      for (int idx = 0; idx < cells.size(); ++idx) {
            QRect r = idxRect(idx);
            p.setPen(pen);
            if (idx == selectedIdx) {
                  if (idx == currentIdx)
                        p.fillRect(r, QColor(220, 220, 200));
                  else
                        p.fillRect(r, QColor(200, 200, 180));
                  }
            else if (idx == currentIdx)
                  p.fillRect(r, p.background().color().light(118));
            if (cells[idx] == 0)
                  continue;
            Element* el = cells[idx]->element;
            if (el->type() != ICON) {
                  int row    = idx / c;
                  int column = idx % c;
                  // hack:
                  el->layout(&layout);
                  el->setPos(0.0, 0.0);   // HACK

                  if (staff) {
                        qreal y = r.y() + vgrid * .5 - dy + _yOffset;
                        qreal x = r.x() + 3;
                        qreal w = hgrid - 6;
                        for (int i = 0; i < 5; ++i) {
                              qreal yy = y + PALETTE_SPATIUM * i * extraMag;
                              p.drawLine(QLineF(x, yy, x + w, yy));
                              }
                        }
                  p.save();
                  p.scale(mag, mag);

                  double gw = hgrid / mag;
                  double gh = vgrid / mag;
                  double gx = column * gw;
                  double gy = row    * gh;

                  double sw = el->width();
                  double sh = el->height();
                  double sy;

                  if (staff)
                        sy = gy + gh * .5 - 2.0 * _spatium;
                  else
                        sy  = gy + (gh - sh) * .5 - el->bbox().y();
                  double sx  = gx + (gw - sw) * .5 - el->bbox().x();

                  sy += _yOffset / mag;

                  QList<const Element*> elist;
                  el->collectElements(elist);
                  p.translate(QPointF(sx, sy));
// printf("%f %f\n", sx, sy);
                  foreach(const Element* e, elist) {
                        p.save();
                        p.translate(e->pos());
                        p.setPen(QPen(e->curColor()));
                        e->draw(p);
                        p.restore();
                        }
                  p.restore();
                  }
            else {
                  int x      = r.x();
                  int y      = r.y();
                  QIcon icon = ((Icon*)el)->icon();
                  int border = 2;
                  int size   = (hgrid < vgrid ? hgrid : vgrid) - 2 * border;
                  p.drawPixmap(x + (hgrid - size) / 2, y + (vgrid - size) / 2,
                     icon.pixmap(size, QIcon::Normal, QIcon::On)
                     );
                  }
            }
      }

//---------------------------------------------------------
//   event
//---------------------------------------------------------

bool Palette::event(QEvent* ev)
      {
      if (ev->type() == QEvent::ToolTip) {
            QHelpEvent* he = (QHelpEvent*)ev;
            int x = he->pos().x();
            int y = he->pos().y();

            int row = y / vgrid;
            int col = x / hgrid;

            if (row < 0 || row >= rows())
                  return false;
            if (col < 0 || col >= columns())
                  return false;
            int idx = row * columns() + col;
            if (idx >= cells.size())
                  return false;
            if (cells[idx] == 0)
                  return false;
            QToolTip::showText(he->globalPos(), cells[idx]->name, this);
            return false;
            }
      return QWidget::event(ev);
      }

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void Palette::dragEnterEvent(QDragEnterEvent* event)
      {
      const QMimeData* data = event->mimeData();
      if (data->hasUrls()) {
            QList<QUrl>ul = event->mimeData()->urls();
            QUrl u = ul.front();
            if (debugMode)
                  printf("drag Url: %s\n", u.toString().toLatin1().data());
            printf("scheme <%s> path <%s>\n", u.scheme().toLatin1().data(),
               u.path().toLatin1().data());
            if (u.scheme() == "file") {
                  QFileInfo fi(u.path());
                  if (fi.suffix() == "svg"
                     || fi.suffix() == "jpg"
                     || fi.suffix() == "png"
                     || fi.suffix() == "xpm"
                     ) {
                        event->acceptProposedAction();
                        }
                  }
            }
      else if (data->hasFormat(mimeSymbolFormat))
            event->acceptProposedAction();
      else {
            if (debugMode) {
                  printf("dragEnterEvent: formats:\n");
                  foreach(QString s, event->mimeData()->formats())
                        printf("   %s\n", s.toLatin1().data());
                  }
            }
      }

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void Palette::dragMoveEvent(QDragMoveEvent* ev)
      {
      int i = idx(ev->pos());
      if (i == -1)
            return;

      int n = rows() * columns();
      int ii = i;
      for (; ii < n; ++ii) {
            if (cells[ii] == 0)
                  break;
            }
      if (ii == n)
            return;

      QRect r;
      if (currentIdx != -1)
            r = idxRect(currentIdx);
      update(r | idxRect(ii));
      currentIdx = ii;
      }

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void Palette::dropEvent(QDropEvent* event)
      {
      Element* e = 0;
      QString name;

      const QMimeData* data = event->mimeData();
      if (data->hasUrls()) {
            QList<QUrl>ul = event->mimeData()->urls();
            QUrl u = ul.front();
            if (u.scheme() == "file") {
                  QFileInfo fi(u.path());
                  Image* s = 0;
                  if (fi.suffix() == "svg")
                        s = new SvgImage(0);
                  else if (fi.suffix() == "jpg"
                     || fi.suffix() == "png"
                     || fi.suffix() == "xpm"
                        )
                        s = new RasterImage(0);
                  else
                        return;
                  qreal mag = PALETTE_SPATIUM * extraMag / _spatium;
                  s->setPath(u.path());
                  s->setSize(QSizeF(hgrid / mag, hgrid / mag));
//                  s->setAnchor(ANCHOR_PARENT);
                  e = s;
                  name = s->path();
                  }
            }
      else if (data->hasFormat(mimeSymbolFormat)) {
            QByteArray data(event->mimeData()->data(mimeSymbolFormat));
            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(data, &err, &line, &column)) {
                  printf("error reading drag data\n");
                  return;
                  }
            docName = "--";
            QDomElement el = doc.documentElement();
            QPointF dragOffset;
            int type = Element::readType(el, &dragOffset);

            if (type == IMAGE) {
                  // look ahead for image type
                  QString path;
                  for (QDomElement ee = el.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "path") {
                              path = ee.text();
                              break;
                              }
                        }
                  Image* image = 0;
                  if (path.endsWith(".svg"))
                        image = new SvgImage(0);
                  else if (path.endsWith(".jpg")
                     || path.endsWith(".png")
                     || path.endsWith(".xpm")
                        )
                        image = new RasterImage(0);
                  else {
                        printf("unknown image format <%s>\n", path.toLatin1().data());
                        }
                  if (image) {
                        image->read(el);
                        e = image;
                        }
                  }
            else if (type == SYMBOL) {
                  Symbol* s = new Symbol(0);
                  s->read(el);
                  e = s;
                  }
            }
      if (e) {
            int i = idx(event->pos());
            if (i == -1)
                  return;

            int n = rows() * columns();
            int ii = i;
            for (; ii < n; ++ii) {
                  if (cells[ii] == 0)
                        break;
                  }
            addObject(ii, e, name);
            emit droppedElement(e);
            if (event->source() == this) {
                  event->setDropAction(Qt::MoveAction);
                  event->accept();
                  }
            else
                  event->acceptProposedAction();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Palette::write(Xml& xml, const char* name) const
      {
      xml.stag(QString(name));
      int n = cells.size();
      for (int i = 0; i < n; ++i) {
            if (cells[i] == 0)
                  continue;
            xml.tag("idx", i);
            if (!cells[i]->name.isEmpty())
                  xml.tag("name", cells[i]->name);
            cells[i]->element->write(xml);
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Palette::read(QDomElement e)
      {
      int idx = 0;
      QString name = "";
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "idx") {
                  idx = e.text().toInt();
                  name = "";
                  }
            else if (tag == "name") {
                  name = e.text();
                  }
            else if (tag == "rows")
                  ;
            else if (tag == "columns")
                  ;
            else if (tag == "Image") {
                  // look ahead for image type
                  QString path;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "path") {
                              path = ee.text();
                              break;
                              }
                        }
                  Image* image = 0;
                  if (path.endsWith(".svg"))
                        image = new SvgImage(0);
                  else if (path.endsWith(".jpg")
                     || path.endsWith(".png")
                     || path.endsWith(".xpm")
                        )
                        image = new RasterImage(0);
                  else {
                        printf("unknown image format <%s>\n", path.toLatin1().data());
                        }
                  if (image) {
                        image->read(e);
                        addObject(idx, image, name);
                        }
                  }
            else if (tag == "Symbol") {
                  Symbol* s = new Symbol(0);
                  s->read(e);
                  addObject(idx, s, name);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Palette::clear()
      {
      foreach(PaletteCell* cell, cells)
            delete cell;
      cells.clear();
      }

//---------------------------------------------------------
//   PaletteBoxButton
//---------------------------------------------------------

PaletteBoxButton::PaletteBoxButton(QWidget* w, QWidget* parent)
   : QPushButton(parent)
      {
      setCheckable(true);
//      setFlat(true);
      setFocusPolicy(Qt::NoFocus);
      connect(this, SIGNAL(clicked(bool)), w, SLOT(setVisible(bool)));
      setFixedHeight(QFontMetrics(font()).height() + 2);
      }

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void PaletteBoxButton::changeEvent(QEvent* ev)
      {
      if (ev->type() == QEvent::FontChange)
            setFixedHeight(QFontMetrics(font()).height() + 2);
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void PaletteBoxButton::paintEvent(QPaintEvent* e)
      {
      QPushButton::paintEvent(e);
      }

//---------------------------------------------------------
//   PaletteBox
//---------------------------------------------------------

PaletteBox::PaletteBox(QWidget* parent)
   : QDockWidget(tr("Palettes"), parent)
      {
      setObjectName("palette-box");
      setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
      QWidget* mainWidget = new QWidget;
      vbox = new QVBoxLayout;
      vbox->setMargin(0);
      vbox->setSpacing(0);
      mainWidget->setLayout(vbox);
      vbox->addStretch(1);
      setWidget(mainWidget);
      }

//---------------------------------------------------------
//   PaletteBox::sizeHint
//---------------------------------------------------------

QSize PaletteBox::sizeHint() const
      {
//      return QSize(180, 10);
      return QSize();
      }

//---------------------------------------------------------
//   PaletteScrollArea
//---------------------------------------------------------

PaletteScrollArea::PaletteScrollArea(QWidget* w, QWidget* parent)
   : QScrollArea(parent)
      {
      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      setWidget(w);
//      QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//      policy.setHeightForWidth(true);
//      setSizePolicy(policy);
//      setWidgetResizable(true);
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void PaletteScrollArea::resizeEvent(QResizeEvent* re)
      {
      Palette* palette = static_cast<Palette*>(widget());
      int h = palette->resizeWidth(width());
      setMaximumHeight(h+8);
      QScrollArea::resizeEvent(re);
      }

//---------------------------------------------------------
//   resizeWidth
//---------------------------------------------------------

int Palette::resizeWidth(int w)
      {
      int c = w / hgrid;
      int r = (cells.size() + c - 1) / c;
      int h = r * vgrid;
      setFixedSize(w, h);
      return h;
      }

//---------------------------------------------------------
//   addPalette
//---------------------------------------------------------

void PaletteBox::addPalette(const QString& s, QWidget* w)
      {
      PaletteScrollArea* sa = new PaletteScrollArea(w);
      sa->setVisible(false);
      PaletteBoxButton* b = new PaletteBoxButton(sa);
      b->setText(s);
      int slot = widgets.size() * 2;
      vbox->insertWidget(slot, b);
      vbox->insertWidget(slot+1, sa, 1000);
      widgets.append(w);
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void PaletteBox::closeEvent(QCloseEvent* ev)
      {
      emit paletteVisible(false);
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   actionToggled
//---------------------------------------------------------

void Palette::actionToggled(bool /*val*/)
      {
      selectedIdx = -1;
      for (int n = 0; n < (rows() * columns()); ++n) {
            Element* e = cells[n]->element;
            if (e && e->type() == ICON) {
                  if (((Icon*)e)->action()->isChecked()) {
                        selectedIdx = n;
                        break;
                        }
                  }
            }
      update();
      }

//---------------------------------------------------------
//   rows
//---------------------------------------------------------

int Palette::rows() const
      {
      int c = columns();
      if (c == 0)
            return 0;
      return (cells.size() + c - 1) / c;
      }

