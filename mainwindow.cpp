#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QBuffer>
#include <QList>
#include <QVariantList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ptrCategoria = new Categoria();
    ptrProductos = new Productos();
    ptrProveedores = new Proveedores();

    QSqlDatabase dbnodestock = QSqlDatabase::addDatabase("QSQLITE");
    dbnodestock.setDatabaseName("/home/ram/Documents/university/os/parcial2/NodeStock/nodeStock.db");

    if (!dbnodestock.open()) {
        qDebug() << "Conexión Fallida";
    } else {
        qDebug() << "Conexión Exitosa";

        loadCategories();
        loadItems();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clearFields()
{
    ui->prodNombre->clear();              // Clear QLineEdit for product name
    ui->prodDescripcion->clear();          // Clear QLineEdit for description
    ui->precioDoubleSpin->setValue(0.0);   // Reset QDoubleSpinBox to 0.0 for price
    ui->categoriaCbx->setCurrentIndex(-1); // Set QComboBox to no selection
    ui->fechaRegistro->setDate(QDate::currentDate());  // Set QDateEdit to today's date
    ui->imageContainerLbl->clear();        // Clear QLabel used for image display
    imageData.clear();                     // Clear the QByteArray holding the image data
}

void MainWindow::on_prodRegistrar_clicked()
{
    if (isNewProductMode) {
        clearFields();
        ui->prodRegistrar->setText("Guardar Producto");
        isNewProductMode = false;
    } else {

        if (ui->prodNombre->text().isEmpty() ||
            ui->prodDescripcion->text().isEmpty() ||
            ui->precioDoubleSpin->value() == 0.0 ||
            ui->categoriaCbx->currentIndex() == -1) {

            QMessageBox::warning(this, "Campos Vacíos", "Por favor, complete todos los campos obligatorios antes de registrar el producto.");
            return;
        }

        QSqlQuery prod_insert;
        prod_insert.prepare("INSERT INTO productos (nombre, descripcion, precio, categoria_id, imagen) "
                            "VALUES (:nombre, :descripcion, :precio, :categoria_id, :imagen)");

        prod_insert.bindValue(":nombre", ui->prodNombre->text());
        prod_insert.bindValue(":descripcion", ui->prodDescripcion->text());
        prod_insert.bindValue(":precio", ui->precioDoubleSpin->value());

        int categoria_id = ui->categoriaCbx->currentData().toInt();
        prod_insert.bindValue(":categoria_id", categoria_id);
        prod_insert.bindValue(":imagen", imageData);

        if (!prod_insert.exec()) {
            QMessageBox::critical(this, "Error", "No se realizó el registro: " + prod_insert.lastError().text());
        } else {
            QMessageBox::information(this, "Éxito", "El producto se ha registrado con éxito.");
            loadItems();
            ui->prodRegistrar->setText("Nuevo Producto");
            isNewProductMode = true;
        }
    }
}

void MainWindow::on_imageBtn_clicked()
{
    QString imagePath = QFileDialog::getOpenFileName(this, "Seleccionar Imagen", "", "Images (*.png *.xpm *.jpg)");
    if (!imagePath.isEmpty()) {
        QImage image(imagePath);

        ui->imageContainerLbl->setPixmap(QPixmap::fromImage(image).scaled(ui->imageContainerLbl->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

        QBuffer buffer(&imageData);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
    }
}

void MainWindow::loadItems()
{
    items.clear();

    QSqlQuery itemsQuery("SELECT id, nombre, descripcion, precio, categoria_id, imagen, fecha_creacion FROM productos");
    while (itemsQuery.next()) {
        QVariantMap item;
        item["id"] = itemsQuery.value("id").toInt();
        item["nombre"] = itemsQuery.value("nombre").toString();
        item["descripcion"] = itemsQuery.value("descripcion").toString();
        item["precio"] = itemsQuery.value("precio").toDouble();
        item["categoria_id"] = itemsQuery.value("categoria_id").toInt();
        item["fecha_creacion"] = itemsQuery.value("fecha_creacion").toDate();
        item["imagen"] = itemsQuery.value("imagen").toByteArray();

        items.append(item);
    }

    if (!items.isEmpty()) {
        currentIndex = 0;
        displayItems(currentIndex);
    } else {
        QMessageBox::information(this, "No Data", "No items found in the database.");
    }
}

void MainWindow::displayItems(int index)
{
    if (index >= 0 && index < items.size()) {
        qDebug() << index;
        ui->prodNombre->setText(items[index]["nombre"].toString());
        ui->prodDescripcion->setText(items[index]["descripcion"].toString());
        ui->precioDoubleSpin->setValue(items[index]["precio"].toDouble());

        int categoryId = items[index]["categoria_id"].toInt();
        int categoryIndex = ui->categoriaCbx->findData(categoryId);
        if (categoryIndex != -1) {
            ui->categoriaCbx->setCurrentIndex(categoryIndex);
        }

        ui->fechaRegistro->setDate(items[index]["fecha_creacion"].toDate());

        // Set imageData to the current item's image data
        imageData = items[index]["imagen"].toByteArray();

        // Display the image
        QPixmap pixmap;
        if (pixmap.loadFromData(imageData)) {
            ui->imageContainerLbl->setPixmap(pixmap.scaled(ui->imageContainerLbl->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            ui->imageContainerLbl->clear();
        }
    }
}

void MainWindow::on_prodAnterior_clicked()
{
    if (currentIndex > 0) {
        currentIndex--;
        displayItems(currentIndex);
    } else {
        QMessageBox::information(this, "Inicio", "Ya se encuentra en el primer producto.");
    }
}

void MainWindow::on_prodSiguiente_clicked()
{
    if (currentIndex < items.size() - 1) {
        currentIndex++;
        displayItems(currentIndex);
    } else {
        QMessageBox::information(this, "Final", "Ya se encuentra en el último producto.");
    }
}

void MainWindow::loadCategories()
{
    ui->categoriaCbx->clear();
    QSqlQuery query("SELECT id, nombre FROM categorias");
    while (query.next()) {
        int id = query.value("id").toInt();
        QString name = query.value("nombre").toString();
        ui->categoriaCbx->addItem(name, id);
    }
}

void MainWindow::on_prodActualizar_clicked()
{
    if (currentIndex < 0 || currentIndex >= items.size()) {
        QMessageBox::warning(this, "Error", "No ha seleccionado ningún producto.");
        return;
    }

    int itemId = items[currentIndex]["id"].toInt();

    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE productos SET nombre = :nombre, descripcion = :descripcion, "
                        "precio = :precio, categoria_id = :categoria_id, "
                        "imagen = :imagen, fecha_creacion = :fecha_creacion "
                        "WHERE id = :id");

    updateQuery.bindValue(":nombre", ui->prodNombre->text());
    updateQuery.bindValue(":descripcion", ui->prodDescripcion->text());
    updateQuery.bindValue(":precio", ui->precioDoubleSpin->value());
    updateQuery.bindValue(":categoria_id", ui->categoriaCbx->currentData().toInt());

    // Ensure we use the current imageData, whether updated or not
    updateQuery.bindValue(":imagen", imageData);

    updateQuery.bindValue(":fecha_creacion", ui->fechaRegistro->date());
    updateQuery.bindValue(":id", itemId);

    if (!updateQuery.exec()) {
        QMessageBox::critical(this, "Error", "Error al actualizar el producto: " + updateQuery.lastError().text());
    } else {
        QMessageBox::information(this, "Éxito", "El producto se ha actualizado con éxito.");
        loadItems();
        displayItems(currentIndex);
    }
}


void MainWindow::on_prodEliminar_clicked()
{
    if (currentIndex < 0 || currentIndex >= items.size()) {
        QMessageBox::warning(this, "Error", "No hay ningún producto seleccionado.");
        return;
    }

    int itemId = items[currentIndex]["id"].toInt();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmación", "Está seguro de eliminar el producto?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery deleteQuery;
        deleteQuery.prepare("DELETE FROM productos WHERE id = :id");
        deleteQuery.bindValue(":id", itemId);

        if (!deleteQuery.exec()) {
            QMessageBox::critical(this, "Error", "No se pudo eliminar el producto: " + deleteQuery.lastError().text());
        } else {
            QMessageBox::information(this, "Éxito", "El producto se ha elimado con éxito.");

            loadItems();
            if (currentIndex >= items.size()) {
                currentIndex = items.size() - 1;
            }
            displayItems(currentIndex);
        }
    }
}


void MainWindow::on_buscarBtn_clicked()
{
    QSqlQuery buscar_productos;
    buscar_productos.prepare("SELECT nombre, descripcion, precio FROM productos WHERE nombre = :nombre");

    buscar_productos.bindValue(":nombre", formatearPalabra( ui->buscarLineEdit->text()));

    if(!buscar_productos.exec()){
      QMessageBox::critical(this, "Error", "No se encontró el producto: " + buscar_productos.lastError().text());
    } else {
        QSqlQueryModel *model = new QSqlQueryModel(this);
        model->setQuery(std::move(buscar_productos));
        ui->prodTableview->setModel(model);

        model->setHeaderData(0, Qt::Horizontal, "Nombre");
        model->setHeaderData(1, Qt::Horizontal, "Descripción");
        model->setHeaderData(2, Qt::Horizontal, "Precio");

        ui->prodTableview->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->prodTableview->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        ui->prodTableview->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    }
}

QString MainWindow::formatearPalabra(const QString& s)
{
    QStringList parts = s.split(' ', Qt::SkipEmptyParts);
    for (int i = 0; i < parts.size(); ++i)
        parts[i].replace(0, 1, parts[i][0].toUpper());

    return parts.join(" ");
}

