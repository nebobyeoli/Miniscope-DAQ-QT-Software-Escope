import QtQuick //2.12
import QtQuick.Window // 2.12
import QtQuick.Controls //2.15//2.12
import QtQuick.Layouts 1.3
import QtQuick.Dialogs //import QtQuick.Dialogs 1.2
import QtQuick.Controls.Basic
import Qt.labs.platform 1.0

import Qt.labs.qmlmodels


// import "TreeViewerJson.qml" // "no such directory"


Window {
    id: root
    objectName: "windowMain"
    visible: true
    width: 720
    height: 720
    color: "#afafaf"
    title: qsTr("Miniscope DAQ")


    FileDialog {
        // Used to select user config file

        id: fileDialog
        title: "Please choose a user configuration file."
        property string configPath: StandardPaths.writableLocation(StandardPaths.ConfigLocation) + "/AppName"
        folder: StandardPaths.writableLocation(StandardPaths.HomeLocation)
        nameFilters: [ "JSON files (*.json)", "All files (*)" ]
        onAccepted: {
            // Send file name to c++ backend

            let file = fileDialog.file; // `file` 속성 사용
            console.log("Selected file (file): ", file);

            if (!file || file === "") {
                console.error("No file selected.");
                return;
            }

            backend.userConfigFileName = file;
            treeView.visible = true;
            view.visible = false;
        }

        onRejected: {
            console.log("Canceled");
        }
        visible: false
    }






    Window {
        id: helpDialog
        width: 600
        height: 200
        visible: false
        title: "Miniscope DAQ Help"
        ColumnLayout {
            anchors.fill: parent

            TextArea {
                text: "Miniscope DAQ Software version " + backend.versionNumber + "<br/>" +
                      "Your OpenGL verions: " + GraphicsInfo.majorVersion + "." + GraphicsInfo.minorVersion + "<br/>" +
                      "Developed by the <a href='https://aharoni-lab.github.io/'>Aharoni Lab</a>, UCLA <br/> " +
                      "Overview of the UCLA Miniscope project: <a href='http://www.miniscope.org'>click here</a> <br/>" +
                      "Miniscope Wiki for newest projects: <a href='https://github.com/Aharoni-Lab/Miniscope-v4/wiki'>click here</a> <br/>" +
                      "Miniscope Discussion Board: <a href='https://groups.google.com/d/forum/miniscope'>click here</a> <br/>" +
                      "Please submit issues, comments, suggestions to the Miniscope DAQ Software Github Repository: <a href='https://github.com/Aharoni-Lab/Miniscope-DAQ-QT-Software'>click here</a> <br/>" +
                      "Miniscope Twitter Link: <a href='https://twitter.com/MiniscopeTeam'>click here</a> <br/> <br/>" +
                      "Icons from <a href='https://icons8.com/'>icon8</a>"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                Layout.margins: 5
                Layout.fillWidth: true
                textFormat: Text.RichText
                onLinkActivated: Qt.openUrlExternally(link)
                font.pointSize: 12
                font.family: "Arial"
                wrapMode: Text.WordWrap
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }
            Button {
                text: "Close"
                font.family: "Arial"
                font.pointSize: 12
                font.bold: true
                font.weight: Font.Normal
                background: Rectangle {
                    id: closeRect
                    border.width: 1
                    color: "#a8a7fd"
                }
                Layout.margins: 5
                Layout.fillWidth: true
                onClicked: helpDialog.visible = false
                onHoveredChanged: hovered ? closeRect.color = "#f8a7fd" : closeRect.color = "#a8a7fd"
            }
        }
    }

    MessageDialog {
        id: errorMessageDialog
        title: "User Config File Error"
        text: "The selected user configuration file contains device name repeats. Please edit the file the so each 'deviceName' entry is unique."
        onAccepted: {
            visible = false
        }
        visible: false
    }

    MessageDialog {
        id: errorMessageDialogCompression
        title: "User Config File Error"
        text: "The selected user configuration file contains video compression(s) that are not supported by your computer. Please edit the file so each 'compression' entry is a supported option from the following list: " + backend.availableCodecList
        onAccepted: {
            visible = false
        }
        visible: false
    }

    MessageDialog {
        id: saveMessageDialog
        property string fName: backend.userConfigFileName
        title: "User Config File Saved"
        text:  "The user config file has been saved to " + fName.replace(".json", "_new.json")
        onAccepted: {
            visible = false
        }
        visible: false
    }


    ColumnLayout {
        id: columnLayout
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        spacing: 10
        anchors.fill: parent

        RowLayout {
            id: rowLayoutTop
            height: 40
            Layout.minimumHeight: 40
            Layout.preferredHeight: 40
            Layout.fillHeight: false
            Layout.fillWidth: true
            spacing: 10


            RoundButton {
                id: rbSelectUserConfig
                height: 40
                text: "Select Config File"
                Layout.minimumHeight: 40
                Layout.preferredHeight: 40
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.fillWidth: true
                font.family: "Arial"
                font.pointSize: 18
                font.bold: true
                font.weight: Font.Normal
                radius: 10

                Layout.minimumWidth: 20
                Layout.preferredWidth: 400
                Layout.maximumWidth: 600

                background: Rectangle {
                    id: configRect
                    radius: rbSelectUserConfig.radius
                    border.width: 1
                    color: "#a8a7fd"
                }
                //onClicked: fileDialog.setVisible(true) //1->true
                onClicked: fileDialog.open()
                onHoveredChanged: hovered ? configRect.color = "#f8a7fd" : configRect.color = "#a8a7fd"

            }

            RoundButton {
                id: rbSaveUserConfig
                height: 40
                text: "Save User Config File"
                Layout.minimumHeight: 40
                Layout.preferredHeight: 40
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.fillWidth: true
                font.family: "Arial"
                font.pointSize: 18
                font.bold: true
                font.weight: Font.Normal
                radius: 10
                enabled: backend.userConfigOK

                Layout.minimumWidth: 100
                Layout.preferredWidth: 200
                Layout.maximumWidth: 700

                background: Rectangle {
                    id: configSaveRect
                    radius: rbSaveUserConfig.radius
                    border.width: 1
                    color: "#a8a7fd"
                }
                onClicked: {

                    backend.saveConfigObject()
                    saveMessageDialog.visible = true
                }
                onHoveredChanged: hovered ? configSaveRect.color = "#f8a7fd" : configSaveRect.color = "#a8a7fd"

            }
        }
        
        // TableView {
        //     visible: true
        //     model: TableModel {
        //         TableModelColumn {
        //             display: "key"
        //         }
        //         TableModelColumn {
        //             display: "value"
        //         }
        //         TableModelColumn {
        //             display: "type"
        //         }
        //     }
        //     Component.onCompleted: {
        //         console.log("tablemodel loaded")
        //         console.log(treeView.visible)
        //     }
        //     // visible: treeView.visible
        //     // TextArea { font.pointSize: 12 }
        // }

        // TableView {
        //     anchors.fill: parent
        //     // Layout.alignment: Qt.AlignLeft
            
        //     visible: treeView.visible  // enabled과의 차이=?
        //     columnSpacing: 0
        //     rowSpacing: 0
        //     clip: true
        //     model: TableModel {
        //         TableModelColumn { display: "key" }
        //         TableModelColumn { display: "value" }
        //         TableModelColumn { display: "type" }

        //         rows: [{"key": "1", "value":"2", "type":"3"}]
        //     }

        //     delegate: Rectangle {
        //         implicitWidth: 100
        //         implicitHeight: 50
        //         border.width: 1; color: "lightgray"
        //         Text {
        //             text: display
        //             anchors.centerIn: parent
        //         }
        //     }
        // }

        ColumnLayout {

            /* 헤더 (Key Value Type) 표시 */

            Row {
                // Column { Rectangle { border.width: 1; width: headerRow.roleW; height: headerRow.roleH; color: "lightgray";  Text { text: "key"; anchors.fill: parent; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter } } }
                // Column { Rectangle { border.width: 1; width: headerRow.roleW; height: headerRow.roleH; color: "lightgray";  Text { text: "value"; anchors.fill: parent; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter } } }
                // Column { Rectangle { border.width: 1; width: headerRow.roleW; height: headerRow.roleH; color: "lightgray";  Text { text: "type"; anchors.fill: parent; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter } } }

                id: headerRow
                // spacing: 0
                // Layout.fillHeight: false; Layout.fillWidth: true

                Layout.preferredHeight: 24  // 그냥 height:는 무시됨
                visible: treeView.visible  // enabled과의 차이=?

                readonly property int roleH:  30;  // row 높이
                readonly property int roleW:  (Window.width - treeView.labelRectW - 10*2) / 3;

                Layout.alignment: Qt.AlignLeft
                // Layout.alignment: parent
                // anchors.fill: parent


                z: 1  /* headerRow가 항상 맨 위에 그려지게 함 */

                TextField {
                    // anchors.fill: parent // do not use anchors for child items directly inside Row/RowLayout
                    color: "black"
                    Layout.fillHeight: false; Layout.fillWidth: false
                    height: headerRow.roleH; width: headerRow.roleW + treeView.labelRectW
                    enabled: false
                    text: "Key"
                }

                TextField {
                    color: "black"
                    Layout.fillHeight: false; Layout.fillWidth: false
                    height: headerRow.roleH; width: headerRow.roleW
                    enabled: false
                    text: "Value"
                }

                TextField {
                    color: "black"
                    Layout.fillHeight: false; Layout.fillWidth: false
                    height: headerRow.roleH; width: headerRow.roleW
                    enabled: false
                    text: "Type"
                }
            }

            /* 각 항목들 (TreeView Items) 표시 */

            RowLayout {
                TreeView {
                    id: treeView
                    objectName: "treeView"
                    model: backend.jsonTreeModel
                    
                    visible: false
                    // Layout.rowSpan: 4  // = ??
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    implicitHeight: 100
                    // columnWidthProvider: (column) => column === 0 ? 200 : 100  // no effect?
                    Layout.alignment: Qt.AlignLeft // Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                    property string toolTipText: ""

                    readonly property int itemH:  30;  // row 높이
                    readonly property int itemW:  headerRow.roleW;

                    readonly property int labelRectH:  itemH - 2;
                    readonly property int labelRectW:  12;

                    delegate: Item {
                        id: treeItem
                        implicitHeight: treeView.itemH  // row 높이 간격
                        width: parent.width // width: console.log("parent.width: " + parent.width)?parent.width:parent.width //했더니 50,50,...,50,400,400,400 출력됨 // qml에서 console로 출력하는 방법

                        readonly property real indentation: 5
                        readonly property real padding: 100

                        // Assigned to by TreeView:
                        required property TreeView treeView
                        required property bool isTreeNode
                        required property bool expanded
                        required property int hasChildren
                        required property int depth
                        required property int row
                        required property int column
                        required property bool current
                        
                        // qml에서 출력하는 법: Component.onCompleted: console.log("text here") 사용
                        Component.onCompleted: {
                            console.log("\n\n")
                            console.log(
                            "treeView: "+treeView  // QQuickTreeView(0x1ff1be772c0, "treeView")
                            +"\nisTreeNode: "+isTreeNode
                            +"\nexpanded: "+expanded
                            +"\nhasChildren: "+hasChildren
                            +"\ndepth: "+depth  // 다 0
                            +"\n-model.key: "+model.key
                            +"\n-model.value: "+model.value
                            +"\n-model.type: "+model.type)
                            // +"\n-model.parent.type: "+model.parent // undefined
                            // +"\n")
                            // +"\n\n")
                            // console.log(model.type==undefined)
                            // +"\nmodel.type==QString: "+(model.type==QString)+"\n\n")
                            // console.log("model.value: ")
                            // console.log(QType(model.value))
                        }
                        
                        Row {
                            Layout.alignment: Qt.AlignLeft
                            anchors.fill: parent

                            Component.onCompleted: console.log("padding: "+treeItem.padding+", indentation: "+treeItem.indentation);
                            Rectangle {
                                height: treeView.labelRectH; width: (depth + 1)*treeView.labelRectW
                                color: "transparent" // color: "yellow"
                                anchors.verticalCenter: parent.verticalCenter

                                Label {
                                    id: indicator
                                    visible: isTreeNode && hasChildren
                                    // x: treeItem.padding + (depth * treeItem.indentation)
                                    
                                    anchors.horizontalCenter: parent.right
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: treeView.isExpanded(row) ? "▼   " : "▶  "  // 공백문자는 토글문자가 너무 오른쪽으로 넘어가는 것을 방지하기 위함
                                }

                                TapHandler {
                                    onSingleTapped: {
                                        treeView.toggleExpanded(row)
                                    }
                                }
                            }

                            /* Key */
                            TextField {
                                // anchors.fill: parent // do not use anchors for child items directly inside Row/RowLayout
                                Layout.fillHeight: true; Layout.fillWidth: false
                                height: treeView.itemH; width: treeView.itemW
                                readOnly: true

                                text: model.key ? model.key : "-"
                                Component.onCompleted: console.log("[key] width: " + width); // 200

                                /* Show Tool Tip when each "Key" area is clicked */
                                Rectangle {
                                    height: treeView.itemH; width: treeView.itemW;
                                    color: "transparent" // color: "yellow"

                                    TapHandler {
                                        onSingleTapped: {
                                            treeView.toolTipText = "<b><i>"+model.key+"</i></b><br>" + (model.tips ? model.tips : "No tips available");
                                            console.log("Clicked cell. Tips: " + model.tips);
                                        }
                                    }
                                }
                            }

                            /* Value */
                            TextField {
                                Layout.fillHeight: true; Layout.fillWidth: false
                                height: treeView.itemH; width: treeView.itemW
                                enabled: !indicator.visible
                                // enabled: (model.type !== "Object") && (model.type !== "Array")

                                CheckBox {
                                    visible: model.type === "Bool" // singular bool이므로 indicator.visible 확인 필요X
                                    checked: (model.value === true) || (model.value === "true")
                                    onClicked: backend.treeViewTextChanged(/*currentIndex*/treeView.index(row, column), checked/*함수내에서 QString으로 변환됨*/);
                                    anchors.centerIn: parent
                                }

                                readOnly: model.type === "Bool";
                                text: indicator.visible ? "Toggle to edit values" : (model.type !== "Bool" ? (model.value ? model.value : "") : "")
                                onEditingFinished: {
                                    if (model.type !== "Bool") backend.treeViewTextChanged(treeView.index(row, column), text);
                                }

                                Component.onCompleted: console.log("[value] width: " + width); // 45 ~ 70
                            }

                            /* Type */
                            TextField {
                                Layout.fillHeight: true; Layout.fillWidth: false
                                height: treeView.itemH; width: treeView.itemW - depth*treeView.labelRectW
                                enabled: false; color: "black"

                                // text: (model.type === "Unknown") ? "<i>Unknown</i>" : (model.type ? model.type : "---"); // rich text only
                                text: model.type ? model.type : "---"
                                Component.onCompleted: console.log("[type] width: " + width); // 45 ~ 70
                            }
                        }
                    }
                }
            }



            ScrollView {
                id: view2
                ScrollBar.horizontal.interactive: true
                ScrollBar.vertical.interactive: true

                visible: treeView.visible
                Layout.maximumHeight: 100
                Layout.minimumHeight: 30
                Layout.preferredHeight: 40
                Layout.fillHeight: true

                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                TextArea {
                    id: toolTipTextArea
                    text: "<b style=\"white-space:pre-wrap;\">Tool Tip:  </b>" + treeView.toolTipText
                    visible: treeView.visible
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop
                    readOnly: true
                    wrapMode: TextArea.WordWrap
                    textFormat: Text.RichText
                    font.pointSize: 12
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    // 배경 색상 및 테두리 추가
                    background: Rectangle {
                        color: "#f9f9f9"
                        border.color: "#cccccc"
                        border.width: 1
                    }
                }
            }
        }

        ScrollView {
            id: view

            Layout.minimumHeight: 80
            Layout.preferredHeight: 80
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.rowSpan: 4
            ScrollBar.horizontal.interactive: true
            ScrollBar.vertical.interactive: true

            TextArea {
                id: taConfigDesc
                text: backend.userConfigDisplay
                // wrapMode: Text.NoWrap
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                //                anchors.fill: parent
                font.pointSize: 12
                readOnly: true
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                implicitWidth: 400
                background: Rectangle {
                    radius: rbSelectUserConfig.radius
                    //width: taConfigDesc.width
                    //height: taConfigDesc.height
                    anchors.fill: parent
                    border.width: 1
                    color: "#ebebeb"
                }

                DropArea {
                    id: drop1
                    anchors.fill: parent
                    onDropped: {
                        // Send file name to c++ backend
                        if (drop1.hasUrls) {
                            backend.userConfigFileName = drop1.urls[0];
                            backend.userConfigFileNameChanged();
                            treeView.visible = true;
                            view.visible = false;

                        }
                        //rbRun.enabled = true
                    }
                }
            }

        }

        RoundButton {
            id: rbRun
            height: 40
            // implicitHeight: 40
            radius: 10
            text: "Run"
            enabled: backend.userConfigOK
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.preferredHeight: 40
            font.family: "Arial"
            font.bold: true
            font.pointSize: 18
            Layout.minimumHeight: 40
            Layout.fillHeight: false
            font.weight: Font.Normal
            background: Rectangle {
                id: runRect
                color: "#a8a7fd"
                radius: rbRun.radius
                border.width: 1
            }

            Layout.fillWidth: true
            onClicked: backend.onRunClicked()
            onHoveredChanged: hovered ? runRect.color = "#f8a7fd" : runRect.color = "#a8a7fd"
        }

        RowLayout {
            id: rowLayout
            height: 40
            Layout.minimumHeight: 40
            Layout.preferredHeight: 40
            Layout.fillHeight: false
            Layout.fillWidth: true
            spacing: 10

            RoundButton {
                id: rbHelp
                height: 40
                radius: 10
                text: "Help"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                font.family: "Arial"
                font.pointSize: 18
                font.bold: true
                font.weight: Font.Normal
                background: Rectangle {
                    id: helpRect
                    color: "#a8a7fd"
                    radius: rbSelectUserConfig.radius
                    border.width: 1
                }
                onClicked: {
                    helpDialog.visible = true
                }
                onHoveredChanged: hovered ? helpRect.color = "#f8a7fd" : helpRect.color = "#a8a7fd"
            }

            RoundButton {
                id: rbExit
                height: 40
                text: "Exit"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                radius: 10
                font.family: "Arial"
                font.pointSize: 18
                font.bold: true
                font.weight: Font.Normal
                background: Rectangle {
                    id: exitRect
                    radius: rbSelectUserConfig.radius
                    border.width: 1
                    color: "#a8a7fd"
                }
                onHoveredChanged: hovered ? exitRect.color = "#f8a7fd" : exitRect.color = "#a8a7fd"
                onClicked: backend.exitClicked()
                // onClicked: Qt.quit()
            }
        }

    }

    Connections{
        target: backend
        //onShowErrorMessage: errorMessageDialog.visible = true
        function  onShowErrorMessage(){
            errorMessageDialog.visible = true;
        }
    }
    Connections{
        target: backend
        //onShowErrorMessageCompression: errorMessageDialogCompression.visible = true
        function onShowErrorMessageCompression() {
            errorMessageDialogCompression.visible = true;
        }
    }
    Component.onCompleted: {
        // Set window position to center of screen
        setX(Screen.width / 2 - width / 2);
        setY(Screen.height / 2 - height / 2);
    }
}
