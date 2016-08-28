#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mazeview.h"

#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //QGLFormat format;
    //format.setDepth(true);

    MazeView* mazeView = new MazeView();

    QGLFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 2);
    //format.setProfile(QSurfaceFormat::CoreProfile);
    //mazeView->setFormat(format);
    mazeView->setFormat(format);

    ui->contentFrame->layout()->addWidget(mazeView);

    //QTimer::singleShot(1000, this, SLOT(showFullScreen()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
