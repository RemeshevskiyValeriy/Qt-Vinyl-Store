#include "add_product.h"
#include "ui_add_product.h"

add_product::add_product(QWidget *parent) : //конструктор
    QWidget(parent),
    ui(new Ui::add_product)
{
    ui->setupUi(this);
    this->setWindowTitle("Добавление товара");
    ui->tableWidget_addProduct->setRowCount(ui->spinBox_productsCount->value());
    ui->tableWidget_addProduct->setColumnCount(7);
    for (int i=0; i<7; i++)
    {
        ui->tableWidget_addProduct->setColumnWidth(i, 150);
    }

    //Заголовки таблицы
    QStringList list_labels;
    list_labels << "Исполнитель" << "Название альбома" << "Жанр" << "Год выпуска" << "Страна" << "Стоимость, ₽" << "Количество";

    //Жанры
    list_genres << "Pop" << "Rock" << "Hip-Hop" << "R'n'B" << "Electronic" << "Alternative";

    ui->tableWidget_addProduct->setHorizontalHeaderLabels(list_labels);

    for (int i=0; i<ui->tableWidget_addProduct->columnCount(); i++)
    {
        QTableWidgetItem* ti = new QTableWidgetItem;
        ui->tableWidget_addProduct->setItem(0, i, ti);
    }

    QComboBox *comboBox = new QComboBox(ui->tableWidget_addProduct);
    comboBox->addItems(list_genres);
    ui->tableWidget_addProduct->setCellWidget(0, 2, comboBox);

    flag_auto = true;
}

add_product::~add_product() //деструктор
{
    delete ui;
}

void add_product::on_spinBox_productsCount_valueChanged(int arg1) //добавление строк в таблицу
{
    int rows = ui->tableWidget_addProduct->rowCount();
    ui->tableWidget_addProduct->setRowCount(arg1);

    for (int i=rows; i<arg1; i++)
    {
        for (int j=0; j<ui->tableWidget_addProduct->columnCount(); j++)
        {
            QTableWidgetItem* ti = new QTableWidgetItem;
            ui->tableWidget_addProduct->setItem(i, j, ti);
        }
    }

    for (int i=rows; i<arg1; i++)
    {
        QComboBox *comboBox = new QComboBox(ui->tableWidget_addProduct);
        comboBox->addItems(list_genres);
        ui->tableWidget_addProduct->setCellWidget(i, 2, comboBox);
    }
}


void add_product::on_tableWidget_addProduct_cellChanged(int row, int column) //перекраска ячеек в бедый цвет после редактирования
{
    if (flag_auto)
    {
        if (!ui->tableWidget_addProduct->item(row, column)->text().isEmpty())
        {
            ui->tableWidget_addProduct->item(row, column)->setBackground(QBrush(Qt::white));
        }
    }
}


void add_product::on_pushButton_addToCatalog_clicked() //добавление товаров в каталог
{
    flag_auto = false;

    addedProducts.clear();

    bool int_flag;
    bool errorFlag = true;
    for (int i=0; i<ui->tableWidget_addProduct->rowCount(); i++)
    {
        for (int j=0; j<ui->tableWidget_addProduct->columnCount(); j++)
        {
            if (j == 3 || j == 5 || j == 6)
            {
                ui->tableWidget_addProduct->item(i, j)->text().toInt(&int_flag);
                if (!int_flag)
                {
                    ui->tableWidget_addProduct->item(i, j)->setBackground(QBrush(Qt::red));
                    errorFlag = false;
                }
            }
            else
            {
                if (ui->tableWidget_addProduct->item(i, j)->text().isEmpty() && j != 2)
                {
                    ui->tableWidget_addProduct->item(i, j)->setBackground(QBrush(Qt::red));
                    errorFlag = false;
                }
            }
        }
    }

    flag_auto = true;

    if (!errorFlag)
    {
        QMessageBox::warning(this, "Error", "Данные для добавления товара введены некорректно");
    }
    else
    {
        //Заполняем контейнер информацией о добавляемых товарах
        for (int i=0; i<ui->tableWidget_addProduct->rowCount(); i++)
        {
            QVector<QString> tempVector;
            for (int j=0; j<ui->tableWidget_addProduct->columnCount(); j++)
            {
                if (j != 2)
                {
                    tempVector.push_back(ui->tableWidget_addProduct->item(i, j)->text());
                }
                else
                {
                    QComboBox *cb = qobject_cast<QComboBox*>(ui->tableWidget_addProduct->cellWidget(i,2));
                    tempVector.push_back(cb->currentText());
                }
            }
            addedProducts.push_back(tempVector);
        }

        emit signalProductsAdded(addedProducts);

        this->close();
    }
}


void add_product::on_pushButton_backToAdmin_clicked() //возвращаемся на главную страницу
{
    this->close();
}

