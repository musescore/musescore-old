//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "inspector.h"
#include "mscore.h"
#include "element.h"
#include "page.h"
#include "segment.h"
#include "score.h"
#include "rest.h"
#include "note.h"
#include "chord.h"
#include "measure.h"
#include "text.h"
#include "textstyle.h"
#include "hairpin.h"
#include "beam.h"
#include "tuplet.h"
#include "globals.h"
#include "clef.h"
#include "barline.h"
#include "hook.h"
#include "dynamics.h"
#include "slur.h"
#include "lyrics.h"
#include "volta.h"
#include "line.h"
#include "textline.h"
#include "system.h"
#include "arpeggio.h"
#include "glissando.h"
#include "tremolo.h"
#include "articulation.h"

extern bool useFactorySettings;

//---------------------------------------------------------
//   ElementItem
//---------------------------------------------------------

class ElementItem : public QTreeWidgetItem
      {
      Element* el;

   public:
      ElementItem(QTreeWidget* lv, Element* e);
      ElementItem(QTreeWidgetItem* ei, Element* e);
      Element* element() const { return el; }
      void init();
      };

ElementItem::ElementItem(QTreeWidget* lv, Element* e)
   : QTreeWidgetItem(lv, e->type())
      {
      el = e;
      init();
      }

ElementItem::ElementItem(QTreeWidgetItem* ei, Element* e)
   : QTreeWidgetItem(ei, e->type())
      {
      el = e;
      init();
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void ElementItem::init()
      {
      QString s;
      switch(el->type()) {
            case PAGE:
                  {
                  QString no;
                  no.setNum(((Page*)el)->no()+1);
                  s = "Page-" + no;
                  }
                  break;
            case MEASURE:
                  {
                  QString no;
                  no.setNum(((Measure*)el)->no()+1);
                  s = "Measure-" + no;
                  }
                  break;
            default:
                  s = el->name();
                  break;
            }
      setText(0, s);
      }

//---------------------------------------------------------
//   Inspector
//---------------------------------------------------------

Inspector::Inspector(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowTitle(tr("MuseScore: Object Inspector"));

      curElement   = 0;
      pagePanel    = new ShowPageWidget;
      systemPanel  = new ShowSystemWidget;
      measurePanel = new MeasureView;
      chordPanel   = new ShowChordWidget;
      notePanel    = new ShowNoteWidget;
      restPanel    = new ShowRestWidget;
      timesigPanel = new ShowTimesigWidget;
      keysigPanel  = new ShowKeysigWidget;
      clefPanel    = new ShowClefWidget;
      segmentView  = new SegmentView;
      textView     = new TextView;
      elementView  = new ElementView;
      hairpinView  = new HairpinView;
      barLineView  = new BarLineView;
      dynamicView  = new DynamicView;
      tupletView   = new TupletView;
      slurView     = new SlurView;
      tieView      = new TieView;
      voltaView    = new VoltaView;
      voltaSegmentView = new VoltaSegmentView;
      lyricsView   = new LyricsView;
      beamView     = new BeamView;
      tremoloView  = new TremoloView;

      stack->addWidget(pagePanel);
      stack->addWidget(systemPanel);
      stack->addWidget(measurePanel);
      stack->addWidget(chordPanel);
      stack->addWidget(notePanel);
      stack->addWidget(restPanel);
      stack->addWidget(timesigPanel);
      stack->addWidget(keysigPanel);
      stack->addWidget(clefPanel);
      stack->addWidget(segmentView);
      stack->addWidget(textView);
      stack->addWidget(elementView);
      stack->addWidget(hairpinView);
      stack->addWidget(barLineView);
      stack->addWidget(dynamicView);
      stack->addWidget(tupletView);
      stack->addWidget(slurView);
      stack->addWidget(tieView);
      stack->addWidget(voltaView);
      stack->addWidget(voltaSegmentView);
      stack->addWidget(lyricsView);
      stack->addWidget(beamView);
      stack->addWidget(tremoloView);

      connect(pagePanel,    SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(systemPanel,  SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(measurePanel, SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(chordPanel,   SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(notePanel,    SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(restPanel,    SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(timesigPanel, SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(keysigPanel,  SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(clefPanel,    SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(segmentView,  SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(textView,     SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(elementView,  SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(hairpinView,  SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(barLineView,  SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(dynamicView,  SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(tupletView,   SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(slurView,     SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(tieView,      SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(voltaView,    SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(voltaSegmentView, SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(lyricsView,   SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(beamView,     SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(tremoloView,  SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
      connect(tupletView,   SIGNAL(scoreChanged()), SLOT(layoutScore()));
      connect(notePanel,    SIGNAL(scoreChanged()), SLOT(layoutScore()));

      connect(list, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(itemClicked(QTreeWidgetItem*,int)));
      connect(list, SIGNAL(itemExpanded(QTreeWidgetItem*)), SLOT(itemExpanded(QTreeWidgetItem*)));
      connect(list, SIGNAL(itemCollapsed(QTreeWidgetItem*)), SLOT(itemExpanded(QTreeWidgetItem*)));
      list->resizeColumnToContents(0);
      if (!useFactorySettings) {
            QSettings settings;
            settings.beginGroup("Inspector");
            split->restoreState(settings.value("splitter").toByteArray());
            resize(settings.value("size", QSize(1000, 500)).toSize());
            move(settings.value("pos", QPoint(10, 10)).toPoint());
            settings.endGroup();
            }
      back->setEnabled(false);
      forward->setEnabled(false);
      connect(back,    SIGNAL(clicked()), SLOT(backClicked()));
      connect(forward, SIGNAL(clicked()), SLOT(forwardClicked()));
      connect(reload,  SIGNAL(clicked()), SLOT(reloadClicked()));
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void Inspector::writeSettings()
      {
      QSettings settings;
      settings.beginGroup("Inspector");
      settings.setValue("size", size());
      settings.setValue("pos", pos());
      settings.setValue("splitter", split->saveState());
      settings.endGroup();
      }

//---------------------------------------------------------
//   layoutScore
//---------------------------------------------------------

void Inspector::layoutScore()
      {
      cs->setLayoutAll(true);
      cs->end();
      }

//---------------------------------------------------------
//   addSymbol
//---------------------------------------------------------

void Inspector::addSymbol(ElementItem* parent, BSymbol* bs)
      {
      const QList<Element*>el = bs->getLeafs();
      ElementItem* i = new ElementItem(parent, bs);
      if (!el.isEmpty()) {
            foreach(Element* g, el)
                  addSymbol(i, static_cast<BSymbol*>(g));
            }
      }

//---------------------------------------------------------
//   addMeasureBaseToList
//---------------------------------------------------------

static void addMeasureBaseToList(ElementItem* mi, MeasureBase* mb)
      {
      foreach(Element* e, *mb->el()) {
            ElementItem* mmi = new ElementItem(mi, e);
            if (e->type() == HBOX || e->type() == VBOX)
                  addMeasureBaseToList(mmi, static_cast<MeasureBase*>(e));
            }
      }

//---------------------------------------------------------
//   showEvent
//---------------------------------------------------------

void Inspector::showEvent(QShowEvent*)
      {
      updateList(cs);
      }

//---------------------------------------------------------
//   updateList
//---------------------------------------------------------

void Inspector::updateList(Score* s)
      {
      if (cs != s) {
            backStack.clear();
            forwardStack.clear();
            back->setEnabled(false);
            forward->setEnabled(false);
            cs = s;
            }
      curElement = 0;
      list->clear();
      if (!isVisible())
            return;

      QTreeWidgetItem* li = new QTreeWidgetItem(list, INVALID);
      li->setText(0, "Global");
      foreach(Element* el, *cs->gel()) {
            if (el->type() == SLUR) {
                  ElementItem* se = new ElementItem(li, el);
                  Slur* slur = (Slur*)el;
                  foreach(Element* el1, *slur->slurSegments())
                        new ElementItem(se, el1);
                  }
            else if (el->type() == VOLTA) {
                  ElementItem* ei = new ElementItem(li, el);
                  Volta* volta = static_cast<Volta*>(el);
                  if (volta->beginText())
                        new ElementItem(ei, volta->beginText());
                  if (volta->continueText())
                        new ElementItem(ei, volta->continueText());
                  SLine* line = static_cast<SLine*>(el);
                  foreach(LineSegment* ls, line->lineSegments()) {
                        ElementItem* sse = new ElementItem(ei, ls);
                        if (ls->type() == TEXTLINE_SEGMENT) {
                              if (static_cast<TextLineSegment*>(ls)->text())
                                    new ElementItem(sse, static_cast<TextLineSegment*>(ls)->text());
                              }
                        }
                  }
            else if (el->isSLine()) {
                  ElementItem* se = new ElementItem(li, el);
                  SLine* line = static_cast<SLine*>(el);
                  foreach(LineSegment* ls, line->lineSegments()) {
                        ElementItem* sse = new ElementItem(se, ls);
                        if (ls->type() == TEXTLINE_SEGMENT) {
                              if (static_cast<TextLineSegment*>(ls)->text())
                                    new ElementItem(sse, static_cast<TextLineSegment*>(ls)->text());
                              }
                        }
                  }
            else
                  new ElementItem(li, el);
            }
      foreach(Beam* beam, cs->beams())
	      new ElementItem(li, beam);

      int staves = cs->nstaves();
      int tracks = staves * VOICES;
      foreach(Page* page, cs->pages()) {
            ElementItem* pi = new ElementItem(list, page);

            if (page->copyright())
                  new ElementItem(pi, page->copyright());
            if (page->pageNo())
                  new ElementItem(pi, page->pageNo());

            foreach(System* system, *page->systems()) {
                  ElementItem* si = new ElementItem(pi, system);

                  if (system->getBarLine())
                        new ElementItem(si, system->getBarLine());

                  // SysStaffList* staffList = system->staves();

                  foreach (MeasureBase* mb, system->measures()) {
                        ElementItem* mi = new ElementItem(si, mb);

                        addMeasureBaseToList(mi, mb);

                        if (mb->type() != MEASURE)
                              continue;
                        Measure* measure = (Measure*) mb;
                        if (measure->noText())
                              new ElementItem(mi, measure->noText());
                        for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                              ElementItem* segItem = new ElementItem(mi, segment);
                              for (int track = 0; track < tracks; ++track) {
                                    Element* e = segment->element(track);
                                    if (!e)
                                          continue;
                                    ElementItem* sei = new ElementItem(segItem, e);
                                    if (e->type() == CHORD) {
                                          Chord* chord = (Chord*)e;
                                          if (chord->hook())
                                                new ElementItem(sei, chord->hook());
                                          if (chord->stem())
                                                new ElementItem(sei, chord->stem());
                                          if (chord->arpeggio())
                                                new ElementItem(sei, chord->arpeggio());
                                          if (chord->tremolo())
                                                new ElementItem(sei, chord->tremolo());
                                          if (chord->glissando())
                                                new ElementItem(sei, chord->glissando());

                                          foreach(Articulation* a, *chord->getArticulations())
                                                new ElementItem(sei, a);
                                          foreach(LedgerLine* h, *chord->ledgerLines())
                                                new ElementItem(sei, h);
                                          foreach(Note* note, chord->notes()) {
                                                ElementItem* ni = new ElementItem(sei, note);
                                                if (note->accidental()) {
                                                      new ElementItem(ni, note->accidental());
                                                      }
                                                foreach(Element* f, *note->el()) {
                                                      if (f->type() == SYMBOL || f->type() == IMAGE) {
                                                            BSymbol* bs = static_cast<BSymbol*>(f);
                                                            addSymbol(ni, bs);
                                                            }
                                                      else
                                                            new ElementItem(ni, f);
                                                      }
                                                if (note->tieFor()) {
                                                      Tie* tie = note->tieFor();
                                                      ElementItem* ti = new ElementItem(ni, tie);
                                                      foreach(Element* el1, *tie->slurSegments())
                                                            new ElementItem(ti, el1);
                                                      }
                                                }
                                          }
                                    }
                              for (int i = 0; i < staves; ++i) {
                                    foreach(Lyrics* l, *(segment->lyricsList(i))) {
                                          if (l)
                                                new ElementItem(segItem, l);
                                          }
                                    }
                              }
                        foreach(Tuplet* tuplet, *measure->tuplets()) {
					ElementItem* item = new ElementItem(mi, tuplet);
                              if (tuplet->number())
                                    new ElementItem(item, tuplet->number());
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   searchElement
//---------------------------------------------------------

bool Inspector::searchElement(QTreeWidgetItem* pi, Element* el)
      {
      for (int i = 0;; ++i) {
            QTreeWidgetItem* item = pi->child(i);
            if (item == 0)
                  break;
            ElementItem* ei = (ElementItem*)item;
            if (ei->element() == el) {
                  QTreeWidget* tw = pi->treeWidget();
                  tw->setItemExpanded(item, true);
                  tw->setCurrentItem(item);
                  tw->scrollToItem(item);
                  return true;
                  }
            if (searchElement(item, el)) {
                  pi->treeWidget()->setItemExpanded(item, true);
                  return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void Inspector::setElement(Element* el)
      {
      if (curElement) {
            backStack.push(curElement);
            back->setEnabled(true);
            forwardStack.clear();
            forward->setEnabled(false);
            }
      updateElement(el);
      }

//---------------------------------------------------------
//   itemExpanded
//---------------------------------------------------------

void Inspector::itemExpanded(QTreeWidgetItem*)
      {
      list->resizeColumnToContents(0);
      }

//---------------------------------------------------------
//   itemClicked
//---------------------------------------------------------

void Inspector::itemClicked(QTreeWidgetItem* i, int)
      {
      if (i == 0)
            return;
      if (i->type() == INVALID)
            return;
      Element* el = static_cast<ElementItem*>(i)->element();
      if (curElement) {
            backStack.push(curElement);
            back->setEnabled(true);
            forwardStack.clear();
            forward->setEnabled(false);
            }
      updateElement(el);
      }

//---------------------------------------------------------
//   updateElement
//---------------------------------------------------------

void Inspector::updateElement(Element* el)
      {
      if (el == 0 || !isVisible())
            return;
      for (int i = 0;; ++i) {
            QTreeWidgetItem* item = list->topLevelItem(i);
            if (item == 0) {
                  printf("Inspector::Element not found %s %p\n", el->name(), el);
                  break;
                  }
            ElementItem* ei = (ElementItem*)item;
            if (ei->element() == el) {
                  list->setItemExpanded(item, true);
                  list->setCurrentItem(item);
                  list->scrollToItem(item);
                  break;
                  }
            if (searchElement(item, el)) {
                  list->setItemExpanded(item, true);
                  break;
                  }
            }
      setWindowTitle(QString("MuseScore: List Edit: ") + el->name());
      ShowElementBase* ew = 0;
      switch (el->type()) {
            case PAGE:          ew = pagePanel;    break;
            case SYSTEM:        ew = systemPanel;  break;
            case MEASURE:       ew = measurePanel; break;
            case CHORD:         ew = chordPanel;   break;
            case NOTE:          ew = notePanel;    break;
            case REST:          ew = restPanel;    break;
            case CLEF:          ew = clefPanel;    break;
            case TIMESIG:       ew = timesigPanel; break;
            case KEYSIG:        ew = keysigPanel;  break;
            case SEGMENT:       ew = segmentView;  break;
            case HAIRPIN:       ew = hairpinView;  break;
            case BAR_LINE:      ew = barLineView;  break;
            case DYNAMIC:       ew = dynamicView;  break;
            case TUPLET:        ew = tupletView;   break;
            case SLUR:          ew = slurView;     break;
            case TIE:           ew = tieView;      break;
            case VOLTA:         ew = voltaView;    break;
            case VOLTA_SEGMENT: ew = voltaSegmentView; break;
            case LYRICS:        ew = lyricsView;   break;
            case BEAM:          ew = beamView;     break;
            case TREMOLO:       ew = tremoloView;  break;
            case MARKER:
            case JUMP:
            case TEXT:
                  ew = textView;
                  break;
            default:
                  ew = elementView;
                  break;
            }
      curElement = el;
      ew->setElement(el);
      stack->setCurrentWidget(ew);
      }

//---------------------------------------------------------
//   ElementListWidgetItem
//---------------------------------------------------------

class ElementListWidgetItem : public QListWidgetItem {
      Element* e;

   public:
      ElementListWidgetItem(Element* el) : QListWidgetItem () {
            e = el;
            setText(e->name());
            }
      Element* element() const { return e; }
      };

//---------------------------------------------------------
//   ShowPageWidget
//---------------------------------------------------------

ShowPageWidget::ShowPageWidget()
   : ShowElementBase()
      {
      QWidget* page = new QWidget;
      pb.setupUi(page);
      layout->addWidget(page);
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ShowPageWidget::setElement(Element* e)
      {
      Page* p = (Page*)e;
      ShowElementBase::setElement(e);
      pb.pageNo->setValue(p->no());
      }

//---------------------------------------------------------
//   itemClicked
//---------------------------------------------------------

void ShowPageWidget::itemClicked(QListWidgetItem* i)
      {
      ElementListWidgetItem* item = (ElementListWidgetItem*)i;
      emit elementChanged(item->element());
      }

//---------------------------------------------------------
//   ShowSystemWidget
//---------------------------------------------------------

ShowSystemWidget::ShowSystemWidget()
   : ShowElementBase()
      {
      layout->addStretch(100);
      }

//---------------------------------------------------------
//   MeasureView
//---------------------------------------------------------

MeasureView::MeasureView()
   : ShowElementBase()
      {
      QWidget* seg = new QWidget;
      mb.setupUi(seg);
      layout->addWidget(seg);
      layout->addStretch(10);
      connect(mb.sel, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(elementClicked(QTreeWidgetItem*)));
      connect(mb.nextButton, SIGNAL(clicked()), SLOT(nextClicked()));
      connect(mb.prevButton, SIGNAL(clicked()), SLOT(prevClicked()));
      seg->show();
      }

//---------------------------------------------------------
//   nextClicked
//---------------------------------------------------------

void MeasureView::nextClicked()
      {
      emit elementChanged(((MeasureBase*)element())->next());
      }

//---------------------------------------------------------
//   prevClicked
//---------------------------------------------------------

void MeasureView::prevClicked()
      {
      emit elementChanged(((MeasureBase*)element())->prev());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void MeasureView::setElement(Element* e)
      {
      Measure* m = (Measure*)e;
      ShowElementBase::setElement(e);

      mb.segments->setValue(m->size());
      mb.staves->setValue(m->staffList()->size());
//      mb.beams->setValue(m->beams()->size());
      mb.tuplets->setValue(m->tuplets()->size());
      mb.measureNo->setValue(m->no());
      mb.noOffset->setValue(m->noOffset());
      mb.stretch->setValue(m->userStretch());
      mb.lineBreak->setChecked(m->lineBreak());
      mb.pageBreak->setChecked(m->pageBreak());
      mb.irregular->setChecked(m->irregular());
      mb.endRepeat->setValue(m->repeatCount());
      mb.repeatFlags->setText(QString("0x%1").arg(m->repeatFlags(), 6, 16, QChar('0')));
      mb.breakMultiMeasureRest->setChecked(m->getBreakMultiMeasureRest());
      mb.breakMMRest->setChecked(m->breakMMRest());
      mb.endBarLineType->setValue(m->endBarLineType());
      mb.endBarLineGenerated->setChecked(m->endBarLineGenerated());
      mb.endBarLineVisible->setChecked(m->endBarLineVisible());
      mb.multiMeasure->setValue(m->multiMeasure());

      mb.sel->clear();
      foreach(const Element* e, *m->el()) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, e->name());
            item->setText(1, QString("%1").arg(e->subtype()));
            void* p = (void*) e;
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(p));
            mb.sel->addTopLevelItem(item);
            }
      mb.prevButton->setEnabled(m->prev());
      mb.nextButton->setEnabled(m->next());
      }

//---------------------------------------------------------
//   elementClicked
//---------------------------------------------------------

void MeasureView::elementClicked(QTreeWidgetItem* item)
      {
      Element* e = (Element*)item->data(0, Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   SegmentView
//---------------------------------------------------------

SegmentView::SegmentView()
   : ShowElementBase()
      {
      QWidget* seg = new QWidget;
      sb.setupUi(seg);
      layout->addWidget(seg);
      layout->addStretch(10);
      sb.segmentType->clear();
      sb.segmentType->addItem("SegClef",               0x1);
      sb.segmentType->addItem("SegKeySig",             0x2);
      sb.segmentType->addItem("SegTimeSig",            0x4);
      sb.segmentType->addItem("SegStartRepeatBarLine", 0x8);
      sb.segmentType->addItem("SegBarLine",            0x10);
      sb.segmentType->addItem("SegGrace",              0x20);
      sb.segmentType->addItem("SegChordRest",          0x40);
      sb.segmentType->addItem("SegBreath",             0x80);
      sb.segmentType->addItem("SegEndBarLine",         0x100);
      sb.segmentType->addItem("SegTimeSigAnnounce",    0x200);
      sb.segmentType->addItem("SegKeySigAnnounce",     0x400);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void SegmentView::setElement(Element* e)
      {
      ShowElementBase::setElement(e);

      Segment* s = (Segment*)e;
      ShowElementBase::setElement(e);
      int st = s->subtype();
      int idx;
      for (idx = 0; idx < 11; ++idx) {
            if ((1 << idx) == st)
                  break;
            }
      sb.segmentType->setCurrentIndex(idx);
      sb.lyrics->clear();

      Score* cs = e->score();
      for (int i = 0; i < cs->nstaves(); ++i) {
            const LyricsList* ll = s->lyricsList(i);
            if (ll) {
                  foreach(Lyrics* l, *ll) {
                        QString s;
                        s.setNum(long(l), 16);
                        QListWidgetItem* item = new QListWidgetItem(s, 0, long(l));
                        sb.lyrics->addItem(item);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   ShowChordWidget
//---------------------------------------------------------

ShowChordWidget::ShowChordWidget()
   : ShowElementBase()
      {
      // chord rest
      QWidget* chr = new QWidget;
      crb.setupUi(chr);
      layout->addWidget(chr);
      connect(crb.beamButton, SIGNAL(clicked()), SLOT(beamClicked()));
      connect(crb.tupletButton, SIGNAL(clicked()), SLOT(tupletClicked()));
      connect(crb.upFlag,   SIGNAL(toggled(bool)), SLOT(upChanged(bool)));
      connect(crb.beamMode, SIGNAL(activated(int)), SLOT(beamModeChanged(int)));
      connect(crb.attributes, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(crb.slurFor, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(crb.slurBack, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));

      // chord
      QWidget* ch = new QWidget;
      cb.setupUi(ch);
      layout->addWidget(ch);
      layout->addStretch(100);
      connect(cb.hookButton, SIGNAL(clicked()), SLOT(hookClicked()));
      connect(cb.stemButton, SIGNAL(clicked()), SLOT(stemClicked()));
      connect(cb.stemSlashButton, SIGNAL(clicked()), SLOT(stemSlashClicked()));
      connect(cb.arpeggioButton, SIGNAL(clicked()), SLOT(arpeggioClicked()));
      connect(cb.tremoloButton, SIGNAL(clicked()), SLOT(tremoloClicked()));
      connect(cb.glissandoButton, SIGNAL(clicked()), SLOT(glissandoClicked()));

      connect(cb.stemDirection, SIGNAL(activated(int)), SLOT(directionChanged(int)));
      connect(cb.helplineList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(cb.notes, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));

      crb.beamMode->addItem(tr("auto"));
      crb.beamMode->addItem(tr("beam begin"));
      crb.beamMode->addItem(tr("beam mid"));
      crb.beamMode->addItem(tr("beam end"));
      crb.beamMode->addItem(tr("no beam"));
      crb.beamMode->addItem(tr("begin 1/32"));

      cb.stemDirection->addItem(tr("Auto"), 0);
      cb.stemDirection->addItem(tr("Up"), 1);
      cb.stemDirection->addItem(tr("Down"), 2);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ShowChordWidget::setElement(Element* e)
      {
      Chord* chord = (Chord*)e;
      ShowElementBase::setElement(e);

      crb.beamButton->setEnabled(chord->beam());
      crb.tupletButton->setEnabled(chord->tuplet());
      crb.upFlag->setChecked(chord->up());
      crb.beamMode->setCurrentIndex(int(chord->beamMode()));
      crb.dots->setValue(chord->dots());
      crb.ticks->setValue(chord->ticks());
      crb.duration->setValue(int(chord->duration().type()));
      crb.move->setValue(chord->staffMove());

      cb.hookButton->setEnabled(chord->hook());
      cb.stemButton->setEnabled(chord->stem());
      cb.graceNote->setChecked(chord->noteType() != NOTE_NORMAL);
      cb.stemDirection->setCurrentIndex(int(chord->stemDirection()));

      cb.stemSlashButton->setEnabled(chord->stemSlash());
      cb.arpeggioButton->setEnabled(chord->arpeggio());
      cb.tremoloButton->setEnabled(chord->tremolo());
      cb.glissandoButton->setEnabled(chord->glissando());

      crb.attributes->clear();
      foreach(Articulation* a, *chord->getArticulations()) {
            QString s;
            s.setNum(long(a), 16);
            QListWidgetItem* item = new QListWidgetItem(s, 0, long(a));
            crb.attributes->addItem(item);
            }
      crb.slurFor->clear();
      foreach(Slur* slur, chord->slurFor()) {
            QString s;
            s.setNum(long(slur), 16);
            QListWidgetItem* item = new QListWidgetItem(s, 0, long(slur));
            crb.slurFor->addItem(item);
            }
      crb.slurBack->clear();
      foreach(Slur* slur, chord->slurBack()) {
            QString s;
            s.setNum(long(slur), 16);
            QListWidgetItem* item = new QListWidgetItem(s, 0, long(slur));
            crb.slurBack->addItem(item);
            }

      cb.helplineList->clear();
      foreach(LedgerLine* h, *chord->ledgerLines()) {
            QString s;
            s.setNum(long(h), 16);
            QListWidgetItem* item = new QListWidgetItem(s, 0, long(h));
            cb.helplineList->addItem(item);
            }
      cb.notes->clear();
      foreach(Note* n, chord->notes()) {
            QString s;
            s.setNum(long(n), 16);
            QListWidgetItem* item = new QListWidgetItem(s, 0, long(n));
            cb.notes->addItem(item);
            }
      }

//---------------------------------------------------------
//   hookClicked
//---------------------------------------------------------

void ShowChordWidget::hookClicked()
      {
      emit elementChanged(((Chord*)element())->hook());
      }

//---------------------------------------------------------
//   stemClicked
//---------------------------------------------------------

void ShowChordWidget::stemClicked()
      {
      emit elementChanged(((Chord*)element())->stem());
      }

//---------------------------------------------------------
//   beamClicked
//---------------------------------------------------------

void ShowChordWidget::beamClicked()
      {
      emit elementChanged(((Chord*)element())->beam());
      }

//---------------------------------------------------------
//   tupletClicked
//---------------------------------------------------------

void ShowChordWidget::tupletClicked()
      {
      emit elementChanged(((Chord*)element())->tuplet());
      }

void ShowChordWidget::stemSlashClicked()
      {
      emit elementChanged(((Chord*)element())->stemSlash());
      }

void ShowChordWidget::arpeggioClicked()
      {
      emit elementChanged(((Chord*)element())->arpeggio());
      }

void ShowChordWidget::tremoloClicked()
      {
      emit elementChanged(((Chord*)element())->tremolo());
      }

void ShowChordWidget::glissandoClicked()
      {
      emit elementChanged(((Chord*)element())->glissando());
      }

//---------------------------------------------------------
//   upChanged
//---------------------------------------------------------

void ShowChordWidget::upChanged(bool val)
      {
      ((Chord*)element())->setUp(val);
      }

//---------------------------------------------------------
//   beamModeChanged
//---------------------------------------------------------

void ShowChordWidget::beamModeChanged(int n)
      {
      ((Chord*)element())->setBeamMode(BeamMode(n));
      element()->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   directionChanged
//---------------------------------------------------------

void ShowChordWidget::directionChanged(int val)
      {
      ((Chord*)element())->setStemDirection(Direction(val));
      }

//---------------------------------------------------------
//   ShowNoteWidget
//---------------------------------------------------------

ShowNoteWidget::ShowNoteWidget()
   : ShowElementBase()
      {
      QWidget* note = new QWidget;
      nb.setupUi(note);
      layout->addWidget(note);
      layout->addStretch(10);

      connect(nb.tieFor, SIGNAL(clicked()), SLOT(tieForClicked()));
      connect(nb.tieBack, SIGNAL(clicked()), SLOT(tieBackClicked()));
      connect(nb.accidental, SIGNAL(clicked()), SLOT(accidentalClicked()));
      connect(nb.fingering, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(nb.tpc, SIGNAL(valueChanged(int)), SLOT(tpcChanged(int)));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ShowNoteWidget::setElement(Element* e)
      {
      Note* note = (Note*)e;
      ShowElementBase::setElement(e);

      nb.pitch->setValue(note->pitch());
      nb.ppitch->setValue(note->ppitch());
      nb.velo->setValue(note->velocity());
      nb.tuning->setValue(note->tuning());
      nb.line->setValue(note->line());
      nb.mirror->setChecked(note->mirror());
      nb.tpc->setValue(note->tpc());
      nb.headGroup->setValue(note->headGroup());
      nb.hidden->setChecked(note->hidden());
      nb.subchannel->setValue(note->subchannel());

      nb.tieFor->setEnabled(note->tieFor());
      nb.tieBack->setEnabled(note->tieBack());
      nb.accidental->setEnabled(note->accidental());
      nb.userAccidental->setValue(note->userAccidental());

      nb.onTimeType->setCurrentIndex(note->onTimeType());
      nb.onTimeOffset->setValue(note->onTimeOffset());
      nb.offTimeOffset->setValue(note->offTimeOffset());
      nb.offTimeType->setCurrentIndex(note->offTimeType());
      nb.onTimeUserOffset->setValue(note->onTimeUserOffset());
      nb.offTimeUserOffset->setValue(note->offTimeUserOffset());

      foreach(Element* text, *note->el()) {
            QString s;
            s.setNum(long(text), 16);
            QListWidgetItem* item = new QListWidgetItem(s, 0, long(text));
            nb.fingering->addItem(item);
            }
      }

//---------------------------------------------------------
//   tpcChanged
//---------------------------------------------------------

void ShowNoteWidget::tpcChanged(int val)
      {
      ((Note*)element())->setTpc(val);
      emit scoreChanged();
      }

//---------------------------------------------------------
//   tieForClicked
//---------------------------------------------------------

void ShowNoteWidget::tieForClicked()
      {
      emit elementChanged(((Note*)element())->tieFor());
      }

//---------------------------------------------------------
//   tieBackClicked
//---------------------------------------------------------

void ShowNoteWidget::tieBackClicked()
      {
      emit elementChanged(((Note*)element())->tieBack());
      }

//---------------------------------------------------------
//   accidentalClicked
//---------------------------------------------------------

void ShowNoteWidget::accidentalClicked()
      {
      emit elementChanged(((Note*)element())->accidental());
      }

//---------------------------------------------------------
//   ShowRestWidget
//---------------------------------------------------------

ShowRestWidget::ShowRestWidget()
   : ShowElementBase()
      {
      // chort rest
      QWidget* chr = new QWidget;
      crb.setupUi(chr);
      layout->addWidget(chr);
      crb.beamMode->addItem(tr("auto"));
      crb.beamMode->addItem(tr("beam begin"));
      crb.beamMode->addItem(tr("beam mid"));
      crb.beamMode->addItem(tr("beam end"));
      crb.beamMode->addItem(tr("no beam"));
      crb.beamMode->addItem(tr("begin 1/32"));

      connect(crb.beamButton, SIGNAL(clicked()), SLOT(beamClicked()));
      connect(crb.tupletButton, SIGNAL(clicked()), SLOT(tupletClicked()));
      connect(crb.attributes, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(crb.slurFor, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(crb.slurBack, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));

      QFrame* line = new QFrame(this);
      line->setFrameStyle(QFrame::HLine | QFrame::Raised);
      line->setLineWidth(1);
      layout->addWidget(line);

      QHBoxLayout* hb = new QHBoxLayout;
      segment = new QSpinBox(this);
      hb->addWidget(new QLabel(tr("Segment:"), this));
      hb->addWidget(segment);
      hb->addStretch(100);

      layout->addLayout(hb);
      layout->addStretch(100);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ShowRestWidget::setElement(Element* e)
      {
      Rest* rest = (Rest*)e;
      ShowElementBase::setElement(e);

      crb.beamButton->setEnabled(rest->beam());
      crb.tupletButton->setEnabled(rest->tuplet());
      crb.upFlag->setChecked(rest->up());
      crb.beamMode->setCurrentIndex(int(rest->beamMode()));
      crb.attributes->clear();
      crb.dots->setValue(rest->dots());
      crb.ticks->setValue(rest->ticks());
      crb.duration->setValue(int(rest->duration().type()));
      crb.move->setValue(rest->staffMove());

      crb.slurFor->clear();
      foreach(Slur* slur, rest->slurFor()) {
            QString s;
            s.setNum(long(slur), 16);
            QListWidgetItem* item = new QListWidgetItem(s, 0, long(slur));
            crb.slurFor->addItem(item);
            }
      crb.slurBack->clear();
      foreach(Slur* slur, rest->slurBack()) {
            QString s;
            s.setNum(long(slur), 16);
            QListWidgetItem* item = new QListWidgetItem(s, 0, long(slur));
            crb.slurBack->addItem(item);
            }

      foreach(Articulation* a, *rest->getArticulations()) {
            QString s;
            s.setNum(long(a), 16);
            QListWidgetItem* item = new QListWidgetItem(s, 0, long(a));
            crb.attributes->addItem(item);
            }

      Measure* m = rest->measure();
      int seg = 0;
      int tracks = 0; // TODO cs->nstaves() * VOICES;
      for (Segment* s = m->first(); s; s = s->next(), ++seg) {
            int track;
            for (track = 0; track < tracks; ++track) {
                  Element* e = s->element(track);
                  if (e == rest)
                        break;
                  }
            if (track < tracks)
                  break;
            }
      segment->setValue(seg);
      }

//---------------------------------------------------------
//   beamClicked
//---------------------------------------------------------

void ShowRestWidget::beamClicked()
      {
      emit elementChanged(static_cast<Rest*>(element())->beam());
      }

//---------------------------------------------------------
//   tupletClicked
//---------------------------------------------------------

void ShowRestWidget::tupletClicked()
      {
      emit elementChanged(static_cast<Rest*>(element())->tuplet());
      }

//---------------------------------------------------------
//   ShowClefWidget
//---------------------------------------------------------

ShowClefWidget::ShowClefWidget()
   : ShowElementBase()
      {
      QFrame* line = new QFrame(this);
      line->setFrameStyle(QFrame::HLine | QFrame::Raised);
      line->setLineWidth(1);
      layout->addWidget(line);

      QHBoxLayout* hb = new QHBoxLayout;
      idx = new QSpinBox(this);
      idx->setRange(0, 1000000);
      hb->addWidget(new QLabel(tr("Clef Type:"), this));
      hb->addWidget(idx);
      hb->addStretch(100);

      layout->addLayout(hb);
      layout->addStretch(100);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ShowClefWidget::setElement(Element* e)
      {
      Clef* clef = (Clef*)e;
      ShowElementBase::setElement(e);
      idx->setValue(clef->subtype());
      }

//---------------------------------------------------------
//   ShowTimesigWidget
//---------------------------------------------------------

ShowTimesigWidget::ShowTimesigWidget()
   : ShowElementBase()
      {
      layout->addStretch(100);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ShowTimesigWidget::setElement(Element* e)
      {
//      TimeSig* tsig = (TimeSig*)e;
      ShowElementBase::setElement(e);
      }

//---------------------------------------------------------
//   ShowKeysigWidget
//---------------------------------------------------------

ShowKeysigWidget::ShowKeysigWidget()
   : ShowElementBase()
      {
      layout->addStretch(100);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ShowKeysigWidget::setElement(Element* e)
      {
//      KeySig* tsig = (KeySig*)e;
      ShowElementBase::setElement(e);
      }

//---------------------------------------------------------
//   ElementView
//---------------------------------------------------------

ElementView::ElementView()
   : ShowElementBase()
      {
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   TextView
//---------------------------------------------------------

TextView::TextView()
   : ShowElementBase()
      {
      QWidget* page = new QWidget;
      tb.setupUi(page);
      layout->addWidget(page);
      layout->addStretch(10);
      connect(tb.text, SIGNAL(textChanged()), SLOT(textChanged()));
      }

//---------------------------------------------------------
//   textChanged
//---------------------------------------------------------

void TextView::textChanged()
      {
      emit scoreChanged();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void TextView::setElement(Element* e)
      {
      Text* te = (Text*)e;
      tb.textStyle->clear();
      foreach(const TextStyle* s, e->score()->textStyles()) {
            tb.textStyle->addItem(s->name);
            }

      ShowElementBase::setElement(e);
      tb.text->setDocument(te->doc()->clone());
      tb.xoffset->setValue(te->xoff());
      tb.yoffset->setValue(te->yoff());
      tb.rxoffset->setValue(te->reloff().x());
      tb.ryoffset->setValue(te->reloff().y());
      tb.offsetType->setCurrentIndex(int(te->offsetType()));
      tb.textStyle->setCurrentIndex(te->textStyle());
      }

//---------------------------------------------------------
//   HairpinView
//---------------------------------------------------------

HairpinView::HairpinView()
   : ShowElementBase()
      {
      QWidget* hairpin = new QWidget;
      hp.setupUi(hairpin);
      layout->addWidget(hairpin);
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void HairpinView::setElement(Element* e)
      {
      Hairpin* hairpin = (Hairpin*)e;
      ShowElementBase::setElement(e);
      hp.tick1->setValue(hairpin->tick());
      hp.tick2->setValue(hairpin->tick2());
      }

//---------------------------------------------------------
//   BarLineView
//---------------------------------------------------------

BarLineView::BarLineView()
   : ShowElementBase()
      {
      QWidget* barline = new QWidget;
      bl.setupUi(barline);
      layout->addWidget(barline);
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void BarLineView::setElement(Element* e)
      {
      BarLine* barline = (BarLine*)e;
      ShowElementBase::setElement(e);
      bl.subType->setValue(barline->subtype());
      }

//---------------------------------------------------------
//   DynamicView
//---------------------------------------------------------

DynamicView::DynamicView()
   : ShowElementBase()
      {
      QWidget* tw = new QWidget;
      tb.setupUi(tw);
      layout->addWidget(tw);

      QWidget* dynamic = new QWidget;
      bl.setupUi(dynamic);
      layout->addWidget(dynamic);
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void DynamicView::setElement(Element* e)
      {
      Dynamic* dynamic = (Dynamic*)e;

//      tb.style->clear();
//      foreach (TextStyle* s, dynamic->score()->textStyles())
//            tb.style->addItem(s->name);
      tb.text->setText(dynamic->getText());
//      tb.xoffset->setValue(dynamic->styleOffset().x());
//      tb.yoffset->setValue(dynamic->styleOffset().y());

      ShowElementBase::setElement(e);
      bl.subType->setValue(dynamic->subtype());
      }

//---------------------------------------------------------
//   TupletView
//---------------------------------------------------------

TupletView::TupletView()
   : ShowElementBase()
      {
      QWidget* tw = new QWidget;
      tb.setupUi(tw);
      layout->addWidget(tw);
      layout->addStretch(10);

      connect(tb.number, SIGNAL(clicked()), SLOT(numberClicked()));
      connect(tb.elements, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(elementClicked(QTreeWidgetItem*)));
      }

//---------------------------------------------------------
//   numberClicked
//---------------------------------------------------------

void TupletView::numberClicked()
      {
      emit elementChanged(((Tuplet*)element())->number());
      }

//---------------------------------------------------------
//   elementClicked
//---------------------------------------------------------

void TupletView::elementClicked(QTreeWidgetItem* item)
      {
      Element* e = (Element*)item->data(0, Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void TupletView::setElement(Element* e)
      {
      ShowElementBase::setElement(e);
      Tuplet* tuplet = (Tuplet*)e;
      tb.baseLen->setText(tuplet->baseLen().name());
      tb.ratioZ->setValue(tuplet->ratio().numerator());
      tb.ratioN->setValue(tuplet->ratio().denominator());
      tb.number->setEnabled(tuplet->number());
      tb.elements->clear();
      foreach(DurationElement* e, tuplet->elements()) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, e->name());
            item->setText(1, QString("%1").arg(e->tick()));
            item->setText(2, QString("%1").arg(e->tickLen()));
            void* p = (void*) e;
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(p));
            tb.elements->addTopLevelItem(item);
            }
      }

//---------------------------------------------------------
//   DoubleLabel
//---------------------------------------------------------

DoubleLabel::DoubleLabel(QWidget* parent)
   : QLabel(parent)
      {
//      setFrameStyle(QFrame::LineEditPanel | QFrame::Sunken);
  //    setPaletteBackgroundColor(palette().active().brightText());
      }

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void DoubleLabel::setValue(double val)
      {
      QString s;
      setText(s.setNum(val, 'g', 3));
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize DoubleLabel::sizeHint() const
      {
      QFontMetrics fm = fontMetrics();
      int h           = fm.height() + 4;
      int n           = 3 + 3;
      int w = fm.width(QString("-0.")) + fm.width('0') * n + 6;
      return QSize(w, h);
      }

//---------------------------------------------------------
//   ShowElementBase
//---------------------------------------------------------

ShowElementBase::ShowElementBase()
   : QWidget()
      {
      QWidget* elemView = new QWidget;
      eb.setupUi(elemView);
      layout = new QVBoxLayout;
      setLayout(layout);
      layout->addWidget(elemView);
      connect(eb.parentButton,   SIGNAL(clicked()), SLOT(parentClicked()));
      connect(eb.offsetx,        SIGNAL(valueChanged(double)), SLOT(offsetxChanged(double)));
      connect(eb.offsety,        SIGNAL(valueChanged(double)), SLOT(offsetyChanged(double)));
      connect(eb.selected,       SIGNAL(clicked(bool)), SLOT(selectedClicked(bool)));
      connect(eb.visible,        SIGNAL(clicked(bool)), SLOT(visibleClicked(bool)));
      }

//---------------------------------------------------------
//   gotoElement
//---------------------------------------------------------

void ShowElementBase::gotoElement(QListWidgetItem* ai)
      {
      Element* e = (Element*)(ai->type());
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   ShowElementBase
//---------------------------------------------------------

void ShowElementBase::setElement(Element* e)
      {
      el = e;

      eb.address->setText(QString("%1").arg((unsigned long)e, 0, 16));

      eb.subtype->setValue(e->subtype());
      eb.selected->setChecked(e->selected());
      eb.selectable->setChecked(e->selectable());
      eb.droptarget->setChecked(e->dropTarget());
      eb.generated->setChecked(e->generated());
      eb.visible->setChecked(e->visible());
      eb.track->setValue(e->track());
      eb.time->setValue(e->tick());
      eb.posx->setValue(e->ipos().x());
      eb.posy->setValue(e->ipos().y());
      eb.cposx->setValue(e->canvasPos().x());
      eb.cposy->setValue(e->canvasPos().y());
      eb.offsetx->setValue(e->userOff().x());
      eb.offsety->setValue(e->userOff().y());
      eb.bboxx->setValue(e->bbox().x());
      eb.bboxy->setValue(e->bbox().y());
      eb.bboxw->setValue(e->bbox().width());
      eb.bboxh->setValue(e->bbox().height());
      eb.color->setColor(e->color());
      eb.parentButton->setEnabled(e->parent());
      eb.mag->setValue(e->mag());
      eb.systemFlag->setChecked(e->systemFlag());
      }

//---------------------------------------------------------
//   selectedClicked
//---------------------------------------------------------

void ShowElementBase::selectedClicked(bool val)
      {
      QRectF r(el->abbox());
      if (val)
            el->score()->select(el, SELECT_ADD, 0);
      else
            el->score()->deselect(el);
      el->score()->addRefresh(r | el->abbox());
      }

//---------------------------------------------------------
//   visibleClicked
//---------------------------------------------------------

void ShowElementBase::visibleClicked(bool val)
      {
      QRectF r(el->abbox());
      el->setVisible(val);
      el->score()->addRefresh(r | el->abbox());
      }

//---------------------------------------------------------
//   parentClicked
//---------------------------------------------------------

void ShowElementBase::parentClicked()
      {
      emit elementChanged(el->parent());
      }

//---------------------------------------------------------
//   offsetxChanged
//---------------------------------------------------------

void ShowElementBase::offsetxChanged(double val)
      {
      QRectF r(el->abbox());
      el->setUserXoffset(val);
//      Element* e = el;
//TODO      while ((e = e->parent()))
      el->score()->addRefresh(r | el->abbox());
      }

//---------------------------------------------------------
//   offsetyChanged
//---------------------------------------------------------

void ShowElementBase::offsetyChanged(double val)
      {
      QRectF r(el->abbox());
      el->setUserYoffset(val);
      el->score()->addRefresh(r | el->abbox());
      }

//---------------------------------------------------------
//   SlurView
//---------------------------------------------------------

SlurView::SlurView()
   : ShowElementBase()
      {
      QWidget* slurTie = new QWidget;
      st.setupUi(slurTie);
      QWidget* slur = new QWidget;
      sb.setupUi(slur);
      layout->addWidget(slurTie);
      layout->addWidget(slur);
      layout->addStretch(10);
      connect(st.segments, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(segmentClicked(QTreeWidgetItem*)));
      connect(st.startElement,   SIGNAL(clicked()), SLOT(startClicked()));
      connect(st.endElement,   SIGNAL(clicked()), SLOT(endClicked()));
      }

//---------------------------------------------------------
//   startClicked
//---------------------------------------------------------

void SlurView::startClicked()
      {
      emit elementChanged(static_cast<SlurTie*>(element())->startElement());
      }

//---------------------------------------------------------
//   endClicked
//---------------------------------------------------------

void SlurView::endClicked()
      {
      emit elementChanged(static_cast<SlurTie*>(element())->endElement());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void SlurView::setElement(Element* e)
      {
      Slur* slur = (Slur*)e;
      ShowElementBase::setElement(e);

      st.segments->clear();
      QList<SlurSegment*>* el = slur->slurSegments();
      foreach(const Element* e, *el) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, QString("%1").arg((unsigned long)e, 8, 16));
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>((void*)e));
            st.segments->addTopLevelItem(item);
            }
      st.upFlag->setChecked(slur->isUp());
      st.direction->setCurrentIndex(slur->slurDirection());
      st.startElement->setEnabled(slur->startElement());
      st.endElement->setEnabled(slur->endElement());

      sb.tick2->setValue(slur->tick2());
      sb.staff2->setValue(slur->track2() / VOICES);
      sb.voice2->setValue(slur->track2() % VOICES);
      }

//---------------------------------------------------------
//   segmentClicked
//---------------------------------------------------------

void SlurView::segmentClicked(QTreeWidgetItem* item)
      {
      Element* e = (Element*)item->data(0, Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   segmentClicked
//---------------------------------------------------------

void TieView::segmentClicked(QTreeWidgetItem* item)
      {
      Element* e = (Element*)item->data(0, Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   TieView
//---------------------------------------------------------

TieView::TieView()
   : ShowElementBase()
      {
      QWidget* tie = new QWidget;
      st.setupUi(tie);
      layout->addWidget(tie);
      layout->addStretch(10);
      connect(st.segments, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(segmentClicked(QTreeWidgetItem*)));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void TieView::setElement(Element* e)
      {
      Tie* tie = (Tie*)e;
      ShowElementBase::setElement(e);

      st.segments->clear();
      QList<SlurSegment*>* el = tie->slurSegments();
      foreach(const Element* e, *el) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, QString("%1").arg((unsigned long)e, 8, 16));
item->setText(1, "klops");
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>((void*)e));
            st.segments->addTopLevelItem(item);
            }
      st.upFlag->setChecked(tie->isUp());
      st.direction->setCurrentIndex(tie->slurDirection());
      }

//---------------------------------------------------------
//   segmentClicked
//---------------------------------------------------------

void VoltaView::segmentClicked(QTreeWidgetItem* item)
      {
      Element* e = (Element*)item->data(0, Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   beginTextClicked
//---------------------------------------------------------

void VoltaView::beginTextClicked()
      {
      emit elementChanged(static_cast<Volta*>(element())->beginText());
      }

//---------------------------------------------------------
//   continueTextClicked
//---------------------------------------------------------

void VoltaView::continueTextClicked()
      {
      emit elementChanged(static_cast<Volta*>(element())->continueText());
      }

//---------------------------------------------------------
//   VoltaView
//---------------------------------------------------------

VoltaView::VoltaView()
   : ShowElementBase()
      {
      // SLineBase
      QWidget* w = new QWidget;
      lb.setupUi(w);
      layout->addWidget(w);

      // TextLineBase
      w = new QWidget;
      tlb.setupUi(w);
      layout->addWidget(w);

      layout->addStretch(10);
      connect(lb.segments, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(segmentClicked(QTreeWidgetItem*)));
      connect(tlb.beginText, SIGNAL(clicked()), SLOT(beginTextClicked()));
      connect(tlb.continueText, SIGNAL(clicked()), SLOT(continueTextClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void VoltaView::setElement(Element* e)
      {
      Volta* volta = (Volta*)e;
      ShowElementBase::setElement(e);

      tlb.lineWidth->setValue(volta->lineWidth().val());
      lb.tick2->setValue(volta->tick2());
      lb.anchor->setCurrentIndex(int(volta->anchor()));
      lb.diagonal->setChecked(volta->diagonal());

      lb.segments->clear();
      const QList<LineSegment*>& el = volta->lineSegments();
      foreach(const Element* e, el) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, QString("%1").arg((unsigned long)e, 8, 16));
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>((void*)e));
            lb.segments->addTopLevelItem(item);
            }
      tlb.beginText->setEnabled(volta->beginText());
      tlb.continueText->setEnabled(volta->continueText());
      }

//---------------------------------------------------------
//   VoltaSegmentView
//---------------------------------------------------------

VoltaSegmentView::VoltaSegmentView()
   : ShowElementBase()
      {
      QWidget* w = new QWidget;
      lb.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void VoltaSegmentView::setElement(Element* e)
      {
      VoltaSegment* vs = (VoltaSegment*)e;
      ShowElementBase::setElement(e);

      lb.segmentType->setCurrentIndex(vs->subtype());
      lb.pos2x->setValue(vs->pos2().x());
      lb.pos2y->setValue(vs->pos2().y());
      lb.offset2x->setValue(vs->userOff2().x());
      lb.offset2y->setValue(vs->userOff2().y());
      }

//---------------------------------------------------------
//   LyricsView
//---------------------------------------------------------

LyricsView::LyricsView()
   : ShowElementBase()
      {
      QWidget* w = new QWidget;
      lb.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void LyricsView::setElement(Element* e)
      {
      Lyrics* l = (Lyrics*)e;
      ShowElementBase::setElement(e);

      lb.row->setValue(l->no());
      lb.endTick->setValue(l->endTick());
      lb.syllabic->setCurrentIndex(l->syllabic());
      }

//---------------------------------------------------------
//   backClicked
//---------------------------------------------------------

void Inspector::backClicked()
      {
      if (backStack.isEmpty())
            return;
      forwardStack.push(curElement);
      forward->setEnabled(true);
      updateElement(backStack.pop());
      back->setEnabled(!backStack.isEmpty());
      }

//---------------------------------------------------------
//   forwardClicked
//---------------------------------------------------------

void Inspector::forwardClicked()
      {
      if (forwardStack.isEmpty())
            return;
      backStack.push(curElement);
      back->setEnabled(true);
      updateElement(forwardStack.pop());
      forward->setEnabled(!forwardStack.isEmpty());
      }

//---------------------------------------------------------
//   reloadClicked
//---------------------------------------------------------

void Inspector::reloadClicked()
      {
      Element* e = curElement;
	updateList(cs);
	if (e)
	      updateElement(e);
      }

//---------------------------------------------------------
//   BeamView
//---------------------------------------------------------

BeamView::BeamView()
   : ShowElementBase()
      {
      QWidget* w = new QWidget;
      bb.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
      connect(bb.elements, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(elementClicked(QTreeWidgetItem*)));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void BeamView::setElement(Element* e)
      {
      Beam* b = (Beam*)e;
      ShowElementBase::setElement(e);

      bb.up->setValue(b->up());
      bb.elements->clear();
      foreach (ChordRest* cr, b->elements()) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, QString("%1").arg((unsigned long)e, 8, 16));
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>((void*)cr));
            item->setText(1, cr->name());
            bb.elements->addTopLevelItem(item);
            }
      }

//---------------------------------------------------------
//   elementClicked
//---------------------------------------------------------

void BeamView::elementClicked(QTreeWidgetItem* item)
      {
      Element* e = (Element*)item->data(0, Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   TremoloView
//---------------------------------------------------------

TremoloView::TremoloView()
   : ShowElementBase()
      {
      QWidget* w = new QWidget;
      tb.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
      connect(tb.firstChord, SIGNAL(clicked()), SLOT(chord1Clicked()));
      connect(tb.secondChord, SIGNAL(clicked()), SLOT(chord2Clicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void TremoloView::setElement(Element* e)
      {
      Tremolo* b = static_cast<Tremolo*>(e);
      ShowElementBase::setElement(e);

      tb.firstChord->setEnabled(b->chord1());
      tb.secondChord->setEnabled(b->chord2());
      }

//---------------------------------------------------------
//   chord1Clicked
//---------------------------------------------------------

void TremoloView::chord1Clicked()
      {
      emit elementChanged(static_cast<Tremolo*>(element())->chord1());
      }

//---------------------------------------------------------
//   chord2Clicked
//---------------------------------------------------------

void TremoloView::chord2Clicked()
      {
      emit elementChanged(static_cast<Tremolo*>(element())->chord2());
      }

