#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QFileInfo>
#include <QtDebug>
#include "categoria.h"
#include "productos.h"
#include "proveedores.h"

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
    void on_categoriaBtn_clicked();

    void on_productosBtn_clicked();

    void on_proveedoresBtn_clicked();

    void on_tabWidget_tabBarClicked(int index);

    void on_tabWidget_tabCloseRequested(int index);

private:
    Ui::MainWindow *ui;
    Categoria *ptrCategoria;
    Productos *ptrProductos;
    Proveedores *ptrProveedores;
};
#endif // MAINWINDOW_H
