#include "backend.h"
#include <QDebug>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QApplication>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>
#include <QObject>
#include <QVariant>
#include <QDir>
#include <QVector>
#include <QUrl>
#include <QString>
#include <QDateTime>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>
#include <QStandardPaths>

#include <opencv2/core/core.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "miniscope.h"
#include "behaviorcam.h"
#include "controlpanel.h"
#include "datasaver.h"
#include "behaviortracker.h"
#include "tracedisplay.h"

#ifdef USE_USB
 #include <libusb.h>
#endif

backEnd::backEnd(QObject *parent) :
    QObject(parent),
    m_versionNumber(""),
    m_userConfigFileName(""),
    m_userConfigOK(false),
    traceDisplay(nullptr),
    behavTracker(nullptr),
    m_jsonTreeModel(new QStandardItemModel())
{
#ifdef DEBUG
//    QString homePath = QDir::homePath();
    QDir dir;
    qDebug()<< "current path : "<<dir.currentPath();
    QDir::setCurrent("C:/Escope/Miniscope-DAQ-QT-V5V6-attempt/source");
    qDebug() <<"set current path to: " <<dir.currentPath();
    // m_userConfigFileName = "C:/Escope/QT_Software_V5/build/release/colortest2/deviceConfigs/userConfigEscope1.json"; //"./userConfigs/UserConfigEscope1.json";
    m_userConfigFileName = "./userConfigs/UserConfigEscope1.json";
    loadUserConfigFile();
    handleUserConfigFileNameChanged();
    setUserConfigOK(true);
#endif
    m_softwareStartTime = QDateTime().currentMSecsSinceEpoch();

    // User Config default values
    researcherName = "";
    dataDirectory = "";
    experimentName = "";
    animalName = "";
    dataStructureOrder = {"researcherName", "experimentName", "animalName", "date"};

    ucExperiment["type"] = "None";
//    ucMiniscopes = {"None"};
//    ucBehaviorCams = {"None"};
    ucBehaviorTracker["type"] = "None";
    ucTraceDisplay["type"] = "None";

    dataSaver = new DataSaver();

#ifdef USE_USB
    testLibusb();
#endif

    testCodecSupport();
    QString tempStr;
    for (int i = 0; i < m_availableCodec.length(); i++)
        m_availableCodecList += m_availableCodec[i] + ", ";

    m_availableCodecList = m_availableCodecList.chopped(2);
    for (int i = 0; i < unAvailableCodec.length(); i++)
        tempStr += unAvailableCodec[i] + ", ";

    // Build initial text in main screen -------
    QFile file;
    QString jsonFile;
    QJsonObject jObj;
    QStringList supportedDevices;

    qDebug() <<"supportedDevices: "<< supportedDevices;

    // 절대 경로로 파일 설정-> 경로 문제 수정
//    QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/deviceConfigs/videoDevices.json";
    QString filePath = QCoreApplication::applicationDirPath() + "/deviceConfigs/videoDevices.json";
    file.setFileName(filePath);
    //file.setFileName("./deviceConfigs/videoDevices.json");
    qDebug() << "Selected path:" << filePath;
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/AppName";
    qDebug() << "Config Path:" << configPath;



    // 파일 열기
    bool status = file.open(QIODevice::ReadOnly | QIODevice::Text);
    if (status == true) {
        jsonFile = file.readAll();
        file.close();
        qDebug() << "Loaded videoDevices";

        // JSON parsing
        QJsonDocument d = QJsonDocument::fromJson(jsonFile.toUtf8());
        jObj = d.object();
        supportedDevices = jObj.keys();
    }
    else {
        qWarning() << "Failed to open videoDevice file! Path:" << file.fileName();
        return;
    }

    QString initDisplayMessage;
    initDisplayMessage = "Escope_test\n\nSelect a User Configuration file. You can click the button above or just drag and drop a user config file here.\n\n";
    initDisplayMessage.append("Supported devices are:\n");
    for (int i=0; i < supportedDevices.length(); i++) {
        initDisplayMessage.append("\t" + supportedDevices[i] + "\n");
    }
    initDisplayMessage.append("More information on the devices can be found in /deviceConfigs/videoDevices.json.\n\n");
    initDisplayMessage.append("Available compression Codecs on your computer are:\n\t" + m_availableCodecList +
                              "\n\nUnavailable compression Codes on your computer are:\n\t" + tempStr.chopped(2));

    setUserConfigDisplay(initDisplayMessage);

//    QObject::connect(this, SIGNAL (userConfigFileNameChanged()), this, SLOT( handleUserConfigFileNameChanged() ));

    file.setFileName(QCoreApplication::applicationDirPath() + "/deviceConfigs/userConfigProps.json"); //C:/Escope/QT_Software_V5/build/release/colortest2
    if (!file.exists()) {
        qWarning() << "파일이 존재하지 않습니다:" << file.fileName();
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "파일을 열 수 없습니다:" << file.fileName();
        return;
    }

    // 파일 내용 읽기
    jsonFile = file.readAll();
    file.close();

    // JSON 파싱
    QJsonDocument d = QJsonDocument::fromJson(jsonFile.toUtf8());
    if (d.isNull() || !d.isObject()) {
        qWarning() << "JSON 형식이 잘못되었습니다:" << file.fileName();
        return;
    }

    // JSON 객체로 변환
    m_configProps = d.object();
    qDebug() << "설정 프로퍼티 로드 완료:" << m_configProps.keys();

}

void backEnd::setUserConfigFileName(const QString &input)
{
    const QUrl url(input);
    QString furl = url.toLocalFile();
    if (furl.contains(".json")) {
        if (furl != m_userConfigFileName) {
            m_userConfigFileName = furl;
            emit userConfigFileNameChanged();
        }

        handleUserConfigFileNameChanged();
    }
    else {
        setUserConfigDisplay("Must select a .json User Config File.");
    }
}

void backEnd::setUserConfigDisplay(const QString &input)
{
    if (input != m_userConfigDisplay) {
        m_userConfigDisplay = input;
        emit userConfigDisplayChanged();
    }
}

void backEnd::setAvailableCodecList(const QString &input)
{
    m_availableCodecList = input;
}

void backEnd::printJsonTreeModel(QAbstractItemModel* model, QModelIndex parent = QModelIndex(), int depth = 0)
{
    printForEach(model, parent, depth, QString(""));
}

void backEnd::printForEach(QAbstractItemModel* model, QModelIndex parent = QModelIndex(), int depth = 0, QString parentKey = QString(""))
{
    QString depthStr = QString("+-->").repeated(depth);
    
    for (int row = 0; row < model->rowCount(parent); ++row) {
        QModelIndex index = model->index(row, 0, parent);

        // QString printStr = QString("");
        // printStr.append(depthStr).append(QString(" "));
        // printStr.append(QString("Key: ")).append(model->data(index, Qt::UserRole + 1).toString());
        // printStr.append(QString(", Value: ")).append(model->data(index, Qt::UserRole + 2).toString());
        // printStr.append(QString(", Type: ")).append(model->data(index, Qt::UserRole + 3).toString());
        // printStr.append(QString(", Tips: ")).append(model->data(index, Qt::UserRole + 4).toString());
        // qDebug() << printStr;
        qDebug().noquote() << depthStr
            << "Key:" << model->data(index, Qt::UserRole + 1).toString()
            << ", Value:" << model->data(index, Qt::UserRole + 2).toString()
            << ", Type:" << model->data(index, Qt::UserRole + 3).toString()
            << ", Tips:" << model->data(index, Qt::UserRole + 4).toString();

        if (model->hasChildren(index)) {
            QString selfKey = model->data(index, Qt::UserRole + 1).toString();
            printForEach(model, index, depth + 1, selfKey);
        }
    }

    qDebug().noquote() << depthStr << "["+parentKey+"] end of children";
}

