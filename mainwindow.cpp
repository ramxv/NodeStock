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

    qDebug() << QSqlDatabase::drivers();


    // Simplified database connection with error handling

        dbnodestock = QSqlDatabase::addDatabase("QMYSQL");
        dbnodestock.setHostName("192.168.19.153");  // Fedora server IP
        dbnodestock.setDatabaseName("nodestock");
        dbnodestock.setUserName("fedoraserver");
        dbnodestock.setPassword("F3dora!xvpassword");
        dbnodestock.setPort(3306);


    if (!dbnodestock.open()) {
        qDebug() << "Conexión Fallida";
        QMessageBox::critical(this, "Database Error", "Failed to connect to the database.");
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
    ui->prodNombre->clear();               // Clear QLineEdit for product name
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
        prod_insert.bindValue(":categoria_id", ui->categoriaCbx->currentData().toInt());
        prod_insert.bindValue(":imagen", imageData);

        if (!prod_insert.exec()) {
            QMessageBox::critical(this, "Error", "No se realizó el registro: " + prod_insert.lastError().text());
        } else {
            QMessageBox::information(this, "Éxito", "El producto se ha registrado con éxito.");
            loadItems();  // Refresh the list of items after insertion
            ui->prodRegistrar->setText("Nuevo Producto");
            isNewProductMode = true;
        }
    }
}

