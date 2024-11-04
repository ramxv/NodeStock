#include "proveedores.h"
#include "ui_proveedores.h"

Proveedores::Proveedores(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Proveedores)
{
    ui->setupUi(this);

    QSqlDatabase dbnodestock = QSqlDatabase::addDatabase("QSQLITE");
    dbnodestock.setDatabaseName("/home/ram/Documents/university/os/parcial2/NodeStock/nodestock.db");

    if (!dbnodestock.open()) {
        qDebug() << "Conexión Fallida";
    } else {
        qDebug() << "Conexión Exitosa";

        QSqlQueryModel *model = new QSqlQueryModel(this);
        QSqlQuery select_cat("SELECT * FROM proveedores");

        if (select_cat.exec()) {
            model->setQuery(std::move(select_cat));
            ui->tableViewProveedores->setModel(model);

            model->setHeaderData(0, Qt::Horizontal, "ID");
            model->setHeaderData(1, Qt::Horizontal, "Nombre");
            model->setHeaderData(2, Qt::Horizontal, "Contacto");
            model->setHeaderData(3, Qt::Horizontal, "Telefono");
            model->setHeaderData(4, Qt::Horizontal, "email");

            ui->tableViewProveedores->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
            ui->tableViewProveedores->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            ui->tableViewProveedores->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
            ui->tableViewProveedores->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
            ui->tableViewProveedores->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
        } else {
            qDebug() << "Selección Fallida:" << select_cat.lastError().text();
        }
    }
}

Proveedores::~Proveedores()
{
    delete ui;
}

void Proveedores::on_guardarProveedorBtn_clicked()
{

}


void Proveedores::on_actualizarProveedorBtn_clicked()
{

}

void Proveedores::on_eliminarProveedorBtn_clicked()
{

}

