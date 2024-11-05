#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QFileInfo>
#include <QtDebug>
#include <QByteArray>
#include <QImage>
#include <QList>
#include <QVariantList>
#include "categoria.h"
#include "productos.h"
#include "proveedores.h"
#include "dbnodestock.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    // Productos

    void on_prodRegistrar_clicked();

    void on_imageBtn_clicked();

    void loadItems();

    void displayItems(int index);

    void on_prodAnterior_clicked();

    void on_prodSiguiente_clicked();

    void loadCategories();

    void on_prodActualizar_clicked();

    void on_prodEliminar_clicked();

    void on_buscarBtn_clicked();

    void clearFields();

    // Inventario

    void cargarInventario();

    void cargarDato(int index);

    void on_guardarBtn_clicked();

    void on_actualizarBtn_clicked();

    void on_eliminarBtn_clicked();

    void on_impresionBtn_clicked();

    bool cambiarBotonGuardar();

    void loadProveedores();

    void loadProductos();

    // Categoría

    void on_guardarCatBtn_clicked();

    void on_eliminarCatBtn_clicked();

    void on_actualizarCatBtn_clicked();

    void on_impresionCatBtn_clicked();

    void on_tableViewCategoria_doubleClicked(const QModelIndex &index);

    void loadCategoriesInTable();

    void on_prodImprimir_clicked();

    void loadProveedoresTable();

private:
    Ui::MainWindow *ui;
    Categoria *ptrCategoria;
    Productos *ptrProductos;
    Proveedores *ptrProveedores;
    QByteArray imageData;
    QList<QVariantMap> items;
    int currentIndex = 0;
    QString formatearPalabra(const QString& s);
    bool isNewProductMode = true;
    QSqlDatabase dbnodestock;
    int currentCatId = -1;
};
#endif // MAINWINDOW_H