void MainWindow::on_imageBtn_clicked()
{
    QString imagePath = QFileDialog::getOpenFileName(this, "Seleccionar Imagen", "", "Images (*.png *.jpg *.jpeg)");
    if (!imagePath.isEmpty()) {
        QImage image(imagePath);
        if (image.isNull()) {
            QMessageBox::warning(this, "Image Error", "Failed to load image.");
            return;
        }

        // Display image in QLabel and save to QByteArray
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
        ui->prodNombre->setText(items[index]["nombre"].toString());
        ui->prodDescripcion->setText(items[index]["descripcion"].toString());
        ui->precioDoubleSpin->setValue(items[index]["precio"].toDouble());

        int categoryId = items[index]["categoria_id"].toInt();
        int categoryIndex = ui->categoriaCbx->findData(categoryId);
        if (categoryIndex != -1) {
            ui->categoriaCbx->setCurrentIndex(categoryIndex);
        }

        ui->fechaRegistro->setDate(items[index]["fecha_creacion"].toDate());

        // Set image data and display it
        imageData = items[index]["imagen"].toByteArray();
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
    QSqlQuery itemsQuery("SELECT id, nombre FROM productos");
    while (itemsQuery.next()) {
        int id = itemsQuery.value("id").toInt();
        QString name = itemsQuery.value("nombre").toString();
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
    updateQuery.bindValue(":imagen", imageData);
    updateQuery.bindValue(":fecha_creacion", ui->fechaRegistro->date());
    updateQuery.bindValue(":id", itemId);

    if (!updateQuery.exec()) {
        QMessageBox::critical(this, "Error", "Error al actualizar el producto: " + updateQuery.lastError().text());
    } else {
        QMessageBox::information(this, "Éxito", "El producto se ha actualizado con éxito.");
        loadItems();
        displayItems(currentIndex);  // Refresh display with updated product details
    }
}


void MainWindow::on_prodEliminar_clicked()
{
    if (currentIndex < 0 || currentIndex >= items.size()) {
        QMessageBox::warning(this, "Error", "No hay ningún producto seleccionado.");
        return;
    }

    int itemId = items[currentIndex]["id"].toInt();
    auto reply = QMessageBox::question(this, "Confirmación", "Está seguro de eliminar el producto?",
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery deleteQuery;
        deleteQuery.prepare("DELETE FROM productos WHERE id = :id");
        deleteQuery.bindValue(":id", itemId);

        if (!deleteQuery.exec()) {
            QMessageBox::critical(this, "Error", "No se pudo eliminar el producto: " + deleteQuery.lastError().text());
        } else {
            QMessageBox::information(this, "Éxito", "El producto se ha eliminado con éxito.");
            loadItems();
            currentIndex = qMin(currentIndex, items.size() - 1);  // Adjust index to avoid out-of-bounds access
            if (!items.isEmpty()) {
                displayItems(currentIndex);
            } else {
                clearFields();
            }
        }
    }
}



void MainWindow::on_buscarBtn_clicked()
{
    QSqlQuery buscar_productos;
    buscar_productos.prepare("SELECT nombre, descripcion, precio FROM productos WHERE nombre LIKE :nombre");
    buscar_productos.bindValue(":nombre", "%" + formatearPalabra(ui->buscarLineEdit->text()) + "%");

    if (!buscar_productos.exec()) {
        QMessageBox::critical(this, "Error", "No se encontró el producto: " + buscar_productos.lastError().text());
    } else {
        QSqlQueryModel *model = new QSqlQueryModel(this);
        model->setQuery(std::move(buscar_productos));
        ui->prodTableview->setModel(model);

        model->setHeaderData(0, Qt::Horizontal, "Nombre");
        model->setHeaderData(1, Qt::Horizontal, "Descripción");
        model->setHeaderData(2, Qt::Horizontal, "Precio");

        // Adjust column widths
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

    htmlContent += "<h1>Producto</h1><table>";
    htmlContent += "<tr><th>Nombre</th><td>" + ui->prodNombre->text() + "</td></tr>";
    htmlContent += "<tr><th>Descripción</th><td>" + ui->prodDescripcion->text() + "</td></tr>";
    htmlContent += "<tr><th>Precio</th><td>" + QString::number(ui->precioDoubleSpin->value()) + "</td></tr>";
    htmlContent += "<tr><th>Categoría</th><td>" + ui->categoriaCbx->currentText() + "</td></tr>";
    htmlContent += "<tr><th>Fecha de Registro</th><td>" + ui->fechaRegistro->date().toString("yyyy-MM-dd") + "</td></tr>";

    if (!imageData.isEmpty()) {
        QString base64Image = QString::fromLatin1(imageData.toBase64().data());
        htmlContent += "<tr><th>Imagen</th><td><img src='data:image/png;base64," + base64Image + "' width='200' /></td></tr>";
    } else {
        htmlContent += "<tr><th>Imagen</th><td>No image available</td></tr>";
    }

    htmlContent += "</table></body></html>";

    QTextDocument document;
    document.setHtml(htmlContent);
    document.setPageSize(printer.pageRect(QPrinter::Point).size());
    document.print(&printer);
}


//// ========================> INVENTARIO <======================== ////
/// \brief MainWindow::siguiente
///
void MainWindow::clearInventoryFields()
{
    ui->id->clear();
    ui->comboBoxProducto->setCurrentIndex(0);
    ui->comboBoxProveedor->setCurrentIndex(0);
    ui->spinBoxCantidad->clear();
    ui->fechaEntrada->setDate(QDate::currentDate());
}

bool MainWindow::validateInventoryFields()
{
    if (ui->comboBoxProducto->currentIndex() == -1 || ui->comboBoxProveedor->currentIndex() == -1 || ui->spinBoxCantidad->value() == 0) {
        QMessageBox::warning(this, "Campos vacíos", "Todos los campos deben estar llenos.");
        return false;
    }
    return true;
}


void MainWindow::siguiente()
{
    cambiarBotonGuardar();
    if (indexActual < ui->tableViewInventario->model()->rowCount() - 1) {
        indexActual++;
        cargarDato(indexActual);
    } else {
        QMessageBox::information(this, "Fin", "Ya se encuentra en el último registro.");
    }
}

void MainWindow::anterior()
{
    cambiarBotonGuardar();
    if (indexActual > 0) {
        indexActual--;
        cargarDato(indexActual);
    } else {
        QMessageBox::information(this, "Inicio", "Ya se encuentra en el primer registro.");
    }
}


void MainWindow::cargarInventario()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    QSqlQuery select_inventario("SELECT i.id, i.cantidad, i.fecha_entrada, p.nombre AS nombre_producto, p2.nombre AS nombre_proveedor "
                                "FROM inventario i "
                                "INNER JOIN productos p ON p.id = i.producto_id "
                                "INNER JOIN proveedores p2 ON p2.id = i.proveedor_id");

    if (select_inventario.exec()) {
        model->setQuery(std::move(select_inventario));
        ui->tableViewInventario->setModel(model);

        // Setting custom headers
        model->setHeaderData(0, Qt::Horizontal, "ID");
        model->setHeaderData(1, Qt::Horizontal, "Cantidad");
        model->setHeaderData(2, Qt::Horizontal, "Fecha Entrada");
        model->setHeaderData(3, Qt::Horizontal, "Producto");
        model->setHeaderData(4, Qt::Horizontal, "Proveedor");

        // Resize columns
        ui->tableViewInventario->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tableViewInventario->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->tableViewInventario->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        ui->tableViewInventario->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
        ui->tableViewInventario->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);

        // Load the initial record into the form fields
        indexActual = 0;
        cargarDato(indexActual);
    } else {
        QMessageBox::critical(this, "Error", "No se pudo cargar el inventario: " + select_inventario.lastError().text());
    }
}

void MainWindow::cargarDato(int index)
{
    if (index < 0 || index >= ui->tableViewInventario->model()->rowCount()) return;

    QModelIndex idIndex = ui->tableViewInventario->model()->index(index, 0);
    QString id = ui->tableViewInventario->model()->data(idIndex).toString();

    if (!dbnodestock.isOpen() && !dbnodestock.open()) {
        qDebug() << "Error: No se pudo abrir la base de datos.";
        return;
    }

    QSqlQuery QueryInsertData(dbnodestock);
    QueryInsertData.prepare("SELECT * FROM inventario WHERE id = :id");
    QueryInsertData.bindValue(":id", id);

    if (!QueryInsertData.exec()) {
        qDebug() << "Error al obtener los datos en la tabla:" << QueryInsertData.lastError().text();
    } else if (QueryInsertData.next()) {
        ui->id->setText(QueryInsertData.value(0).toString());
        ui->comboBoxProducto->setCurrentIndex(ui->comboBoxProducto->findData(QueryInsertData.value(1).toInt()));
        ui->comboBoxProveedor->setCurrentIndex(ui->comboBoxProveedor->findData(QueryInsertData.value(2).toInt()));
        ui->spinBoxCantidad->setValue(QueryInsertData.value(3).toInt());
        ui->fechaEntrada->setDate(QueryInsertData.value(4).toDate());
        qDebug() << "Datos cargados";
    } else {
        qDebug() << "No existen datos";
    }
}


void MainWindow::on_guardarBtn_clicked()
{
    QPushButton *button = ui->guardarBtn;
    if (button->text() == "Agregar Nuevo") {
        button->setText("Guardar");
        clearInventoryFields();
    } else {
        if (!validateInventoryFields()) return;

        QString producto = ui->comboBoxProducto->currentData().toString();
        QString proveedor = ui->comboBoxProveedor->currentData().toString();
        QString cantidad = ui->spinBoxCantidad->text();
        QString fecha_entrada = ui->fechaEntrada->date().toString("yyyy-MM-dd");

        if (!dbnodestock.isOpen() && !dbnodestock.open()) {
            qDebug() << "Error: No se pudo abrir la base de datos.";
            return;
        }

        if (!dbnodestock.transaction()) {
            qDebug() << "Error al iniciar la transacción:" << dbnodestock.lastError().text();
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
            dbnodestock.rollback();
        } else {
            dbnodestock.commit();
            cargarInventario();  // Refresh inventory data
            button->setText("Agregar Nuevo");
        }
    }
}

void MainWindow::on_actualizarBtn_clicked()
{
    if (indexActual < 0 || indexActual >= ui->tableViewInventario->model()->rowCount()) {
        QMessageBox::warning(this, "Error", "Seleccione un registro para actualizar.");
        return;
    }

    if (!validateInventoryFields()) return;

    QString id = ui->id->text();
    QString producto = ui->comboBoxProducto->currentData().toString();
    QString proveedor = ui->comboBoxProveedor->currentData().toString();
    QString cantidad = ui->spinBoxCantidad->text();
    QString fecha_entrada = ui->fechaEntrada->date().toString("yyyy-MM-dd");

    if (!dbnodestock.isOpen() && !dbnodestock.open()) {
        qDebug() << "Error: No se pudo abrir la base de datos.";
        return;
    }

    if (!dbnodestock.transaction()) {
        qDebug() << "Error al iniciar la transacción:" << dbnodestock.lastError().text();
        return;
    }

    QSqlQuery QueryUpdateData(dbnodestock);
    QueryUpdateData.prepare("UPDATE inventario SET producto_id = :producto, proveedor_id = :proveedor, "
                            "cantidad = :cantidad, fecha_entrada = :fecha_entrada WHERE id = :id");
    QueryUpdateData.bindValue(":producto", producto);
    QueryUpdateData.bindValue(":proveedor", proveedor);
    QueryUpdateData.bindValue(":cantidad", cantidad);
    QueryUpdateData.bindValue(":fecha_entrada", fecha_entrada);
    QueryUpdateData.bindValue(":id", id);

    if (!QueryUpdateData.exec()) {
        qDebug() << "Error al actualizar en la tabla:" << QueryUpdateData.lastError().text();
        dbnodestock.rollback();
    } else {
        dbnodestock.commit();
        cargarInventario();
    }
}

void MainWindow::on_eliminarBtn_clicked()
{
    if (indexActual < 0 || indexActual >= ui->tableViewInventario->model()->rowCount()) {
        QMessageBox::warning(this, "Advertencia", "Seleccione un registro para eliminar.");
        return;
    }

    QString id = ui->id->text();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Campos vacíos", "El ID no puede estar vacío.");
        return;
    }

    if (QMessageBox::question(this, "Eliminar Registro", "¿Está seguro de que quiere eliminar el registro con ID: " + id + "?") == QMessageBox::Yes) {

        if (!dbnodestock.isOpen() && !dbnodestock.open()) {
            qDebug() << "Error: No se pudo abrir la base de datos.";
            return;
        }

        if (!dbnodestock.transaction()) {
            qDebug() << "Error al iniciar la transacción:" << dbnodestock.lastError().text();
            return;
        }

        QSqlQuery QueryDeleteData(dbnodestock);
        QueryDeleteData.prepare("DELETE FROM inventario WHERE id = :id");
        QueryDeleteData.bindValue(":id", id);

        if (!QueryDeleteData.exec()) {
            qDebug() << "Error al eliminar en la tabla:" << QueryDeleteData.lastError().text();
            dbnodestock.rollback();
            QMessageBox::critical(this, "Error", "No se pudo eliminar el registro.");
        } else {
            dbnodestock.commit();
            qDebug() << "Eliminación exitosa.";
            QMessageBox::information(this, "Éxito", "El registro ha sido eliminado con éxito.");
            cargarInventario();

            if (indexActual >= ui->tableViewInventario->model()->rowCount()) {
                indexActual = ui->tableViewInventario->model()->rowCount() - 1;
            }
            cargarDato(indexActual);
        }
    }
}


