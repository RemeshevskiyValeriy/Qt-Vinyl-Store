#include "confirm_order.h"
#include "ui_confirm_order.h"

Confirm_order::Confirm_order(QWidget *parent) : //конструктор
    QWidget(parent),
    ui(new Ui::Confirm_order)
{
    ui->setupUi(this);
    this->setWindowTitle("Подтверждение заказов");

    //Установим построчный единичный выбор в таблице
    ui->tableWidget_confirmOrders->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget_confirmOrders->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget_confirmOrders->setRowCount(1);
    ui->tableWidget_confirmOrders->setColumnCount(6);

    //Заголовки таблицы
    QStringList list_labels;
    list_labels << "order_id" << "user_id" << "order_format" << "order_date" << "order_state" << "order_sum";
    ui->tableWidget_confirmOrders->setHorizontalHeaderLabels(list_labels);

    ui->pushButton_confirmation->setEnabled(false);

    connect(ui->tableWidget_confirmOrders, &QTableWidget::itemSelectionChanged, [=]()
                                                                                    {
                                                                                        if(ui->tableWidget_confirmOrders->selectionModel()->selectedRows().size() > 0)
                                                                                        {
                                                                                            ui->pushButton_confirmation->setEnabled(true);
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            ui->pushButton_confirmation->setEnabled(false);
                                                                                        }
                                                                                    });

}

Confirm_order::~Confirm_order() //деструктор
{
    delete ui;
}

void Confirm_order::slotConfirm(QVector<QVector<QString>> confirmation) //отображаем необработанные заказы
{
    if (confirmation.isEmpty())
    {
        ui->label_allConfirmed->show();
        ui->label_allConfirmed->setText("Нет заказов для подтверждения");
        ui->tableWidget_confirmOrders->hide();
        ui->pushButton_confirmation->hide();
    }
    else
    {
        ui->label_allConfirmed->hide();
        ui->tableWidget_confirmOrders->show();
        ui->pushButton_confirmation->show();

        ui->tableWidget_confirmOrders->setRowCount(confirmation.size());
        ui->tableWidget_confirmOrders->setColumnCount(6);

        for (int i=0; i<ui->tableWidget_confirmOrders->rowCount(); i++)
        {
            QTableWidgetItem* ptwi_1 = new QTableWidgetItem(confirmation.value(i).value(0));
            ui->tableWidget_confirmOrders->setItem(i, 0, ptwi_1);
            ptwi_1->setFlags(ptwi_1->flags()&0xfffffffd);

            QTableWidgetItem* ptwi_2 = new QTableWidgetItem(confirmation.value(i).value(1));
            ui->tableWidget_confirmOrders->setItem(i, 1, ptwi_2);
            ptwi_2->setFlags(ptwi_2->flags()&0xfffffffd);

            QTableWidgetItem* ptwi_3 = new QTableWidgetItem(confirmation.value(i).value(2));
            ui->tableWidget_confirmOrders->setItem(i, 2, ptwi_3);
            ptwi_3->setFlags(ptwi_3->flags()&0xfffffffd);

            QTableWidgetItem* ptwi_4 = new QTableWidgetItem(confirmation.value(i).value(3));
            ui->tableWidget_confirmOrders->setItem(i, 3, ptwi_4);
            ptwi_4->setFlags(ptwi_4->flags()&0xfffffffd);

            QTableWidgetItem* ptwi_5 = new QTableWidgetItem(confirmation.value(i).value(4));
            ui->tableWidget_confirmOrders->setItem(i, 4, ptwi_5);
            ptwi_5->setFlags(ptwi_5->flags()&0xfffffffd);

            QTableWidgetItem* ptwi_6 = new QTableWidgetItem(confirmation.value(i).value(5));
            ui->tableWidget_confirmOrders->setItem(i, 5, ptwi_6);
            ptwi_6->setFlags(ptwi_6->flags()&0xfffffffd);
        }
    }
}

void Confirm_order::on_pushButton_confirmation_clicked() //подтверждение заказа
{
    int currentOrder = ui->tableWidget_confirmOrders->currentRow();
    int order_id = ui->tableWidget_confirmOrders->item(currentOrder, 0)->text().toInt();
    ui->tableWidget_confirmOrders->removeRow(currentOrder);

    if (ui->tableWidget_confirmOrders->rowCount() == 0)
    {
        ui->tableWidget_confirmOrders->hide();
        ui->pushButton_confirmation->hide();
        ui->label_allConfirmed->setText("Все заказы подтверждены");
        ui->label_allConfirmed->show();
    }

    emit signalConfirmation(order_id);
}


void Confirm_order::on_pushButton_backToAdmin_clicked() //возвращение на главную страницу админа
{
    this->close();
}

