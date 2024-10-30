#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ptrCategoria = new Categoria();
    ptrProductos = new Productos();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    ptrCategoria->show();
}


void MainWindow::on_pushButton_5_clicked()
{
    ptrProductos->show();
}

