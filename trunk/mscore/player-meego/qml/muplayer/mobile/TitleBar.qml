import QtQuick 1.0

Item {
      id: titleBar

      signal search(string text);

      BorderImage {
            source: "images/titlebar.sci"
            width: parent.width
            height: parent.height + 14
            y: -7
            }

      Text {
            id: categoryText
            anchors {
                  left: parent.left
                  right: parent.right
                  verticalCenter: parent.verticalCenter
                  }
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideLeft
            text: "MyScores"
            font.bold: true
            font.pixelSize: 15
            color: "White"
            style: Text.Raised
            styleColor: "Black"
            }

      SearchBox {
            id: searchBox
            state: 'Show'
            height: parent.height - 10
            focus: parent.focus
            anchors.right: parent.right
            anchors.rightMargin: 5
            //anchors.left: parent.left
            //anchors.leftMargin: 5
            anchors.verticalCenter: parent.verticalCenter

            onReturnPressed: {
                  if (searchBox.text !== '') {
                        titleBar.search(searchBox.text);
                  }
            }

            states: [
                  State {
                        name: "Hide"
                        PropertyChanges {
                              target: searchBox
                              opacity: 0.0
                        }
                  },
                  State {
                        name: "Show"
                        PropertyChanges {
                              target: searchBox
                              opacity: 1.0
                        }
                  }
            ]
            }
      }

