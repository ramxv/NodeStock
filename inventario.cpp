#include "inventario.h"
#include "ui_inventario.h"

Inventario::Inventario(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Inventario)
{
    ui->setupUi(this);
}

Inventario::~Inventario()
{
    delete ui;
}
