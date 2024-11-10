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
    void on_prodImprimir_clicked();

    // Inventario
    void clearInventoryFields();
    bool validateInventoryFields();
    void cargarInventario();
    void cargarDato(int index);
    void on_guardarBtn_clicked();
    void on_actualizarBtn_clicked();
    void on_eliminarBtn_clicked();
    void on_impresionBtn_clicked();
    bool cambiarBotonGuardar();
    void loadProveedores();
    void loadProductos();
    void anterior();
    void siguiente();
    void on_anteriorBtn_clicked();
    void on_siguienteBtn_clicked();

    // Categoría
    void on_guardarCatBtn_clicked();
    void on_eliminarCatBtn_clicked();
    void on_actualizarCatBtn_clicked();
    void on_impresionCatBtn_clicked();
    void on_tableViewCategoria_doubleClicked(const QModelIndex &index);
    void loadCategoriesInTable();

    // Proveedores
    void clearSupplierFields();
    void loadProveedoresTable();
    void on_guardarProveBtn_clicked();
    void on_eliminarProveBtn_clicked();
    void on_actualizarProveBtn_clicked();
    void on_tableViewProveedores_doubleClicked(const QModelIndex &index);
    void on_impresionProveBtn_clicked();

private:
    Ui::MainWindow *ui;

    QSqlDatabase dbnodestock;
    QByteArray imageData;
    QList<QVariantMap> items;

    int currentIndex = 0;
    int currentCatId = -1;
    int currentProvId = -1;

    bool isNewProductMode = true;
    bool isNewProveedor = true;

    QString formatearPalabra(const QString& s);
};

#endif // MAINWINDOW_H
