// "qt creator" how to automatically reload externally changed file ?
// Take a look at look at Qt Creator's preference under:
    Environment -> System -> When file are externally modified.


// "파일이 다른 곳에서 열려 있음" 해결법
    resmon (Resource Monitor) 실행 -> CPU 탭 들어가기 -> 프로세스 모두 체크 -> 연결된 핸들 - 핸들 검색 -> 프로세스 우클릭 - 프로세스 끝내기
// https://superuser.com/a/643312


// Powershell에서
// 지난 1일 동안 수정된 파일들 모두 찾기:
    Get-ChildItem -Path . -Recurse| Where-Object {$_.LastWriteTime -gt (Get-Date).AddDays(-1)}
// 지난 1시간 동안 수정된 파일들 모두 찾기:
    Get-ChildItem -Path . -Recurse| Where-Object {$_.LastWriteTime -gt (Get-Date).AddHours(-1)}
// 지난 1분 동안 수정된 파일들 모두 찾기:
    Get-ChildItem -Path . -Recurse| Where-Object {$_.LastWriteTime -gt (Get-Date).AddMinutes(-1)}




// VSCode (Visual Studio Code)에서 Git 사용할 때, 콘솔창에서 텍스트 편집 대신 VSCode 편집기 사용하기
// How to use Visual Studio Code as default editor for git?  // https://stackoverflow.com/questions/30024353/how-to-use-visual-studio-code-as-default-editor-for-git
    git config --global core.editor "code --wait"
// 이렇게 설정하면 VSCode에서 git을 사용할 때 실수도 줄어들고 편리함




// How to change an old commit message?  // https://stackoverflow.com/a/77357720
// Instead of rebasing and force pushing the modified branch, it's possible to replace a commit with a different message without affecting the existing commit hashes.
// The syntax looks like this:
    git replace --edit <commit>
// edit: this actually doesn't work (works only locally)
// github 서버에도 제대로 적용시키고 싶으면 git rebase 명령어 사용해야 함




// these work
<< commit message 바꾸기 >>
git rebase -i HEAD~N    // HEAD부터 N개 커밋 편집

- 문구 바꾸고 싶은 commit에서 "pick" -> "reword"로 바꾸기, 저장 후 닫기
- 창 다시 뜨면 commit 메시지 수정하기, 저장 후 닫기

git push --force-with-lease



<< commit 내역 바꾸기 >>
git rebase -i HEAD~N

  - 문구 바꾸고 싶은 commit에서 "pick" -> "edit"로 바꾸기, 저장 후 닫기
2 - 그러면 폴더 내 파일들이 그 시점의 것들로 바뀐다. 그러면 파일들 수정하고 git add . 하거나 vscode로 stage all changes 하기
  - git rebase --continue
  - commit message 수정, 저장 후 닫기
  - 마지막 commit까지 모두 수정 완료될 때까지 2에서부터 다시 반복

git push --force-with-lease




<< git rebase 취소하기 >>
git rebase --abort





private repo에 git push origin 할 때 "repository not found" 오류:
시도할 것 1:
    git remote -v   // url 제대로 박혀 있는지 확인
    git remote remove origin
    git remote add origin https://[username]@github.com/[reponame].git

시도할 것 2:
    // https://help.appveyor.com/discussions/problems/26502-git-push-origin-to-private-github-repo
    제어판 > 사용자 계정 > 자격 증명 관리자 > Windows 자격 증명 관리 > Windows 자격 증명 > git:https://[계정ID]@github.com  <---  제거하기







type 규칙?
    "Object" - map with key(string)-values
    "Array" - array with index-values

    value="" 이면 type="Unknown"


<backend.h>
read "backend.m_jsonTreeModel" as "backend.jsonTreeModel" :
    Q_PROPERTY(QStandardItemModel* jsonTreeModel READ jsonTreeModel WRITE setJsonTreeModel NOTIFY jsonTreeModelChanged)
    QStandardItemModel* jsonTreeModel() { return m_jsonTreeModel; }  // 읽기: jsonTreeModel을 반환하는 함수
    // 그래서 m_jsonTreeModel이 어떻게 전달되는지?
    // 그냥 <추가 함수 사용 없이> 그대로 전달됨 ..


<backEnd.h>
얘네들은 (처음에 config json 띄우는데에?) 꽤 중요해 보이는데 왜 <backEnd.cpp>와 <backEnd.h>의 선언/정의 부분에만 사용되어 있지 (실질적으로 호출된 곳이 없음)?
    void updateTreeViewModel(const QJsonObject &jConfig);
    void addJsonObjectToTree(QStandardItem *parent, const QJsonObject &jsonObject);
    void updateJsonModel(int row, int column, const QString &newValue);







type factoring:  <handleJsonArray()> 에서 TYPE "Array(Integer)" 등을 출력함.

tempType = objProps[keys[i]].toObject()["type"].toString();

