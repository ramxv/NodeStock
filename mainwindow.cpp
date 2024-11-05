#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QBuffer>
#include <QList>
#include <QVariantList>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>

int indexActual = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("NodeStock");

    ptrCategoria = new Categoria();
    ptrProductos = new Productos();
    ptrProveedores = new Proveedores();

    dbnodestock = QSqlDatabase::addDatabase("QSQLITE");
    dbnodestock.setDatabaseName("/home/ram/Documents/university/os/parcial2/NodeStock/nodeStock.db");

    if (!dbnodestock.open()) {
        qDebug() << "Conexión Fallida";
    } else {
        qDebug() << "Conexión Exitosa";
        loadProveedoresTable();
        loadCategoriesInTable();
        loadCategories();
        loadProductos();
        loadProveedores();
        loadItems();
        cargarInventario();
    }
}

MainWindow::~MainWindow()
{
    dbnodestock.close();
    delete ui;
}

//// ========================> PRODUCTOS <======================== ////

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

        prod_insert.bindValue(":nombre", formatearPalabra(ui->prodNombre->text()));
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

void MainWindow::loadCategoriesInTable(){
    QSqlQueryModel *model = new QSqlQueryModel(this);
    QSqlQuery select_cat("SELECT * FROM categorias");

    if (select_cat.exec()) {
        model->setQuery(std::move(select_cat));
        ui->tableViewCategoria->setModel(model);

        model->setHeaderData(0, Qt::Horizontal, "ID");
        model->setHeaderData(1, Qt::Horizontal, "Nombre de la categoría");
        model->setHeaderData(2, Qt::Horizontal, "Descripción");

        ui->tableViewCategoria->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tableViewCategoria->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        ui->tableViewCategoria->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    } else {
        qDebug() << "Selección Fallida:" << select_cat.lastError().text();
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

void MainWindow::loadProveedores()
{
    ui->comboBoxProveedor->clear();
    QSqlQuery query("SELECT id, nombre FROM proveedores");
    while (query.next()) {
        int id = query.value("id").toInt();
        QString name = query.value("nombre").toString();
        ui->comboBoxProveedor->addItem(name, id);
    }
}

void MainWindow::loadProductos()
{
    ui->comboBoxProducto->clear();
    QSqlQuery query("SELECT id, nombre FROM productos");
    while (query.next()) {
        int id = query.value("id").toInt();
        QString name = query.value("nombre").toString();
        ui->comboBoxProducto->addItem(name, id);
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

void MainWindow::on_prodImprimir_clicked()
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

    // Step 3: Create HTML content with the current display data
    QString htmlContent;
    htmlContent += "<html><head><style>";
    htmlContent += "body { font-family: Arial, sans-serif; }";
    htmlContent += "h1 { text-align: center; font-size: 18px; }";
    htmlContent += "table { width: 100%; border-collapse: collapse; }";
    htmlContent += "th, td { border: 1px solid black; padding: 8px; text-align: left; font-size: 18px; }";
    htmlContent += "th { background-color: #f2f2f2; font-weight: bold; }";
    htmlContent += "</style></head><body>";

    htmlContent += "<h1>Producto</h1>";

    // Display the current values
    htmlContent += "<table>";
    htmlContent += "<tr><th>Nombre</th><td>" + ui->prodNombre->text() + "</td></tr>";
    htmlContent += "<tr><th>Descripción</th><td>" + ui->prodDescripcion->text() + "</td></tr>";
    htmlContent += "<tr><th>Precio</th><td>" + QString::number(ui->precioDoubleSpin->value()) + "</td></tr>";
    htmlContent += "<tr><th>Categoría</th><td>" + ui->categoriaCbx->currentText() + "</td></tr>";
    htmlContent += "<tr><th>Fecha de Registro</th><td>" + ui->fechaRegistro->date().toString("yyyy-MM-dd") + "</td></tr>";

    // Check if there is an image
    if (!imageData.isEmpty()) {
        // Convert image data to Base64 and embed it in HTML
        QString base64Image = QString::fromLatin1(imageData.toBase64().data());
        htmlContent += "<tr><th>Imagen</th><td><img src='data:image/png;base64," + base64Image + "' width='200' /></td></tr>";
    } else {
        htmlContent += "<tr><th>Imagen</th><td>No image available</td></tr>";
    }

    htmlContent += "</table></body></html>";

    // Step 4: Load HTML content into a QTextDocument and print it
    QTextDocument document;
    document.setHtml(htmlContent);
    document.setPageSize(printer.pageRect(QPrinter::Point).size());
    document.print(&printer);
}

//// ========================> INVENTARIO <======================== ////

void MainWindow::cargarInventario()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    QSqlQuery select_cat(dbnodestock);
    select_cat.prepare("SELECT id, cantidad, fecha_entrada FROM inventario");

    if (select_cat.exec()) {
        model->setQuery(std::move(select_cat));
        ui->tableViewInventario->setModel(model);

        model->setHeaderData(0, Qt::Horizontal, "ID");
        model->setHeaderData(1, Qt::Horizontal, "Cantidad");
        model->setHeaderData(2, Qt::Horizontal, "Fecha Entrada");

        ui->tableViewInventario->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tableViewInventario->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        ui->tableViewInventario->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

        cargarDato(indexActual);
    } else {
        qDebug() << "Selección Fallida:" << select_cat.lastError().text();
    }
}

void MainWindow::cargarDato(int index)
{
    if (index >= 0 && index <= ui->tableViewInventario->model()->rowCount()) {
        QModelIndex idIndex = ui->tableViewInventario->model()->index(index, 0);
        QString id = ui->tableViewInventario->model()->data(idIndex).toString();

        qDebug() << id << " id dentro";

        if (!dbnodestock.isOpen() && !dbnodestock.open()) {
            qDebug() << "Error: No se pudo abrir la base de datos.";
            return;
        }

        QSqlQuery QueryInsertData(dbnodestock);
        QueryInsertData.prepare("SELECT * FROM inventario WHERE id = :id");
        QueryInsertData.bindValue(":id", id);

        if (!QueryInsertData.exec()) {
            qDebug() << "Error al obtener los datos en la tabla:" << QueryInsertData.lastError().text();
        } else {
            if (QueryInsertData.next()) {
                QString id = QueryInsertData.value(0).toString();
                QString producto = QueryInsertData.value(1).toString();
                QString proveedor = QueryInsertData.value(2).toString();
                QString cantidad = QueryInsertData.value(3).toString();

                ui->id->setText(id);
                ui->comboBoxProducto->setCurrentText(producto);
                ui->comboBoxProveedor->setCurrentText(proveedor);
                ui->spinBoxCantidad->setDisplayIntegerBase(cantidad.toInt());
                ui->fechaEntrada->setDate(QDate::currentDate());

                qDebug() << "Datos cargados";
            } else {
                qDebug() << "No existen datos";
            }
        }
    }
}

void MainWindow::on_guardarBtn_clicked()
{
    QPushButton *button = ui->guardarBtn;

    if (button->text() == "Agregar Nuevo") {
        button->setText("Guardar");
        button->setStyleSheet("background-color: green; color: white;");

        ui->id->clear();
        ui->comboBoxProducto->clear();
        ui->comboBoxProveedor->clear();
        ui->spinBoxCantidad->clear();
        ui->fechaEntrada->clear();
    } else {
        QString producto = ui->comboBoxProducto->currentText();
        QString proveedor = ui->comboBoxProveedor->currentText();
        QString cantidad = ui->spinBoxCantidad->text();
        QString fecha_entrada = ui->fechaEntrada->date().toString("yyyy-MM-dd");

        if (producto.isEmpty() || proveedor.isEmpty() || cantidad.isEmpty() || fecha_entrada.isEmpty()) {
            QMessageBox::warning(this, "Campos vacíos", "Todos los campos deben estar llenos");
            return;
        }

        button->setText("Agregar Nuevo");
        button->setStyleSheet("background-color: white; color: black;");

        if (!dbnodestock.isOpen() && !dbnodestock.open()) {
            qDebug() << "Error: No se pudo abrir la base de datos.";
            return;
        }

        QSqlQuery QueryInsertData(dbnodestock);
        QueryInsertData.prepare("INSERT INTO inventario (producto_id, proveedor_id, cantidad, fecha_entrada) "
                                "VALUES (:producto, :proveedor, :cantidad, :fecha_entrada)");

        QueryInsertData.bindValue(":producto", producto);
        QueryInsertData.bindValue(":proveedor", proveedor);
        QueryInsertData.bindValue(":cantidad", cantidad);
        QueryInsertData.bindValue(":fecha_entrada", fecha_entrada);

        if (!QueryInsertData.exec()) {
            qDebug() << "Error al insertar en la tabla:" << QueryInsertData.lastError().text();
        } else {
            qDebug() << "Inserción exitosa.";
            cargarInventario();
        }
    }
}

void MainWindow::on_actualizarBtn_clicked()
{
    if (cambiarBotonGuardar()) {
        cargarDato(indexActual);
        return;
    }

    QString id = ui->id->text();
    QString producto = ui->comboBoxProducto->currentText();
    QString proveedor = ui->comboBoxProveedor->currentText();
    QString cantidad = ui->spinBoxCantidad->text();
    QString fecha_entrada = ui->fechaEntrada->date().toString("yyyy-MM-dd");

    if (id.isEmpty() || producto.isEmpty() || proveedor.isEmpty() || cantidad.isEmpty() || fecha_entrada.isEmpty()) {
        QMessageBox::warning(this, "Campos vacíos", "Todos los campos deben estar llenos");
        return;
    }

    if (QMessageBox::question(this, "Actualizar Registro", "¿Estás seguro de que quieres actualizar el registro con " + id + "?") == QMessageBox::Yes) {
        if (!dbnodestock.isOpen() && !dbnodestock.open()) {
            qDebug() << "Error: No se pudo abrir la base de datos.";
            return;
        }

        QSqlQuery QueryUpdateData(dbnodestock);
        QueryUpdateData.prepare("UPDATE inventario SET "
                                "producto_id = :producto, "
                                "proveedor_id = :proveedor, "
                                "cantidad = :cantidad, "
                                "fecha_entrada = :fecha_entrada "
                                "WHERE id = :id");

        QueryUpdateData.bindValue(":producto", producto);
        QueryUpdateData.bindValue(":proveedor", proveedor);
        QueryUpdateData.bindValue(":cantidad", cantidad);
        QueryUpdateData.bindValue(":fecha_entrada", fecha_entrada);
        QueryUpdateData.bindValue(":id", id);

        if (!QueryUpdateData.exec()) {
            qDebug() << "Error al actualizar en la tabla:" << QueryUpdateData.lastError().text();
        } else {
            qDebug() << "Actualización exitosa.";
            cargarInventario();
        }
    }
}

void MainWindow::on_eliminarBtn_clicked()
{
    if (cambiarBotonGuardar()) {
        cargarDato(indexActual);
        return;
    }

    QString id = ui->id->text();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Campos vacíos", "el id no puede estar vacío");
        return;
    }

    if (QMessageBox::question(this, "Eliminar Registro", "¿Estás seguro de que quieres eliminar el registro con " + id + "?") == QMessageBox::Yes) {
        if (!dbnodestock.isOpen() && !dbnodestock.open()) {
            qDebug() << "Error: No se pudo abrir la base de datos.";
            return;
        }

        QSqlQuery QueryDeleteData(dbnodestock);
        QueryDeleteData.prepare("DELETE FROM inventario WHERE id = :id");
        QueryDeleteData.bindValue(":id", id);

        if (!QueryDeleteData.exec()) {
            qDebug() << "Error al eliminar en la tabla:" << QueryDeleteData.lastError().text();
        } else {
            qDebug() << "Eliminación exitosa.";
            cargarInventario();
            cargarDato(indexActual - 1);
        }
    }
}

void MainWindow::on_impresionBtn_clicked()
{
    if (QMessageBox::question(this, "Impresión", "¿Deseas imprimir el registro?") == QMessageBox::Yes) {
        QPrinter printer(QPrinter::HighResolution);
        printer.setPageSize(QPageSize(QPageSize::A4));
        printer.setFullPage(true);

        QPageLayout pageLayout;
        pageLayout.setOrientation(QPageLayout::Portrait);
        printer.setPageLayout(pageLayout);

        QPrintDialog printDialog(&printer, this);
        if (printDialog.exec() != QDialog::Accepted) {
            return;
        }

        QString htmlContent;
        htmlContent += "<html><head><style>";
        htmlContent += "body { font-family: Arial, sans-serif; }";
        htmlContent += "h1 { text-align: center; font-size: 18px; }";
        htmlContent += "table { width: 100%; border-collapse: collapse; }";
        htmlContent += "th, td { border: 1px solid black; padding: 8px; text-align: left; font-size: 18px; }";
        htmlContent += "th { background-color: #f2f2f2; font-weight: bold; }";
        htmlContent += "</style></head><body>";
        htmlContent += "<h1>Listado de Inventario</h1><table><tr><th>ID</th><th>Producto ID</th><th>Proveedor ID</th><th>Cantidad</th><th>Fecha Entrada</th></tr>";

        if (!dbnodestock.isOpen() && !dbnodestock.open()) {
            qDebug() << "Error: No se pudo abrir la base de datos.";
            return;
        }

        QSqlQuery QuerySelectData(dbnodestock);
        QuerySelectData.prepare("SELECT * FROM inventario");

        if (!QuerySelectData.exec()) {
            qDebug() << "Error al obtener los datos en la tabla:" << QuerySelectData.lastError().text();
        } else {
            while (QuerySelectData.next()) {
                int id = QuerySelectData.value(0).toInt();
                QString productoId = QuerySelectData.value(1).toString();
                QString proveedorId = QuerySelectData.value(2).toString();
                QString cantidad = QuerySelectData.value(3).toString();
                QString fechaEntrada = QuerySelectData.value(4).toString();

                htmlContent += "<tr>";
                htmlContent += "<td>" + QString::number(id) + "</td>";
                htmlContent += "<td>" + productoId + "</td>";
                htmlContent += "<td>" + proveedorId + "</td>";
                htmlContent += "<td>" + cantidad + "</td>";
                htmlContent += "<td>" + fechaEntrada + "</td>";
                htmlContent += "</tr>";
            }
        }

        htmlContent += "</table></body></html>";

        QTextDocument document;
        document.setHtml(htmlContent);
        document.setPageSize(printer.pageRect(QPrinter::Point).size());
        document.print(&printer);
    }
}

bool MainWindow::cambiarBotonGuardar()
{
    QPushButton *button = ui->guardarBtn;
    if (button->text() == "Guardar") {
        button->setText("Agregar Nuevo");
        button->setStyleSheet("background-color: white; color: black;");
        return true;
    }
    return false;
}

//// ========================> CATEGORÍA <======================== ////

void MainWindow::on_guardarCatBtn_clicked()
{
    QSqlQuery insert_cat;
    QSqlQueryModel *modal = new QSqlQueryModel;

    insert_cat.prepare("INSERT INTO categorias (nombre, descripcion) VALUES (:nombre, :descripcion)");
    insert_cat.bindValue(":nombre", ui->nombreLineEdit->text());
    insert_cat.bindValue(":descripcion", ui->descLineEditCat->text());

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
            ui->tableViewCategoria->setModel(modal);

            modal->setHeaderData(0, Qt::Horizontal, "ID");
            modal->setHeaderData(1, Qt::Horizontal, "Nombre de la categoría");
            modal->setHeaderData(2, Qt::Horizontal, "Descripción");

            ui->tableViewCategoria->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
            ui->tableViewCategoria->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            ui->tableViewCategoria->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        } else {
            qDebug() << "Selección Fallida: " << select_cat.lastError().text();
        }
    }
}

