#include "proveedores2.h"
#include "ui_proveedores2.h"
#include "mainwindow.h"
#include <QtSql/QSqlDatabase>
#include <QSqlTableModel>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QMessageBox>


Proveedores2::Proveedores2(MainWindow *mainWindow, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Proveedores2)
    , mainWindow(mainWindow) // Store the pointer to MainWindow

{
    ui->setupUi(this);


    DB_Connection = QSqlDatabase::addDatabase("QSQLITE");
    DB_Connection.setDatabaseName("C:/Users/hiryo/OneDrive/Documentos/NodeStock/release/database1.db3"); // Use the correct absolute path

    qDebug() << QCoreApplication::applicationDirPath();
    if (!DB_Connection.open()) {
        // If the database fails to open, show the error message
        qDebug() << "data base not connected";
        qDebug() << "error: " << DB_Connection.lastError();
    } else {
        // If the connection is successful, display a success message
        qDebug() << "data base connected";
    }


    if (!DB_Connection.open()) {
        qDebug() << "Error: No se pudo conectar a la base de datos.";
        return;
    }

    cargarProveedores();
}

Proveedores2::~Proveedores2()
{
    delete ui;
}


/*
 *
 * cargar la tabla de proveedores
 *
 */

void Proveedores2::cargarProveedores() {
    QSqlTableModel *model = new QSqlTableModel(this);
    model->setTable("proveedores");
    model->select();  // Carga todos los datos de la tabla "proveedores"

    ui->tableViewProveedores->setModel(model);
    ui->tableViewProveedores_2->setModel(model);
    ui->tableViewProveedores_3->setModel(model);


}




/*
 *
 *navegacion entre las pantallas
 *
 */


void Proveedores2::on_pushButtonActualizar_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}
void Proveedores2::on_pushButtonEliminar_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);

}


void Proveedores2::on_pushButtonVolver_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void Proveedores2::on_pushButtonVolver_2_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}




void Proveedores2::on_proveedorMenu_clicked()
{
    regresar();
}


void Proveedores2::on_proveedorMenu_2_clicked()
{
    regresar();
}

void Proveedores2::on_proveedorMenu_3_clicked()
{
    regresar();

}


void Proveedores2::regresar() {
    if (mainWindow) {
        mainWindow->show();
        this->close();
    }
}

void Proveedores2::on_pushButtonGuardar_clicked()
{
    QString nombre = ui->textEditNombre->toPlainText();
    QString contacto = ui->textEditContacto->toPlainText();
    QString telefono = ui->textEditTelefono->toPlainText();
    QString email = ui->textEditEmail->toPlainText();

    if (nombre.isEmpty() || contacto.isEmpty() || telefono.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "campos vacios", "Todos los campos deben estar llenos");
        return;
    }

    // asegura la conexion a la db
    if (!DB_Connection.isOpen() && !DB_Connection.open()) {
        qDebug() << "Error: No se pudo abrir la base de datos.";
        return;
    }


    QSqlQuery QueryInsertData(DB_Connection);
    QueryInsertData.prepare("INSERT INTO proveedores (nombre, contacto, telefono, email) "
                            "VALUES (:nombre, :contacto, :telefono, :email)");

    QueryInsertData.bindValue(":nombre", nombre);
    QueryInsertData.bindValue(":contacto", contacto);
    QueryInsertData.bindValue(":telefono", telefono);
    QueryInsertData.bindValue(":email", email);

    // Execute the insert query
    if (!QueryInsertData.exec()) {
        qDebug() << "Error al insertar en la tabla:" << QueryInsertData.lastError().text();
        DB_Connection.rollback();
    } else {
        QMessageBox::information(this, "Insercion Exitosa", "Los datos se han insertado correctamente.");
        qDebug() << "Inserción exitosa.";
        DB_Connection.commit(); //hace rcommit si todo sale bien
        cargarProveedores();


        ui->textEditNombre->clear();
        ui->textEditContacto->clear();
        ui->textEditTelefono->clear();
        ui->textEditEmail->clear();


    }

    DB_Connection.close();
}





/*
 *
 *ACrtualizar consultas
 *
 */

void Proveedores2::on_pushButtonActualizar_2_clicked()
{
    QString id = ui->textEditId->toPlainText();
    QString nombre = ui->textEditNombre_2->toPlainText();
    QString contacto = ui->textEditContacto_2->toPlainText();
    QString telefono = ui->textEditTelefono_2->toPlainText();
    QString email = ui->textEditEmail_2->toPlainText();

    if (id.isEmpty() || nombre.isEmpty() || contacto.isEmpty() || telefono.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "campos vacios", "Todos los campos deben estar llenos");
        return;
    }

    // asegura la conexion a la db
    if (!DB_Connection.isOpen() && !DB_Connection.open()) {
        qDebug() << "Error: No se pudo abrir la base de datos.";
        return;
    }


    QSqlQuery QueryUpdateData(DB_Connection);
    QueryUpdateData.prepare("UPDATE proveedores SET "
                            "nombre = :nombre, "
                            "contacto = :contacto, "
                            "telefono = :telefono, "
                            "email = :email "
                            "WHERE id = :id");


    QueryUpdateData.bindValue(":nombre", nombre);
    QueryUpdateData.bindValue(":contacto", contacto);
    QueryUpdateData.bindValue(":telefono", telefono);
    QueryUpdateData.bindValue(":email", email);
    QueryUpdateData.bindValue(":id", id);

    // Execute the insert query
    if (!QueryUpdateData.exec()) {
        qDebug() << "Error al actualizar en la tabla:" << QueryUpdateData.lastError().text();
        DB_Connection.rollback();
    } else {
        QMessageBox::information(this, "Actualización Exitosa", "Los datos se han actualizado correctamente.");
        qDebug() << "actualizacion exitosa.";
        DB_Connection.commit(); //hace rcommit si todo sale bien
        cargarProveedores();

        ui->textEditId->clear();
        ui->textEditNombre_2->clear();
        ui->textEditContacto_2->clear();
        ui->textEditTelefono_2->clear();
        ui->textEditEmail_2->clear();


    }

    DB_Connection.close();
}





/*
 *
 *  eliminar proveedor
 *
 */

void Proveedores2::on_pushButtonEliminar_2_clicked()
{
    QString id = ui->textEditId_2->toPlainText();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "ID vacío", "Por favor, ingresa un ID válido para eliminar el proveedor.");
        return;
    }

    if (!DB_Connection.isOpen() && !DB_Connection.open()) {
        qDebug() << "Error: No se pudo abrir la base de datos.";
        return;
    }

    QSqlQuery QueryDeleteData(DB_Connection);
    QueryDeleteData.prepare("DELETE FROM proveedores WHERE id = :id");
    QueryDeleteData.bindValue(":id", id);

    // Ejecutar la consulta de eliminación
    if (!QueryDeleteData.exec()) {
        qDebug() << "Error al eliminar el proveedor: " << QueryDeleteData.lastError();
        QMessageBox::warning(this, "Error de Eliminación", "No se pudo eliminar el proveedor. Por favor, verifica el ID.");
    } else {
        QMessageBox::information(this, "Eliminación Exitosa", "El proveedor se ha eliminado correctamente.");
        qDebug() << "Eliminación exitosa.";
        cargarProveedores();
    }

    ui->textEditId_2->clear();

    DB_Connection.close();
}




