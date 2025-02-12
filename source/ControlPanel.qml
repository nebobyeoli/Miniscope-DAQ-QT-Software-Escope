import QtQuick //2.0
import QtQuick.Controls //2.12
import QtQuick.Layouts //1.12
import QtQuick.Controls.Basic // for TextArea.flickable: TextArea { background: Rectangle }

Item {
    id: root
    objectName: "root"
    // width: parent.width
    // height: parent.height

    property double currentRecordTime: 0
    property double ucRecordLength: 1
    property bool recording: false

    property var ucProps: []
    property var ucValues: ["testMouse"] // placeholder value
    property var ucIsNumber: []

    signal submitNoteSignal(string note)
    signal extTriggerSwitchToggled(bool checkedState)

    Rectangle {
        id: rectangle
        Layout.fillWidth: false; width: Window.width
        Layout.fillHeight: false; height: Window.height
        color: "#cacaca"
    }

    // GridLayout { // Log상자 안 뜸. 어찌어찌 뜨게 해도 개별 "Row" 형태로 정렬 안 됨. 강제로 columns: 1 설정하려 하면 crash됨
    //     id: gridLayout
    //     anchors.fill: parent
    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true

        RowLayout {
            TextField {
                id: textNote
                objectName: "textNote"
                text: qsTr("Enter Note")
                font.pointSize: 10
                font.family: "Arial"

                Layout.fillWidth: true
                width: (Window.width - 10) / 3 * 2
                onFocusChanged: {
                    if (focus)
                        selectAll()
                }
            }
            Button {
                id: bNoteSubmit
                objectName: "bNoteSubmit"
                text: qsTr("Submit Note")
                enabled: false
                checkable: false

                Layout.fillWidth: true
                width: (Window.width - 10) / 3
                onClicked: {
                    root.submitNoteSignal(textNote.text);
                    textNote.text = "Enter Note";
                }
            }
        }

        Switch {
            id: switchExtTrigger
            objectName: "switchExtTrigger"
            text: qsTr("Triggerable")
            checked: false
            enabled: true
            onToggled: {
                if (checked) {
                    bRecord.text = qsTr("Record [Ext. Trig. Enabled]");
                    bStop.text = qsTr("Stop [Ext. Trig. Enabled]");
                    // bRecord.enabled = false;
                    // bStop.enabled = false;
                }
                else {
                    bRecord.text = qsTr("Record");
                    bStop.text = qsTr("Stop");
                    // if (root.recording == false) {
                    //     bRecord.enabled = true;
                    // }
                    // else {
                    //     bStop.enabled = true;
                    // }
                }

                root.extTriggerSwitchToggled(switchExtTrigger.checked);
            }
        }

        TabBar {
            id: bar
            currentIndex: 0
            Layout.columnSpan: 2
            width: parent.width
            TabButton {
                text: qsTr("Home")
                width: 80 // width: implicitWidth
                height: 40
                background: Rectangle {
                    color: bar.currentIndex === 0 ? "white" : "black"
                }
                // onClicked: {
                //     stack.currentIndex = 0;
                // }
            }
            TabButton {
                text: qsTr("User Config")
                width: 100 // width: implicitWidth
                height: 40
                background: Rectangle {
                    color: bar.currentIndex === 1 ? "white" : "black"
                }
                // onClicked: {
                //     stack.currentIndex = 1;
                // }
            }
        }

        StackLayout {
            id: stack
            currentIndex: bar.currentIndex

            // Rectangle { // StackLayout (TabBar) test
            //     color: 'teal'
            //     Layout.fillWidth: true
            //     // implicitWidth: 200
            //     // implicitHeight: 200
            // }
            // Rectangle {
            //     color: 'plum'
            //     Layout.fillWidth: true
            //     // implicitWidth: 300
            //     // implicitHeight: 200
            // }

            Flickable { // visible on currentIndex 0 // Rectangle 껍데기 doesn't work
                id: flick1
                flickableDirection: Flickable.VerticalFlick

                contentWidth: Window.width
                // contentHeight: 600
                
                clip: true

                TextArea.flickable: TextArea {
                    id: messageTextArea
                    objectName: "messageTextArea"

                    textFormat: TextEdit.RichText
                    text: "'Space Bar': screenshot of video stream.<br/>'h': hides/shows video stream controls.<br/> Double click track to select it.<br/> Scroll wheel adjust width of trace display.<br/> CTRL + scroll wheel adjusts trace scale.<br/>Messages:"
                    wrapMode: Text.WrapAnywhere

                    font.pointSize: 10
                    readOnly: true

                    background: Rectangle {
                        radius: rbSelectUserConfig.radius
                        Layout.fillWidth: false;  width: Window.width
                        // Layout.fillHeight: false;  height: 600
                        
                        border.width: 1
                        color: "#ebebeb"
                    }
                    function log_color(msg, color) {
                        return "<span style='color: " + color +  ";' >" + msg + "</span>";
                    }
                    function logMessage(time, msg) {
                        var color = "darkgreen";
                        if (msg.toLowerCase().indexOf('error') >= 0) {
                            color = "red";
                        } else if (msg.toLowerCase().indexOf('warning') >= 0) {
                            color = "goldenrod";
                        }

                        var _time = log_color(time, "0xFFFFFF")
                        var _msg = log_color(msg, color);
                        messageTextArea.append(_time + ": " + _msg);

                        // scroll to bottom
                        flick1.contentY = (messageTextArea.height - flick1.height);
                    }
                }
            
                ScrollBar.vertical: ScrollBar {
                    width: 20
                }
            }

            ScrollView { // visible on currentIndex 1
                id: flick0
                Layout.columnSpan: 2

                background: Rectangle {
                    Layout.fillWidth: false; width: Window.width
                    border.width: 1
                    color: "gray"
                }

                clip: false

                Column {
                    id: cL
                    spacing: 5

                    // Layout.fillWidth: true;
                    // Layout.fillHeight: true
                    clip: true

                    Repeater {
                        id: repeater
                        model: root.ucProps.length

                        Rectangle {
                            Layout.fillWidth: false; width: Window.width - 5 // i think this one is the master controller
                            // width: root.width - 10
                            height: 60
                            Component.onCompleted: console.log("Window.width: " + Window.width + ", root.width: " + root.width)

                            color: "lightgray"  // color: "transparent"

                            anchors.left: parent.left  // AnchorChanges { anchors.left: parent.left }
                            anchors.leftMargin: 5      // PropertyChanges { anchors.leftMargin: 5 }

                            border.color: "#555555"

                            Label {
                                // Layout.fillWidth: true
                                anchors.fill: parent

                                text: root.ucProps[index]
                                anchors.horizontalCenter: parent.horizontalCenter
                                horizontalAlignment: Text.AlignHCenter
                                anchors.top: parent.top
                                anchors.topMargin: 7
                                font.pointSize: 12
                                font.family: "Arial"
                            }
                            TextField {
                                // Layout.fillWidth: true
                                anchors.fill: parent
                                // width: parent.width - 10
                                height: 40

                                property var validNumber : DoubleValidator { bottom:0;}
                                property var validAll : RegularExpressionValidator { regularExpression: /[0-9A-F]+/ }

                                text: root.ucValues[index]  // Unable to assign [undefined] to QString error --> is fixed by itself after initial load
                                // anchors.horizontalCenter: parent.horizontalCenter
                                anchors.top: parent.top
                                anchors.topMargin: 25
                                font.pointSize: 12
                                font.family: "Arial"
                                color: "green"
                                enabled: !root.recording
                                // canPaste: true
                                selectByMouse: true
                                validator: if(root.ucIsNumber[index]) {validNumber} else {validAll}

                                onTextChanged: {
                                    if (focus)
                                        color = "red"
                                }

                                onEditingFinished: {
                                    color = "green"
                                    // console.log("index: "+index+", text: "+text);
                                    root.ucValues[index] = text;
                                    if (root.ucProps[index] === "recordLengthinSeconds") {
                                        root.ucRecordLength = text;
                                        ucRecordLengthChanged();
                                    }
                                    // ucPropsChanged();
                                }
                            }
                        }
                    }
                    TextField {  // <-- additional bottom filler
                        // Layout.fillHeight: true
                        Layout.fillHeight: false; height: 300
                        background: Rectangle {
                            anchors.fill: parent
                            color: "transparent"
                        }
                    }
                }
            }
        
        } // StackLayout


        RowLayout {
            DelayButton {
                id: bRecord
                objectName: "bRecord"
                text: qsTr("Record")
                // autoExclusive: true
                font.family: "Arial"
                font.pointSize: 12
                Layout.fillWidth: true
                delay: 1000

                onActivated: {
                    switchExtTrigger.enabled = false;
                    bNoteSubmit.enabled = true;
                    bStop.enabled = true;
                    bRecord.enabled = false;
                }
            }

            DelayButton {
                id: bStop
                objectName: "bStop"
                text: qsTr("Stop")
                enabled: false
                // autoExclusive: true
                font.pointSize: 12
                font.family: "Arial"
                // enabled: false
                checkable: true
                Layout.fillWidth: true
                delay: 1000

                // Layout.row: 0
                // Layout.column: 1
                Layout.rowSpan: 1
                Layout.columnSpan: 1

                onActivated: {
                    switchExtTrigger.enabled = true;
                    bNoteSubmit.enabled = false;
                    bStop.enabled = false;
                    bRecord.enabled = true;
                }
            }
        }

        RowLayout {
            ProgressBar {
                id: progressBar
                height: 40
                Layout.minimumHeight: 40
                // Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.fillWidth: true
                value: (root.ucRecordLength > 0) ? root.currentRecordTime/root.ucRecordLength : root.currentRecordTime%2
                from: 0
                to: 1
                Layout.columnSpan: 1
            }

            Text {
                id: recordTimeText
                objectName: "recordTimeText"

                text: (root.ucRecordLength > 0) ? root.currentRecordTime.toString() + "/" + root.ucRecordLength.toString() + "s" : root.currentRecordTime.toString() + "s"
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                font.pointSize: 12
                font.family: "Arial"
            }
        }
    }
    Connections{
        target: root
        function onCurrentRecordTimeChanged() {
            if ((root.currentRecordTime >= root.ucRecordLength) && root.ucRecordLength > 0) {
                bStop.enabled = false;
                bRecord.enabled = true;
            }
        }
    }
}




/*##^##
Designer {
    D{i:1;anchors_height:200;anchors_width:200}D{i:17;anchors_height:60}D{i:2;anchors_height:200;anchors_width:200}
}
##^##*/
