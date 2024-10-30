#ifndef PRODUCTOS_H
#define PRODUCTOS_H

#include <QDialog>

namespace Ui {
class Productos;
}

class Productos : public QDialog
{
    Q_OBJECT

public:
    explicit Productos(QWidget *parent = nullptr);
    ~Productos();

private:
    Ui::Productos *ui;
};

#endif // PRODUCTOS_H
