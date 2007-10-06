//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: listedit.cpp,v 1.42 2006/09/15 09:34:57 wschweer Exp $
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

#include "listedit.h"
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
#include "layout.h"
#include "lyrics.h"
#include "volta.h"
#include "line.h"

//---------------------------------------------------------
//   ElementItem
//---------------------------------------------------------

class ElementItem : public QTreeWidgetItem
      {
      Element* el;

   public:
      ElementItem(QTreeWidget* lv, Element* e);
      ElementItem(ElementItem* ei, Element* e);
      Element* element() const { return el; }
      void init();
      };

ElementItem::ElementItem(QTreeWidget* lv, Element* e)
   : QTreeWidgetItem(lv, e->type())
      {
      el = e;
      init();
      }

ElementItem::ElementItem(ElementItem* ei, Element* e)
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
//   PageListEditor
//---------------------------------------------------------

PageListEditor::PageListEditor(Score* s)
   : QWidget()
      {
      setWindowTitle(tr("MuseScore: List Edit"));
      cs = s;
      QHBoxLayout* hbox = new QHBoxLayout;
      setLayout(hbox);

      QSplitter* split  = new QSplitter;
      split->setOpaqueResize(true);

      hbox->addWidget(split);
      list = new QTreeWidget;
      list->setColumnCount(1);
      split->addWidget(list);
      split->setStretchFactor(0, 10);

      list->setSelectionMode(QAbstractItemView::SingleSelection);
      list->setRootIsDecorated(true);
      list->setColumnCount(1);
      list->setHeaderLabels(QStringList("Element"));
      list->setSortingEnabled(false);
      list->setUniformRowHeights(true);

      stack = new QStackedWidget;
      split->addWidget(stack);

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
      connect(tupletView,   SIGNAL(scoreChanged()), SLOT(layoutScore()));
      connect(notePanel,    SIGNAL(scoreChanged()), SLOT(layoutScore()));

      connect(list, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
         SLOT(itemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
      connect(list, SIGNAL(itemExpanded(QTreeWidgetItem*)), SLOT(itemExpanded(QTreeWidgetItem*)));
      connect(list, SIGNAL(itemCollapsed(QTreeWidgetItem*)), SLOT(itemExpanded(QTreeWidgetItem*)));
      list->resizeColumnToContents(0);
      resize(1000, 300);
      }

//---------------------------------------------------------
//   layoutScore
//---------------------------------------------------------

void PageListEditor::layoutScore()
      {
      cs->setLayoutAll(true);
      cs->end();
      }

//---------------------------------------------------------
//   updateList
//---------------------------------------------------------

void PageListEditor::updateList()
      {
      list->clear();
      ElementItem* li = new ElementItem(list, cs->mainLayout());
      foreach(Element* el, *cs->gel()) {
            if (el->type() == SLUR) {
                  ElementItem* se = new ElementItem(li, el);
                  Slur* slur = (Slur*)el;
                  foreach(Element* el1, *slur->elements())
                        new ElementItem(se, el1);
                  }
            else if (el->type() == HAIRPIN
               || el->type() == OTTAVA
               || el->type() == PEDAL
               || el->type() == TRILL
               || el->type() == SLUR
               ) {
                  ElementItem* se = new ElementItem(li, el);
                  SLine* line = (SLine*)el;
                  foreach(LineSegment* ls, line->lineSegments())
                        new ElementItem(se, ls);
                  }
            else
                  new ElementItem(li, el);
            }

      PageList* pl = cs->mainLayout()->pages();
      int staves = cs->nstaves();
      int tracks = staves * VOICES;
      for (iPage ip = pl->begin(); ip != pl->end(); ++ip) {
            Page* page = *ip;
            ElementItem* pi = new ElementItem(list, page);

            if (page->copyright())
                  new ElementItem(pi, page->copyright());
            if (page->pageNo())
                  new ElementItem(pi, page->pageNo());

            foreach(Element* el, page->pel())
                  new ElementItem(pi, el);

            foreach(System* system, *page->systems()) {
                  ElementItem* si = new ElementItem(pi, system);

                  if (system->getBarLine())
                        new ElementItem(si, system->getBarLine());

                  // SysStaffList* staffList = system->staves();

                  foreach (Measure* measure, system->measures()) {
                        ElementItem* mi = new ElementItem(si, measure);

#if 0
                        MStaffList* sl = measure->staffList();
                        int staff = 0;
                        for (iMStaff i = sl->begin(); i != sl->end(); ++i, ++staff) {
                              MStaff* ms = &*i;
                              if (ms->endBarLine) {
                                    new ElementItem(mi, ms->endBarLine);
                                    }
                              }
#endif

                        foreach(Element* e, *measure->el()) {
                              switch(e->type()) {
                                    case HAIRPIN:
                                    case OTTAVA:
                                    case PEDAL:
                                    case TRILL:
                                    case VOLTA:
                                          {
                                          ElementList eel;
                                          ((SLine*)e)->collectElements(eel);
                                          foreach(Element* e, eel)
                                                new ElementItem(mi, e);
                                          }
                                          // fall through:
                                    default:
                                          new ElementItem(mi, e);
                                          break;
                                    }
                              }

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
                                          foreach(NoteAttribute* a, *chord->getAttributes())
                                                new ElementItem(sei, a);
                                          foreach(LedgerLine* h, *chord->ledgerLines())
                                                new ElementItem(sei, h);
                                          NoteList* notes = chord->noteList();
                                          for (iNote in = notes->begin(); in != notes->end(); ++in) {
                                                Note* note = in->second;
                                                ElementItem* ni = new ElementItem(sei, note);
                                                if (note->accidental()) {
                                                      new ElementItem(ni, note->accidental());
                                                      }
                                                foreach(Text* f, note->fingering())
                                                      new ElementItem(ni, f);
                                                if (note->tieFor())
                                                      new ElementItem(ni, note->tieFor());
                                                }
                                          }
                                    }
                              for (int i = 0; i < staves; ++i) {
                                    foreach(Lyrics* l, *(segment->lyricsList(i))) {
                                          new ElementItem(segItem, l);
                                          }
                                    }
                              }
                        foreach(Beam* beam, *measure->beamList())
					new ElementItem(mi, beam);
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

bool PageListEditor::searchElement(QTreeWidgetItem* pi, Element* el)
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

void PageListEditor::setElement(Element* el)
      {
      for (int i = 0;; ++i) {
            QTreeWidgetItem* item = list->topLevelItem(i);
            if (item == 0) {
                  printf("PageListEditor::Element not found %s %p\n",
                    el->name(), el);
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
      }

//---------------------------------------------------------
//   itemExpanded
//---------------------------------------------------------

void PageListEditor::itemExpanded(QTreeWidgetItem*)
      {
      list->resizeColumnToContents(0);
      }

//---------------------------------------------------------
//   itemChanged
//---------------------------------------------------------

void PageListEditor::itemChanged(QTreeWidgetItem* i, QTreeWidgetItem*)
      {
      if (i == 0)
            return;
      Element* el = ((ElementItem*)i)->element();
      setWindowTitle(QString("MuseScore: List Edit: ") + el->name());
      ShowElementBase* ew = 0;
      switch (el->type()) {
            case PAGE:     ew = pagePanel;    break;
            case SYSTEM:   ew = systemPanel;  break;
            case MEASURE:  ew = measurePanel; break;
            case CHORD:    ew = chordPanel;   break;
            case NOTE:     ew = notePanel;    break;
            case REST:     ew = restPanel;    break;
            case CLEF:     ew = clefPanel;    break;
            case TIMESIG:  ew = timesigPanel; break;
            case KEYSIG:   ew = keysigPanel;  break;
            case SEGMENT:  ew = segmentView;  break;
            case HAIRPIN:  ew = hairpinView;  break;
            case BAR_LINE: ew = barLineView;  break;
            case DYNAMIC:  ew = dynamicView;  break;
            case TUPLET:   ew = tupletView;   break;
            case SLUR:     ew = slurView;     break;
            case TIE:      ew = tieView;      break;
            case VOLTA:    ew = voltaView;    break;
            case VOLTA_SEGMENT: ew = voltaSegmentView; break;
            case TEXT:
                  ew = textView;
                  break;
            default:
                  ew = elementView;
                  break;
            }
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
      connect(pb.elementList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(itemClicked(QListWidgetItem*)));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ShowPageWidget::setElement(Element* e)
      {
      Page* p = (Page*)e;
      ShowElementBase::setElement(e);
      pb.pageNo->setValue(p->no());
      pb.elementList->clear();
      foreach(Element* el, p->pel()) {
            ElementListWidgetItem* item = new ElementListWidgetItem(el);
            pb.elementList->addItem(item);
            }
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
      connect(mb.pel, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(elementClicked(QTreeWidgetItem*)));
      seg->show();
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
      mb.beams->setValue(m->beamList()->size());
      mb.tuplets->setValue(m->tuplets()->size());
      mb.pageElements->setValue(m->pel()->size());
      mb.measureNo->setValue(m->no());
      mb.noOffset->setValue(m->noOffset());
      mb.stretch->setValue(m->userStretch());
      mb.lineBreak->setChecked(m->lineBreak());
      mb.pageBreak->setChecked(m->pageBreak());
      mb.irregular->setChecked(m->irregular());
      mb.endRepeat->setValue(m->endRepeat());
      mb.ending->setValue(m->ending());
      mb.repeatFlags->setText(QString("0x%1").arg(m->repeatFlags(), 6, 16, QChar('0')));
      mb.sel->clear();
      foreach(const Element* e, *m->el()) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, e->name());
            item->setText(1, QString("%1").arg(e->subtype()));
            void* p = (void*) e;
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(p));
            mb.sel->addTopLevelItem(item);
            }
      mb.pel->clear();
      foreach(const Element* e, *m->pel()) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, e->name());
            item->setText(1, QString("%1").arg(e->subtype()));
            void* p = (void*) e;
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(p));
            mb.pel->addTopLevelItem(item);
            }
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
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void SegmentView::setElement(Element* e)
      {
      ShowElementBase::setElement(e);

      Segment* s = (Segment*)e;
      ShowElementBase::setElement(e);
      sb.segmentType->setCurrentIndex(s->subtype());
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
      // chort rest
      QWidget* chr = new QWidget;
      crb.setupUi(chr);
      layout->addWidget(chr);
      connect(crb.beamButton, SIGNAL(clicked()), SLOT(beamClicked()));
      connect(crb.tupletButton, SIGNAL(clicked()), SLOT(tupletClicked()));
      connect(crb.upFlag, SIGNAL(toggled(bool)), SLOT(upChanged(bool)));
      connect(crb.beamMode, SIGNAL(activated(int)), SLOT(beamModeChanged(int)));
      connect(crb.attributes, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));

      // chord
      QWidget* ch = new QWidget;
      cb.setupUi(ch);
      layout->addWidget(ch);
      layout->addStretch(100);
      connect(cb.hookButton, SIGNAL(clicked()), SLOT(hookClicked()));
      connect(cb.stemButton, SIGNAL(clicked()), SLOT(stemClicked()));
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
      crb.upFlag->setChecked(chord->isUp());
      crb.beamMode->setCurrentIndex(int(chord->beamMode()));

      cb.hookButton->setEnabled(chord->hook());
      cb.stemButton->setEnabled(chord->stem());
      cb.graceNote->setChecked(chord->grace());
      cb.stemDirection->setCurrentIndex(int(chord->stemDirection()));

      crb.attributes->clear();
      foreach(NoteAttribute* a, *chord->getAttributes()) {
            QString s;
            s.setNum(long(a), 16);
            QListWidgetItem* item = new QListWidgetItem(s, 0, long(a));
            crb.attributes->addItem(item);
            }
      cb.helplineList->clear();
      foreach(LedgerLine* h, *chord->ledgerLines()) {
            QString s;
            s.setNum(long(h), 16);
            QListWidgetItem* item = new QListWidgetItem(s, 0, long(h));
            cb.helplineList->addItem(item);
            }
      cb.notes->clear();
      NoteList* nl = chord->noteList();
      for (ciNote in = nl->begin(); in != nl->end(); ++in) {
            Note* n = in->second;
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
      nb.line->setValue(note->line());
      nb.move->setValue(note->move());
      nb.dots->setValue(note->dots());
      nb.graceNote->setChecked(note->grace());
      nb.mirror->setChecked(note->mirror());
      nb.tpc->setValue(note->tpc());
      nb.head->setValue(note->noteHead());
      nb.headGroup->setValue(note->headGroup());

      nb.tieFor->setEnabled(note->tieFor());
      nb.tieBack->setEnabled(note->tieBack());
      nb.accidental->setEnabled(note->accidental());

      foreach(Text* text, note->fingering()) {
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
      idx->setValue(clef->subtype() & clefSmallBit);
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
      ShowElementBase::setElement(e);
      tb.style->clear();
      foreach(TextStyle* s, te->score()->textStyles())
            tb.style->addItem(s->name);
      tb.text->setDocument(te->getDoc());
      tb.xoffset->setValue(te->xoff());
      tb.yoffset->setValue(te->yoff());
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

      tb.style->clear();
      foreach (TextStyle* s, dynamic->score()->textStyles())
            tb.style->addItem(s->name);
//      tb.style->setCurrentIndex(dynamic->style());
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
      connect(tb.hasNumber, SIGNAL(toggled(bool)), SLOT(hasNumberToggled(bool)));
      connect(tb.hasLine, SIGNAL(toggled(bool)), SLOT(hasLineToggled(bool)));
      }

//---------------------------------------------------------
//   hasNumberToggled
//---------------------------------------------------------

void TupletView::hasNumberToggled(bool val)
      {
      Tuplet* tuplet = (Tuplet*)element();
      tuplet->setHasNumber(val);
      emit scoreChanged();
      }

//---------------------------------------------------------
//   hasLineToggled
//---------------------------------------------------------

void TupletView::hasLineToggled(bool val)
      {
      Tuplet* tuplet = (Tuplet*)element();
      tuplet->setHasLine(val);
      emit scoreChanged();
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
      tb.hasNumber->setChecked(tuplet->hasNumber());
      tb.hasLine->setChecked(tuplet->hasLine());
      tb.baseLen->setValue(tuplet->baseLen());
      tb.normalNotes->setValue(tuplet->normalNotes());
      tb.actualNotes->setValue(tuplet->actualNotes());
      tb.number->setEnabled(tuplet->number());
      ChordRestList* el = tuplet->elements();
      tb.elements->clear();
      for (iChordRest i = el->begin(); i != el->end(); ++i) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            ChordRest* cr = i->second;
            item->setText(0, cr->name());
            item->setText(1, QString("%1").arg(cr->tick()));
            item->setText(2, QString("%1").arg(cr->tickLen()));
            void* p = (void*) cr;
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
      connect(eb.nextButton,     SIGNAL(clicked()), SLOT(nextClicked()));
      connect(eb.previousButton, SIGNAL(clicked()), SLOT(previousClicked()));
      connect(eb.parentButton,   SIGNAL(clicked()), SLOT(parentClicked()));
      connect(eb.anchorButton,   SIGNAL(clicked()), SLOT(anchorClicked()));
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
      eb.generated->setChecked(e->generated());
      eb.visible->setChecked(e->visible());
      eb.voice->setValue(e->voice() + 1);    // show 1-4
      eb.staff->setValue(e->staffIdx() + 1); // show 1-n
      eb.time->setValue(e->tick());
      eb.duration->setValue(e->tickLen());
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
      eb.nextButton->setEnabled(e->next());
      eb.previousButton->setEnabled(e->prev());
      eb.parentButton->setEnabled(e->parent());
      eb.anchorButton->setEnabled(e->anchorMeasure());
      }

//---------------------------------------------------------
//   selectedClicked
//---------------------------------------------------------

void ShowElementBase::selectedClicked(bool val)
      {
      QRectF r(el->abbox());
      if (val)
            el->score()->select(el, Qt::ShiftModifier, 0);
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
//   nextClicked
//---------------------------------------------------------

void ShowElementBase::nextClicked()
      {
      emit elementChanged(el->next());
      }

//---------------------------------------------------------
//   previousClicked
//---------------------------------------------------------

void ShowElementBase::previousClicked()
      {
      emit elementChanged(el->prev());
      }

//---------------------------------------------------------
//   parentClicked
//---------------------------------------------------------

void ShowElementBase::parentClicked()
      {
      emit elementChanged(el->parent());
      }

//---------------------------------------------------------
//   anchorClicked
//---------------------------------------------------------

void ShowElementBase::anchorClicked()
      {
      emit elementChanged(el->anchorMeasure());
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
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void SlurView::setElement(Element* e)
      {
      Slur* slur = (Slur*)e;
      ShowElementBase::setElement(e);

      st.segments->clear();
      QList<SlurSegment*>* el = slur->elements();
      foreach(const Element* e, *el) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, QString("%1").arg((unsigned long)e, 8, 16));
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>((void*)e));
            st.segments->addTopLevelItem(item);
            }
      st.upFlag->setChecked(slur->isUp());
      st.direction->setCurrentIndex(slur->slurDirection());

      sb.tick1->setValue(slur->tick1());
      sb.staff1->setValue(slur->track1() / VOICES);
      sb.voice1->setValue(slur->track1() % VOICES);
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
      QList<SlurSegment*>* el = tie->elements();
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
//   VoltaView
//---------------------------------------------------------

VoltaView::VoltaView()
   : ShowElementBase()
      {
      QWidget* w = new QWidget;
      lb.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
      connect(lb.segments, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(segmentClicked(QTreeWidgetItem*)));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void VoltaView::setElement(Element* e)
      {
      Volta* volta = (Volta*)e;
      ShowElementBase::setElement(e);

      lb.tick2->setValue(volta->tick2());
      lb.segments->clear();
      const QList<LineSegment*>& el = volta->lineSegments();
      foreach(const Element* e, el) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, QString("%1").arg((unsigned long)e, 8, 16));
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>((void*)e));
            lb.segments->addTopLevelItem(item);
            }
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