void MainWindow::on_impresionBtn_clicked()
{
    if (QMessageBox::question(this, "Impresión", "¿Desea imprimir el listado completo de inventario?") != QMessageBox::Yes) {
        return;
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setFullPage(true);

    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() != QDialog::Accepted) return;

    QString htmlContent;
    htmlContent += "<html><head><style>";
    htmlContent += "body { font-family: Arial, sans-serif; }";
    htmlContent += "h1 { text-align: center; font-size: 18px; }";
    htmlContent += "table { width: 100%; border-collapse: collapse; }";
    htmlContent += "th, td { border: 1px solid black; padding: 8px; text-align: left; font-size: 14px; }";
    htmlContent += "th { background-color: #f2f2f2; font-weight: bold; }";
    htmlContent += "</style></head><body>";
    htmlContent += "<h1>Listado de Inventario</h1><table>";
    htmlContent += "<tr><th>ID</th><th>Producto</th><th>Proveedor</th><th>Cantidad</th><th>Fecha Entrada</th></tr>";

    if (!dbnodestock.isOpen() && !dbnodestock.open()) {
        qDebug() << "Error: No se pudo abrir la base de datos.";
        return;
    }

    QSqlQuery QuerySelectData(dbnodestock);
    QuerySelectData.prepare("SELECT i.id, p.nombre AS producto, pr.nombre AS proveedor, i.cantidad, i.fecha_entrada "
                            "FROM inventario i "
                            "INNER JOIN productos p ON i.producto_id = p.id "
                            "INNER JOIN proveedores pr ON i.proveedor_id = pr.id");

    if (QuerySelectData.exec()) {
        while (QuerySelectData.next()) {
            htmlContent += "<tr>";
            htmlContent += "<td>" + QuerySelectData.value(0).toString() + "</td>";
            htmlContent += "<td>" + QuerySelectData.value(1).toString() + "</td>";
            htmlContent += "<td>" + QuerySelectData.value(2).toString() + "</td>";
            htmlContent += "<td>" + QuerySelectData.value(3).toString() + "</td>";
            htmlContent += "<td>" + QuerySelectData.value(4).toString() + "</td>";
            htmlContent += "</tr>";
        }
    } else {
        qDebug() << "Error al obtener los datos:" << QuerySelectData.lastError().text();
        QMessageBox::critical(this, "Error", "No se pudo obtener los datos de inventario.");
    }

    htmlContent += "</table></body></html>";

    QTextDocument document;
    document.setHtml(htmlContent);
    document.setPageSize(printer.pageRect(QPrinter::Point).size());
    document.print(&printer);
}


