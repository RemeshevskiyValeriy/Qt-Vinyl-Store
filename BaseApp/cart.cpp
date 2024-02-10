#include "cart.h"
#include "ui_cart.h"

Cart::Cart(QWidget *parent) : //конструктор
    QWidget(parent),
    ui(new Ui::Cart)
{
    ui->setupUi(this);
    this->setWindowTitle("Корзина");

    //Установим построчный единичный выбор в таблице
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->pushButton_clearProduct->setEnabled(false);

    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, [=]()
                                                                {
                                                                    if(ui->tableWidget->selectionModel()->selectedRows().size() > 0)
                                                                    {
                                                                        ui->pushButton_clearProduct->setEnabled(true);
                                                                    }
                                                                    else
                                                                    {
                                                                        ui->pushButton_clearProduct->setEnabled(false);
                                                                    }
                                                                });

}

Cart::~Cart() //деструктор
{
    delete ui;
}

void Cart::slot(QVector<QVector<QString>> userCart) //заполнение и визуализация корзины
{
    if (userCart.isEmpty())
    {
        ui->tableWidget->hide();
        ui->pushButton_placeOrder->hide();
        ui->pushButton_clearProduct->hide();
        ui->label_cartIsEmpry->setText("Корзина товаров пуста");
    }
    else
    {      
        totalPrice = 0;
        mainUserCart.clear();

        //Передадим содержимое корзины для контекста всего класса
        for (int i=0; i<userCart.size(); i++)
        {
            mainUserCart.push_back(userCart.value(i));
            totalPrice += mainUserCart.value(i).value(4).toInt() * mainUserCart.value(i).value(5).toInt();
        }

        ui->tableWidget->show();
        ui->pushButton_placeOrder->show();
        ui->pushButton_clearProduct->show();
        ui->label_cartIsEmpry->setText("Общая стоимость корзины - " + QString::number(totalPrice) + " ₽");

        ui->tableWidget->setRowCount(userCart.size());
        ui->tableWidget->setColumnCount(4);

        ui->tableWidget->setColumnWidth(0, 108);
        ui->tableWidget->setColumnWidth(1, 240);
        ui->tableWidget->setColumnWidth(2, 150);
        ui->tableWidget->setColumnWidth(3, 150);

        ui->tableWidget->setFont(QFont("Franklin Gothic Book", 14));
        ui->tableWidget->setStyleSheet("QTableWidget::item {padding-left: 0px}");

        for (int i=0; i<ui->tableWidget->rowCount(); i++)
        {
            ui->tableWidget->setRowHeight(i, 108);

            QTableWidgetItem* ptwi_1 = new QTableWidgetItem;
            ptwi_1->setIcon(QIcon("../BaseApp/AlbumCovers/" + mainUserCart.value(i).value(2) + ".jpg"));
            ui->tableWidget->setItem(i, 0, ptwi_1);
            ui->tableWidget->setIconSize(QSize(108, 108));
            ptwi_1->setFlags(ptwi_1->flags()&0xfffffffd);

            QTableWidgetItem* ptwi_2 = new QTableWidgetItem(mainUserCart.value(i).value(1) + " - " + mainUserCart.value(i).value(2));
            ui->tableWidget->setItem(i, 1, ptwi_2);
            ptwi_2->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ptwi_2->setFlags(ptwi_2->flags()&0xfffffffd);

            QTableWidgetItem* ptwi_3  = new QTableWidgetItem(
                        QString::number(mainUserCart.value(i).value(4).toInt() * mainUserCart.value(i).value(5).toInt()) + " ₽");
            ui->tableWidget->setItem(i, 2, ptwi_3);
            ptwi_3->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ptwi_3->setFlags(ptwi_3->flags()&0xfffffffd);

            QTableWidgetItem* ptwi_4  = new QTableWidgetItem("Количество: " + mainUserCart.value(i).value(5));
            ui->tableWidget->setItem(i, 3, ptwi_4);
            ptwi_4->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ptwi_4->setFlags(ptwi_4->flags()&0xfffffffd);
        }
    }
}

void Cart::slorClearCart() //очистка корзины после завершения покупки
{
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);
    ui->tableWidget->hide();
    ui->pushButton_placeOrder->hide();
    ui->pushButton_clearProduct->hide();
    ui->label_cartIsEmpry->setText("Корзина товаров пуста");
    totalPrice = 0;
}

void Cart::on_pushButton_clearProduct_clicked() //удаляем выделенный товар из корзины
{
    int currentProduct = ui->tableWidget->currentRow();

    totalPrice -= mainUserCart.value(currentProduct).value(4).toInt() * mainUserCart.value(currentProduct).value(5).toInt();

    ui->label_cartIsEmpry->setText("Общая стоимость корзины - " + QString::number(totalPrice) + " ₽");
    mainUserCart.removeAt(currentProduct);
    ui->tableWidget->removeRow(currentProduct);

    if (ui->tableWidget->rowCount() == 0)
    {
        ui->tableWidget->hide();
        ui->pushButton_placeOrder->hide();
        ui->pushButton_clearProduct->hide();
        ui->label_cartIsEmpry->setText("Корзина товаров пуста");
        totalPrice = 0;
    }

    emit signalCart(currentProduct);
}


void Cart::on_pushButton_placeOrder_clicked() //переход к оформлению заказа
{
    emit signalOfPurchase(mainUserCart);
}