void backEnd::constructJsonTreeModel()
{
    //QFile file;
    //QByteArray jsonFile; //QString에서 변경
    // QJsonObject jObj;

    m_jsonTreeModel->clear();
    m_jsonTreeModel->setColumnCount(1);
    qDebug() << "m_jsonTreeModel->columnCount(): " << m_jsonTreeModel->columnCount();
    m_standardItem.clear();


    // 열 이름들 할당
    roles[Qt::UserRole + 1] = "key";
    roles[Qt::UserRole + 2] = "value";
    roles[Qt::UserRole + 3] = "type";
    roles[Qt::UserRole + 4] = "tips";

    m_jsonTreeModel->setItemRoleNames(roles);
//    qDebug() << "ROLE" << m_jsonTreeModel->roleNames();

//    QStandardItem *parentItem = m_jsonTreeModel->invisibleRootItem();

    QStringList keys = m_userConfig.keys();

    QString tempType;
    QString tempS;

    // handleJsonObject() 내부에서는: tempType = objProps[keys[i]].toObject()["type"].toString();

    for (int i=0; i < keys.length(); i++) {
        QString initialType;

        if (!keys[i].contains("COMMENT")) {
            tempType = m_configProps[keys[i]].toObject()["type"].toString("String");
            tempS = m_userConfig[keys[i]].toString();
            if (tempType.contains("path") || tempType.contains("Path")) {

                tempS = tempS.replace("\\","/"); // Corrects the slashes
            }

            if(m_userConfig[keys[i]].isObject()) {
                m_standardItem.append(new QStandardItem());
                m_standardItem.last()->setData(keys[i], Qt::UserRole + 1);      // set key
                m_standardItem.last()->setData("", Qt::UserRole + 2);           // set value
                m_standardItem.last()->setData("Object", Qt::UserRole + 3);     // set type
    //            m_standardItem.append(handleJsonObject(m_standardItem.last(), m_userConfig[keys[i]].toObject()));
                initialType = m_standardItem.last()->data(Qt::UserRole + 3).toString();
                m_jsonTreeModel->appendRow(handleJsonObject(m_standardItem.last(), m_userConfig[keys[i]].toObject(), m_configProps[keys[i]].toObject()));
            }
            else if (m_userConfig[keys[i]].isArray()) {
                m_standardItem.append(new QStandardItem());
                m_standardItem.last()->setData(keys[i], Qt::UserRole + 1);                                                  // key
                m_standardItem.last()->setData("", Qt::UserRole + 2);                                                       // value
                m_standardItem.last()->setData(m_configProps[keys[i]].toObject()["type"].toString(), Qt::UserRole + 3);     // type
                m_standardItem.last()->setData(m_configProps[keys[i]].toObject()["tips"].toString(), Qt::UserRole + 4);     // tips
                initialType = m_standardItem.last()->data(Qt::UserRole + 3).toString();

                qDebug() << "handleJsonObject: " << "Key:" << keys[i] << ", Value:" << m_standardItem.last()->data(Qt::UserRole + 2).toString() << ", Type:" << "[initial]" << initialType;

                m_jsonTreeModel->appendRow(handleJsonArray(m_standardItem.last(), m_userConfig[keys[i]].toArray(), m_configProps[keys[i]].toObject()["type"].toString()));

            }
            else if (m_userConfig[keys[i]].isString()){
                m_standardItem.append(new QStandardItem());
                m_standardItem.last()->setColumnCount(3);
                m_standardItem.last()->setData(keys[i], Qt::UserRole + 1);                                                      // key
                m_standardItem.last()->setData(tempS, Qt::UserRole + 2);                                                        // value
                m_standardItem.last()->setData(m_configProps[keys[i]].toObject()["type"].toString("String"), Qt::UserRole + 3); // type
                m_standardItem.last()->setData(m_configProps[keys[i]].toObject()["tips"].toString(), Qt::UserRole + 4);         // tips
                initialType = m_standardItem.last()->data(Qt::UserRole + 3).toString();
                m_jsonTreeModel->appendRow(m_standardItem.last());
            }
            else if (m_userConfig[keys[i]].isBool()) {
                m_standardItem.append(new QStandardItem());
                m_standardItem.last()->setColumnCount(3);
                m_standardItem.last()->setData(keys[i], Qt::UserRole + 1);                                                      // key
                m_standardItem.last()->setData(m_userConfig[keys[i]].toBool(), Qt::UserRole + 2);                               // value
                m_standardItem.last()->setData(m_configProps[keys[i]].toObject()["type"].toString("Bool"), Qt::UserRole + 3);   // type
                m_standardItem.last()->setData(m_configProps[keys[i]].toObject()["tips"].toString(), Qt::UserRole + 4);         // tips
                initialType = m_standardItem.last()->data(Qt::UserRole + 3).toString();
                m_jsonTreeModel->appendRow(m_standardItem.last());
            }
            else if (m_userConfig[keys[i]].isDouble()) {
                m_standardItem.append(new QStandardItem());
                m_standardItem.last()->setColumnCount(3);
                m_standardItem.last()->setData(keys[i], Qt::UserRole + 1);                                                      // key
                m_standardItem.last()->setData(m_userConfig[keys[i]].toDouble(), Qt::UserRole + 2);                             // value
                m_standardItem.last()->setData(m_configProps[keys[i]].toObject()["type"].toString("Double"), Qt::UserRole + 3); // type
                m_standardItem.last()->setData(m_configProps[keys[i]].toObject()["tips"].toString(), Qt::UserRole + 4);         // tips
                initialType = m_standardItem.last()->data(Qt::UserRole + 3).toString();
                m_jsonTreeModel->appendRow(m_standardItem.last());
            }

            if (!m_userConfig[keys[i]].isArray()) { // isArray()인 경우는 상위 if-else에서 해당 Array정보 출력
                // qDebug() << "handleJsonObject: " << "Key:" << keys[i] << ", Value:" << m_userConfig[keys[i]].toString() << ", Type:" << initialType;
                QString finalType = m_standardItem.last()->data(Qt::UserRole + 3).toString();
                QDebug dbg(QtDebugMsg);
                dbg << "constructJsonTreeModel: " << "Key:" << keys[i] << /*m_standardItem.last()->data(Qt::UserRole + 1).toString() <<*/ ", Value:" << m_standardItem.last()->data(Qt::UserRole + 2).toString() << ", Type:";
                if (initialType == finalType) dbg << initialType;
                else dbg << "[initial]" << initialType << " [final]" << finalType;
            }
        }
    }

    qDebug() << "\nprint m_userConfig:\n" << m_userConfig; // userConfig는 JSON 형태로 그대로 <똑같이> 출력됨

    qDebug() << "\nprint m_standardItem:";
    for (int i=0; i < keys.length(); i++) {
        qDebug() << "Key:" << keys[i]
            << ", Value:" << m_standardItem.last()->data(Qt::UserRole + 2).toString()
            << ", Type:" << m_standardItem.last()->data(Qt::UserRole + 3).toString()
            << ", Tips:" << m_standardItem.last()->data(Qt::UserRole + 4).toString();
    }

    qDebug() << "\nprint m_jsonTreeModel:\n" << m_jsonTreeModel;
    printJsonTreeModel(m_jsonTreeModel);

    //
    // 끝, 이제 <추가 함수 사용 없이> main.qml 에게 그대로 전달됨 ..
    //


    // /*
    //  * hera testcodes
    //  */

    // qDebug() << "Opening userConfigProps.json file...";
    // // QFile file("C:/Escope/QT_Software_V5/build/release/colortest2/deviceConfigs/userConfigEscope1.json");
    // QFile file("C:/Escope/Miniscope-DAQ-QT-V5V6-attempt/deviceConfigs/userConfigEscope1.json");
    // if (!file.exists()) {
    //     qWarning() << "File does not exist!";
    //     return;
    // }
    // QString jsonString = file.readAll();
    // if (jsonString.isEmpty()) {
    //     qWarning() << "File is empty!";
    //     return;
    // }
    // else {
    //     qDebug() << "backend.cpp: test-printing jsonString:\n" << jsonString << "\n";
    // }


    // if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    //     qDebug() << "File status:" << file.isOpen();
    //     QString jsonFile = file.readAll();//QByteArray
    //     file.close();

    //     QJsonDocument d = QJsonDocument::fromJson(jsonFile.toUtf8());
    //     m_configProps = d.object();

    // //  "m_configProps 내용 확인    qDebug() << "Loaded userConfigProps:" << m_configProps;
    // } else {
    //     qDebug() << "Failed to open userConfigProps.json file!";
    // }

    // //qDebug() << "JSON data:" << m_userConfig;

    // if (m_userConfig.isEmpty()) {
    //     qDebug() << "Error: m_userConfig is empty. Please check the JSON file.";
    //     return;
    // }
    // if (m_configProps.isEmpty()) {
    //     qDebug() << "Error: m_configProps is empty. Ensure the 'userConfigProps.json' file is properly loaded.";
    //     return;
    // }
}

