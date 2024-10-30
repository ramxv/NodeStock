#ifndef CATEGORIA_H
#define CATEGORIA_H

#include <QDialog>
#include "dbnodestock.h"


namespace Ui {
class Categoria;
}

class Categoria : public QDialog
{
    Q_OBJECT

public:
    explicit Categoria(QWidget *parent = nullptr);
    ~Categoria();

private slots:

    void on_registrarBtn_clicked();

    void on_eliminarBtn_clicked();

    void on_actualizarBtn_clicked();

    void on_tableView_doubleClicked(const QModelIndex &index);

    void on_printBtn_clicked();

private:
    Ui::Categoria *ui;
    int currentId = -1;
};

#endif // CATEGORIA_H
