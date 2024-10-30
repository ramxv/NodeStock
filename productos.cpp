#include "productos.h"
#include "ui_productos.h"

Productos::Productos(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Productos)
{
    ui->setupUi(this);
}

Productos::~Productos()
{
    delete ui;
}
