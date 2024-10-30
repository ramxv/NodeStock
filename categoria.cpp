#include "categoria.h"
#include "ui_categoria.h"

#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>

Categoria::Categoria(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Categoria)
{
    ui->setupUi(this);
    setWindowTitle("Categorías");

    QSqlDatabase dbnodestock = QSqlDatabase::addDatabase("QSQLITE");
    dbnodestock.setDatabaseName("/home/ram/Documents/university/os/parcial2/NodeStock/nodestock.db");

    if (!dbnodestock.open()) {
        qDebug() << "Conexión Fallida";
    } else {
        qDebug() << "Conexión Exitosa";

        QSqlQueryModel *model = new QSqlQueryModel(this);
        QSqlQuery select_cat("SELECT * FROM categorias");

        if (select_cat.exec()) {
            model->setQuery(std::move(select_cat));
            ui->tableView->setModel(model);

            model->setHeaderData(0, Qt::Horizontal, "ID");
            model->setHeaderData(1, Qt::Horizontal, "Nombre de la categoría");
            model->setHeaderData(2, Qt::Horizontal, "Descripción");

            ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
            ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        } else {
            qDebug() << "Selección Fallida:" << select_cat.lastError().text();
        }
    }
}

Categoria::~Categoria()
{
    delete ui;
}

void Categoria::on_registrarBtn_clicked()
{
    QSqlQuery insert_cat;
    QSqlQueryModel *modal = new QSqlQueryModel;

    insert_cat.prepare("INSERT INTO categorias (nombre, descripcion) VALUES (:nombre, :descripcion)");
    insert_cat.bindValue(":nombre", ui->nombreLineEdit->text());
    insert_cat.bindValue(":descripcion", ui->descLineEdit->text());

    if (!insert_cat.exec()) {
        qDebug() << "Inserción de Datos Fallida: " << insert_cat.lastError().text();
        QMessageBox::critical(this, "Error", "Data insertion failed: " + insert_cat.lastError().text());
    } else {
        QMessageBox::information(this, "Success", "Inserción de Datos Exitosa.");
        qDebug() << "Inserción de Datos Exitosa.";

        QSqlQuery select_cat;
        select_cat.prepare("SELECT * FROM categorias");

        if (select_cat.exec()) {
            modal->setQuery(QSqlQuery("SELECT * FROM categorias"));
            ui->tableView->setModel(modal);

            modal->setHeaderData(0, Qt::Horizontal, "ID");
            modal->setHeaderData(1, Qt::Horizontal, "Nombre de la categoría");
            modal->setHeaderData(2, Qt::Horizontal, "Descripción");

            ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
            ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        } else {
            qDebug() << "Selección Fallida: " << select_cat.lastError().text();
        }
    }
}

void Categoria::on_eliminarBtn_clicked()
{
    ui->nombreLineEdit->setDisabled(true);
    ui->descLineEdit->setDisabled(true);
    ui->nombreLineEdit->clear();
    ui->descLineEdit->clear();

    QItemSelectionModel *selectionModel = ui->tableView->selectionModel();
    if(!selectionModel->hasSelection()){
        QMessageBox::warning(this, "Advertencia", "Porfavor, seccione la fila que desea eliminar.");
        return;
    }

    QModelIndex selectedIndex = selectionModel->currentIndex();
    int row = selectedIndex.row();
    int id = ui->tableView->model()->index(row, 0).data().toInt();

    QMessageBox::StandardButton respuesta;
    respuesta = QMessageBox::question(this, "Eliminar", "Esta seguro de eliminar la categoría?", QMessageBox::Yes | QMessageBox::No);

    if(respuesta == QMessageBox::Yes){
        QSqlQuery delete_cat;
        delete_cat.prepare("DELETE FROM categorias WHERE id = :id");
        delete_cat.bindValue(":id", id);
        if(!delete_cat.exec()){
            qDebug() << "La eliminacion Falló: " + delete_cat.lastError().text();
            QMessageBox::critical(this,"Error","Error al eliminar la categoria: " + delete_cat.lastError().text());
        } else {
            qDebug() << "Se eliminó con éxito la categoría";
            QMessageBox::information(this,"Éxito","La categoría ha sido elimada con éxito.");

            QSqlQueryModel *model = new QSqlQueryModel(this);
            model->setQuery("SELECT * FROM categorias");
            ui->tableView->setModel(model);

            model->setHeaderData(0, Qt::Horizontal, "ID");
            model->setHeaderData(1, Qt::Horizontal, "Nombre de la categoría");
            model->setHeaderData(2, Qt::Horizontal, "Descripción");

            ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
            ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        }

    }

}


