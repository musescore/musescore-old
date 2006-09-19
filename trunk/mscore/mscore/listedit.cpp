//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: listedit.cpp,v 1.42 2006/09/15 09:34:57 wschweer Exp $
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
#include "textelement.h"
#include "textstyle.h"
#include "hairpin.h"
#include "beam.h"
#include "tuplet.h"
#include "globals.h"
#include "clef.h"
#include "barline.h"

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
      setWindowTitle(QString("MuseScore: List Edit"));
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

      connect(list, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
         SLOT(itemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
      connect(list, SIGNAL(itemExpanded(QTreeWidgetItem*)), SLOT(itemExpanded(QTreeWidgetItem*)));
      connect(list, SIGNAL(itemCollapsed(QTreeWidgetItem*)), SLOT(itemExpanded(QTreeWidgetItem*)));
      list->resizeColumnToContents(0);
      resize(900, 300);
      }

//---------------------------------------------------------
//   updateList
//---------------------------------------------------------

void PageListEditor::updateList()
      {
      list->clear();
      PageList* pl = cs->pages();
      int tracks = cs->nstaves() * VOICES;
      for (iPage ip = pl->begin(); ip != pl->end(); ++ip) {
            Page* page = *ip;
            ElementItem* pi = new ElementItem(list, page);

            if (page->copyright())
                  new ElementItem(pi, page->copyright());
            if (page->pageNo())
                  new ElementItem(pi, page->pageNo());

            ElementList* el = page->pel();
            for (iElement i = el->begin(); i != el->end(); ++i) {
                  new ElementItem(pi, *i);
                  }

            SystemList* sl = page->systems();
            for (iSystem s = sl->begin(); s != sl->end(); ++s) {
                  System* system = *s;
                  ElementItem* si = new ElementItem(pi, system);

                  if (system->getBarLine())
                        new ElementItem(si, system->getBarLine());

                  // SysStaffList* staffList = system->staves();

                  MeasureList* ml = (*s)->measures();
                  for (iMeasure im = ml->begin(); im != ml->end(); ++im) {
                        Measure* measure = *im;
                        ElementItem* mi = new ElementItem(si, measure);

                        MStaffList* sl = measure->staffList();
                        int staff = 0;
                        for (iMStaff i = sl->begin(); i != sl->end(); ++i, ++staff) {
                              MStaff* ms = &*i;
                              if (ms->endBarLine) {
                                    new ElementItem(mi, ms->endBarLine);
                                    }
                              }

                        ElementList* el = measure->el();
                        for (iElement ie = el->begin(); ie != el->end(); ++ie)
                              new ElementItem(mi, *ie);

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
                                          pstl::plist<NoteAttribute*>* al = chord->getAttributes();
                                          pstl::plist<HelpLine*>* hl = chord->getHelpLines();
                                          for (pstl::plist<NoteAttribute*>::iterator i = al->begin(); i != al->end(); ++i)
                                                new ElementItem(sei, *i);
                                          for (pstl::plist<HelpLine*>::iterator i = hl->begin(); i != hl->end(); ++i)
                                                new ElementItem(sei, *i);
                                          NoteList* notes = chord->noteList();
                                          for (iNote in = notes->begin(); in != notes->end(); ++in) {
                                                Note* note = in->second;
                                                ElementItem* ni = new ElementItem(sei, note);
                                                if (note->accidental()) {
                                                      new ElementItem(ni, note->accidental());
                                                      }
                                                if (note->fingering()) {
                                                      new ElementItem(ni, note->fingering());
                                                      }
                                                }
                                          }
                                    }
                              }
      			BeamList* bl = measure->beamList();
                        for (iBeam ibl = bl->begin(); ibl != bl->end(); ++ibl) {
					new ElementItem(mi, *ibl);
                              }
                        foreach(Tuplet* tuplet, *(measure->tuplets()))
					new ElementItem(mi, tuplet);
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
            case PAGE:    ew = pagePanel;    break;
            case SYSTEM:  ew = systemPanel;  break;
            case MEASURE: ew = measurePanel; break;
            case CHORD:   ew = chordPanel;   break;
            case NOTE:    ew = notePanel;    break;
            case REST:    ew = restPanel;    break;
            case CLEF:    ew = clefPanel;    break;
            case TIMESIG: ew = timesigPanel; break;
            case KEYSIG:  ew = keysigPanel;  break;
            case SEGMENT: ew = segmentView;  break;
            case HAIRPIN: ew = hairpinView;  break;
            case BAR_LINE: ew = barLineView; break;
            case FINGERING:
            case TEXT:
                  ew = textView;
                  break;
            case ACCIDENTAL:
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
      ElementList* el =p->pel();
      for (iElement i = el->begin(); i != el->end(); ++i) {
            ElementListWidgetItem* item = new ElementListWidgetItem(*i);
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
      mb.startRepeat->setChecked(m->startRepeat());
      mb.endRepeat->setValue(m->endRepeat());
      mb.ending->setValue(m->ending());
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
      sb.segmentType->setCurrentIndex(s->type());
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

      // chord
      QWidget* ch = new QWidget;
      cb.setupUi(ch);
      layout->addWidget(ch);
      layout->addStretch(100);
      connect(cb.hookButton, SIGNAL(clicked()), SLOT(hookClicked()));
      connect(cb.stemButton, SIGNAL(clicked()), SLOT(stemClicked()));
      connect(cb.stemDirection, SIGNAL(activated(int)), SLOT(directionChanged(int)));
      connect(cb.attributeList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoAttribute(QListWidgetItem*)));
      connect(cb.helplineList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoHelpline(QListWidgetItem*)));
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
      crb.beamMode->addItem(tr("auto"));
      crb.beamMode->addItem(tr("beam begin"));
      crb.beamMode->addItem(tr("beam mid"));
      crb.beamMode->addItem(tr("beam end"));
      crb.beamMode->addItem(tr("no beam"));
      crb.beamMode->addItem(tr("begin 1/32"));
      crb.beamMode->setCurrentIndex(int(chord->beamMode()));

      cb.hookButton->setEnabled(chord->hook());
      cb.stemButton->setEnabled(chord->stem());
      cb.graceNote->setChecked(chord->grace());
      cb.stemDirection->addItem(tr("Auto"), 0);
      cb.stemDirection->addItem(tr("Up"), 1);
      cb.stemDirection->addItem(tr("Down"), 2);
      cb.stemDirection->setCurrentIndex(int(chord->stemDirection()));

      pstl::plist<NoteAttribute*>* al = chord->getAttributes();
      pstl::plist<HelpLine*>* hl = chord->getHelpLines();

      for (pstl::plist<NoteAttribute*>::iterator i = al->begin(); i != al->end(); ++i) {
            QString s;
            s.setNum(long(*i), 16);
            QListWidgetItem* item = new QListWidgetItem(s, 0, long(*i));
            cb.attributeList->addItem(item);
            }
      for (pstl::plist<HelpLine*>::iterator i = hl->begin(); i != hl->end(); ++i) {
            QString s;
            s.setNum(long(*i), 16);
            QListWidgetItem* item = new QListWidgetItem(s, 0, long(*i));
            cb.helplineList->addItem(item);
            }
      }

