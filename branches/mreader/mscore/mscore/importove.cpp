//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: importove.cpp 2814 2010-03-04 18:27:09Z vanferry $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include <vector>
#include "ove.h"

#include "al/sig.h"
#include "al/tempo.h"
#include "arpeggio.h"
#include "articulation.h"
#include "barline.h"
#include "bracket.h"
#include "breath.h"
#include "chord.h"
#include "clef.h"
#include "dynamics.h"
#include "hairpin.h"
#include "harmony.h"
#include "glissando.h"
#include "keysig.h"
#include "layoutbreak.h"
#include "lyrics.h"
#include "measure.h"
#include "note.h"
#include "ottava.h"
#include "part.h"
#include "pedal.h"
#include "pitchspelling.h"
#include "repeat.h"
#include "rest.h"
#include "score.h"
#include "slur.h"
#include "staff.h"
#include "tempotext.h"
#include "text.h"
#include "timesig.h"
#include "tuplet.h"
#include "tremolo.h"
#include "volta.h"

class MeasureToTick {
public:
	MeasureToTick();
	~MeasureToTick();

public:
	void build(OVE::OveSong* ove, int quarter);

	int getTick(int measure, int tick_pos);
	static int unitToTick(int unit, int quarter);

	struct TimeTick	{
		int numerator_;
		int denominator_;
		int measure_;
		int tick_;
		bool isSymbol_;

		TimeTick():numerator_(4), denominator_(4), measure_(0), tick_(0), isSymbol_(false){}
	};
	std::vector<TimeTick> getTimeTicks() const;

private:
	int quarter_;
	OVE::OveSong* ove_;
	std::vector<TimeTick> tts_;
};

int getMeasureTick(int quarter, int num, int den){
	return quarter * 4 * num / den;
}

MeasureToTick::MeasureToTick(){
	quarter_ = 480;
	ove_ = 0;
}

MeasureToTick::~MeasureToTick(){
}

void MeasureToTick::build(OVE::OveSong* ove, int quarter){
	unsigned int i;
	int currentTick = 0;
	unsigned int measureCount = ove->getMeasureCount();

	quarter_ = quarter;
	ove_ = ove;
	tts_.clear();

	for( i=0; i<measureCount; ++i )	{
		OVE::Measure* measure = ove_->getMeasure(i);
		OVE::TimeSignature* time = measure->getTime();
		TimeTick tt;
		bool change = false;

		tt.tick_ = currentTick;
		tt.numerator_ = time->getNumerator();
		tt.denominator_ = time->getDenominator();
		tt.measure_ = i;
		tt.isSymbol_ = time->getIsSymbol();

		if( i == 0 ){
			change = true;
		}
		else {
			OVE::TimeSignature* previousTime = ove_->getMeasure(i-1)->getTime();

			if( time->getNumerator() != previousTime->getNumerator() ||
				time->getDenominator() != previousTime->getDenominator() ){
				change = true;
			}
			else if(time->getIsSymbol() != previousTime->getIsSymbol()){
				change = true;
			}
		}

		if( change ){
			tts_.push_back(tt);
		}

		currentTick += getMeasureTick(quarter_, tt.numerator_, tt.denominator_);
	}
}

int MeasureToTick::getTick(int measure, int tick_pos){
	unsigned int i;
	TimeTick tt;

	for( i=0; i<tts_.size(); ++i ) {
		if( measure >= tts_[i].measure_ && ( i==tts_.size()-1 || measure < tts_[i+1].measure_ ) ) {
			int measuresTick = (measure - tts_[i].measure_) * getMeasureTick(quarter_, tts_[i].numerator_, tts_[i].denominator_);
			return tts_[i].tick_ + measuresTick + tick_pos;
		}
	}

	return 0;
}

int MeasureToTick::unitToTick(int unit, int quarter) {
	// 0x100 correspond to quarter tick
	float ratio = (float)unit / (float)256.0;
	int tick = ratio * quarter;
	return tick;
}

std::vector<MeasureToTick::TimeTick> MeasureToTick::getTimeTicks() const {
	return tts_;
}

///////////////////////////////////////////////////////////////////
class OveToMScore {
public:
	OveToMScore();
	~OveToMScore();

public:
	void convert(OVE::OveSong* oveData, Score* score);

private:
	void createStructure();
	void convertHeader();
	void convertGroups();
	void convertTrackHeader(OVE::Track* track, Part* part);
	void convertTrackElements(int track);
	void convertLineBreak();
	void convertSignatures();
	void convertMeasures();
	void convertMeasure(Measure* measure);
	void convertMeasureMisc(Measure* measure, int part, int staff, int track);
	void convertNotes(Measure* measure, int part, int staff, int track);
	void convertArticulation(Measure* measure, ChordRest* cr, int track, int absTick, OVE::Articulation* art);
	void convertLyrics(Measure* measure, int part, int staff, int track);
	void convertHarmonys(Measure* measure, int part, int staff, int track);
	void convertRepeats(Measure* measure, int part, int staff, int track);
	void convertSlurs(Measure* measure, int part, int staff, int track);
	void convertDynamics(Measure* measure, int part, int staff, int track);
	void convertExpressions(Measure* measure, int part, int staff, int track);
	void convertGlissandos(Measure* measure, int part, int staff, int track);
	void convertWedges(Measure* measure, int part, int staff, int track);
	void convertOctaveShifts(Measure* measure, int part, int staff, int track);

	OVE::NoteContainer* getContainerByPos(int part, int staff, const OVE::MeasurePos& pos);
	//OVE::MusicData* getMusicDataByUnit(int part, int staff, int measure, int unit, OVE::MusicDataType type);
	OVE::MusicData* getCrossMeasureElementByPos(int part, int staff, const OVE::MeasurePos& pos, int voice, OVE::MusicDataType type);

	void clearUp();

private:
	OVE::OveSong* ove_;
	Score* score_;
	MeasureToTick* mtt_;

	Pedal* pedal_;
};

OveToMScore::OveToMScore() {
	ove_ = 0;
	mtt_ = new MeasureToTick();
	pedal_ = 0;
}

OveToMScore::~OveToMScore() {
	delete mtt_;
}

void OveToMScore::convert(OVE::OveSong* ove, Score* score) {
	ove_ = ove;
	score_ = score;
	mtt_->build(ove_, ove_->getQuarter());

	createStructure();

    convertHeader();
    convertGroups();
    convertSignatures();
    //convertLineBreak();

    convertMeasures();

    // convert elements by ove track sequence
    int staffCount = 0;
	for(int i=0; i<ove_->getPartCount(); ++i ){
		int partStaffCount = ove_->getStaffCount(i) ;
		Part* part = score_->part(i);

		for(int j=0; j<partStaffCount; ++j){
			OVE::Track* track = ove_->getTrack(i, j);
			int trackIndex = staffCount + j;

			convertTrackHeader(track, part);
			convertTrackElements(trackIndex);
		}

		staffCount += partStaffCount;
	}

	clearUp();
}

void OveToMScore::createStructure() {
	int i;
	for(i=0; i<ove_->getPartCount(); ++i ){
		int partStaffCount = ove_->getStaffCount(i) ;
		Part* part = new Part(score_);

		for(int j=0; j<partStaffCount; ++j){
			//OVE::Track* track = ove_->getTrack(i, j);
			Staff* staff = new Staff(score_, part, j);

			part->staves()->push_back(staff);
	        score_->staves().push_back(staff);
		}

		score_->appendPart(part);
		part->setStaves(partStaffCount);
	}

    for(i = 0; i <ove_->getMeasureCount(); ++i) {
    	Measure* measure  = new Measure(score_);
    	int tick = mtt_->getTick(i, 0);
    	measure->setTick(tick);
    	measure->setNo(i);
    	score_->measures()->add(measure);
    }
}

void OveToMScore::clearUp() {
	if(pedal_ != NULL) {
		delete pedal_;
		pedal_ = 0;
	}
}

OVE::Staff* getStaff(const OVE::OveSong* ove, int track) {
	if (ove->getLineCount() > 0) {
		OVE::Line* line = ove->getLine(0);
		if(line != 0 && line->getStaffCount() > 0) {
			OVE::Staff* staff = line->getStaff(track);
			return staff;
		}
	}

	return 0;
}

void addText(Score* s, QString strTxt, int sbtp, int stl) {
	if (!strTxt.isEmpty()) {
		Text* text = new Text(s);
		text->setSubtype(sbtp);
		text->setTextStyle(stl);
		text->setText(strTxt);
	}
}