void Categoria::on_actualizarBtn_clicked()
{
    if (currentId == -1) {
        QMessageBox::warning(this, "Advertencia", "Realice Doble-Click y cargue una categoría antes de actualizar.");
        return;
    }

    QMessageBox::StandardButton respuesta;
    respuesta = QMessageBox::question(this, "Confirmar Actualización", "¿Desea guardar los cambios realizados en la categoría?", QMessageBox::Yes | QMessageBox::No);

    if (respuesta == QMessageBox::Yes) {
        QString newName = ui->nombreLineEdit->text();
        QString newDescription = ui->descLineEdit->text();

        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE categorias SET nombre = :nombre, descripcion = :descripcion WHERE id = :id");
        updateQuery.bindValue(":nombre", newName);
        updateQuery.bindValue(":descripcion", newDescription);
        updateQuery.bindValue(":id", currentId);

        if (!updateQuery.exec()) {
            qDebug() << "Update Failed: " << updateQuery.lastError().text();
            QMessageBox::critical(this, "Error", "Error al actualizar la categoría: " + updateQuery.lastError().text());
        } else {
            QMessageBox::information(this, "Éxito", "La categoría se ha actualizado exitosamente.");

            QSqlQueryModel *model = new QSqlQueryModel(this);
            model->setQuery("SELECT * FROM categorias");
            ui->tableView->setModel(model);

            ui->nombreLineEdit->clear();
            ui->descLineEdit->clear();
            currentId = -1;
        }
    }
}

void Categoria::on_tableView_doubleClicked(const QModelIndex &index)
{
    int row = index.row();
    int id = ui->tableView->model()->index(row, 0).data().toInt();

    currentId = id;

    QString currentName = ui->tableView->model()->index(row, 1).data().toString();
    QString currentDescription = ui->tableView->model()->index(row, 2).data().toString();

    ui->nombreLineEdit->setText(currentName);
    ui->descLineEdit->setText(currentDescription);
}

void Categoria::on_printBtn_clicked()
{
    // Step 1: Set up the printer
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setFullPage(true);

    QPageLayout pageLayout;
    pageLayout.setOrientation(QPageLayout::Portrait);
    printer.setPageLayout(pageLayout);

    // Step 2: Show the print dialog
    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() != QDialog::Accepted) {
        return; // Exit if the user cancels the dialog
    }

    // Step 3: Create HTML content with CSS for styling
    QString htmlContent;
    htmlContent += "<html><head><style>";
    htmlContent += "body { font-family: Arial, sans-serif; }";
    htmlContent += "h1 { text-align: center; font-size: 18px; }";  // Title font size
    htmlContent += "table { width: 100%; border-collapse: collapse; }";
    htmlContent += "th, td { border: 1px solid black; padding: 8px; text-align: left; font-size: 18px; }";  // Matching title font size
    htmlContent += "th { background-color: #f2f2f2; font-weight: bold; }";
    htmlContent += "</style></head><body>";

    // Add a title
    htmlContent += "<h1>Listado de Categorías</h1>";

    // Step 4: Add table headers
    htmlContent += "<table>";
    htmlContent += "<tr><th>ID</th><th>Nombre</th><th>Descripción</th></tr>";

    // Step 5: Query the database and add each row as an HTML table row
    QSqlQuery query("SELECT id, nombre, descripcion FROM categorias");
    while (query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        QString descripcion = query.value(2).toString();

        htmlContent += "<tr>";
        htmlContent += "<td>" + QString::number(id) + "</td>";
        htmlContent += "<td>" + nombre + "</td>";
        htmlContent += "<td>" + descripcion + "</td>";
        htmlContent += "</tr>";
    }

    htmlContent += "</table></body></html>";

    // Step 6: Load HTML content into a QTextDocument
    QTextDocument document;
    document.setHtml(htmlContent);

    // Set the document's size to match the printer's page size
    document.setPageSize(printer.pageRect(QPrinter::Point).size());

    // Step 7: Print the document
    document.print(&printer);
}
