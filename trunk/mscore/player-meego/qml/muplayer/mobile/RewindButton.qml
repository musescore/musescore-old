import QtQuick 1.0

Item {
      id: container

      signal clicked

      width: 64; height: 64
      Image {
            id: buttonImage
            anchors.fill: parent
            source: "images/rewindbuttonOff.png"
            width: 50
            height: 50
            }
      MouseArea {
            id: mouseRegion
            anchors.fill: parent
            onClicked:    container.clicked()
            }
      states: [
            State {
                  name: "Pressed"
                  when: mouseRegion.pressed === true
                  PropertyChanges {
                        target: buttonImage
                        opacity: 1
                        }
                  PropertyChanges {
                        target: buttonImage
                        source: "images/rewindbuttonOn.png"
                        }
                  }
            ]
      }