void backEnd::treeViewTextChanged(const QModelIndex &index, QString text)
{
    if (index.isValid()) {

        QStandardItem *item = m_jsonTreeModel->itemFromIndex(index);

        if (item->data(Qt::UserRole + 3).toString().contains("path") || item->data(Qt::UserRole + 3).toString().contains("Path") ) {
            text = text.replace("\\","/");
        }

        item->setData(text, Qt::UserRole + 2);

        qDebug() << "Value changed at" << item->data(Qt::UserRole + 1).toString() << "to" << text;
    }
}

QStandardItem *backEnd::handleJsonObject(QStandardItem *parent, QJsonObject obj, QJsonObject objProps)
{
    QStringList keys = obj.keys();

    QString tempType;
    QString tempS;

    for (int i=0; i < keys.length(); i++) {
        QString initialType;

        if (!keys[i].contains("COMMENT")) {

            tempType = objProps[keys[i]].toObject()["type"].toString();
            tempS = obj[keys[i]].toString();
            if (tempType.contains("path") || tempType.contains("Path")) {
                tempS = tempS.replace("\\","/"); // Corrects the slashes
            }

            if(obj[keys[i]].isObject()) {
                m_standardItem.append(new QStandardItem());
                m_standardItem.last()->setData(keys[i], Qt::UserRole + 1);      // key
                m_standardItem.last()->setData("", Qt::UserRole + 2);           // value
                m_standardItem.last()->setData("Object", Qt::UserRole + 3);     // type
                m_standardItem.last()->setData(m_configProps[keys[i]].toObject()["tips"].toString("No tips available"), Qt::UserRole + 4);  // tips
                initialType = m_standardItem.last()->data(Qt::UserRole + 3).toString();

                if (parent->data(Qt::UserRole + 1).toString() == "cameras")
                    parent->appendRow(handleJsonObject(m_standardItem.last(), obj[keys[i]].toObject(), objProps["cameraDeviceName"].toObject()));
                else if (parent->data(Qt::UserRole + 1).toString() == "miniscopes")
                    parent->appendRow(handleJsonObject(m_standardItem.last(), obj[keys[i]].toObject(), objProps["miniscopeDeviceName"].toObject()));
                else
                    parent->appendRow(handleJsonObject(m_standardItem.last(), obj[keys[i]].toObject(), objProps[keys[i]].toObject()));
            }
            else if (obj[keys[i]].isArray()) {
                m_standardItem.append(new QStandardItem());
                m_standardItem.last()->setData(keys[i], Qt::UserRole + 1);                                          // key
                m_standardItem.last()->setData("", Qt::UserRole + 2);                                               // value
                m_standardItem.last()->setData(objProps[keys[i]].toObject()["type"].toString(), Qt::UserRole + 3);  // type
                m_standardItem.last()->setData(objProps[keys[i]].toObject()["tips"].toString(), Qt::UserRole + 4);  // tips
                initialType = m_standardItem.last()->data(Qt::UserRole + 3).toString();

                qDebug() << "handleJsonObject: " << "Key:" << keys[i] << ", Value:" << m_standardItem.last()->data(Qt::UserRole + 2).toString() << ", Type:" << "[initial]" << initialType;

                parent->appendRow(handleJsonArray(m_standardItem.last(), obj[keys[i]].toArray(), objProps[keys[i]].toObject()["type"].toString()));
            }
            else if (obj[keys[i]].isString()){
                m_standardItem.append(new QStandardItem());
    //            m_standardItem.last()->setColumnCount(3);
                m_standardItem.last()->setData(keys[i], Qt::UserRole + 1);                                          // key
                m_standardItem.last()->setData(tempS.isEmpty() ? "N/A" : tempS, Qt::UserRole + 2);                  // value
                m_standardItem.last()->setData(tempType.isEmpty() ? "Unknown" : tempType, Qt::UserRole + 3);        // type
                m_standardItem.last()->setData(objProps[keys[i]].toObject()["tips"].toString(), Qt::UserRole + 4);  // tips
                initialType = m_standardItem.last()->data(Qt::UserRole + 3).toString();

                parent->appendRow(m_standardItem.last());
            }
            else if (obj[keys[i]].isBool()) {
                m_standardItem.append(new QStandardItem());
    //            m_standardItem.last()->setColumnCount(3);
                m_standardItem.last()->setData(keys[i],Qt::UserRole + 1);                                                   // key
                m_standardItem.last()->setData(obj[keys[i]].toBool(),Qt::UserRole + 2);                                     // value
                m_standardItem.last()->setData(objProps[keys[i]].toObject()["type"].toString("Bool"), Qt::UserRole + 3);    // type
                m_standardItem.last()->setData(objProps[keys[i]].toObject()["tips"].toString(), Qt::UserRole + 4);          // tips
                initialType = m_standardItem.last()->data(Qt::UserRole + 3).toString();

                parent->appendRow(m_standardItem.last());
            }
            else if (obj[keys[i]].isDouble()) {
                m_standardItem.append(new QStandardItem());
    //            m_standardItem.last()->setColumnCount(3);
                m_standardItem.last()->setData(keys[i], Qt::UserRole + 1);                                                  // key
                m_standardItem.last()->setData(obj[keys[i]].toDouble(), Qt::UserRole + 2);                                  // value
                m_standardItem.last()->setData(objProps[keys[i]].toObject()["type"].toString("Double"), Qt::UserRole + 3);  // type
                m_standardItem.last()->setData(objProps[keys[i]].toObject()["tips"].toString(), Qt::UserRole + 4);          // tips
                initialType = m_standardItem.last()->data(Qt::UserRole + 3).toString();

                parent->appendRow(m_standardItem.last());
            }

            if (!obj[keys[i]].isArray()) { // isArray()인 경우는 상위 if-else에서 해당 Array정보 출력
                // qDebug() << "handleJsonObject: " << "Key:" << keys[i] << ", Value:" << m_userConfig[keys[i]].toString() << ", Type:" << initialType;
                QString finalType = m_standardItem.last()->data(Qt::UserRole + 3).toString();
                QDebug dbg(QtDebugMsg);
                dbg << "handleJsonObject: " << "Key:" << keys[i] << ", Value:" << m_standardItem.last()->data(Qt::UserRole + 2).toString() << ", Type:";
                if (initialType == finalType) dbg << initialType;
                else dbg << "[initial]" << initialType << " [final]" << finalType;
            }
        }
    }
    return parent;
}