void OveToMScore::convertHeader() {
	std::vector<std::string> titles = ove_->getTitles();
	if( !titles.empty() && titles[0] != std::string() ) {
		QString title = QString::fromLocal8Bit(titles[0].c_str());
		score_->setMovementTitle(title);
	}

	std::vector<std::string> writers = ove_->getWriters();
	if(!writers.empty()) {
		QString composer = QString::fromLocal8Bit(writers[0].c_str());
		addText(score_, composer, TEXT_COMPOSER, TEXT_STYLE_COMPOSER);
	}

	if(writers.size() > 1) {
		QString lyricist = QString::fromLocal8Bit(writers[1].c_str());
		addText(score_, lyricist, TEXT_POET, TEXT_STYLE_POET);
	}
}

void OveToMScore::convertGroups() {
	int i;
	int staffCount = 0;
	const QList<Part*>* parts = score_->parts();
	for(i=0; i<ove_->getPartCount(); ++i ){
		int partStaffCount = ove_->getStaffCount(i);
		if(parts == 0)
			continue;
		Part* part = parts->at(i);
		if(part == 0)
			continue;

		for(int j=0; j<partStaffCount; ++j){
			//OVE::Track* track = ove_->getTrack(i, j);
			int staffIndex = staffCount + j;
			Staff* staff = score_->staff(staffIndex);
			if(staff == 0)
				continue;

			// brace
	        if( j == 0 && partStaffCount == 2 ) {
	        	staff->setBracket(0, BRACKET_AKKOLADE);
	        	staff->setBracketSpan(0, 2);
	        	staff->setBarLineSpan(2);
	        }

	        // bracket
	        OVE::Staff* staffPtr = getStaff(ove_, staffIndex);
	        if(staffPtr != 0 && staffPtr->getGroupType() == OVE::Group_Bracket) {
	        	int span = staffPtr->getGroupStaffCount() + 1;
	        	int endStaff = staffIndex + span;
	        	if(span > 0 && endStaff >= staffIndex && endStaff <= ove_->getTrackCount()) {
	        		staff->addBracket(BracketItem(BRACKET_NORMAL, span));
					staff->setBarLineSpan(span);
	            }
	        }
		}

		staffCount += partStaffCount;
	}
}

int OveClefToClef(OVE::ClefType type){
	int clef = CLEF_G;
	switch(type){
	case OVE::Clef_Treble:{
		clef = CLEF_G;
		break;
	}
	case OVE::Clef_Bass:{
		clef = CLEF_F;
		break;
	}
	case OVE::Clef_Alto:{
		clef = CLEF_C3;
		break;
	}
	case OVE::Clef_UpAlto:{
		clef = CLEF_C4;
		break;
	}
	case OVE::Clef_DownDownAlto:{
		clef = CLEF_C1;
		break;
	}
	case OVE::Clef_DownAlto:{
		clef = CLEF_C2;
		break;
	}
	case OVE::Clef_UpUpAlto:{
		clef = CLEF_C5;
		break;
	}
	case OVE::Clef_Treble8va:{
		clef = CLEF_G1;
		break;
	}
	case OVE::Clef_Bass8va:{
		clef = CLEF_F_8VA;
		break;
	}
	case OVE::Clef_Treble8vb:{
		clef = CLEF_G3;
		break;
	}
	case OVE::Clef_Bass8vb:{
		clef = CLEF_F8;
		break;
	}
	case OVE::Clef_Percussion1:{
		clef = CLEF_PERC;
		break;
	}
	case OVE::Clef_Percussion2:{
		clef = CLEF_PERC2;
		break;
	}
	case OVE::Clef_TAB:{
		clef = CLEF_TAB;
		break;
	}
	default:
		break;
	}
	return clef;
}

void OveToMScore::convertTrackHeader(OVE::Track* track, Part* part){
	if(track == 0 || part == 0)
		return;

	QString longName = QString::fromLocal8Bit(track->getName().c_str());
	if (longName != QString() && track->getShowName()){
		part->setLongName(longName);
	}

	QString shortName = QString::fromLocal8Bit(track->getBriefName().c_str());
	if (shortName != QString() && track->getShowBriefName()) {
		part->setShortName(shortName);
	}

	part->setMidiProgram(track->getPatch());
	//part->setMidiChannel(track->getChannel());

	if (ove_->getShowTransposeTrack() && track->getTranspose() != 0 ) {
		// part->setTransposeDiatonic(-(track->getTranspose()));
            Interval interval = part->transpose();
            interval.diatonic = -track->getTranspose();
            part->setTranspose(interval);
	}
}

int OctaveShiftTypeToInt(OVE::OctaveShiftType type) {
	int subtype = 0;
	switch (type) {
	case OVE::OctaveShift_8: {
		subtype = 0;
		break;
	}
	case OVE::OctaveShift_15: {
		subtype = 1;
		break;
	}
	case OVE::OctaveShift_Minus_8: {
		subtype = 2;
		break;
	}
	case OVE::OctaveShift_Minus_15: {
		subtype = 3;
		break;
	}
	default:
		break;
	}

	return subtype;
}

void OveToMScore::convertTrackElements(int track) {
	Ottava* ottava = 0;

	for(int i=0; i<ove_->getTrackBarCount(); ++i) {
		OVE::MeasureData* measureData = ove_->getMeasureData(track, i);
		if(measureData == 0)
			continue;

		// octave shift
		std::vector<OVE::MusicData*> octaves = measureData->getMusicDatas(OVE::MusicData_OctaveShift_EndPoint);
		for(unsigned int j=0; j<octaves.size(); ++j) {
			OVE::OctaveShiftEndPoint* octave = static_cast<OVE::OctaveShiftEndPoint*>(octaves[j]);

			if(octave->getOctaveShiftPosition() == OVE::OctavePosition_Start) {
				if(ottava == 0) {
					int absTick = mtt_->getTick(i, octave->getTick());
					ottava = new Ottava(score_);
                    ottava->setTrack(track * VOICES);
                    ottava->setTick(absTick);
                    ottava->setSubtype(OctaveShiftTypeToInt(octave->getOctaveShiftType()));

                    int y_off = 0;
                	switch (octave->getOctaveShiftType()) {
                	case OVE::OctaveShift_8:
                	case OVE::OctaveShift_15: {
                		y_off = -3;
                		break;
                	}
                	case OVE::OctaveShift_Minus_8:
                	case OVE::OctaveShift_Minus_15: {
                		y_off = 8;
                		break;
                	}
                	default:{
                		break;
                	}
                	}

                	if(y_off != 0) {
                		ottava->lineSegments().front()->setUserOff(QPointF(0, y_off * score_->spatium()));
                	}
				} else {
					printf("overlapping octave-shift not supported\n");
					delete ottava;
					ottava = 0;
				}
			} else if (octave->getOctaveShiftPosition() == OVE::OctavePosition_Stop) {
				if(ottava != 0) {
					int absTick = mtt_->getTick(i, octave->getEndTick());
                    ottava->setTick2(absTick);
                    if(ottava->tick2() > ottava->tick()){
                    	score_->add(ottava);
                    } else {
                    	delete ottava;
                    }

                    ottava = 0;
				} else {
                    printf("octave-shift stop without start\n");
				}
			}
		}
	}
}

void OveToMScore::convertLineBreak(){
    for (MeasureBase* mb = score_->measures()->first(); mb; mb = mb->next()) {
		if (mb->type() != MEASURE)
			continue;
		Measure* measure = static_cast<Measure*> (mb);

		for (int i = 0; i < ove_->getLineCount(); ++i) {
			OVE::Line* line = ove_->getLine(i);
			if (measure->no() > 0) {
				if ((int)line->getBeginBar() + (int)line->getBarCount()-1 == measure->no()) {
					LayoutBreak* lb = new LayoutBreak(score_);
					lb->setTrack(0);
					lb->setSubtype(LAYOUT_BREAK_LINE);
					measure->add(lb);
				}
			}
		}
	}
}