type =

// if(obj[keys[i]].isObject()) : <handleJsonObject()>
// objProps["cameraDeviceName"].toObject()
// objProps["miniscopeDeviceName"].toObject()
// objProps[keys[i]].toObject()

else if (obj[keys[i]].isArray())
objProps[keys[i]].toObject()["type"].toString()

// else if (obj[keys[i]].isString())   "String"
// else if (obj[keys[i]].isBool())     "Bool"
// else if (obj[keys[i]].isDouble())   "Double"



<handleJsonArray()> : factoring =
.right(type.length() - 6).chopped(1)





거의 똑같은 대량 코드가 있는 두 부분:
여기서 key/value/type/tips를 분리해 내는 듯:
for문 통해  QVector<QStandardItem*> m_standardItem  에 할당함
-> void backEnd::constructJsonTreeModel()
-> QStandardItem *backEnd::handleJsonObject(QStandardItem *parent, QJsonObject obj, QJsonObject objProps)   // 재귀있음!


QJsonObject m_userConfig는
1
void backEnd::generateUserConfigFromModel() 마지막 부분에서
    m_userConfig = jConfig;
로 할당됨
2
void backEnd::loadUserConfigFile() 초기 부분에서
    QJsonDocument d = QJsonDocument::fromJson(jsonFile.toUtf8());
    m_userConfig = d.object();
로 할당됨





"QVariants"
    QVariant(QString, "Integer") "Integer"
    QVariant(QString, "String") "String"
    QVariant(QString, "DirPath") "DirPath"




QDebug dbg(QtDebugMsg);
dbg << "a";
dbg << "sdflksjfkldsj";
// result:    a sdflksjfkldsj






ControlPanel::ControlPanel(QObject *parent, QJsonObject userConfig) :
    QObject(parent),
    ================ ?






backend.cpp > constructJsonTreeModel()에서 m_jsonTreeModel을 iteration해서 출력해보기
참고:
how to iterate QStandardItemModel
// https://stackoverflow.com/questions/33124903/how-to-iterate-through-a-qstandarditemmodel-completely







qDebug without quotes/quotation marks:
// https://stackoverflow.com/questions/27976581/why-is-qstring-printed-with-quotation-marks

    // Here's an example of how you'd use this feature:
    QDebug debug = qDebug();
    debug << QString("This string is quoted") << endl;
    debug.noquote();
    debug << QString("This string is not") << endl;

    // Another option is to use QTextStream with stdout.
    // There's an example of this in the documentation:
    QTextStream out(stdout);
    out << "Qt rocks!" << endl;



columnWidthProvider
columnWidthProvider: (column) => column === 0 ? 200 : 100



이 두 align 방식의 차이?
    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter




QJsonArray backEnd::getArrayFromModel(QModelIndex idx) ,
QJsonObject backEnd::getObjectFromModel(QModelIndex idx) 는
    void backEnd::generateUserConfigFromModel() 에만 사용됨




// ctrl+F setColumn ~ 으로 추적한 결과:
// 여기서 main.qml 밀림 발생했던 듯
void backEnd::constructJsonTreeModel()
{
    //QFile file;
    //QByteArray jsonFile; //QString에서 변경
    // QJsonObject jObj;

    m_jsonTreeModel->clear();
    m_jsonTreeModel->setColumnCount(3); // <--------
    m_standardItem.clear();

    // ...
}




DropArea?


QML file include? (structure QML code)?
https://stackoverflow.com/questions/22168457/include-another-qml-file-from-a-qml-file
https://stackoverflow.com/questions/19541369/qml-file-include-or-one-monolithic-file-structure-qml-code


qml mousearea vs taphandler ?
MouseArea {
    onClicked: {
        console.log("Clicked cell. Tips:" + model.tips);
    }
}
TapHandler {
    onSingleTapped: {
        treeView.toggleExpanded(row)
    }
}



How to make component always on top:
    https://forum.qt.io/topic/139577/how-to-make-always-on-top
    set "raise" or "z: 1"



loaders and delegate loaders
Qt 6 버전에서는 이 Loader를 사용해서 TreeViewerJSON.qml을 불러와야 하는 듯?
https://doc.qt.io/qt-6/qml-qtquick-loader.html



alignments:
    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    Layout.alignment: Qt.AlignLeft
    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignTop

    anchors.centerIn: parent
    indicator.anchors.right: right





다음으로 할 일:
    opencv camera connection error
    image to color
        DAQ = grayscale
        ISP -> 'to color in the SW without matlab'
    dual led 설정작업(?)
    

    main.qml에서 하나씩 주석추가/해제하는건 어디서 밀림 발생하는거??
    multiple columns in treeview "bug" qml....

    나중에 보기(정리용):
    How to define a "template" with child placeholders in QML?
    https://stackoverflow.com/questions/12477425/how-to-define-a-template-with-child-placeholders-in-qml

