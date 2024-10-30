#ifndef PROVEEDORES_H
#define PROVEEDORES_H

#include <QDialog>
#include "dbnodestock.h"

namespace Ui {
class Proveedores;
}

class Proveedores : public QDialog
{
    Q_OBJECT

public:
    explicit Proveedores(QWidget *parent = nullptr);
    ~Proveedores();

private slots:
    void on_guardarProveedorBtn_clicked();

    void on_actualizarProveedorBtn_clicked();

    void on_tableViewProveedores_doubleClicked(const QModelIndex &index);

    void on_eliminarProveedorBtn_clicked();

private:
    Ui::Proveedores *ui;
};

#endif // PROVEEDORES_H
