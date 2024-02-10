#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QSqlRecord>
#include <QValidator>
#include <QListWidget>
#include <QIcon>
#include <QFont>
#include <cart.h>
#include <QRadioButton>
#include <QGridLayout>
#include <QDate>
#include <purchase.h>
#include <QSqlQueryModel>
#include <confirm_order.h>
#include <add_product.h>
#include <QSqlTableModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_radioButton_user_clicked();

    void on_pushButton_registration_clicked();

    void on_pushButton_autorization_clicked();

    void on_pushButton_go_clicked();

    bool checkRegDataFill();

    void on_lineEdit_login_textChanged(const QString &arg1);

    void on_lineEdit_password_textChanged(const QString &arg1);

    void on_lineEdit_mail_textChanged(const QString &arg1);

    void on_lineEdit_age_textChanged(const QString &arg1);

    void on_radioButton_male_clicked();

    void on_radioButton_female_clicked();

    bool checkAuthDataFill();

    void on_pushButton_auth_clicked();

    bool checkMPSelected();

    void on_pushButton_saveAnswer_clicked();

    void catalogFilling();

    void on_listWidget_catalog_itemClicked(QListWidgetItem *item);

    void on_pushButton_addToCart_clicked();

    void on_pushButton_cart_clicked();

    void on_pushButton_back_clicked();

    void on_pushButton_changeMP_clicked();

    void on_pushButton_saveChanges_clicked();

    void on_pushButton_leaveReview_clicked();

    void on_pushButton_changeReview_clicked();

    void on_pushButton_saveReview_clicked();

    QString dateRebuild(QDate reviewDate);

    void on_pushButton_exit_clicked();

    void on_radioButton_administrator_clicked();

    void on_pushButton_adminAuth_clicked();

    void on_comboBox_table_currentIndexChanged(const QString &arg1);

    void on_pushButton_confirmOrder_clicked();

    void on_pushButton_addProduct_clicked();

    void on_pushButton_editData_clicked();

    void on_comboBox_editableTables_currentIndexChanged(const QString &arg1);


    void on_tableWidget_editTable_cellChanged(int row, int column);

public slots:
    void slotCart(int currentProduct); //обработка сигнала удаления товара из корзины

    void slotSetPurchase(QVector<QVector<QString>> userCart); //переход на страницу покупки

    void slotPurchaseIsCompleted(QVector<QVector<QString>> cartOfPurchase, QString format, int totalPrice); //обработка завершения покупки

    void slotBackToCatalog(); //возврат на страницу каталога

    void slotConfirmedOrder(int order_id); //слот обработки подтверждения заказа

    void slotAddedProducts(QVector<QVector<QString>> addedProducts); //обработка информации о добавленных товарах

signals:
    void signal(QVector<QVector<QString>> userCart); //сигнал открытия страницы корзины

    void signalPurchasePage(QVector<QVector<QString>> userCart, int orderN); //переход на страницу офрмления покупки

    void signalConfirm(QVector<QVector<QString>> confirmation); //сигнал перехода к подтверждению заказов

    void signalClearCart(); //сигнал очистки корзины после завершения покупки

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QSqlQuery *query;
    QString globalLogin = "";
    int user_id;
    int previousPage = 0;
    int orderN = 1;
    bool isReviewLeft;
    QSqlQueryModel *model;
    QSqlTableModel *modelTable;
    bool flag_auto;
    QStringList NA; //Северная Америка
    QStringList Latin; //Латинская Америка
    QStringList EUAustralia; //Европа и Австралия
    QStringList Asia; //Азия

    QGridLayout *lt_cart;
    QGridLayout *lt_purchase;

    QVector<QVector<QString>> userCart;
    QVector<QVector<QString>> productInfo;
    QVector<QRadioButton*> radioBtnList;

    Cart *cart;
    Purchase *purchase;
    Confirm_order *confirm;
    add_product *add;

};
#endif // MAINWINDOW_H