void MainWindow::on_eliminarCatBtn_clicked()
{
    ui->nombreLineEdit->setDisabled(true);
    ui->descLineEditCat->setDisabled(true);
    ui->nombreLineEdit->clear();
    ui->descLineEditCat->clear();

    QItemSelectionModel *selectionModel = ui->tableViewCategoria->selectionModel();
    if(!selectionModel->hasSelection()){
        QMessageBox::warning(this, "Advertencia", "Porfavor, seccione la fila que desea eliminar.");
        return;
    }

    QModelIndex selectedIndex = selectionModel->currentIndex();
    int row = selectedIndex.row();
    int id = ui->tableViewCategoria->model()->index(row, 0).data().toInt();

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
            ui->tableViewCategoria->setModel(model);

            model->setHeaderData(0, Qt::Horizontal, "ID");
            model->setHeaderData(1, Qt::Horizontal, "Nombre de la categoría");
            model->setHeaderData(2, Qt::Horizontal, "Descripción");

            ui->tableViewCategoria->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
            ui->tableViewCategoria->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            ui->tableViewCategoria->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        }

    }

}


void MainWindow::on_actualizarCatBtn_clicked()
{
    if (currentCatId == -1) {
        QMessageBox::warning(this, "Advertencia", "Realice Doble-Click y cargue una categoría antes de actualizar.");
        return;
    }

    QMessageBox::StandardButton respuesta;
    respuesta = QMessageBox::question(this, "Confirmar Actualización", "¿Desea guardar los cambios realizados en la categoría?", QMessageBox::Yes | QMessageBox::No);

    if (respuesta == QMessageBox::Yes) {
        QString newName = ui->nombreLineEdit->text();
        QString newDescription = ui->descLineEditCat->text();

        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE categorias SET nombre = :nombre, descripcion = :descripcion WHERE id = :id");
        updateQuery.bindValue(":nombre", newName);
        updateQuery.bindValue(":descripcion", newDescription);
        updateQuery.bindValue(":id", currentCatId);

        if (!updateQuery.exec()) {
            qDebug() << "Update Failed: " << updateQuery.lastError().text();
            QMessageBox::critical(this, "Error", "Error al actualizar la categoría: " + updateQuery.lastError().text());
        } else {
            QMessageBox::information(this, "Éxito", "La categoría se ha actualizado exitosamente.");

            QSqlQueryModel *model = new QSqlQueryModel(this);
            model->setQuery("SELECT * FROM categorias");
            ui->tableViewCategoria->setModel(model);

            ui->nombreLineEdit->clear();
            ui->descLineEditCat->clear();
            currentCatId = -1;
        }
    }
}