QStandardItem *backEnd::handleJsonArray(QStandardItem *parent, QJsonArray arry, QString type)  // arry : passed as reference or object ?
{
//    QStringList keys = obj.keys();
//    type = type.right(6);
    type = type.right(type.length() - 6);
    type = type.chopped(1);
    qDebug() << "-> TYPE" << type;
    if (type != "String" && type != "Bool" && type != "Integer" && type != "Double" && type != "Number" && type != "Object" && type.left(5) != "Array") {
        qDebug() << "-> TYPE" << type;
        type = "String";
    }

    for (int i=0; i < arry.size(); i++) {
        QString initialType;

        if(arry[i].isObject()) {
            m_standardItem.append(new QStandardItem());
            m_standardItem.last()->setData("Object", Qt::UserRole + 1);     // key
            m_standardItem.last()->setData("", Qt::UserRole + 2);           // value
            m_standardItem.last()->setData("Object", Qt::UserRole + 3);     // type
            initialType = m_standardItem.last()->data(Qt::UserRole + 3).toString();
            parent->appendRow(handleJsonObject(m_standardItem.last(), arry[i].toObject(), QJsonObject()));
        }
        else if (arry[i].isArray()) {  // 이미 isArray인데
            m_standardItem.append(new QStandardItem());
            m_standardItem.last()->setData("Array", Qt::UserRole + 1);      // key
            m_standardItem.last()->setData("", Qt::UserRole + 2);           // value
            m_standardItem.last()->setData(type, Qt::UserRole + 3);         // type
            initialType = m_standardItem.last()->data(Qt::UserRole + 3).toString();
            parent->appendRow(handleJsonArray(m_standardItem.last(), arry[i].toArray(), type));  // toArray를 또 하는 이유는 ?

        }
        else if (arry[i].isString()){
            m_standardItem.append(new QStandardItem());
//            m_standardItem.last()->setColumnCount(3);
            m_standardItem.last()->setData("", Qt::UserRole + 1);                   // key
            m_standardItem.last()->setData(arry[i].toString(), Qt::UserRole + 2);   // value
            m_standardItem.last()->setData(type, Qt::UserRole + 3);                 // type
            initialType = m_standardItem.last()->data(Qt::UserRole + 3).toString();
            parent->appendRow(m_standardItem.last());
        }
        else if (arry[i].isBool()) {
            m_standardItem.append(new QStandardItem());
//            m_standardItem.last()->setColumnCount(3);
            m_standardItem.last()->setData("",Qt::UserRole + 1);                    // key
            m_standardItem.last()->setData(arry[i].toBool(), Qt::UserRole + 2);     // value
            m_standardItem.last()->setData(type, Qt::UserRole + 3);                 // type
            initialType = m_standardItem.last()->data(Qt::UserRole + 3).toString();
            parent->appendRow(m_standardItem.last());
        }
        else if (arry[i].isDouble()) {
            m_standardItem.append(new QStandardItem());
//            m_standardItem.last()->setColumnCount(3);
            m_standardItem.last()->setData("", Qt::UserRole + 1);                    // key
            m_standardItem.last()->setData(arry[i].toDouble(), Qt::UserRole + 2);    // value
            m_standardItem.last()->setData(type, Qt::UserRole + 3);                  // type
            initialType = m_standardItem.last()->data(Qt::UserRole + 3).toString();
            parent->appendRow(m_standardItem.last());
        }

        QString finalType = m_standardItem.last()->data(Qt::UserRole + 3).toString();
        QDebug dbg(QtDebugMsg);
        dbg << "-> handleJsonArray: " << i << ", Value:" << m_standardItem.last()->data(Qt::UserRole + 2).toString() << ", Type:";
        if (initialType == finalType) dbg << initialType;
        else dbg << "[initial]" << initialType << " [final]" << finalType;
    }

    qDebug() << "parent: " << parent;
    int rows = parent->rowCount();
    int cols = parent->columnCount();
    for (int i=0; i<rows; ++i) {
        for (int j=0; j<cols; ++j) {
            qDebug() << "child("<<i<<", "<<j<<"): " << parent->child(i, j);
        }
    }
    return parent;
}



void backEnd::generateUserConfigFromModel()
{
    // void forEach(QAbstractItemModel* model, QModelIndex parent = QModelIndex()) {
    //     for(int r = 0; r < model->rowCount(parent); ++r) {
    //         QModelIndex index = model->index(r, 0, parent);
    //         QVariant name = model->data(index);
    //         qDebug() << name;
    //         // here is your applicable code
    //         if( model->hasChildren(index) ) {
    //             forEach(model, index);
    //         }
    //     }
    // }

    QJsonObject jConfig;
    QString key, value, type;
    for (int i=0; i < m_jsonTreeModel->rowCount(); i++) {
        QModelIndex index = m_jsonTreeModel->index(i, 0);
        key = m_jsonTreeModel->data(index, Qt::UserRole + 1).toString();
        value = m_jsonTreeModel->data(index, Qt::UserRole + 2).toString();
        type = m_jsonTreeModel->data(index, Qt::UserRole + 3).toString();
        if (type == "Object") {
            jConfig[key] = getObjectFromModel(index);
        }
        else if (type.left(5) == "Array") {
            jConfig[key] = getArrayFromModel(index);
        }
        else if (type == "String" || type == "DirPath" || type == "FilePath" || type == "CameraDeviceType" || type == "MiniscopeDeviceType") {
            jConfig[key] = value;
        }
        else if (type == "Bool") {
            if (value == "true")
                jConfig[key] = true;
            else if (value == "false")
                jConfig[key] = false;
        }
        else if (type == "Number" || type == "Integer" || type == "Double") {
            jConfig[key] = value.toDouble();
        }
    }

    // qDebug() << "from backEnd::generateUserConfigFromModel():\n\n" << "Key:" << key << ", Value:" << value << ", Type:" << type << "\n";


    m_userConfig = jConfig;
//    QJsonDocument d;
//    d.setObject(jConfig);
//    QFile file;
//    file.setFileName("JSONNNNNNN.json");
//    file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
//    file.write(d.toJson());
//    file.close();
}



// void backEnd::updateTreeViewModel(const QJsonObject &jConfig)
// {
//     m_jsonTreeModel->clear();  // Clear the current model
//     m_jsonTreeModel->setColumnCount(3);
//     m_jsonTreeModel->setHorizontalHeaderLabels({"Key", "Value", "Type"});
//
//     for (auto it = jConfig.begin(); it != jConfig.end(); ++it) {
//         QList<QStandardItem *> row;
//         row << new QStandardItem(it.key())                          // Key
//             << new QStandardItem(it.value().toString())             // Value
//             << new QStandardItem(it.value().type() == QJsonValue::Object ? "Object" : "String"); // Type
//         m_jsonTreeModel->appendRow(row);
//     }
// }

// void backEnd::addJsonObjectToTree(QStandardItem *parent, const QJsonObject &jsonObject) {
//     for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
//         QStandardItem *item = new QStandardItem(it.key());
//         parent->appendRow(item);
//         if (it->isObject()) {
//             addJsonObjectToTree(item, it->toObject());
//         } else {
//             item->appendRow(new QStandardItem(it->toString()));
//         }
//     }
// }

// void backEnd::updateJsonModel(int row, int column, const QString &newValue) {
//     if (!m_jsonTreeModel) return;
//
//     QModelIndex index = m_jsonTreeModel->index(row, column);
//     m_jsonTreeModel->setData(index, newValue, Qt::EditRole);
//     emit jsonTreeModelChanged(); // UI 업데이트를 위해 신호 발생
// }