bool MainWindow::cambiarBotonGuardar()
{
    QPushButton *button = ui->guardarBtn;
    if (button->text() == "Guardar") {
        button->setText("Agregar Nuevo");
        button->setStyleSheet("background-color:#3498db; color:white; border-radius:5px; border:1px solid black; padding:10px 8px;");
        return true;
    }
    return false;
}

void MainWindow::on_anteriorBtn_clicked()
{
    anterior();
}

void MainWindow::on_siguienteBtn_clicked()
{
    siguiente();
}

//// ========================> CATEGORÍA <======================== ////

void MainWindow::on_guardarCatBtn_clicked()
{
    if (ui->nombreLineEdit->text().isEmpty() || ui->descLineEditCat->text().isEmpty()) {
        QMessageBox::warning(this, "Campos Vacíos", "Por favor, complete todos los campos obligatorios antes de registrar la categoría.");
        return;
    }

    QSqlQuery insertCatQuery;
    insertCatQuery.prepare("INSERT INTO categorias (nombre, descripcion) VALUES (:nombre, :descripcion)");
    insertCatQuery.bindValue(":nombre", ui->nombreLineEdit->text().trimmed());
    insertCatQuery.bindValue(":descripcion", ui->descLineEditCat->text().trimmed());

    if (insertCatQuery.exec()) {
        QMessageBox::information(this, "Éxito", "La categoría se ha registrado con éxito.");
        loadCategoriesInTable();
        ui->nombreLineEdit->clear();
        ui->descLineEditCat->clear();
    } else {
        qDebug() << "Error en la inserción de categoría:" << insertCatQuery.lastError().text();
        QMessageBox::critical(this, "Error", "No se pudo registrar la categoría.");
    }
}