void OveToMScore::convertSignatures(){
	int i;
	int j;
	int k;

	// Time
	const std::vector<MeasureToTick::TimeTick> tts = mtt_->getTimeTicks() ;
	for( i=0; i<(int)tts.size(); ++i ){
		MeasureToTick::TimeTick tt = tts[i];
		AL::TimeSigMap* sigmap = score_->sigmap();
		Fraction f(tt.numerator_, tt.denominator_);

		sigmap->add(tt.tick_, f);

		Measure* measure  = score_->tick2measure(tt.tick_);
		if(measure){
			for(int staffIdx = 0; staffIdx < score_->nstaves(); ++staffIdx) {
				TimeSig* ts = new TimeSig(score_);
				ts->setTick(tt.tick_);
				ts->setSig(tt.denominator_, tt.numerator_);
				ts->setTrack(staffIdx * VOICES);

				/*int subtype = 0;
				if(tt.numerator_ == 4 && tt.denominator_ == 4 && tt.isSymbol_ ){
					subtype = TSIG_FOUR_FOUR;
				}
				if(tt.numerator_ == 2 && tt.denominator_ == 2 && tt.isSymbol_ ){
					subtype = TSIG_ALLA_BREVE;
				}
				ts->setSubtype(subtype);*/

				Segment* seg = measure->getSegment(ts);
				seg->add(ts);
			}
		}
	}

	// Key
	int staffCount = 0;
	bool createKey = false ;
	for(i=0; i<ove_->getPartCount(); ++i ){
		int partStaffCount = ove_->getStaffCount(i) ;

		for(j=0; j<partStaffCount; ++j){
			//OVE::Track* track = ove_->getTrack(i, j);

			for(k=0; k<ove_->getMeasureCount(); ++k){
				OVE::MeasureData* measureData = ove_->getMeasureData(i, j, k) ;

				if( measureData != 0 ){
					OVE::Key* keyPtr = measureData->getKey() ;

					if( k == 0 || keyPtr->getKey() != keyPtr->getPreviousKey() )	{
						int tick = mtt_->getTick(k, 0);
						int keyValue = keyPtr->getKey();
		                Measure* measure = score_->tick2measure(tick);
		                if(measure){
		                	KeySigEvent key;
		                	key.setAccidentalType(keyValue);
		                	(*score_->staff(staffCount+j)->keymap())[tick] = key;

		                    KeySig* keysig = new KeySig(score_);
		                    keysig->setTick(tick);
		                    keysig->setTrack((staffCount+j) * VOICES);
		                    keysig->setSubtype(key);

		                	Segment* s = measure->getSegment(keysig);
		                	s->add(keysig);

		                	createKey = true;
		                }
					}
				}
			}
		}

		staffCount += partStaffCount;
	}

	if( !createKey ){
		staffCount = 0;
		for(i=0; i<ove_->getPartCount(); ++i ){
			int partStaffCount = ove_->getStaffCount(i) ;

			for(j=0; j<partStaffCount; ++j){
				//OVE::Track* track = ove_->getTrack(i, j);

				Measure* measure = score_->tick2measure(mtt_->getTick(0, 0));
				if(measure){
					KeySig* keysig = new KeySig(score_);
			        keysig->setTick(0);
			        keysig->setTrack((staffCount+j) * VOICES);
			        keysig->setSubtype(0);

					Segment* s = measure->getSegment(keysig);
					s->add(keysig);
				}
			}
			staffCount += partStaffCount;
		}
	}

	// Clef
	staffCount = 0;
	for(i=0; i<ove_->getPartCount(); ++i){
		int partStaffCount = ove_->getStaffCount(i) ;

		for(j=0; j<partStaffCount; ++j){
			// start clef
			Staff* staff = score_->staff(staffCount+j);
			if(staff){
				OVE::Track* track = ove_->getTrack(i, j);
				ClefList* ct = staff->clefList();
				(*ct)[0] = OveClefToClef(track->getStartClef());
			}

			// clef in measure
			for(k=0; k<ove_->getMeasureCount(); ++k){
				//OVE::Measure* oveMeasure = ove_->getMeasure(k);
				OVE::MeasureData* measureData = ove_->getMeasureData(i, j, k);
				std::vector<OVE::MusicData*> clefs = measureData->getMusicDatas(OVE::MusicData_Clef);
				Measure* measure = score_->tick2measure(mtt_->getTick(k, 0));

				for( int l=0; l<(int)clefs.size(); ++l){
					if(measure != 0){
						OVE::Clef* clefPtr = static_cast<OVE::Clef*>(clefs[l]);
						int absTick = mtt_->getTick(k, clefPtr->getTick());
						int clefIndex = OveClefToClef(clefPtr->getClefType());

			            Clef* clef = new Clef(score_, clefIndex);
			            clef->setTick(absTick);
			            clef->setTrack((staffCount+j)*VOICES);

			            Segment* s = measure->getSegment(clef);
			            s->add(clef);

						if(staff){
							ClefList* ct = staff->clefList();
							(*ct)[absTick] = clefIndex;
						}
					}
				}
			}
		}

		staffCount += partStaffCount;
	}

	// Tempo
	std::map<int, int> tempos;
	for(i=0; i<ove_->getPartCount(); ++i){
		int partStaffCount = ove_->getStaffCount(i);

		for(j=0; j<partStaffCount; ++j){
			for(k=0; k<ove_->getMeasureCount(); ++k){
				OVE::Measure* measure = ove_->getMeasure(k);
				OVE::MeasureData* measureData = ove_->getMeasureData(i, j, k);
				std::vector<OVE::MusicData*> tempoPtrs = measureData->getMusicDatas(OVE::MusicData_Tempo);

				if(k==0 || ( k>0 && abs(measure->getTypeTempo()-ove_->getMeasure(k-1)->getTypeTempo())>0.01 )){
					int tick = mtt_->getTick(k, 0);
					tempos[tick] = (int)measure->getTypeTempo();
				}

				for(unsigned int l=0; l<tempoPtrs.size(); ++l) {
					OVE::Tempo* ptr = static_cast<OVE::Tempo*>(tempoPtrs[l]);
					int tick = mtt_->getTick(measure->getBarNumber()->getIndex(), ptr->getTick());
					int tempo = ptr->getQuarterTempo()>0 ? ptr->getQuarterTempo() : 1;

					tempos[tick] = tempo;
				}
			}
		}
	}

	std::map<int, int>::iterator it;
	int lastTempo = 0;
	for(it=tempos.begin(); it!=tempos.end(); ++it) {
		if( it==tempos.begin() || (*it).second != lastTempo ) {
	        AL::TempoMap* tl = score_->tempomap();
	        if(tl){
	        	double tpo = ((double)(*it).second) / 60.0;
	        	tl->addTempo((*it).first, tpo);
	        }
		}

		lastTempo = (*it).second;
	}
}

int ContainerToTick(OVE::NoteContainer* container, int quarter){
	int tick = OVE::NoteTypeToTick(container->getNoteType(), quarter);

	int dotLength = tick;
	for( int i=0; i<container->getDot(); ++i ) {
		dotLength /= 2;
	}
	dotLength = tick - dotLength;

	if(container->getTuplet() > 0) {
		tick = tick * container->getSpace() / container->getTuplet();
	}

	tick += dotLength;

	return tick;
}

OVE::Tuplet* getTuplet(const std::vector<OVE::MusicData*>& tuplets, int unit){
	for(int i=0; i<(int)tuplets.size(); ++i){
		if(unit >= tuplets[i]->start()->getOffset() && unit <= tuplets[i]->stop()->getOffset()){
			OVE::Tuplet* tuplet = static_cast<OVE::Tuplet*>(tuplets[i]);
			return tuplet;
		}
	}
	return 0;
}

Duration OveNoteType_To_Duration(OVE::NoteType noteType){
	Duration d;
	switch(noteType){
	case OVE::Note_DoubleWhole: {
		d.setType(Duration::V_BREVE);
		break;
	}
	case OVE::Note_Whole: {
		d.setType(Duration::V_WHOLE);
		break;
	}
	case OVE::Note_Half: {
		d.setType(Duration::V_HALF);
		break;
	}
	case OVE::Note_Quarter: {
		d.setType(Duration::V_QUARTER);
		break;
	}
	case OVE::Note_Eight: {
		d.setType(Duration::V_EIGHT);
		break;
	}
	case OVE::Note_Sixteen: {
		d.setType(Duration::V_16TH);
		break;
	}
	case OVE::Note_32: {
		d.setType(Duration::V_32ND);
		break;
	}
	case OVE::Note_64: {
		d.setType(Duration::V_64TH);
		break;
	}
	case OVE::Note_128: {
		d.setType(Duration::V_128TH);
		break;
	}
	case OVE::Note_256: {
		d.setType(Duration::V_256TH);
		break;
	}
	default:
		d.setType(Duration::V_QUARTER);
		break;
	}

	return d;
}

int accidental_to_alter(OVE::AccidentalType type) {
	int alter = 0;

	switch( type ) {
	case OVE::Accidental_Normal:
	case OVE::Accidental_Natural:
	case OVE::Accidental_Natural_Caution: {
			alter = 0;
			break;
		}
	case OVE::Accidental_Sharp:
	case OVE::Accidental_Sharp_Caution: {
			alter = 1;
			break;
		}
	case OVE::Accidental_Flat:
	case OVE::Accidental_Flat_Caution: {
			alter = -1;
			break;
		}
	case OVE::Accidental_DoubleSharp:
	case OVE::Accidental_DoubleSharp_Caution: {
			alter = 2;
			break;
		}
	case OVE::Accidental_DoubleFlat:
	case OVE::Accidental_DoubleFlat_Caution: {
			alter = -2;
			break;
		}
	default:
		break;
	}

	return alter;
}