void MainWindow::on_tableViewCategoria_doubleClicked(const QModelIndex &index)
{
    int row = index.row();
    int id = ui->tableViewCategoria->model()->index(row, 0).data().toInt();

    currentCatId = id;

    QString currentName = ui->tableViewCategoria->model()->index(row, 1).data().toString();
    QString currentDescription = ui->tableViewCategoria->model()->index(row, 2).data().toString();

    ui->nombreLineEdit->setText(currentName);
    ui->descLineEditCat->setText(currentDescription);
}

void MainWindow::on_impresionCatBtn_clicked()
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

//// ========================> PROVEEDORES <======================== ////

void MainWindow::loadProveedoresTable()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    QSqlQuery select_proveedor("SELECT nombre, telefono, email FROM proveedores");

    if (select_proveedor.exec()) {
        model->setQuery(std::move(select_proveedor));
        ui->tableViewProveedores->setModel(model);

        model->setHeaderData(0, Qt::Horizontal, "Nombre del Proveedor");
        model->setHeaderData(1, Qt::Horizontal, "Teléfono del Proveedor");
        model->setHeaderData(2, Qt::Horizontal, "Email del Proveedor");

        ui->tableViewProveedores->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tableViewProveedores->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        ui->tableViewProveedores->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    } else {
        qDebug() << "Selección Fallida:" << select_proveedor.lastError().text();
    }
}