void MainWindow::on_eliminarCatBtn_clicked()
{
    auto selectionModel = ui->tableViewCategoria->selectionModel();
    if (!selectionModel->hasSelection()) {
        QMessageBox::warning(this, "Advertencia", "Seleccione una categoría para eliminar.");
        return;
    }

    int selectedRow = selectionModel->currentIndex().row();
    int categoryId = ui->tableViewCategoria->model()->index(selectedRow, 0).data().toInt();

    if (QMessageBox::question(this, "Confirmar Eliminación", "¿Está seguro de que desea eliminar esta categoría?") == QMessageBox::Yes) {
        QSqlQuery deleteCatQuery;
        deleteCatQuery.prepare("DELETE FROM categorias WHERE id = :id");
        deleteCatQuery.bindValue(":id", categoryId);

        if (deleteCatQuery.exec()) {
            QMessageBox::information(this, "Éxito", "La categoría ha sido eliminada con éxito.");
            loadCategoriesInTable();
        } else {
            qDebug() << "Error en la eliminación de categoría:" << deleteCatQuery.lastError().text();
            QMessageBox::critical(this, "Error", "No se pudo eliminar la categoría.");
        }
    }
}


void MainWindow::on_actualizarCatBtn_clicked()
{
    if (currentCatId == -1) {
        QMessageBox::warning(this, "Advertencia", "Seleccione una categoría para actualizar.");
        return;
    }

    QSqlQuery updateCatQuery;
    updateCatQuery.prepare("UPDATE categorias SET nombre = :nombre, descripcion = :descripcion WHERE id = :id");
    updateCatQuery.bindValue(":nombre", ui->nombreLineEdit->text().trimmed());
    updateCatQuery.bindValue(":descripcion", ui->descLineEditCat->text().trimmed());
    updateCatQuery.bindValue(":id", currentCatId);

    if (updateCatQuery.exec()) {
        QMessageBox::information(this, "Éxito", "La categoría ha sido actualizada.");
        loadCategoriesInTable();
        ui->nombreLineEdit->clear();
        ui->descLineEditCat->clear();
        currentCatId = -1;
    } else {
        qDebug() << "Error en la actualización de categoría:" << updateCatQuery.lastError().text();
        QMessageBox::critical(this, "Error", "No se pudo actualizar la categoría.");
    }
}


