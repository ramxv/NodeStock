#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtSql>
#include <QFileInfo>
#include <QtDebug>
#include "categoria.h"
#include "productos.h"




QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public MainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_pushButton_clicked();

    void on_pushButton_5_clicked();

    void on_poveedor_clicked();


private:
    Ui::MainWindow *ui;
    Categoria *ptrCategoria;
    Productos *ptrProductos;
};
#endif // MAINWINDOW_H
