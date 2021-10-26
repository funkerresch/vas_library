#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../../../rwacreator/rwaheadtrackerconnect.h"
#include "../../../HeadtrackerConnectAndroid/RwaHeadtrackerConnectAndroid/keepawakehelper.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    RwaHeadtrackerConnect *instance = RwaHeadtrackerConnect::getInstance();
    instance->startBluetoothScanning();
    //KeepAwakeHelper helper;
}

MainWindow::~MainWindow()
{
    delete ui;
}

