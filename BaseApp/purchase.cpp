#include "purchase.h"
#include "ui_purchase.h"

Purchase::Purchase(QWidget *parent) : //конструктор
    QWidget(parent),
    ui(new Ui::Purchase)
{
    ui->setupUi(this);

    //Установим построчный единичный выбор в таблице
    ui->tableWidget_purchase->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget_purchase->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableWidget_purchase->setFont(QFont("Franklin Gothic Book", 14));
    ui->tableWidget_purchase->setStyleSheet("QTableWidget::item {padding-left: 0px}");
}

Purchase::~Purchase() //деструктор
{
    delete ui;
}

void Purchase::slotPurchase(QVector<QVector<QString>> userCart, int orderN) //товары из корзины, представленные к покупке
{
    ui->label_thanksForPurchase->hide();
    ui->label_orderNumber->hide();
    ui->pushButton_backToCatalogAfterPurchase->hide();
    ui->label_totalPrice->show();
    ui->tableWidget_purchase->show();
    ui->groupBox_receivingMethod->show();
    ui->pushButton_purchaseCompletion->show();

    ui->label_orderNumber->setText("Ваш заказ №" + QString::number(orderN) + '\n' + "В ближайшее время он будет обработан");

    totalPrice = 0;
    userCartInPurchase.clear();

    //Передадим содержимое корзины для контекста всего класса
    for (int i=0; i<userCart.size(); i++)
    {
        userCartInPurchase.push_back(userCart.value(i));
        totalPrice += userCartInPurchase.value(i).value(4).toInt() * userCartInPurchase.value(i).value(5).toInt();
    }

    ui->label_totalPrice->setText("Итоговая сумма к оплате -  " + QString::number(totalPrice) + " ₽");

    ui->tableWidget_purchase->setRowCount(userCart.size());
    ui->tableWidget_purchase->setColumnCount(4);

    ui->tableWidget_purchase->setColumnWidth(0, 108);
    ui->tableWidget_purchase->setColumnWidth(1, 240);
    ui->tableWidget_purchase->setColumnWidth(2, 150);
    ui->tableWidget_purchase->setColumnWidth(3, 150);

    for (int i=0; i<ui->tableWidget_purchase->rowCount(); i++)
    {
        ui->tableWidget_purchase->setRowHeight(i, 108);

        QTableWidgetItem* ptwi_1 = new QTableWidgetItem;
        ptwi_1->setIcon(QIcon("../BaseApp/AlbumCovers/" + userCartInPurchase.value(i).value(2) + ".jpg"));
        ui->tableWidget_purchase->setItem(i, 0, ptwi_1);
        ui->tableWidget_purchase->setIconSize(QSize(108, 108));
        ptwi_1->setFlags(ptwi_1->flags()&0xfffffffd);

        QTableWidgetItem* ptwi_2 = new QTableWidgetItem(userCartInPurchase.value(i).value(1) + " - " + userCartInPurchase.value(i).value(2));
        ui->tableWidget_purchase->setItem(i, 1, ptwi_2);
        ptwi_2->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ptwi_2->setFlags(ptwi_2->flags()&0xfffffffd);

        QTableWidgetItem* ptwi_3  = new QTableWidgetItem(
                    QString::number(userCartInPurchase.value(i).value(4).toInt() * userCartInPurchase.value(i).value(5).toInt()) + " ₽");
        ui->tableWidget_purchase->setItem(i, 2, ptwi_3);
        ptwi_3->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ptwi_3->setFlags(ptwi_3->flags()&0xfffffffd);

        QTableWidgetItem* ptwi_4  = new QTableWidgetItem("Количество: " + userCartInPurchase.value(i).value(5));
        ui->tableWidget_purchase->setItem(i, 3, ptwi_4);
        ptwi_4->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ptwi_4->setFlags(ptwi_4->flags()&0xfffffffd);
    }
}

void Purchase::on_pushButton_purchaseCompletion_clicked() //завершение покупки
{
    ui->label_thanksForPurchase->show();
    ui->label_totalPrice->hide();
    ui->label_orderNumber->show();
    ui->tableWidget_purchase->hide();
    ui->groupBox_receivingMethod->hide();
    ui->pushButton_purchaseCompletion->hide();
    ui->pushButton_backToCatalogAfterPurchase->show();

    QString receivingFormat;
    if (ui->radioButton_self->isChecked())
    {
        receivingFormat = ui->radioButton_self->text();
    }
    else
    {
        receivingFormat = ui->radioButton_delivery->text();
    }

    emit signalOfPurchaseCompletion(userCartInPurchase, receivingFormat, totalPrice);
}


void Purchase::on_pushButton_backToCatalogAfterPurchase_clicked() //возвращение на страницу каталога
{
    emit signalBackToCatalog();
}