void MainWindow::on_impresionCatBtn_clicked()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setFullPage(true);

    QPageLayout pageLayout;
    pageLayout.setOrientation(QPageLayout::Portrait);
    printer.setPageLayout(pageLayout);

    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() != QDialog::Accepted) {
        return;  // Exit if the user cancels the print dialog
    }

    QString htmlContent;
    htmlContent += "<html><head><style>";
    htmlContent += "body { font-family: Arial, sans-serif; }";
    htmlContent += "h1 { text-align: center; font-size: 18px; }";
    htmlContent += "table { width: 100%; border-collapse: collapse; }";
    htmlContent += "th, td { border: 1px solid black; padding: 8px; text-align: left; font-size: 18px; }";
    htmlContent += "th { background-color: #f2f2f2; font-weight: bold; }";
    htmlContent += "</style></head><body>";

    htmlContent += "<h1>Listado de Categorías</h1>";
    htmlContent += "<table><tr><th>ID</th><th>Nombre</th><th>Descripción</th></tr>";

    QSqlQuery query("SELECT id, nombre, descripcion FROM categorias");
    while (query.next()) {
        htmlContent += "<tr>";
        htmlContent += "<td>" + query.value("id").toString() + "</td>";
        htmlContent += "<td>" + query.value("nombre").toString() + "</td>";
        htmlContent += "<td>" + query.value("descripcion").toString() + "</td>";
        htmlContent += "</tr>";
    }
    htmlContent += "</table></body></html>";

    QTextDocument document;
    document.setHtml(htmlContent);
    document.setPageSize(printer.pageRect(QPrinter::Point).size());
    document.print(&printer);
}


//// ========================> PROVEEDORES <======================== ////
/// \brief MainWindow::loadProveedoresTable
void MainWindow::clearSupplierFields()
{
    // Clears the supplier-related input fields
    ui->nombreProveedor->clear();
    ui->telProveedor->clear();
    ui->emailProveedor->clear();

    // Enables fields in case they were disabled previously
    ui->nombreProveedor->setEnabled(true);
    ui->telProveedor->setEnabled(true);
    ui->emailProveedor->setEnabled(true);

    // Optionally reset any related state variables, like `currentProvId`
    currentProvId = -1;
}


