import QtQuick 1.1

Item {
      id: container

      signal clicked

      property string text

      BorderImage {
            id: buttonImage
            source: "images/toolbutton.sci"
            width: container.width;
            height: container.height
            }
      BorderImage {
            id: pressed
            opacity: 0
            source: "images/toolbutton.sci"
            width: container.width;
            height: container.height
            }
      MouseArea {
            id: mouseRegion
            anchors.fill: buttonImage
            onClicked:    container.clicked()
            preventStealing: true
            }
      Text {
            id: text
            color: "white"
            anchors.centerIn: buttonImage;
            font.bold: true;
            font.pixelSize: 15
            text: container.text;
            style: Text.Raised;
            styleColor: "black"
            }
      states: [
            State {
                  name: "Pressed"
                  when: mouseRegion.containsMouse == true
                  PropertyChanges {
                        target: pressed
                        opacity: 1
                        }
                  PropertyChanges {
                        target: text
                        color: "yellow"
                        }
                  }
            ]
      }