void getMiddleToneOctave(OVE::ClefType clef, OVE::ToneType& tone, int& octave) {
	tone = OVE::Tone_B;
	octave = 4;

	switch ( clef ) {
	case OVE::Clef_Treble: {
			tone = OVE::Tone_B;
			octave = 4;
			break;
		}
	case OVE::Clef_Treble8va: {
			tone = OVE::Tone_B;
			octave = 5;
			break;
		}
	case OVE::Clef_Treble8vb: {
			tone = OVE::Tone_B;
			octave = 3;
			break;
		}
	case OVE::Clef_Bass: {
			tone = OVE::Tone_D;
			octave = 3;
			break;
		}
	case OVE::Clef_Bass8va: {
			tone = OVE::Tone_D;
			octave = 4;
			break;
		}
	case OVE::Clef_Bass8vb: {
			tone = OVE::Tone_D;
			octave = 2;
			break;
		}
	case OVE::Clef_Alto: {
			tone = OVE::Tone_C;
			octave = 4;
			break;
		}
	case OVE::Clef_UpAlto: {
			tone = OVE::Tone_A;
			octave = 3;
			break;
		}
	case OVE::Clef_DownDownAlto: {
			tone = OVE::Tone_G;
			octave = 4;
			break;
		}
	case OVE::Clef_DownAlto: {
			tone = OVE::Tone_E;
			octave = 4;
			break;
		}
	case OVE::Clef_UpUpAlto: {
			tone = OVE::Tone_F;
			octave = 3;
			break;
		}
	default:
		break;
	}
}

OVE::ClefType getClefType(OVE::MeasureData* measure, int tick) {
	unsigned int i;
	OVE::ClefType type = measure->getClef()->getClefType();
	std::vector<OVE::MusicData*> clefs = measure->getMusicDatas(OVE::MusicData_Clef);

	for(i=0; i<clefs.size(); ++i){
		if(tick < clefs[i]->getTick()){
			break;
		}
		if(tick >= clefs[i]->getTick()){
			OVE::Clef* clef = static_cast<OVE::Clef*>(clefs[i]);
			type = clef->getClefType();
		}
	}

	return type;
}

void OveToMScore::convertMeasures() {
    for (MeasureBase* mb = score_->measures()->first(); mb; mb = mb->next()) {
          if (mb->type() != MEASURE)
                continue;
          Measure* measure = static_cast<Measure*>(mb);

          convertMeasure(measure);
    }
}

void OveToMScore::convertMeasure(Measure* measure){
	int staffCount = 0;
	int measureCount = ove_->getMeasureCount();

	for( int i=0; i<ove_->getPartCount(); ++i ){
		int partStaffCount = ove_->getStaffCount(i);

		for( int j=0; j<partStaffCount; ++j ){
			int measureID = measure->no();

			if (measureID >= 0 && measureID < measureCount) {
				int trackIndex = (staffCount + j) * VOICES;

				convertMeasureMisc(measure, i, j, trackIndex);
				convertNotes(measure, i, j, trackIndex);
				convertLyrics(measure, i, j, trackIndex);
				convertHarmonys(measure, i, j, trackIndex);
				convertRepeats(measure, i, j, trackIndex);
				convertSlurs(measure, i, j, trackIndex);
				convertDynamics(measure, i, j, trackIndex);
				convertExpressions(measure, i, j, trackIndex);
				convertGlissandos(measure, i, j, trackIndex);
				convertWedges(measure, i, j, trackIndex);
			}
		}

		staffCount += partStaffCount;
	}
}

void OveToMScore::convertMeasureMisc(Measure* measure, int part, int staff, int track){
	OVE::Measure* measurePtr = ove_->getMeasure(measure->no());
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	if(measurePtr == 0 || measureData == 0)
		return;

	// pickup
	if(measurePtr->getIsPickup()){
		measure->setIrregular(true);
	}

	// multiple measure rest
	if(measurePtr->getIsMultiMeasureRest()){
		measure->setBreakMultiMeasureRest(true);
	}

	// barline
	BarType bartype = NORMAL_BAR;

	switch(measurePtr->getRightBarline()) {
	case OVE::Barline_Default:{
			bartype = NORMAL_BAR;
			break;
		}
	case OVE::Barline_Double:{
			bartype = DOUBLE_BAR;
			break;
		}
	case OVE::Barline_Final:{
			bartype = END_BAR;
			break;
		}
	case OVE::Barline_Null:{
			bartype = NORMAL_BAR;
			break;
		}
	case OVE::Barline_RepeatLeft:{
			bartype = START_REPEAT;
			measure->setRepeatFlags(RepeatStart);
			break;
		}
	case OVE::Barline_RepeatRight:{
			bartype = END_REPEAT;
			measure->setRepeatFlags(RepeatEnd);
			break;
		}
	case OVE::Barline_Dashed:{
			bartype = BROKEN_BAR;
			break;
		}
	default:
		break;
	}

	if(measure->no() == ove_->getMeasureCount()-1){
		bartype = END_BAR;
	}

	measure->setEndBarLineType(bartype, false);

	if(measurePtr->getLeftBarline() == OVE::Barline_RepeatLeft){
		//bartype = START_REPEAT;
		measure->setRepeatFlags(measure->repeatFlags()|RepeatStart);
	}

	// rehearsal
	unsigned int i;
	std::vector<OVE::MusicData*> texts = measureData->getMusicDatas(OVE::MusicData_Text);
	for(i=0; i<texts.size(); ++i){
		OVE::Text* textPtr = static_cast<OVE::Text*>(texts[i]);
		if(textPtr->getTextType() == OVE::Text::Text_Rehearsal){
			Text* text = new Text(score_);
			text->setSubtype(TEXT_REHEARSAL_MARK);
			text->setTextStyle(TEXT_STYLE_REHEARSAL_MARK);
			text->setTick(mtt_->getTick(measure->no(), 0));
            text->setText(QString::fromLocal8Bit(textPtr->getText().c_str()));
            text->setAbove(true);
            text->setTrack(track);
            measure->add(text);
		}
	}

	// tempo
	std::vector<OVE::MusicData*> tempos = measureData->getMusicDatas(OVE::MusicData_Tempo);
	for(i=0; i<tempos.size(); ++i){
		OVE::Tempo* tempoPtr = static_cast<OVE::Tempo*>(tempos[i]);
		TempoText* t = new TempoText(score_);
		int absTick = mtt_->getTick(measure->no(), tempoPtr->getTick());
		double tpo = ((double)tempoPtr->getQuarterTempo())/60.0;

        AL::TempoMap* tl = score_->tempomap();
        if(tl){
        	tl->addTempo(absTick, tpo);
        }

        t->setTempo(tpo);
        t->setTick(absTick);
        t->setText(QString::fromLocal8Bit(tempoPtr->getRightText().c_str()));
        t->setAbove(true);
        t->setTrack(track);
        measure->add(t);
	}
}

// beam in grace
int getGraceLevel(const std::vector<OVE::NoteContainer*>& containers, int tick, int unit){
	int graceCount = 0;
	int level = 0; // normal chord rest

	for(unsigned int i=0; i<containers.size(); ++i){
		OVE::NoteContainer* container = containers[i];
		if(container->getTick() > tick)
			break;

		if(container->getIsGrace() && container->getTick() == tick){
			++graceCount;

			if(unit <= container->start()->getOffset()){
				++level;
			}
		}
	}

	return level;
}

int getHeadGroup(OVE::NoteHeadType type) {
    int headGroup = 0;
	switch (type) {
	case OVE::NoteHead_Standard: {
		break;
	}
	case OVE::NoteHead_Invisible: {
		break;
	}
	case OVE::NoteHead_Rhythmic_Slash: {
		headGroup = 5;
		break;
	}
	case OVE::NoteHead_Percussion: {
		headGroup = 6;
		break;
	}
	case OVE::NoteHead_Closed_Rhythm: {
		headGroup = 1;
		break;
	}
	case OVE::NoteHead_Open_Rhythm: {
		break;
	}
	case OVE::NoteHead_Closed_Slash: {
		headGroup = 5;
		break;
	}
	case OVE::NoteHead_Open_Slash: {
		headGroup = 5;
		break;
	}
	case OVE::NoteHead_Closed_Do: {
		headGroup = 3;
		break;
	}
	case OVE::NoteHead_Open_Do: {
		headGroup = 3;
		break;
	}
	case OVE::NoteHead_Closed_Re: {
		break;
	}
	case OVE::NoteHead_Open_Re: {
		break;
	}
	case OVE::NoteHead_Closed_Mi: {
		headGroup = 4;
		break;
	}
	case OVE::NoteHead_Open_Mi: {
		headGroup = 4;
		break;
	}
	case OVE::NoteHead_Closed_Fa: {
		break;
	}
	case OVE::NoteHead_Open_Fa: {
		break;
	}
	case OVE::NoteHead_Closed_Sol: {
		break;
	}
	case OVE::NoteHead_Open_Sol: {
		break;
	}
	case OVE::NoteHead_Closed_La: {
		break;
	}
	case OVE::NoteHead_Open_La: {
		break;
	}
	case OVE::NoteHead_Closed_Ti: {
		break;
	}
	case OVE::NoteHead_Open_Ti: {
		break;
	}
	default: {
		break;
	}
	}

	return headGroup;
}