void MainWindow::loadProveedoresTable()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    QSqlQuery selectProveedorQuery("SELECT id, nombre, telefono, email FROM proveedores");

    if (selectProveedorQuery.exec()) {
        model->setQuery(std::move(selectProveedorQuery));
        ui->tableViewProveedores->setModel(model);

        // Set header names for better readability
        model->setHeaderData(0, Qt::Horizontal, "ID");
        model->setHeaderData(1, Qt::Horizontal, "Nombre del Proveedor");
        model->setHeaderData(2, Qt::Horizontal, "Teléfono del Proveedor");
        model->setHeaderData(3, Qt::Horizontal, "Email del Proveedor");

        // Resize columns to fit data
        ui->tableViewProveedores->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tableViewProveedores->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        ui->tableViewProveedores->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        ui->tableViewProveedores->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    } else {
        qDebug() << "Error loading suppliers:" << selectProveedorQuery.lastError().text();
        QMessageBox::critical(this, "Error", "No se pudo cargar la tabla de proveedores.");
    }
}


void MainWindow::on_guardarProveBtn_clicked()
{
    if (isNewProveedor) {
        clearSupplierFields();
        ui->guardarProveBtn->setText("Guardar Proveedor");
        isNewProveedor = false;
    } else {
        if (ui->nombreProveedor->text().isEmpty() ||
            ui->telProveedor->text().isEmpty() ||
            ui->emailProveedor->text().isEmpty()) {
            QMessageBox::warning(this, "Campos Vacíos", "Complete todos los campos.");
            return;
        }

        QSqlQuery insertProveedorQuery;
        insertProveedorQuery.prepare("INSERT INTO proveedores (nombre, telefono, email) VALUES (:nombre, :tel, :email)");
        insertProveedorQuery.bindValue(":nombre", ui->nombreProveedor->text().trimmed());
        insertProveedorQuery.bindValue(":tel", ui->telProveedor->text().trimmed());
        insertProveedorQuery.bindValue(":email", ui->emailProveedor->text().trimmed());

        if (insertProveedorQuery.exec()) {
            QMessageBox::information(this, "Éxito", "Proveedor registrado.");
            loadProveedoresTable();
            clearSupplierFields();
            ui->guardarProveBtn->setText("Nuevo Proveedor");
            isNewProveedor = true;
        } else {
            qDebug() << "Error al registrar proveedor:" << insertProveedorQuery.lastError().text();
            QMessageBox::critical(this, "Error", "No se pudo registrar el proveedor.");
        }
    }
}


void MainWindow::on_eliminarProveBtn_clicked()
{
    auto selectionModel = ui->tableViewProveedores->selectionModel();
    if (!selectionModel->hasSelection()) {
        QMessageBox::warning(this, "Advertencia", "Seleccione un proveedor para eliminar.");
        return;
    }

    int selectedRow = selectionModel->currentIndex().row();
    int proveedorId = ui->tableViewProveedores->model()->index(selectedRow, 0).data().toInt();

    if (QMessageBox::question(this, "Confirmar Eliminación", "¿Desea eliminar el proveedor seleccionado?") == QMessageBox::Yes) {
        QSqlQuery deleteProveedorQuery;
        deleteProveedorQuery.prepare("DELETE FROM proveedores WHERE id = :id");
        deleteProveedorQuery.bindValue(":id", proveedorId);

        if (deleteProveedorQuery.exec()) {
            QMessageBox::information(this, "Éxito", "Proveedor eliminado.");
            loadProveedoresTable();
        } else {
            qDebug() << "Error al eliminar proveedor:" << deleteProveedorQuery.lastError().text();
            QMessageBox::critical(this, "Error", "No se pudo eliminar el proveedor.");
        }
    }
}