QJsonObject backEnd::getObjectFromModel(QModelIndex idx)
{
    QJsonObject jObj;

    QString key, value, type;

    for (int i=0; i < m_jsonTreeModel->rowCount(idx); i++) {
        QModelIndex index = m_jsonTreeModel->index(i, 0, idx);
        key = m_jsonTreeModel->data(index, Qt::UserRole + 1).toString();
        value = m_jsonTreeModel->data(index, Qt::UserRole + 2).toString();
        type = m_jsonTreeModel->data(index, Qt::UserRole + 3).toString();

        if (type == "Object") {
            jObj[key] = getObjectFromModel(index);
        }
        else if (type.left(5) == "Array") {
             jObj[key] = getArrayFromModel(index);
        }
        else if (type == "String" || type == "DirPath" || type == "FilePath" || type == "CameraDeviceType" || type == "MiniscopeDeviceType") {
            jObj[key] = value;
        }
        else if (type == "Bool") {
            if (value == "true")
                jObj[key] = true;
            else if (value == "false")
                jObj[key] = false;
        }
        else if (type == "Number" || type == "Integer" || type == "Double") {
            jObj[key] = value.toDouble();
        }

    }

    return jObj;
}

QJsonArray backEnd::getArrayFromModel(QModelIndex idx)
{
    QJsonArray jAry;

    QString key, value, type;
    QString asType;

    for (int i=0; i < m_jsonTreeModel->rowCount(idx); i++) {
        QModelIndex index = m_jsonTreeModel->index(i, 0, idx);
        // qDebug() << m_jsonTreeModel->data(index, Qt::UserRole).toString(); // ""

        key = m_jsonTreeModel->data(index, Qt::UserRole + 1).toString();
        value = m_jsonTreeModel->data(index, Qt::UserRole + 2).toString();
        type = m_jsonTreeModel->data(index, Qt::UserRole + 3).toString();
        
        asType = type;
        
        if (type == "Object") {
            jAry.append(getObjectFromModel(index));
        }
        else if (type.left(5) == "Array") {
            jAry.append(getArrayFromModel(index));
        }
        else if (type == "String" || type == "DirPath" || type == "FilePath" || type == "CameraDeviceType" || type == "MiniscopeDeviceType") {
            asType = "String";
            jAry.append(value);
        }
        else if (type == "Bool") {
            if (value == "true")        jAry.append(true);
            else if (value == "false")  jAry.append(false);
        }
        else if (type == "Number" || type == "Integer" || type == "Double") {
            asType = "Double";
            jAry.append(value.toDouble());
        }

        qDebug() << "Saved [" << key << "," << value << "," << type << "] as:" << asType;
    }

    return jAry;
}

void backEnd::saveConfigObject()
{
    generateUserConfigFromModel();
    QJsonDocument d;
    d.setObject(m_userConfig);
    QFile file;
    file.setFileName(getUserConfigSavedName());
    file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
    file.write(d.toJson());
    file.close();
}

QString backEnd::getUserConfigSavedName()
{
    QString fName = m_userConfigFileName;
    fName.replace(".json", "_" + getDateTimeNowString() + ".json");
    return fName;
}

QString backEnd::getDateTimeNowString()
{
    return QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
}

void backEnd::loadUserConfigFile()
{
    int count;
    QStringList sList;
    QString s;

    QString jsonFile;
    QFile file;
    file.setFileName(m_userConfigFileName);

    file.open(QIODevice::ReadOnly | QIODevice::Text);
    jsonFile = file.readAll();
    setUserConfigDisplay("User Config File Selected: " + m_userConfigFileName + "\n" + jsonFile);
    file.close();
    QJsonDocument d = QJsonDocument::fromJson(jsonFile.toUtf8());
    m_userConfig = d.object();

    // qDebug() << "from backEnd::loadUserConfigFile():\n\n" << m_userConfig << "\n";


    // Correct for old device structure in user config files
    QJsonObject tempObj;
    QJsonObject deviceObj = m_userConfig["devices"].toObject();
    if (m_userConfig["devices"].toObject()["miniscopes"].isArray()) {
        QJsonArray tempAry = m_userConfig["devices"].toObject()["miniscopes"].toArray();
        QJsonObject miniObj;
        sList.clear();
        count = 0;
        for (int i=0; i < tempAry.size(); i++) {
            s = tempAry[i].toObject()["deviceName"].toString();

            // Forces name to be unique
            if (sList.contains(s)) {
                s.append(QString::number(count));
                count++;
            }
            sList.append(s);
//            tempAry[i].toObject().remove("deviceName");
            tempObj = tempAry[i].toObject();
            tempObj.remove("deviceName");
            miniObj[s] = tempObj;
        }
        deviceObj.remove("miniscopes");
        deviceObj["miniscopes"] = miniObj;

    }
    if (m_userConfig["devices"].toObject()["cameras"].isArray()) {
        QJsonArray tempAry = m_userConfig["devices"].toObject()["cameras"].toArray();
        QJsonObject camObj;
        sList.clear();
        count = 0;
        for (int i=0; i < tempAry.size(); i++) {
            s = tempAry[i].toObject()["deviceName"].toString();

            // Forces name to be unique
            if (sList.contains(s)) {
                s.append(QString::number(count));
                count++;
            }
            sList.append(s);
            tempObj = tempAry[i].toObject();
            tempObj.remove("deviceName");
            camObj[s] = tempObj;
        }
        deviceObj.remove("cameras");
        deviceObj["cameras"] = camObj;
    }
    m_userConfig["devices"] = deviceObj;
    //qDebug() << "Loaded JSON Data:\n" << QJsonDocument(m_userConfig).toJson(QJsonDocument::Indented);

}

void backEnd::onRunClicked()
{
//    qDebug() << "Run was clicked!";
    generateUserConfigFromModel();
    parseUserConfig();
    checkUserConfigForIssues();
    if (m_userConfigOK) {

        constructUserConfigGUI();

        setupDataSaver(); // must happen after devices have been made
    }
    else {
        // TODO: throw out error
    }

}

void backEnd::onRecordClicked()
{
    //TODO: tell dataSaver to start recording

    // TODO: start experiment running
}

void backEnd::exitClicked()
{
    // TODO: Do other exit stuff such as stop recording???
    emit closeAll();

}

void backEnd::handleUserConfigFileNameChanged()
{
    loadUserConfigFile();
    constructJsonTreeModel();
    parseUserConfig();
    checkUserConfigForIssues();
}

void backEnd::connectSnS()
{

    // Start and stop recording signals
    QObject::connect(controlPanel, SIGNAL( recordStart(QMap<QString,QVariant>)), dataSaver, SLOT (startRecording(QMap<QString,QVariant>)));
    QObject::connect(controlPanel, SIGNAL( recordStop()), dataSaver, SLOT (stopRecording()));
    QObject::connect((controlPanel), SIGNAL( sendNote(QString) ), dataSaver, SLOT ( takeNote(QString) ));
    QObject::connect(this, SIGNAL( closeAll()), controlPanel, SLOT (close()));



    QObject::connect(dataSaver, SIGNAL(sendMessage(QString)), controlPanel, SLOT( receiveMessage(QString)));

    for (int i = 0; i < miniscope.length(); i++) {
        // For triggering screenshots
        QObject::connect(miniscope[i], SIGNAL(takeScreenShot(QString)), dataSaver, SLOT( takeScreenShot(QString)));
        QObject::connect(this, SIGNAL( closeAll()), miniscope[i], SLOT (close()));

        QObject::connect(controlPanel, &ControlPanel::setExtTriggerTrackingState, miniscope[i], &Miniscope::setExtTriggerTrackingState);
        QObject::connect(miniscope[i], &Miniscope::extTriggered, controlPanel, &ControlPanel::extTriggerTriggered);

        QObject::connect(controlPanel, &ControlPanel::recordStart, miniscope[i], &Miniscope::startRecording);
        QObject::connect(controlPanel, &ControlPanel::recordStop, miniscope[i], &Miniscope::stopRecording);
    }
    for (int i = 0; i < behavCam.length(); i++) {
//        QObject::connect(behavCam[i], SIGNAL(sendMessage(QString)), controlPanel, SLOT( receiveMessage(QString)));
        // For triggering screenshots
        QObject::connect(behavCam[i], SIGNAL(takeScreenShot(QString)), dataSaver, SLOT( takeScreenShot(QString)));

        QObject::connect(this, SIGNAL( closeAll()), behavCam[i], SLOT (close()));

//        if (behavTracker) {
//            QObject::connect(behavCam[i], SIGNAL(newFrameAvailable(QString, int)), behavTracker, SLOT( handleNewFrameAvailable(QString, int)));
//        }
    }
    if (behavTracker)
        QObject::connect(this, SIGNAL( closeAll()), behavTracker, SLOT (close()));
}

