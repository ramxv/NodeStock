#ifndef PROVEEDORES2_H
#define PROVEEDORES2_H

#include <QMainWindow>
#include <QWidget>
#include <QMainWindow>
#include <QtSql>


class MainWindow; // Forward declaration


namespace Ui {
class Proveedores2;
}

class Proveedores2 : public QMainWindow
{
    Q_OBJECT

public:
    explicit Proveedores2(MainWindow *mainWindow, QWidget *parent = nullptr);
    ~Proveedores2();

    void cargarProveedores();
    void regresar();


private slots:
    void on_proveedorMenu_clicked();

    void on_pushButtonGuardar_clicked();

    void on_proveedorMenu_2_clicked();

    void on_pushButtonVolver_clicked();

    void on_pushButtonActualizar_clicked();

    void on_pushButtonActualizar_2_clicked();

    void on_proveedorMenu_3_clicked();

    void on_pushButtonVolver_2_clicked();

    void on_pushButtonEliminar_2_clicked();

    void on_pushButtonEliminar_clicked();

private:
    Ui::Proveedores2 *ui;
    QSqlDatabase DB_Connection;

    MainWindow *mainWindow; // Pointer to MainWindow

};

#endif // PROVEEDORES2_H