//---------------------------------------------------------
//   gotoAttribute
//---------------------------------------------------------

void ShowChordWidget::gotoAttribute(QListWidgetItem* ai)
      {
      NoteAttribute* attr = (NoteAttribute*)(ai->type());
      emit elementChanged(attr);
      }

//---------------------------------------------------------
//   gotoHelpline
//---------------------------------------------------------

void ShowChordWidget::gotoHelpline(QListWidgetItem*)
      {
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
      element()->score()->layout();
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
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ShowNoteWidget::setElement(Element* e)
      {
      Note* note = (Note*)e;
      ShowElementBase::setElement(e);

      nb.pitch->setValue(note->pitch());
      nb.userAccidental->setValue(note->userAccidental());
      nb.line->setValue(note->line());
      nb.move->setValue(note->move());
      nb.dots->setValue(note->dots());
      nb.graceNote->setChecked(note->grace());
      nb.mirror->setChecked(note->mirror());
      //accidental
      //note head
      //fingering
      //tieFor
      //tieBack
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
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void TextView::setElement(Element* e)
      {
      TextElement* te = (TextElement*)e;
      ShowElementBase::setElement(e);
      tb.style->clear();
      for (iTextStyle i = textStyles.begin(); i != textStyles.end(); ++i) {
            TextStyle* s = &*i;
            tb.style->addItem(s->name);
            }
      tb.style->setCurrentIndex(te->style());
      tb.text->setText(te->getText());
//      tb.xoffset->setValue(te->styleOffset().x());
//      tb.yoffset->setValue(te->styleOffset().y());
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
      hp.tick1->setValue(hairpin->tick1());
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
      eb.color->setAutoFillBackground(true);
      connect(eb.nextButton,     SIGNAL(clicked()), SLOT(nextClicked()));
      connect(eb.previousButton, SIGNAL(clicked()), SLOT(previousClicked()));
      connect(eb.parentButton,   SIGNAL(clicked()), SLOT(parentClicked()));
      connect(eb.anchorButton,   SIGNAL(clicked()), SLOT(anchorClicked()));
      connect(eb.offsetx, SIGNAL(valueChanged(double)), SLOT(offsetxChanged(double)));
      connect(eb.offsety, SIGNAL(valueChanged(double)), SLOT(offsetyChanged(double)));
      connect(eb.selected, SIGNAL(clicked(bool)), SLOT(selectedClicked(bool)));
      connect(eb.visible, SIGNAL(clicked(bool)), SLOT(visibleClicked(bool)));
      }

//---------------------------------------------------------
//   ShowElementBase
//---------------------------------------------------------

void ShowElementBase::setElement(Element* e)
      {
      el = e;

      eb.subtype->setValue(e->subtype());
      eb.selected->setChecked(e->selected());
      eb.generated->setChecked(e->generated());
      eb.visible->setChecked(e->visible());
      eb.voice->setValue(e->voice() + 1); // show 1-4
      eb.time->setValue(e->tick());
      eb.duration->setValue(e->tickLen());
      eb.posx->setValue(e->ipos().x());
      eb.posy->setValue(e->ipos().y());
      eb.offsetx->setValue(e->userOff().x());
      eb.offsety->setValue(e->userOff().y());
      eb.bboxx->setValue(e->bbox().x());
      eb.bboxy->setValue(e->bbox().y());
      eb.bboxw->setValue(e->bbox().width());
      eb.bboxh->setValue(e->bbox().height());
      QPalette p(eb.color->palette());
      p.setColor(QPalette::Window, e->color());
      eb.color->setPalette(p);
      eb.nextButton->setEnabled(e->next());
      eb.previousButton->setEnabled(e->prev());
      eb.parentButton->setEnabled(e->parent());
      eb.anchorButton->setEnabled(e->anchor());
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
      el->score()->update(r | el->abbox());
      }

//---------------------------------------------------------
//   visibleClicked
//---------------------------------------------------------

void ShowElementBase::visibleClicked(bool val)
      {
      QRectF r(el->abbox());
      el->setVisible(val);
      el->score()->update(r | el->abbox());
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
      emit elementChanged(el->anchor());
      }

//---------------------------------------------------------
//   offsetxChanged
//---------------------------------------------------------

void ShowElementBase::offsetxChanged(double val)
      {
      QRectF r(el->abbox());
      el->setUserXoffset(val);
      Element* e = el;
      while ((e = e->parent()))
            e->bboxUpdate();
      el->score()->update(r | el->abbox());
      }

//---------------------------------------------------------
//   offsetyChanged
//---------------------------------------------------------

void ShowElementBase::offsetyChanged(double val)
      {
      QRectF r(el->abbox());
      el->setUserYoffset(val);
      el->score()->update(r | el->abbox());
      }

