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
};
#endif // MAINWINDOW_H
