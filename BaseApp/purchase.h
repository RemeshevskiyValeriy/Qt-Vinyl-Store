#ifndef PURCHASE_H
#define PURCHASE_H

#include <QWidget>
#include <QDebug>


namespace Ui {
class Purchase;
}

class Purchase : public QWidget
{
    Q_OBJECT

public:
    explicit Purchase(QWidget *parent = nullptr);
    ~Purchase();

private:
    Ui::Purchase *ui;
    QVector<QVector<QString>> userCartInPurchase;
    int totalPrice = 0;

public slots:
    void slotPurchase (QVector<QVector<QString>> userCart, int orderN); //принимаем корзину для завершения покупки

signals:
    void signalOfPurchaseCompletion (QVector<QVector<QString>> userCart, QString format, int totalPrice); //передаем информацию в основной класс о купленных товарах

    void signalBackToCatalog(); //возврат на страницу каталога

private slots:
    void on_pushButton_purchaseCompletion_clicked();

    void on_pushButton_backToCatalogAfterPurchase_clicked();
};

#endif // PURCHASE_H
