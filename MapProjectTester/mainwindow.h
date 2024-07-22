#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCheckBox>
#include <QLineEdit>
#include <QIntValidator>
#include <QRegularExpressionValidator>
#include <QHBoxLayout>
#include <QUdpSocket>
#include <QSerialPort>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_udpCheckBox_toggled(bool checked);
    void on_serialCheckBox_toggled(bool checked);

    void on_connectPushButton_clicked();

    void on_sendPushButton_clicked();

private:
    Ui::MainWindow *ui;
    void loadSettings();
    void initializeComboBox2();
    void setupValidators();

    QLineEdit *ipLineEdit;
    QLineEdit *portLineEdit;
    QLineEdit *baudRateLineEdit;
    QLineEdit *portNameLineEdit;
    QIntValidator *portValidator;
    QRegularExpressionValidator *ipValidator;
    QHBoxLayout *groupBoxLayout;


    QUdpSocket *udpSocket;
    QSerialPort *serialPort;

    void setupNetworkConnection();

    void setupSerialConnection();

    bool isConnected;
};

#endif // MAINWINDOW_H
