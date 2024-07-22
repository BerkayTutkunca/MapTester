#include "mainwindow.h"
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QStandardPaths>
#include <QRegularExpression>

#include "ui_mainwindow.h"

enum class SensorStateEnum : uint8_t {
    Walking = 1,
    Car = 2,
    Digging = 3,
    On = 4,
    Off = 5,
    Ignore = 6,
    Alarm = 7
};


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,udpSocket(new QUdpSocket(this))
    ,serialPort(new QSerialPort(this))
    ,isConnected(false) // isConnected değişkenini başlatma

{

    ui->setupUi(this);

    // QLineEdit'ların görünürlüğünü başlangıçta false yapma

    ui->ipLineEdit->setVisible(false);
    ui->portLineEdit->setVisible(false);
    ui->baudRateLineEdit->setVisible(false);
    ui->portNameLineEdit->setVisible(false);
    ui->groupBox_2->setVisible(false);
    ui->connectPushButton->setVisible(false);

    loadSettings();
    initializeComboBox2();  // comboBox_2'yi başlatmak için çağırın
    setupValidators();


}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::loadSettings()
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString configFilePath = QDir::cleanPath(desktopPath + "/test.ini");

    if (!QFile::exists(configFilePath)) {
        qDebug() << "Config file does not exist at" << configFilePath;
        return;
    }

    QSettings settings(configFilePath, QSettings::IniFormat);
    settings.beginGroup("Sensor");
    QStringList sensorKeys = settings.childKeys();
    QVector<int> sensorIds;
    for (const QString &key : sensorKeys) {
        sensorIds.append(settings.value(key).toInt());
    }
    settings.endGroup();

    std::sort(sensorIds.begin(), sensorIds.end());

    ui->comboBox->clear();
    for (int sensorId : sensorIds) {
        ui->comboBox->addItem(QString::number(sensorId), QVariant(sensorId));
    }

}

void MainWindow::initializeComboBox2()
{
    ui->comboBox_2->clear();

    // Sabit değerleri enum class'dan QString karşılıkları ile doldur
    ui->comboBox_2->addItem("Yürüme", static_cast<int>(SensorStateEnum::Walking));
    ui->comboBox_2->addItem("Araç", static_cast<int>(SensorStateEnum::Car));
    ui->comboBox_2->addItem("Kazma", static_cast<int>(SensorStateEnum::Digging));
    ui->comboBox_2->addItem("Açık", static_cast<int>(SensorStateEnum::On));
    ui->comboBox_2->addItem("Kapalı", static_cast<int>(SensorStateEnum::Off));
}

void MainWindow::setupValidators()
{
    // IP adresi için validatör
    QRegularExpression ipRegex(R"(^((25[0-5]|2[0-4]\d|[0-1]?\d?\d)(\.(25[0-5]|2[0-4]\d|[0-1]?\d?\d)){3})$)");
    ipValidator = new QRegularExpressionValidator(ipRegex, this);
    ui->ipLineEdit->setValidator(ipValidator);

    // Port için validatör
    portValidator = new QIntValidator(1, 65535, this);
    ui->portLineEdit->setValidator(portValidator);
}




void MainWindow::on_udpCheckBox_toggled(bool checked)
{

    if (checked) {
        ui->serialCheckBox->setChecked(false); // Serial checkbox'ı devre dışı bırak
    }
    // UDP ile ilgili giriş alanlarını ayarla
    ui->ipLineEdit->setVisible(checked);
    ui->portLineEdit->setVisible(checked);
    ui->groupBox_2->setVisible(checked);

    ui->connectPushButton->setVisible(checked || ui->serialCheckBox->isChecked());
    // Serial ile ilgili alanları gizle
    ui->baudRateLineEdit->setVisible(false);
    ui->portNameLineEdit->setVisible(false);
    // Eğer serial checkbox devre dışı ise, alanlarını temizle
    if (!ui->serialCheckBox->isChecked()) {
        ui->baudRateLineEdit->clear();
        ui->portNameLineEdit->clear();
    }
}

void MainWindow::on_serialCheckBox_toggled(bool checked)
{
    if (checked) {
        ui->udpCheckBox->setChecked(false); // UDP checkbox'ı devre dışı bırak
    }
    // Serial ile ilgili giriş alanlarını ayarla
    ui->baudRateLineEdit->setVisible(checked);
    ui->portNameLineEdit->setVisible(checked);
    ui->groupBox_2->setVisible(checked);

    ui->connectPushButton->setVisible(checked || ui->serialCheckBox->isChecked());

    // UDP ile ilgili alanları gizle
    ui->ipLineEdit->setVisible(false);
    ui->portLineEdit->setVisible(false);
    // Eğer UDP checkbox devre dışı ise, alanlarını temizle
    if (!ui->udpCheckBox->isChecked()) {
        ui->ipLineEdit->clear();
        ui->portLineEdit->clear();
    }
}

