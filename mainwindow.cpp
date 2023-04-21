#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "opengl/drawwindow.h"

#include <QSurfaceFormat>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("LBVH");
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::startClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startClicked(){
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setVersion(4,3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    DrawWindow *window = new DrawWindow();
    window->resize(640,480);
    window->show();

    window->setAnimating(true);
}
