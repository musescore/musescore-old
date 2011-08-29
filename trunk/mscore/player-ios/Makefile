
all: mscoreplay.cpp moc_scoreview.cpp moc_qmlapplicationviewer.cpp moc_runtime.cpp

mscoreplay.cpp: mscoreplay.qrc ../player-qt/mplayer.qml
	rcc mscoreplay.qrc -o mscoreplay.cpp

moc_scoreview.cpp: ../player-qt/scoreview.h
	moc ../player-qt/scoreview.h -o moc_scoreview.cpp

moc_qmlapplicationviewer.cpp: qmlapplicationviewer.h
	moc qmlapplicationviewer.h -o moc_qmlapplicationviewer.cpp

moc_runtime.cpp: ../player-qt/runtime.h
	moc ../player-qt/runtime.h -o moc_runtime.cpp

clean:
	rm  mscoreplay.cpp moc_*.cpp