void backEnd::setupDataSaver()
{
    dataSaver->setUserConfig(m_userConfig);
    dataSaver->setRecord(false);
//    dataSaver->startRecording();

    for (int i = 0; i < miniscope.length(); i++) {
        dataSaver->setDataCompression(miniscope[i]->getDeviceName(), miniscope[i]->getCompressionType());
        dataSaver->setFrameBufferParameters(miniscope[i]->getDeviceName(),
                                            miniscope[i]->getFrameBufferPointer(),
                                            miniscope[i]->getTimeStampBufferPointer(),
                                            miniscope[i]->getBNOBufferPointer(),
                                            miniscope[i]->getBufferSize(),
                                            miniscope[i]->getFreeFramesPointer(),
                                            miniscope[i]->getUsedFramesPointer(),
                                            miniscope[i]->getAcqFrameNumPointer());

        dataSaver->setHeadOrientationConfig(miniscope[i]->getDeviceName(), miniscope[i]->getHeadOrienataionStreamState(), miniscope[i]->getHeadOrienataionFilterState());
        dataSaver->setROI(miniscope[i]->getDeviceName(), miniscope[i]->getROI());
    }
    for (int i = 0; i < behavCam.length(); i++) {
        dataSaver->setDataCompression(behavCam[i]->getDeviceName(), behavCam[i]->getCompressionType());
        dataSaver->setFrameBufferParameters(behavCam[i]->getDeviceName(),
                                            behavCam[i]->getFrameBufferPointer(),
                                            behavCam[i]->getTimeStampBufferPointer(),
                                            nullptr,
                                            behavCam[i]->getBufferSize(),
                                            behavCam[i]->getFreeFramesPointer(),
                                            behavCam[i]->getUsedFramesPointer(),
                                            behavCam[i]->getAcqFrameNumPointer());
        dataSaver->setHeadOrientationConfig(behavCam[i]->getDeviceName(), false, false);
        dataSaver->setROI(behavCam[i]->getDeviceName(), behavCam[i]->getROI());
    }

    if (!ucBehaviorTracker.isEmpty()) {
        if (ucBehaviorTracker["enabled"].toBool(true)) {
            dataSaver->setPoseBufferParameters(behavTracker->getPoseBufferPointer(),
                                               behavTracker->getPoseFrameNumBufferPointer(),
                                               behavTracker->getPoseBufferSize(),
                                               behavTracker->getFreePosePointer(),
                                               behavTracker->getUsedPosePointer());
        }
    }

    dataSaverThread = new QThread;
    dataSaver->moveToThread(dataSaverThread);

    QObject::connect(dataSaverThread, SIGNAL (started()), dataSaver, SLOT (startRunning()));
    // TODO: setup start connections

    dataSaverThread->start();
}

void backEnd::testCodecSupport()
{
    // 비디오 기능 비활성화 플래그 추가
    bool videoFeatureEnabled = true; // 나중에 필요할 때 true로 변경

    if (!videoFeatureEnabled) {
        qWarning() << "Video features are disabled. Skipping codec support test.";
        return;
    }

    // This function will test which codecs are supported on host's machine
    cv::VideoWriter testVid;
    //testVid.open("test.avi", -1,20, cv::Size(640, 480), true); //2024.08.29 deactivate as normal
    // QVector<QString> possibleCodec({/*"DIB",*/ "MJPG", "MJ2C", "XVID", "FFV1", "DX50", "FLV1", "H264", "I420","MPEG","mp4v", /*"0000", "LAGS",*/ "ASV1", "GREY"});
    QVector<QString> possibleCodec({"DIB", "MJPG", "MJ2C", "XVID", "FFV1", "DX50", "FLV1", "H264", "I420","MPEG","mp4v", "0000", "LAGS", "ASV1", "GREY"});

    for (int i = 0; i < possibleCodec.length(); i++) {
        testVid.open(
            "test.avi", cv::VideoWriter::fourcc(
                possibleCodec[i].toStdString()[0], possibleCodec[i].toStdString()[1],
                possibleCodec[i].toStdString()[2], possibleCodec[i].toStdString()[3]
            ),
            20, cv::Size(640, 480), true
        );
        if (testVid.isOpened()) {
            m_availableCodec.append(possibleCodec[i]);
            qDebug() << "Codec" << possibleCodec[i] << "supported for color";
            testVid.release();
        }
        else
            unAvailableCodec.append(possibleCodec[i]);
    }

}

bool backEnd::checkUserConfigForIssues()
{
    if (checkForUniqueDeviceNames() == false) {
        // Need to tell user that user config has error(s)
        setUserConfigOK(false);
        emit userConfigOKChanged();
        emit showErrorMessage();
    }
    else if (checkForCompression() == false) {
        // Need to tell user that user config has error(s)
        setUserConfigOK(false);
        emit userConfigOKChanged();
        emit showErrorMessageCompression();
    }
    else {
        setUserConfigOK(true);
        emit userConfigOKChanged();
    }
    // TODO: make return do something or remove
    return true;
}

void backEnd::parseUserConfig()
{
    QJsonObject devices = m_userConfig["devices"].toObject();
    QJsonArray tempArray;
    QJsonObject tempObj;
    QStringList s;
    int count = 0;

    // Main JSON header
    researcherName = m_userConfig["researcherName"].toString();
    dataDirectory = m_userConfig["dataDirectory"].toString();
    dataStructureOrder = m_userConfig["dataStructureOrder"].toArray();
    experimentName = m_userConfig["experimentName"].toString();
    animalName = m_userConfig["animalName"].toString();

    // JSON subsections
    ucExperiment = m_userConfig["experiment"].toObject();

    // exec order 2?
    if (devices["miniscopes"].isArray()) {
        tempArray = devices["miniscopes"].toArray(); // for "quick" index iteration?
        s.clear();
        count = 0;
        for (int i=0; i < tempArray.size(); i++) {
            // make name unique
            if (s.contains(tempArray[i].toObject()["deviceName"].toString())) {
                s.append(tempArray[i].toObject()["deviceName"].toString() + QString::number(count));
                count++;
            }
            else {
                s.append(tempArray[i].toObject()["deviceName"].toString());
            }

            // get "deviceName" of last "miniscopes" attributes?
            tempObj = tempArray[i].toObject();
            tempObj["deviceName"] = s.last();
            ucMiniscopes[s.last()] = tempObj;
        }
    }
    // exec order 1?
    else if (devices["miniscopes"].isObject()) {
        s = devices["miniscopes"].toObject().keys();
        for (int i=0; i < s.length(); i++) {
            tempObj = devices["miniscopes"].toObject()[s[i]].toObject();
            tempObj["deviceName"] = s[i];
            ucMiniscopes[s[i]] = tempObj;
        }

    }
//    ucMiniscopes = devices["miniscopes"].toArray();

    if (devices["cameras"].isArray()) {
        tempArray = devices["cameras"].toArray();
        s.clear();
        count = 0;
        for (int i=0; i < tempArray.size(); i++) {
            if (s.contains(tempArray[i].toObject()["deviceName"].toString())) {
                // make name unique
                s.append(tempArray[i].toObject()["deviceName"].toString() + QString::number(count));
                count++;
            }
            else {
                s.append(tempArray[i].toObject()["deviceName"].toString());
            }
            tempObj = tempArray[i].toObject();
            tempObj["deviceName"] = s.last();
            ucBehaviorCams[s.last()] = tempObj;
        }
    }
    else if (devices["cameras"].isObject()) {
        s = devices["cameras"].toObject().keys();
        for (int i=0; i < s.length(); i++) {
            tempObj = devices["cameras"].toObject()[s[i]].toObject();
            tempObj["deviceName"] = s[i];
            // qDebug() << "DNSOSNDAIOASDNO" << tempObj; // check for tempObj
            ucBehaviorCams[s[i]] = tempObj;
        }

    }

//    ucBehaviorCams = devices["cameras"].toArray();

    ucBehaviorTracker = m_userConfig["behaviorTracker"].toObject();
    ucTraceDisplay = m_userConfig["traceDisplay"].toObject();


}