bool isRestDefaultLine(OVE::Note* rest, OVE::NoteType noteType) {
	bool isDefault = true;
	switch (noteType) {
	case OVE::Note_DoubleWhole:
	case OVE::Note_Whole:
	case OVE::Note_Half:
	case OVE::Note_Quarter: {
		if(rest->getLine() != 0)
			isDefault = false;
		break;
	}
	case OVE::Note_Eight: {
		if(rest->getLine() != 1)
			isDefault = false;
		break;
	}
	case OVE::Note_Sixteen:
	case OVE::Note_32: {
		if(rest->getLine() != -1)
			isDefault = false;
		break;
	}
	case OVE::Note_64: {
		if(rest->getLine() != -3)
			isDefault = false;
		break;
	}
	case OVE::Note_128: {
		if(rest->getLine() != -4)
			isDefault = false;
		break;
	}
	default: {
		break;
	}
	}

	return isDefault;
}

void OveToMScore::convertNotes(Measure* measure, int part, int staff, int track){
	unsigned int i;
	unsigned int j;
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	std::vector<OVE::NoteContainer*> containers = measureData->getNoteContainers();
	std::vector<OVE::MusicData*> tuplets = measureData->getCrossMeasureElements(OVE::MusicData_Tuplet, OVE::MeasureData::PairType_Start);
	std::vector<OVE::MusicData*> beams = measureData->getCrossMeasureElements(OVE::MusicData_Beam, OVE::MeasureData::PairType_Start);
	Tuplet* tuplet = 0;
	ChordRest* cr = 0;
	int partStaffCount = ove_->getStaffCount(part);

	if(containers.empty()){
		Duration duration = OveNoteType_To_Duration(OVE::Note_Whole);
		int absTick = mtt_->getTick(measure->no(), 0);

		cr = new Rest(score_, absTick, duration);
		cr->setTrack(track);
		Segment* s = measure->getSegment(cr);
		s->add(cr);
	}

	for (i = 0; i < containers.size(); ++i) {
		OVE::NoteContainer* container = containers[i];
		int tick = mtt_->getTick(measure->no(), container->getTick());
		int noteTrack = track + container->getVoice();
		//int lengthTick = ContainerToTick(container, ove_->getQuarter());

		if (container->getIsRest()) {
			Duration duration = OveNoteType_To_Duration(container->getNoteType());
			duration.setDots(container->getDot());

			cr = new Rest(score_, tick, duration);
			cr->setTrack(noteTrack);
			cr->setVisible(container->getShow());

			std::vector<OVE::Note*> notes = container->getNotesRests();
			for (j = 0; j < notes.size(); ++j) {
				OVE::Note* notePtr = notes[j];
				if(!isRestDefaultLine(notePtr, container->getNoteType()) && notePtr->getLine() != 0) {
					double yOffset = -((double)notePtr->getLine()/2.0 * score_->spatium());
					cr->setUserYoffset(yOffset);
				}
			}

			Segment* s = measure->getSegment(cr);
			s->add(cr);
		} else {
			std::vector<OVE::Note*> notes = container->getNotesRests();

			cr = measure->findChord(tick, noteTrack, container->getIsGrace());
			if (cr == 0) {
				SegmentType st = SegChordRest;

				cr = new Chord(score_);
				cr->setTick(tick);
				cr->setTrack(noteTrack);

				// grace
				if (container->getIsGrace()) {
					Duration duration = OveNoteType_To_Duration(container->getGraceNoteType());
					duration.setDots(container->getDot());
					((Chord*) cr)->setNoteType(NOTE_APPOGGIATURA);

					if (duration.type() == Duration::V_QUARTER) {
						((Chord*) cr)->setNoteType(NOTE_GRACE4);
						cr->setDuration(Duration::V_QUARTER);
					} else if (duration.type() == Duration::V_16TH) {
						((Chord*) cr)->setNoteType(NOTE_GRACE16);
						cr->setDuration(Duration::V_16TH);
					} else if (duration.type() == Duration::V_32ND) {
						((Chord*) cr)->setNoteType(NOTE_GRACE32);
						cr->setDuration(Duration::V_32ND);
					} else {
						cr->setDuration(Duration::V_EIGHT);
					}

					st = SegGrace;
				} else {
					Duration duration = OveNoteType_To_Duration(container->getNoteType());
					duration.setDots(container->getDot());

					if (duration.type() == Duration::V_INVALID)
						duration.setType(Duration::V_QUARTER);
					cr->setDuration(duration);
				}

				int graceLevel = getGraceLevel(containers, container->getTick(), container->start()->getOffset());
				Segment* s = measure->getSegment(st, cr->tick(), graceLevel);
				if(s != 0) {
					s->add(cr);
				}
			}

			for (j = 0; j < notes.size(); ++j) {
				OVE::Note* notePtr = notes[j];
				Note* note = new Note(score_);

				note->setTrack(noteTrack);
				note->setPitch(notePtr->getNote());
				note->setVelocity(notePtr->getOnVelocity());
				note->setHeadGroup(getHeadGroup(notePtr->getHeadType()));
				//note->setUserAccidental(OveAccidental_to_Accidental(notePtr->getAccidental()));
				cr->setVisible(notePtr->getShow());

				// tpc
				const int OCTAVE = 7;
				OVE::ToneType clefMiddleTone;
				int clefMiddleOctave;
				OVE::ClefType clefType = getClefType(measureData, container->getTick());
				getMiddleToneOctave(clefType, clefMiddleTone, clefMiddleOctave);
				int absLine = (int) clefMiddleTone + clefMiddleOctave * OCTAVE + notePtr->getLine();
				int tone = absLine % OCTAVE;
				int alter = accidental_to_alter(notePtr->getAccidental());
				note->setTpc(step2tpc(tone, alter));

				// tie
				if ((notePtr->getTiePos() & OVE::Tie_LeftEnd) == OVE::Tie_LeftEnd) {
					Tie* tie = new Tie(score_);
					note->setTieFor(tie);
					tie->setStartNote(note);
					tie->setTrack(noteTrack);
				}

				// pitch must be set before adding note to chord as note
				// is inserted into pitch sorted list (ws)
				cr->add(note);

				((Chord*) cr)->setNoStem((int) container->getNoteType() <= OVE::Note_Whole);
				((Chord*) cr)->setStemDirection(container->getStemUp() ? UP : DOWN);

				// cross staff
				int staffMove = 0;
				if(partStaffCount == 2){/*treble-bass*/
					staffMove = notePtr->getOffsetStaff();
				}
				cr->setStaffMove(staffMove);
			}
		}

		// beam
		BeamMode bm = container->getIsRest() ? BEAM_NO : BEAM_AUTO;
		if(container->getInBeam()){
			OVE::MeasurePos pos = container->start()->shiftMeasure(0);
			OVE::MusicData* data = getCrossMeasureElementByPos(part, staff, pos, container->getVoice(), OVE::MusicData_Beam);

			if(data != 0){
				OVE::Beam* beam = static_cast<OVE::Beam*>(data);
				OVE::MeasurePos startPos = beam->start()->shiftMeasure(0);
				OVE::MeasurePos stopPos = beam->stop()->shiftMeasure(beam->start()->getMeasure());

				if(startPos == pos){
					bm = BEAM_BEGIN;
				}
				else if(stopPos == pos){
					bm = BEAM_END;
				}
				else{
					bm = BEAM_MID;
				}
			}
		}
		cr->setBeamMode(bm);

		// tuplet
		if (container->getTuplet() > 0) {
			if (tuplet == 0) {
				tuplet = new Tuplet(score_);
				tuplet->setTrack(noteTrack);
				tuplet->setRatio(Fraction(container->getTuplet(), container->getSpace()));
				tuplet->setTick(tick);
				measure->add(tuplet);
			}

			if (tuplet != 0) {
				cr->setTuplet(tuplet);
				tuplet->add(cr);
			}

			if (tuplet != 0) {
				// check tuplet end
				OVE::Tuplet* tupletPtr = getTuplet(tuplets, container->start()->getOffset());
				if (tupletPtr != 0) {
					//set direction
					tuplet->setDirection(tupletPtr->getLeftShoulder()->getYOffset() < 0 ? UP : DOWN);

					if(container->start()->getOffset() == tupletPtr->stop()->getOffset()){
						tuplet = 0;
					}
				}
			}
		}

		// articulation
		std::vector<OVE::Articulation*> articulations = container->getArticulations();
		for (j = 0; j < articulations.size(); ++j) {
			convertArticulation(measure, cr, noteTrack, tick, articulations[j]);
		}
	}
}

