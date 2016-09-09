#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include <QSignalMapper>
#include <QWidget>
#include <QPushButton>
#include <iostream>
#include "view.h"

view* img_view;

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->pushButton->setText("View 3");
    ui->pushButton_2->setText("View 2");
    ui->pushButton_3->setText("View 1");

    QSignalMapper *signalMapper = new QSignalMapper(this);
    signalMapper->setMapping(ui->pushButton, 2);
    signalMapper->setMapping(ui->pushButton_2, 1);
    signalMapper->setMapping(ui->pushButton_3, 0);

    connect(ui->pushButton, SIGNAL(clicked()), signalMapper, SLOT(map()));
    connect(ui->pushButton_2, SIGNAL(clicked()), signalMapper, SLOT(map()));
    connect(ui->pushButton_3, SIGNAL(clicked()), signalMapper, SLOT(map()));

    img_view = new view;

    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(digitClicked(int)));

    connect(this, SIGNAL(selectView(int)), img_view, SLOT(selectView(int)));
}

void MainWindow::digitClicked(int button)
{
    emit selectView(button);
}


MainWindow::~MainWindow()
{
    delete ui;
}
