import QtQuick 1.0

Item {
      id: toolbar

      property alias button1Label: button1.text
      property alias button2Label: button2.text
      property alias button1Visible: button1.visible
      property alias button2Visible: button2.visible

      signal button1Clicked
      signal button2Clicked

      BorderImage {
            source: "images/titlebar.sci"
            width: parent.width; height: parent.height + 14; y: -7
            }

      Row {
            anchors.right: parent.right; anchors.rightMargin: 5
            y: 3; height: 32; spacing: 30
            Button {
                  id: button1
                  width: 140; height: 32
                  onClicked: toolbar.button1Clicked()
                  }

            Button {
                  id: button2
                  width: 140; height: 32
                  onClicked: toolbar.button2Clicked()
                  }
            }
      }