void OveToMScore::convertArticulation(
		Measure* measure, ChordRest* cr,
		int track, int absTick, OVE::Articulation* art){

	switch ( art->getArtType() ) {
	case OVE::Articulation_Major_Trill :
	case OVE::Articulation_Minor_Trill :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(TrillSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Trill_Section :{
		break;
	}
	case OVE::Articulation_Inverted_Short_Mordent :
	case OVE::Articulation_Inverted_Long_Mordent :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(PrallSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Short_Mordent :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(MordentSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Turn :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(TurnSym);
		cr->add(a);
		break;
	}
//	case OVE::Articulation_Flat_Accidental_For_Trill :
//	case OVE::Articulation_Sharp_Accidental_For_Trill :
//	case OVE::Articulation_Natural_Accidental_For_Trill :
	case OVE::Articulation_Tremolo_Eighth :{
		Tremolo* t = new Tremolo(score_);
		t->setSubtype(TREMOLO_1);
		cr->add(t);
		break;
	}
	case OVE::Articulation_Tremolo_Sixteenth :{
		Tremolo* t = new Tremolo(score_);
		t->setSubtype(TREMOLO_2);
		cr->add(t);
		break;
	}
	case OVE::Articulation_Tremolo_Thirty_Second :
	case OVE::Articulation_Tremolo_Sixty_Fourth :{
		Tremolo* t = new Tremolo(score_);
		t->setSubtype(TREMOLO_3);
		cr->add(t);
		break;
	}
	case OVE::Articulation_Marcato :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(SforzatoaccentSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Marcato_Dot :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(SforzatoaccentSym);
		cr->add(a);

		a = new Articulation(score_);
		a->setSubtype(StaccatoSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Heavy_Attack :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(SforzatoaccentSym);
		cr->add(a);

		a = new Articulation(score_);
		a->setSubtype(TenutoSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_SForzando :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(UmarcatoSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_SForzando_Inverted :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(DmarcatoSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_SForzando_Dot :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(UmarcatoSym);
		cr->add(a);

		a = new Articulation(score_);
		a->setSubtype(StaccatoSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_SForzando_Dot_Inverted :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(DmarcatoSym);
		cr->add(a);

		a = new Articulation(score_);
		a->setSubtype(StaccatoSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Heavier_Attack :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(UmarcatoSym);
		cr->add(a);

		a = new Articulation(score_);
		a->setSubtype(TenutoSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Staccatissimo :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(UstaccatissimoSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Staccato :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(StaccatoSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Tenuto :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(TenutoSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Pause :{
        Breath* b = new Breath(score_);
        b->setTick(absTick);
        b->setTrack(track);
        Segment* seg = measure->getSegment(SegBreath, absTick);
        seg->add(b);
		break;
	}
	case OVE::Articulation_Grand_Pause :{
		break;
	}
	case OVE::Articulation_Up_Bow :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(UpbowSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Down_Bow :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(DownbowSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Up_Bow_Inverted :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(UpbowSym);
		a->setUserYoffset(5.3);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Down_Bow_Inverted :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(DownbowSym);
		a->setUserYoffset(5.3);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Natural_Harmonic :{
		break;
	}
	case OVE::Articulation_Artificial_Harmonic :{
		break;
	}
	case OVE::Articulation_Finger_1 :
	case OVE::Articulation_Finger_2 :
	case OVE::Articulation_Finger_3 :
	case OVE::Articulation_Finger_4 :
	case OVE::Articulation_Finger_5 :{
		break;
	}
	case OVE::Articulation_Plus_Sign :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(PlusstopSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Arpeggio :{
        Arpeggio* a = new Arpeggio(score_);
        a->setSubtype(0);
        /*if (art->getPlacementAbove()){
        	a->setSubtype(1);
        }else {
        	a->setSubtype(2);
        }*/
        cr->add(a);

		break;
	}
	case OVE::Articulation_Fermata :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(UfermataSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Fermata_Inverted :{
		Articulation* a = new Articulation(score_);
		a->setSubtype(DfermataSym);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Pedal_Down :{
		if(pedal_ == NULL) {
			pedal_ = new Pedal(score_);
			pedal_->setTrack(track);
			pedal_->setTick(absTick);
		}
		break;
	}
	case OVE::Articulation_Pedal_Up :{
		if(pedal_ != NULL){
			pedal_->setTick2(absTick);
			if(pedal_->tick2() > pedal_->tick()){
				score_->add(pedal_);
			} else {
				delete pedal_;
			}
			pedal_ = NULL;
		}
		break;
	}
//	case OVE::Articulation_Toe_Pedal :
//	case OVE::Articulation_Heel_Pedal :
//	case OVE::Articulation_Toe_To_Heel_Pedal :
//	case OVE::Articulation_Heel_To_Toe_Pedal :
//	case OVE::Articulation_Open_String :
	default:
		break;
	}
}

void OveToMScore::convertLyrics(Measure* measure, int part, int staff, int track){
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	if(measureData == 0)
		return;

	std::vector<OVE::MusicData*> lyrics = measureData->getMusicDatas(OVE::MusicData_Lyric);

	for(unsigned int i=0; i<lyrics.size(); ++i){
		OVE::Lyric* lyricPtr = static_cast<OVE::Lyric*>(lyrics[i]);
		int tick = mtt_->getTick(measure->no(), lyricPtr->getTick());

		Lyrics* lyric = new Lyrics(score_);
		lyric->setNo(lyricPtr->getVerse());
		lyric->setTick(tick);
		std::string l = lyricPtr->getLyric();
		lyric->setText(QString::fromLocal8Bit(l.c_str()));
		lyric->setTrack(track);
	    Segment* segment = measure->getSegment(lyric);
	    segment->add(lyric);
	}
}

QString OveHarmony_To_String(OVE::HarmonyType type){
	static std::map<unsigned int, QString> harmony_map;

	harmony_map[OVE::Harmony_maj] = "major";
	harmony_map[OVE::Harmony_min] = "minor";
	harmony_map[OVE::Harmony_aug] = "augmented";
	harmony_map[OVE::Harmony_dim] = "diminished";
	harmony_map[OVE::Harmony_dim7] = "diminished-seventh";
	harmony_map[OVE::Harmony_sus2] = "suspended-second";
	harmony_map[OVE::Harmony_sus4] = "suspended-fourth";
	harmony_map[OVE::Harmony_sus24] = "suspended-second";
	harmony_map[OVE::Harmony_add2] = "major";
	harmony_map[OVE::Harmony_add9] = "dominant-ninth";
	//harmony_map[OVE::Harmony_omit3] = "";
	//harmony_map[OVE::Harmony_omit5] = "";
	harmony_map[OVE::Harmony_2] = "2";
	harmony_map[OVE::Harmony_5] = "power";
	harmony_map[OVE::Harmony_6] = "major-sixth";
	harmony_map[OVE::Harmony_69] = "major-sixth";
	harmony_map[OVE::Harmony_7] = "dominant";
	harmony_map[OVE::Harmony_7b5] = "dominant";
	harmony_map[OVE::Harmony_7b9] = "dominant";
	harmony_map[OVE::Harmony_7s9] = "dominant";
	harmony_map[OVE::Harmony_7s11] = "dominant";
	harmony_map[OVE::Harmony_7b5s9] = "dominant";
	harmony_map[OVE::Harmony_7b5b9] = "dominant";
	harmony_map[OVE::Harmony_7b9s9] = "dominant";
	harmony_map[OVE::Harmony_7b9s11] = "dominant";
	harmony_map[OVE::Harmony_7sus4] = "suspended-fourth";
	harmony_map[OVE::Harmony_9] = "dominant-ninth";
	harmony_map[OVE::Harmony_9b5] = "dominant-ninth";
	harmony_map[OVE::Harmony_9s11] = "dominant-ninth";
	harmony_map[OVE::Harmony_9sus4] = "dominant-ninth";
	harmony_map[OVE::Harmony_11] = "dominant-11th";
	harmony_map[OVE::Harmony_13] = "dominant-13th";
	harmony_map[OVE::Harmony_13b5] = "dominant-13th";
	harmony_map[OVE::Harmony_13b9] = "dominant-13th";
	harmony_map[OVE::Harmony_13s9] = "dominant-13th";
	harmony_map[OVE::Harmony_13s11] = "dominant-13th";
	harmony_map[OVE::Harmony_13sus4] = "dominant-13th";
	harmony_map[OVE::Harmony_min_add2] = "minor";
	harmony_map[OVE::Harmony_min_add9] = "minor";
	harmony_map[OVE::Harmony_min_maj7] = "minor-major";
	harmony_map[OVE::Harmony_min6] = "minor-sixth";
	harmony_map[OVE::Harmony_min6_add9] = "minor-sixth";
	harmony_map[OVE::Harmony_min7] = "minor-seventh";
	harmony_map[OVE::Harmony_min7b5] = "half-diminished";
	harmony_map[OVE::Harmony_min7_add4] = "minor-seventh";
	harmony_map[OVE::Harmony_min7_add11] = "minor-seventh";
	harmony_map[OVE::Harmony_min9] = "minor-ninth";
	harmony_map[OVE::Harmony_min9_b5] = "minor-ninth";
	harmony_map[OVE::Harmony_min9_maj7] = "major-minor";
	harmony_map[OVE::Harmony_min11] = "minor-11th";
	harmony_map[OVE::Harmony_min13] = "minor-13th";
	harmony_map[OVE::Harmony_maj7] = "major-seventh";
	harmony_map[OVE::Harmony_maj7_b5] = "major-seventh";
	harmony_map[OVE::Harmony_maj7_s5] = "major-seventh";
	harmony_map[OVE::Harmony_maj7_69] = "major-seventh";
	harmony_map[OVE::Harmony_maj7_add9] = "major-seventh";
	harmony_map[OVE::Harmony_maj7_s11] = "major-seventh";
	harmony_map[OVE::Harmony_maj9] = "major-ninth";
	harmony_map[OVE::Harmony_maj9_sus4] = "major-ninth";
	harmony_map[OVE::Harmony_maj9_b5] = "major-ninth";
	harmony_map[OVE::Harmony_maj9_s5] = "major-ninth";
	harmony_map[OVE::Harmony_maj9_s11] = "major-ninth";
	harmony_map[OVE::Harmony_maj13] = "major-13th";
	harmony_map[OVE::Harmony_maj13_b5] = "major-13th";
	harmony_map[OVE::Harmony_maj13_b9] = "major-13th";
	harmony_map[OVE::Harmony_maj13_b9b5] = "major-13th";
	harmony_map[OVE::Harmony_maj13_s11] = "major-13th";
	harmony_map[OVE::Harmony_aug7] = "augmented-seventh";
	harmony_map[OVE::Harmony_aug7_b9] = "augmented-seventh";
	harmony_map[OVE::Harmony_aug7_s9] = "augmented-seventh";

	return harmony_map[type];
}

void OveToMScore::convertHarmonys(Measure* measure, int part, int staff, int track){
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	if(measureData == 0)
		return;

	//int key = measureData->get_key_ptr()->getKey();
	std::vector<OVE::MusicData*> harmonys = measureData->getMusicDatas(OVE::MusicData_Harmony);

	for(unsigned int i=0; i<harmonys.size(); ++i){
		OVE::Harmony* harmonyPtr = static_cast<OVE::Harmony*>(harmonys[i]);
		int absTick = mtt_->getTick(measure->no(), harmonyPtr->getTick());

		Harmony* harmony = new Harmony(score_);

		harmony->setTick(absTick);
		harmony->setTrack(track);
		harmony->setRootTpc(pitch2tpc(harmonyPtr->getRoot()));
		if(harmonyPtr->getBass() != OVE::INVALID_NOTE && harmonyPtr->getBass() != harmonyPtr->getRoot()){
			harmony->setBaseTpc(pitch2tpc(harmonyPtr->getBass()));
		}
		const ChordDescription* d = harmony->fromXml(OveHarmony_To_String(harmonyPtr->getHarmonyType()));
		if(d != 0){
			harmony->setId(d->id);
			harmony->render();
		}

	    measure->add(harmony);
	}
}

/*OVE::MusicData* OveToMScore::getMusicDataByUnit(int part, int staff, int measure, int unit, OVE::MusicDataType type){
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure);
	if(measureData != 0) {
		const std::vector<OVE::MusicData*>& datas = measureData->getMusicDatas(type);
		for(unsigned int i=0; i<datas.size(); ++i){
			if(datas[i]->getTick() == unit){//different measurement
				return datas[i];
			}
		}
	}

	return 0;
}*/

OVE::MusicData* OveToMScore::getCrossMeasureElementByPos(int part, int staff, const OVE::MeasurePos& pos, int voice, OVE::MusicDataType type){
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, pos.getMeasure());
	if(measureData != 0) {
		const std::vector<OVE::MusicData*>& datas = measureData->getCrossMeasureElements(type, OVE::MeasureData::PairType_All);
		for(unsigned int i=0; i<datas.size(); ++i){
			OVE::MeasurePos dataStart = datas[i]->start()->shiftMeasure(0);
			OVE::MeasurePos dataStop = datas[i]->stop()->shiftMeasure(datas[i]->start()->getMeasure());

			if(dataStart <= pos && dataStop >= pos && datas[i]->getVoice() == voice){
				return datas[i];
			}
		}
	}

	return 0;
}

OVE::NoteContainer* OveToMScore::getContainerByPos(int part, int staff, const OVE::MeasurePos& pos){
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, pos.getMeasure());
	if(measureData != 0) {
		const std::vector<OVE::NoteContainer*>& containers = measureData->getNoteContainers();
		for(unsigned int i=0; i<containers.size(); ++i){
			if(pos == containers[i]->start()->shiftMeasure(0)){
				return containers[i];
			}
		}
	}

	return 0;
}

void OveToMScore::convertRepeats(Measure* measure, int part, int staff, int track){
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	if(measureData == 0)
		return;

	unsigned int i;
	std::vector<OVE::MusicData*> repeats = measureData->getMusicDatas(OVE::MusicData_Repeat);

	for(i=0; i<repeats.size(); ++i){
		OVE::RepeatSymbol* repeatPtr = static_cast<OVE::RepeatSymbol*>(repeats[i]);
		OVE::RepeatType type = repeatPtr->getRepeatType();
		Element* e = 0;

		switch(type) {
		case OVE::Repeat_Segno:{
			Marker* marker = new Marker(score_);
		    marker->setMarkerType(MARKER_SEGNO);
		    e = marker;
			break;
		}
		case OVE::Repeat_Coda:{
			Marker* marker = new Marker(score_);
		    marker->setMarkerType(MARKER_CODA);
		    e = marker;
			break;
		}
		case OVE::Repeat_DSAlCoda:{
            Jump* jp = new Jump(score_);
            jp->setJumpType(JUMP_DS_AL_CODA);
            e = jp;
			break;
		}
		case OVE::Repeat_DSAlFine:{
            Jump* jp = new Jump(score_);
            jp->setJumpType(JUMP_DS_AL_FINE);
            e = jp;
			break;
		}
		case OVE::Repeat_DCAlCoda:{
            Jump* jp = new Jump(score_);
            jp->setJumpType(JUMP_DC_AL_CODA);
            e = jp;
			break;
		}
		case OVE::Repeat_DCAlFine:{
            Jump* jp = new Jump(score_);
            jp->setJumpType(JUMP_DC_AL_FINE);
            e = jp;
			break;
		}
		case OVE::Repeat_ToCoda:{
			Marker* m = new Marker(score_);
			m->setMarkerType(MARKER_TOCODA);
			e = m;
			break;
		}
		case OVE::Repeat_Fine:{
			Marker* m = new Marker(score_);
			m->setMarkerType(MARKER_FINE);
			e = m;
			break;
		}
		default:
			break;
		}

		if(e != 0){
			e->setTrack(track);
			measure->add(e);
		}
	}

	std::vector<OVE::MusicData*> endings = measureData->getCrossMeasureElements(
															OVE::MusicData_Numeric_Ending,
															OVE::MeasureData::PairType_Start);

	for(i=0; i<endings.size(); ++i){
		OVE::NumericEnding* ending = static_cast<OVE::NumericEnding*>(endings[i]);
		int absTick1 = mtt_->getTick(measure->no(), 0);
		int absTick2 = mtt_->getTick(measure->no() + ending->stop()->getMeasure(), 0);

        Volta* volta = new Volta(score_);
        volta->setTrack(track);
        volta->setTick(absTick1);
        volta->setTick2(absTick2);
        volta->setSubtype(Volta::VOLTA_CLOSED);
        volta->setText(QString::fromLocal8Bit(ending->getText().c_str()));

        volta->endings().clear();
        std::vector<int> numbers = ending->getNumbers();
        for(unsigned int j=0; j<numbers.size(); ++j){
        	volta->endings().append(numbers[j]);
        }

		if(volta->tick2() > volta->tick()){
			score_->add(volta);
		} else {
			delete volta;
		}
	}
}

void OveToMScore::convertSlurs(Measure* measure, int part, int staff, int track){
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	if(measureData == 0)
		return;

	std::vector<OVE::MusicData*> slurs = measureData->getCrossMeasureElements(OVE::MusicData_Slur, OVE::MeasureData::PairType_Start);

	for(unsigned int i=0; i<slurs.size(); ++i){
		OVE::Slur* slurPtr = static_cast<OVE::Slur*>(slurs[i]);

		OVE::NoteContainer* startContainer = getContainerByPos(part, staff, slurPtr->start()->shiftMeasure(0));
		OVE::NoteContainer* endContainer = getContainerByPos(
													part,
													staff,
													slurPtr->stop()->shiftMeasure(slurPtr->start()->getMeasure()));

		if(startContainer != 0 && endContainer != 0){
			int absStartTick = mtt_->getTick(slurPtr->start()->getMeasure(), startContainer->getTick());
			int absEndTick = mtt_->getTick(slurPtr->start()->getMeasure()+slurPtr->stop()->getMeasure(), endContainer->getTick());

	        Slur* slur = new Slur(score_);
	        slur->setSlurDirection(slurPtr->getShowOnTop()? UP : DOWN);
	        slur->setStart(absStartTick, track);
	        slur->setEnd(absEndTick, track);
	        slur->setTrack(track);
	        score_->add(slur);
		}
	}
}

QString OveDynamics_To_Dynamics(OVE::DynamicsType type){
	QString dynamic = "other-dynamics";

	switch( type ){
	case OVE::Dynamics_pppp:{
			dynamic = "pppp";
			break;
		}
	case OVE::Dynamics_ppp:{
			dynamic = "ppp";
			break;
		}
	case OVE::Dynamics_pp:{
			dynamic = "pp";
			break;
		}
	case OVE::Dynamics_p:{
			dynamic = "p";
			break;
		}
	case OVE::Dynamics_mp:{
			dynamic = "mp";
			break;
		}
	case OVE::Dynamics_mf:{
			dynamic = "mf";
			break;
		}
	case OVE::Dynamics_f:{
			dynamic = "f";
			break;
		}
	case OVE::Dynamics_ff:{
			dynamic = "ff";
			break;
		}
	case OVE::Dynamics_fff:{
			dynamic = "fff";
			break;
		}
	case OVE::Dynamics_ffff:{
			dynamic = "ffff";
			break;
		}
	case OVE::Dynamics_sf:{
			dynamic = "sf";
			break;
		}
	case OVE::Dynamics_fz:{
			dynamic = "fz";
			break;
		}
	case OVE::Dynamics_sfz:{
			dynamic = "sfz";
			break;
		}
	case OVE::Dynamics_sffz:{
			dynamic = "sffz";
			break;
		}
	case OVE::Dynamics_fp:{
			dynamic = "fp";
			break;
		}
	case OVE::Dynamics_sfp:{
			dynamic = "sfp";
			break;
		}
	default:
	    break;
	}

	return dynamic;
}

void OveToMScore::convertDynamics(Measure* measure, int part, int staff, int track){
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	if(measureData == 0)
		return;

	std::vector<OVE::MusicData*> dynamics = measureData->getMusicDatas(OVE::MusicData_Dynamics);

	for(unsigned int i=0; i<dynamics.size(); ++i){
		OVE::Dynamics* dynamicPtr = static_cast<OVE::Dynamics*>(dynamics[i]);
		int absTick = mtt_->getTick(measure->no(), dynamicPtr->getTick());
		Dynamic* dynamic = new Dynamic(score_);

		dynamic->setSubtype(OveDynamics_To_Dynamics(dynamicPtr->getDynamicsType()));
		dynamic->setTrack(track);
		dynamic->setTick(absTick);

        measure->add(dynamic);
	}
}

void OveToMScore::convertExpressions(Measure* measure, int part, int staff, int track){
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	if(measureData == 0)
		return;

	std::vector<OVE::MusicData*> expressions = measureData->getMusicDatas(OVE::MusicData_Expressions);

	for(unsigned int i=0; i<expressions.size(); ++i){
		OVE::Expressions* expressionPtr = static_cast<OVE::Expressions*>(expressions[i]);
		int absTick = mtt_->getTick(measure->no(), expressionPtr->getTick());
		Text* t = new Text(score_);

		t->setTextStyle(TEXT_STYLE_TECHNIK);

		t->setText(QString::fromLocal8Bit(expressionPtr->getText().c_str()));
		t->setTrack(track);
		t->setTick(absTick);

        measure->add(t);
	}
}

void OveToMScore::convertGlissandos(Measure* measure, int part, int staff, int track){
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	if(measureData == 0)
		return;

	std::vector<OVE::MusicData*> glissandos = measureData->getCrossMeasureElements(OVE::MusicData_Glissando, OVE::MeasureData::PairType_All);

	for(unsigned int i=0; i<glissandos.size(); ++i){
		OVE::Glissando* glissandoPtr = static_cast<OVE::Glissando*>(glissandos[i]);
		OVE::NoteContainer* startContainer = getContainerByPos(part, staff, glissandoPtr->start()->shiftMeasure(0));
		OVE::NoteContainer* endContainer = getContainerByPos(
													part,
													staff,
													glissandoPtr->stop()->shiftMeasure(glissandoPtr->start()->getMeasure()));

		if(startContainer != 0 && endContainer != 0){
			int absTick = mtt_->getTick(measure->no(), glissandoPtr->getTick());
			ChordRest* cr = measure->findChordRest(absTick, track);
			if(cr != 0){
		        Glissando* g = new Glissando(score_);
		        g->setSubtype(1);
		        cr->add(g);
			}
		}
	}
}

int OveWedgeType_To_Type(OVE::WedgeType type) {
	int subtype = 0;
	switch(type) {
	case OVE::Wedge_Cres_Line: {
		subtype = 0;
		break;
	}
	case OVE::Wedge_Double_Line: {
		subtype = 0;
		break;
	}
	case OVE::Wedge_Decresc_Line: {
		subtype = 1;
		break;
	}
	case OVE::Wedge_Cres: {
		subtype = 0;
		break;
	}
	case OVE::Wedge_Decresc: {
		subtype = 1;
		break;
	}
	default:
		break;
	}

	return subtype;
}

void OveToMScore::convertWedges(Measure* measure, int part, int staff, int track) {
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	if(measureData == 0)
		return;

	std::vector<OVE::MusicData*> wedges = measureData->getCrossMeasureElements(OVE::MusicData_Wedge, OVE::MeasureData::PairType_All);

	for(unsigned int i=0; i<wedges.size(); ++i){
		OVE::Wedge* wedgePtr = static_cast<OVE::Wedge*>(wedges[i]);
		int absTick = mtt_->getTick(
							measure->no(),
							MeasureToTick::unitToTick(wedgePtr->start()->getOffset(), ove_->getQuarter()));
		int absTick2 = mtt_->getTick(
							measure->no()+wedgePtr->stop()->getMeasure(),
							MeasureToTick::unitToTick(wedgePtr->stop()->getOffset(), ove_->getQuarter()));

		if(absTick2 > absTick) {
			Hairpin* hp = new Hairpin(score_);

			hp->setTick(absTick);
			hp->setTick2(absTick2);
			hp->setSubtype(OveWedgeType_To_Type(wedgePtr->getWedgeType()));
			//hp->setYoff(wedgePtr->getYOffset());
			hp->setTrack(track);

			if(hp->tick2() > hp->tick()){
				score_->add(hp);
			} else {
				delete hp;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool Score::importOve(const QString& name) {
	OVE::IOVEStreamLoader* oveLoader = OVE::createOveStreamLoader();
	OVE::OveSong oveSong;

	QFile oveFile(name);

	if (!oveFile.open(QFile::ReadOnly)) {
		//messageOutString(QString("can't read file!"));
		return false;
	}

	QByteArray buffer = oveFile.readAll();

	oveFile.close();

	oveLoader->setOve(&oveSong);
	oveLoader->setFileStream((unsigned char*) buffer.data(), buffer.size());
	//oveLoader->setNotify(oveListener_);
	bool result = oveLoader->load();
	oveLoader->release();

	if(result){
		OveToMScore otm;
		otm.convert(&oveSong, this);

		connectTies();
		connectSlurs();
	}

	return result;
}