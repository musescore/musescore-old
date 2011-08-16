
all: mscoreplay.cpp moc_scoreview.cpp moc_qmlapplicationviewer.cpp

mscoreplay.cpp: mscoreplay.qrc
	rcc mscoreplay.qrc -o mscoreplay.cpp

moc_scoreview.cpp: ../player-qt/scoreview.h
	moc ../player-qt/scoreview.h -o moc_scoreview.cpp

moc_qmlapplicationviewer.cpp: qmlapplicationviewer.h
	moc qmlapplicationviewer.h -o moc_qmlapplicationviewer.cpp

clean:
	rm moc_qmlapplicationviewer.cpp moc_scoreview.cpp mscoreplay.cpp