void backEnd::setupBehaviorTracker()
{
    for (int i = 0; i < behavCam.length(); i++) {
        behavTracker->setBehaviorCamBufferParameters(behavCam[i]->getDeviceName(),
                                                     behavCam[i]->getTimeStampBufferPointer(),
                                                     behavCam[i]->getFrameBufferPointer(),
                                                     behavCam[i]->getBufferSize(),
                                                     behavCam[i]->getAcqFrameNumPointer());
    }

    // Start behavior tracker thread
    behavTracker->startThread();
}

bool backEnd::checkForUniqueDeviceNames()
{
    bool repeatingDeviceName = false;
    QString tempName;
    QVector<QString> deviceNames;
    QStringList keys;

    keys = ucMiniscopes.keys();
    for (int i = 0; i < keys.length(); i++) {
        tempName = ucMiniscopes[keys[i]].toObject()["deviceName"].toString();
        if (!deviceNames.contains(tempName))
            deviceNames.append(tempName);
        else {
            repeatingDeviceName = true;
            break;
        }
    }

    keys = ucBehaviorCams.keys();
    for (int i = 0; i < keys.length(); i++) {
        tempName = ucBehaviorCams[keys[i]].toObject()["deviceName"].toString();
        if (!deviceNames.contains(tempName))
            deviceNames.append(tempName);
        else {
            repeatingDeviceName = true;
            break;
        }
    }

    if (repeatingDeviceName == true) {
        qDebug() << "Repeating Device Names!";
        return false;
    }
    else {
        return true;
    }
}

bool backEnd::checkForCompression()
{
    QString tempName;
    QStringList keys;

    keys = ucMiniscopes.keys();
    for (int i = 0; i < keys.length(); i++) {
        tempName = ucMiniscopes[keys[i]].toObject()["compression"].toString("Empty");
        if (!m_availableCodec.contains(tempName) && tempName != "Empty")
            return false;
    }

    keys = ucBehaviorCams.keys();
    for (int i = 0; i < keys.length(); i++) {
        tempName = ucBehaviorCams[keys[i]].toObject()["compression"].toString("Empty");
        if (!m_availableCodec.contains(tempName) && tempName != "Empty")
            return false;
    }
    return true;
}

void backEnd::constructUserConfigGUI()
{
    int idx;
    QStringList keys;

    // Load main control GUI
    controlPanel = new ControlPanel(this, m_userConfig);
    QObject::connect(this, SIGNAL (sendMessage(QString) ), controlPanel, SLOT( receiveMessage(QString)));

    // Make trace display
    if (!ucTraceDisplay.isEmpty()) {
        if (ucTraceDisplay["enabled"].toBool(true))
            traceDisplay = new TraceDisplayBackend(NULL, ucTraceDisplay, m_softwareStartTime);
    }

    // Make Minsicope displays
    keys = ucMiniscopes.keys();
    for (idx = 0; idx < keys.length(); idx++) {
        miniscope.append(new Miniscope(this, ucMiniscopes[keys[idx]].toObject(), m_softwareStartTime));
        QObject::connect(miniscope.last(),
                         SIGNAL (onPropertyChanged(QString, QString, QVariant)),
                         dataSaver,
                         SLOT (devicePropertyChanged(QString, QString, QVariant)));

        // Connect send and receive message to textbox in controlPanel
        QObject::connect(miniscope.last(), SIGNAL(sendMessage(QString)), controlPanel, SLOT( receiveMessage(QString)));
        QObject::connect(miniscope.last(), &Miniscope::addTraceDisplay, traceDisplay, &TraceDisplayBackend::addNewTrace);
        if (miniscope.last()->getErrors() != 0) {
            // Errors have occured in creating this object
            emit sendMessage("ERROR: " + miniscope.last()->getDeviceName() + " has error: " + QString::number(miniscope.last()->getErrors()));
        }
        else {
            miniscope.last()->setTraceDisplayStatus(traceDisplay != nullptr);
            miniscope.last()->createView();
            miniscope.last()->setupBNOTraceDisplay();
        }
    }

    // Make Behav Cam displays
    keys = ucBehaviorCams.keys();
    for (idx = 0; idx < keys.length(); idx++) {
        behavCam.append(new BehaviorCam(this, ucBehaviorCams[keys[idx]].toObject(), m_softwareStartTime));
        QObject::connect(behavCam.last(),
                         SIGNAL (onPropertyChanged(QString,QString,QVariant)),
                         dataSaver,
                         SLOT (devicePropertyChanged(QString,QString,QVariant)));

        // Connect send and receive message to textbox in controlPanel
        QObject::connect(behavCam.last(), SIGNAL(sendMessage(QString)), controlPanel, SLOT( receiveMessage(QString)));

        if (behavCam.last()->getErrors() != 0) {
            // Errors have occured in creating this object
            emit sendMessage("ERROR: " + behavCam.last()->getDeviceName() + " has error: " + QString::number(behavCam.last()->getErrors()));
        }
        else
            behavCam.last()->createView();
    }

    // Create experiment interface
    if (!ucExperiment.isEmpty()){
        // Construct experiment interface
    }

    // Make behavior tracker interface
    if (!ucBehaviorTracker.isEmpty()) {
        if (ucBehaviorTracker["enabled"].toBool(true)) {
            // Behav tracker currently is hardcoded to use first behavior camera
            QSize camRes = behavCam.first()->getResolution();

            behavTracker = new BehaviorTracker(NULL, m_userConfig, m_softwareStartTime);

            QObject::connect(behavTracker, SIGNAL(sendMessage(QString)), controlPanel, SLOT( receiveMessage(QString)));
            QObject::connect(behavTracker, &BehaviorTracker::addTraceDisplay, traceDisplay, &TraceDisplayBackend::addNewTrace);
            behavTracker->createView(camRes);
            setupBehaviorTracker();
        }
    }

    connectSnS();
}

