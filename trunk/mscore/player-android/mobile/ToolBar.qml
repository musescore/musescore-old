import QtQuick 1.1

Item {
      id: toolbar

      property alias button1Label: button1.text
      property alias tempo: tempo.value

      signal button1Clicked
      signal button2Clicked
      signal button3Clicked
      signal tempoChanged

      BorderImage {
            source: "images/titlebar.sci"
            width: parent.width; height: parent.height + 14; y: -7
            }

      Button {
            id: button1
            width: 140
            height: parent.height
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            onClicked: toolbar.button1Clicked()
            }
      Slider {
            id: tempo
            anchors.left:  button1.right
            anchors.right: button3.left
            anchors.leftMargin: 25
            anchors.rightMargin: 25
            anchors.verticalCenter: parent.verticalCenter
            minimum: 0.3
            maximum: 2.0
            value:   1.0
            onValueChanged: toolbar.tempoChanged()
            }
      RewindButton {
            id: button3
            anchors.right: button2.left
            anchors.rightMargin: 25
            anchors.verticalCenter: parent.verticalCenter
            onClicked: toolbar.button3Clicked()
            }
      PlayButton {
            id: button2
            anchors.right: parent.right
            anchors.rightMargin: 25
            anchors.verticalCenter: parent.verticalCenter
            onClicked: toolbar.button2Clicked()
            }
      }