void MainWindow::on_connectPushButton_clicked()
{
    if (!isConnected) {
        ui->connectPushButton->setEnabled(false);

        if (ui->udpCheckBox->isChecked()) {
            setupNetworkConnection();
        } else if (ui->serialCheckBox->isChecked()) {
            setupSerialConnection();
        }

        ui->connectPushButton->setText("Bağlantıyı Kes");
        isConnected = true;
        ui->connectPushButton->setEnabled(true);
    } else {
        // Disconnect logic
        if (ui->udpCheckBox->isChecked()) {
            udpSocket->abort();
            qDebug() << "Disconnected from UDP";
        } else if (ui->serialCheckBox->isChecked()) {
            if (serialPort->isOpen()) {
                serialPort->close();
                qDebug() << "Disconnected from Serial Port";
            }
        }

        ui->connectPushButton->setText("Bağlan");
        isConnected = false;
    }
}



void MainWindow::on_sendPushButton_clicked()
{
    int xValue = ui->comboBox->currentData().toInt();  // comboBox'dan seçili int değeri al
    int yValue = ui->comboBox_2->currentData().toInt();  // comboBox_2'den seçili int değeri al

    QByteArray data = QString("%1,%2\n").arg(xValue).arg(yValue).toUtf8();

    if (ui->udpCheckBox->isChecked()) {
        // UDP üzerinden veri gönder
        if (udpSocket->state() == QAbstractSocket::ConnectedState) {
            udpSocket->write(data); // Veriyi gönder
            qDebug() << "Sending data via UDP:" << data;
        } else {
            qDebug() << "UDP socket is not connected.";
        }
    } else if (ui->serialCheckBox->isChecked()) {
        // Serial Port üzerinden veri gönder
        if (serialPort->isOpen()) {
            serialPort->write(data); // Veriyi gönder
            qDebug() << "Sending data via Serial Port:" << data;
        } else {
            qDebug() << "Serial port is not open.";
        }
    }
}



void MainWindow::setupNetworkConnection()
{
    udpSocket->abort(); // Önceki bağlantıları sonlandır
    udpSocket->connectToHost(ui->ipLineEdit->text(), ui->portLineEdit->text().toInt());

    if (udpSocket->state() != QAbstractSocket::ConnectedState) {
        ui->connectPushButton->setText("Connect");
        isConnected = false;
        ui->connectPushButton->setEnabled(true);
    }

    qDebug() << "Attempting to connect via UDP to" << ui->ipLineEdit->text() << ":" << ui->portLineEdit->text();

}

void MainWindow::setupSerialConnection()
{
    QString ad = ui->portNameLineEdit->text();
    serialPort->setPortName(ad);

    // Baud rate değerini QSerialPort::BaudRate enum tipinden doğrudan ayarla
    QSerialPort::BaudRate baudRate = QSerialPort::Baud9600;  // Varsayılan değer

    // QLineEdit'dan alınan değere göre baudRate ayarlama
    int inputBaudRate = ui->baudRateLineEdit->text().toInt();
    switch (inputBaudRate) {
    case 9600:
        baudRate = QSerialPort::Baud9600;
        break;
    case 19200:
        baudRate = QSerialPort::Baud19200;
        break;
    case 38400:
        baudRate = QSerialPort::Baud38400;
        break;
    case 57600:
        baudRate = QSerialPort::Baud57600;
        break;
    case 115200:
        baudRate = QSerialPort::Baud115200;
        break;
    // Diğer desteklenen baud rate değerleri için ek case blokları eklenebilir
    default:
        qDebug() << "Unsupported baud rate, using default 9600";
        baudRate = QSerialPort::Baud9600;
        break;
    }

    serialPort->setBaudRate(baudRate);

    if (serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "Connected via Serial Port" << ui->portNameLineEdit->text() << "at baud rate" << inputBaudRate;
    } else {
        qDebug() << "Failed to open serial port:" << serialPort->errorString();
        ui->connectPushButton->setText("Bağlan");
        isConnected = false;
        ui->connectPushButton->setEnabled(true);  // Bağlantı başarısız olursa butonu tekrar etkinleştir
    }
}
