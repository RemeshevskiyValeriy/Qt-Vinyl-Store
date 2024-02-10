#ifndef ADD_PRODUCT_H
#define ADD_PRODUCT_H

#include <QWidget>
#include <QDebug>
#include <QList>
#include <QComboBox>
#include <QMessageBox>
#include <QTableWidgetItem>

namespace Ui {
class add_product;
}

class add_product : public QWidget
{
    Q_OBJECT

public:
    explicit add_product(QWidget *parent = nullptr);
    ~add_product();

private:
    Ui::add_product *ui;
    QVector<QVector<QString>> addedProducts;
    QStringList list_genres;
    bool flag_auto;

signals:
    void signalProductsAdded(QVector<QVector<QString>> addedProducts); //отправка информации о добавленных товарах

private slots:
    void on_spinBox_productsCount_valueChanged(int arg1);

    void on_tableWidget_addProduct_cellChanged(int row, int column);
    void on_pushButton_addToCatalog_clicked();

    void on_pushButton_backToAdmin_clicked();
};

#endif // ADD_PRODUCT_H
