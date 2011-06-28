//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

import QtQuick 1.0
import MuseScore 1.0
import Qt.labs.folderlistmodel 1.0

Rectangle {
      id: player
      x: 0; y: 0
      width: 1024; height: 768
      state: "myscores"
      border.width: 1

      states: [
            State {
                  name: "normal"
                  PropertyChanges { target: toolbar; opacity: 0 }
                  PropertyChanges { target: scores; width: 0 }
                  },
            State {
                  name: "toolbar1"
                  PropertyChanges { target: toolbar; opacity: 1 }
                  PropertyChanges { target: scores;  width: 0 }
                  },
            State {
                  name: "myscores"
                  PropertyChanges { target: toolbar; opacity: 1 }
                  PropertyChanges { target: scores;  width: 250 }
                  }
            ]
      transitions: Transition {
            PropertyAnimation {
                  property: "width"
                  easing.type: Easing.Linear
                  duration: 500
                  }
            PropertyAnimation {
                  property: "opacity"
                  easing.type: Easing.Linear
                  duration: 500
                  }
            }

      ListModel {
            id: scorelist
            ListElement {
                  type: "Promenade"
                  path: ":/scores/promenade.mscz"
                  }
            ListElement {
                  type: "Leise rieselt der Schnee"
                  path: ":/scores/schnee.mscz"
                  }
            ListElement {
                  type: "Italienisches Konzert"
                  path: ":/scores/italian-1.mscz"
                  }
            }

      Component {
            id: scorelistdelegate
            Text {
                  id: label
                  font.pixelSize: 18
                  text: type
                  }
            }

      ListView {
            id: scores
            anchors.left: player.left
            anchors.top: player.top
            anchors.bottom: player.bottom
            anchors.margins: 10
            clip: true

            model: scorelist
            delegate: scorelistdelegate
            header: myScoresBanner
            footer: Rectangle {
                  width: parent.width; height: 30
                  gradient: scorecolors
                  }
            highlight: Rectangle {
                  width: parent.width
                  color: "lightgray"
                  }
            MouseArea {
                  anchors.fill: parent
                  onClicked: {
                        var idx = scores.indexAt(mouseX, mouseY)
                        if (idx >= 0) {
                              scores.currentIndex = idx
                              console.log(idx)
                              console.log(scorelist.get(idx).type)
                              scoreview.setScore(scorelist.get(idx).path)
                              }
                        }
                  }
            }
      Component {
            id: myScoresBanner
            Rectangle {
                  id: banner
                  width: parent.width; height: 50
                  gradient: scorecolors
                  border { color: "#9eddf2"; width: 2 }
                  Text {
                        anchors.centerIn: parent
                        text: "My Scores"
                        font.pixelSize: 32
                        }
                  }
            }
      Gradient {
            id: scorecolors
            GradientStop { position: 0.0; color:  "#8ee2fe" }
            GradientStop { position: 0.66; color: "#7ed2ee" }
            }

      ScoreView {
            id: scoreview
            anchors.left: scores.right
            anchors.right: player.right
            anchors.top: player.top
            anchors.bottom: player.bottom

            MouseArea {
                  state: "normal"
                  states: [
                        State { name: "normal" },
                        State { name: "pressed" },
                        State { name: "drag" }
                        ]
                  anchors.fill: parent
                  onPositionChanged: {
                        parent.drag(mouseX, mouseY)
                        state = "drag"
                        }
                  onPressed:         {
                        state = "pressed"
                        parent.startDrag(mouseX, mouseY)
                        }
                  onReleased: {
                        if (state == "pressed") {
                              if (player.state == "normal")
                                    player.state = "toolbar1"
                              else
                                    player.state = "normal"
                              }
                        state = "normal";
                        }
                  }
            }

      Rectangle {
            id: toolbar
            x: 0
            z: 1
            height: 25
            color: "lightblue"
            anchors.bottom: player.bottom
            anchors.left:   player.left
            anchors.right:  player.right

            Rectangle {
                  id: myScoreButton
                  x:      10
                  width:  buttontext.implicitWidth+8
                  border.width: 1
                  smooth: true
                  height: 20
                  radius: 4
                  anchors.verticalCenter: parent.verticalCenter
                  color: "yellow"
                  Text {
                        id: buttontext
                        anchors.centerIn: parent
                        text: "MyScores"
                        }
                  MouseArea {
                        anchors.fill: parent
                        onClicked: {
                              if (player.state == "toolbar1")
                                    player.state = "myscores"
                              else
                                    player.state = "toolbar1"
                              }
                        }
                  }
            }
      }

