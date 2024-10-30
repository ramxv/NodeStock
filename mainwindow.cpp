#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ptrCategoria = new Categoria();
    ptrProductos = new Productos();
    ptrProveedores = new Proveedores();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_categoriaBtn_clicked()
{
    ptrCategoria->show();
}


void MainWindow::on_productosBtn_clicked()
{
    ptrProductos->show();
}


void MainWindow::on_proveedoresBtn_clicked()
{
    ptrProveedores->show();
}


void MainWindow::on_tabWidget_tabBarClicked(int index)
{

}


void MainWindow::on_tabWidget_tabCloseRequested(int index)
{

}