void MainWindow::on_actualizarProveBtn_clicked()
{
    if (currentProvId == -1) {
        QMessageBox::warning(this, "Advertencia", "Seleccione un proveedor para actualizar.");
        return;
    }

    QSqlQuery updateProveedorQuery;
    updateProveedorQuery.prepare("UPDATE proveedores SET nombre = :nombre, telefono = :tel, email = :email WHERE id = :id");
    updateProveedorQuery.bindValue(":nombre", ui->nombreProveedor->text().trimmed());
    updateProveedorQuery.bindValue(":tel", ui->telProveedor->text().trimmed());
    updateProveedorQuery.bindValue(":email", ui->emailProveedor->text().trimmed());
    updateProveedorQuery.bindValue(":id", currentProvId);

    if (updateProveedorQuery.exec()) {
        QMessageBox::information(this, "Éxito", "Proveedor actualizado.");
        loadProveedoresTable();
        clearSupplierFields();
        currentProvId = -1;
    } else {
        qDebug() << "Error al actualizar proveedor:" << updateProveedorQuery.lastError().text();
        QMessageBox::critical(this, "Error", "No se pudo actualizar el proveedor.");
    }
}


void MainWindow::on_tableViewProveedores_doubleClicked(const QModelIndex &index)
{
    int row = index.row();
    int id = ui->tableViewProveedores->model()->index(row, 0).data().toInt();

    currentProvId = id;

    QString currentName = ui->tableViewProveedores->model()->index(row, 1).data().toString();
    QString currentTel = ui->tableViewProveedores->model()->index(row, 2).data().toString();
    QString currentEmail = ui->tableViewProveedores->model()->index(row, 3).data().toString();

    ui->nombreProveedor->setText(currentName);
    ui->telProveedor->setText(currentTel);
    ui->emailProveedor->setText(currentEmail);
}


void MainWindow::on_impresionProveBtn_clicked()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setFullPage(true);

    QPageLayout pageLayout;
    pageLayout.setOrientation(QPageLayout::Portrait);
    printer.setPageLayout(pageLayout);

    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() != QDialog::Accepted) {
        return;  // Exit if the user cancels the print dialog
    }

    QString htmlContent;
    htmlContent += "<html><head><style>";
    htmlContent += "body { font-family: Arial, sans-serif; }";
    htmlContent += "h1 { text-align: center; font-size: 18px; }";
    htmlContent += "table { width: 100%; border-collapse: collapse; }";
    htmlContent += "th, td { border: 1px solid black; padding: 8px; text-align: left; font-size: 18px; }";
    htmlContent += "th { background-color: #f2f2f2; font-weight: bold; }";
    htmlContent += "</style></head><body>";

    htmlContent += "<h1>Listado de Proveedores</h1>";
    htmlContent += "<table><tr><th>ID</th><th>Nombre</th><th>Teléfono</th><th>Email</th></tr>";

    QSqlQuery query("SELECT id, nombre, telefono, email FROM proveedores");
    while (query.next()) {
        htmlContent += "<tr>";
        htmlContent += "<td>" + query.value("id").toString() + "</td>";
        htmlContent += "<td>" + query.value("nombre").toString() + "</td>";
        htmlContent += "<td>" + query.value("telefono").toString() + "</td>";
        htmlContent += "<td>" + query.value("email").toString() + "</td>";
        htmlContent += "</tr>";
    }
    htmlContent += "</table></body></html>";

    QTextDocument document;
    document.setHtml(htmlContent);
    document.setPageSize(printer.pageRect(QPrinter::Point).size());
    document.print(&printer);
}

void MainWindow::on_tableViewCategoria_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    int row = index.row();
    int id = ui->tableViewCategoria->model()->index(row, 0).data().toInt();

    currentCatId = id;

    QString currentName = ui->tableViewCategoria->model()->index(row, 1).data().toString();
    QString currentDescription = ui->tableViewCategoria->model()->index(row, 2).data().toString();

    ui->nombreLineEdit->setText(currentName);
    ui->descLineEditCat->setText(currentDescription);
}
