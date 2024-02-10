#ifndef CART_H
#define CART_H

#include <QWidget>
#include <QDebug>
#include <purchase.h>

namespace Ui {
class Cart;
}

class Cart : public QWidget
{
    Q_OBJECT

public:
    explicit Cart(QWidget *parent = nullptr);
    ~Cart();

private:
    Ui::Cart *ui;
    QVector<QVector<QString>> mainUserCart;

    int totalPrice = 0;

public slots:
    void slot(QVector<QVector<QString>> userCart); //обработка сигнала перехода в корзину

    void slorClearCart(); //очистка корзины после завершения покупки

private slots:
    void on_pushButton_clearProduct_clicked();

    void on_pushButton_placeOrder_clicked();

signals:
    void signalCart(int currentProduct); //удаление товара из корзины

    void signalOfPurchase (QVector<QVector<QString>> userCart); //переход к оформлению покупки
};

#endif // CART_H