void backEnd::testLibusb() {
#ifdef USE_USB
    int mcu_ep_out = 4;
    int mcu_ep_in  = 3;
    int mcu_device_id = 1003;
    int mcu_product_id = 9251;

    uint8_t data[1024];
    uint8_t inData[1024];
    for (int k=0 ; k < 1024; k++) {
        data[k] = k&0xFF;
        inData[k] = 0;
    }

    int actualLength = 0;

    struct libusb_device_descriptor desc;

    const struct libusb_endpoint_descriptor *epdesc;
    const struct libusb_interface_descriptor *interdesc;
    libusb_device_handle *d_h = NULL;
    libusb_device *dev;

    // Notes:
    // When using cypress:
    // Interface 2 with 0x04 endpoint for bulk seems to work.



    libusb_device **devs;
    int r;
    ssize_t cnt;

    r = libusb_init(NULL);
//    libusb_set_debug(NULL, 4);
    if (r < 0)
        qDebug() << "Issue with libusb_init.";
    else {
        cnt = libusb_get_device_list(NULL, &devs);
        if (cnt < 0){
            libusb_exit(NULL);
            qDebug() << "Issue with libusb_get_device_list.";
        }
        else {

            int i = 0, j = 0;
            uint8_t path[8];

            while ((dev = devs[i++]) != NULL) {
                // Loop through USB devices

                int r = libusb_get_device_descriptor(dev, &desc);
                if (r < 0) {
                    qDebug() << "Failed to get device descriptor.";
                }
                else if (desc.idVendor == mcu_device_id && desc.idProduct == mcu_product_id){

                    qDebug() << "VendorID: " << desc.idVendor << " | ProductID: " << desc.idProduct << " | Bus Number:" << libusb_get_bus_number(dev) << " | Device Address:" << libusb_get_device_address(dev);

                    r = libusb_get_port_numbers(dev, path, sizeof(path));
                    if (r > 0) {
                        qDebug() <<"Paths:";
                        for (j = 0; j < r; j++)
                            qDebug() << "    " << j << ": " << path[j];
                    }



                    struct libusb_config_descriptor **configDesc;
                    r = libusb_get_active_config_descriptor(dev,configDesc);

                    if (r < 0)
                        qDebug() << "Config Desc failed:" << r;
                    else {
                        int numAltSettings = configDesc[0]->interface->num_altsetting;
                        qDebug() << "Number of alt settings: " << numAltSettings;
                        interdesc = configDesc[0]->interface->altsetting;
                        for (int w=0; w < numAltSettings; w++) {
                            qDebug() << "Number of endpoints: "<< interdesc[w].bNumEndpoints;
                            for(int m=0; m < interdesc[w].bNumEndpoints; m++) {
                                epdesc = &interdesc[w].endpoint[m];
                                qDebug()<<"Descriptor Type: "<<(int)epdesc->bDescriptorType;
                                qDebug()<<"Attributes Type: "<<(int)epdesc->bmAttributes;
                                qDebug()<<"EP Address: "<<(int)epdesc->bEndpointAddress;
                            }
                        }
                        qDebug() << "HERE~!!";
                        libusb_free_config_descriptor(*configDesc);
                    }

                    r = libusb_open(dev,&d_h);

                    if (r < 0) {
                        qDebug() << "libusb_open failed. Error code is: " << r;
                    }
                    else {
                        unsigned char name[200];
                        r = libusb_get_string_descriptor_ascii(d_h, desc.iProduct, name, 200);
                        if ( r > 0) {
                            qDebug() << "iProduct Name: " << QString::fromUtf8((char *)name, r);
                        }

                        // For Cypress Bulk, use interface 2 and endpoint 0x04|LIBUSB_ENDPOINT_ IN/OUT


                        // I think windows is already setting up configuration 1 so an error in setting config is ok to skip???
                        int conf;
                        libusb_get_configuration(d_h,&conf);
                        qDebug() << "Get Config: " << conf;
                        r = libusb_set_configuration(d_h,1);
                        if (r < 0)
                            qDebug() << "set configuration issue: " << libusb_error_name(r);
                         if (1){
                            r = libusb_claim_interface(d_h,0);

                            if (r < 0)
                                qDebug() << "claim interface issue: " << r;
                            else {
                                r = libusb_set_interface_alt_setting(d_h,0,1);
                                if (r < 0)
                                    qDebug() << "set alt interface issue: " << r;
                                else {

                                    qDebug() <<  "Control Sending" << data[0] << data[1] << data[2] << data[3] << data[4];
                                    r = libusb_control_transfer(d_h,LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_INTERFACE,0,0,0,data,sizeof(data),1000);
//                                    r = libusb_interrupt_transfer(d_h,0x02,data,1024,NULL,1000);
//                                    r = libusb_bulk_transfer(d_h, mcu_ep_out|LIBUSB_ENDPOINT_OUT, data, 1024, NULL, 1000);
                                    if (r < 0)
                                        qDebug() << "Issue sending bulk transfer to device:" << r;
                                    if(1) {
                                        r = libusb_control_transfer(d_h,LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_IN,0,0,0,inData,sizeof(inData),1000);
//                                        r = libusb_interrupt_transfer(d_h, 0x01|LIBUSB_ENDPOINT_IN, inData, 1024, &actualLength, 1000);
//                                        r = libusb_bulk_transfer(d_h, mcu_ep_in|LIBUSB_ENDPOINT_IN, inData, 1024, &actualLength, 1000);
                                        if (r < 0) {
                                            qDebug() << "Receiving issue: " << libusb_error_name(r);
                                        }
                                        else
                                            qDebug() << "Receiving" << inData[0] << inData[1] << inData[2] << inData[3] << inData[4];
                                    }


                                    for (int k=0 ; k < 1024; k++) {
                                        data[k] = k&0xFF;
                                        inData[k] = 0;
                                    }

//
                                    actualLength = 1024;
                                    r = libusb_interrupt_transfer(d_h, 0x02, data, sizeof(data), NULL, 1000);
                                    qDebug() <<  "Interrupt Sending" << data[0] << data[1] << data[2] << data[3] << data[4];
                                    if (r < 0)
                                        qDebug() << "Issue sending interrupt transfer to device:" << r;
                                    if (1) {
                                        actualLength = 0;
//                                        r = libusb_control_transfer(d_h,LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_IN,0,0,0,inData,sizeof(inData),1000);
                                        r = libusb_interrupt_transfer(d_h, 0x01|LIBUSB_ENDPOINT_IN, inData, sizeof(inData), &actualLength, 1000);
//                                        r = libusb_bulk_transfer(d_h, mcu_ep_in|LIBUSB_ENDPOINT_IN, inData, 1024, &actualLength, 1000);
                                        if (r < 0) {
                                            qDebug() << "Receiving interrupt issue: " << libusb_error_name(r);
                                            qDebug() << "interrupt Receiving" << inData[0] << inData[1] << inData[2] << inData[3] << inData[4] << "inLength:" << actualLength;
                                        }
                                        else
                                            qDebug() << "interrupt Receiving" << inData[0] << inData[1] << inData[2] << inData[3] << inData[4] << "inLength:" << actualLength;
                                    }



                                    qDebug() <<  "Bulk Sending" << data[0] << data[1] << data[2] << data[3] << data[4];
//                                    r = libusb_control_transfer(d_h,LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_INTERFACE,0,0,0,data,sizeof(data),1000);
//                                    r = libusb_interrupt_transfer(d_h,0x02,data,1024,NULL,1000);
                                    r = libusb_bulk_transfer(d_h, 0x04, data, 1024, NULL, 1000);
                                    if (r < 0)
                                        qDebug() << "Issue sending bulk transfer to device:" << r;
                                    if (1) {
//                                        r = libusb_control_transfer(d_h,LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_IN,0,0,0,inData,sizeof(inData),1000);
//                                        r = libusb_interrupt_transfer(d_h, 0x01|LIBUSB_ENDPOINT_IN, inData, 1024, &actualLength, 1000);
                                        r = libusb_bulk_transfer(d_h, 0x83, inData, 1024, &actualLength, 1000);
                                        if (r < 0) {
                                            qDebug() << "Bulk Receiving issue: " << libusb_error_name(r);
                                            qDebug() << "Bulk Receiving" << inData[0] << inData[1] << inData[2] << inData[3] << inData[4] << "inLength:" << actualLength;
                                        }
                                        else
                                            qDebug() << "Bulk Receiving" << inData[0] << inData[1] << inData[2] << inData[3] << inData[4] << "inLength:" << actualLength;
                                    }
                                }
                            }
                            libusb_release_interface(d_h, 0);
                        }

                    }
                    libusb_close(d_h);
                }
                qDebug() << " ";
            }
            // ---------------

            libusb_free_device_list(devs, 1);
            libusb_exit(NULL);
        }
    }
#endif
}
