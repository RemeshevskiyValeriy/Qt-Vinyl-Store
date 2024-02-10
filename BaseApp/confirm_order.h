#ifndef CONFIRM_ORDER_H
#define CONFIRM_ORDER_H

#include <QWidget>
#include <QSqlQueryModel>
#include <QDebug>

namespace Ui {
class Confirm_order;
}

class Confirm_order : public QWidget
{
    Q_OBJECT

public:
    explicit Confirm_order(QWidget *parent = nullptr);
    ~Confirm_order();

private:
    Ui::Confirm_order *ui;

public slots:
    void slotConfirm(QVector<QVector<QString>> confirmation);

signals:
    void signalConfirmation(int order_id);

private slots:
    void on_pushButton_confirmation_clicked();
    void on_pushButton_backToAdmin_clicked();
};

#endif // CONFIRM_ORDER_H
