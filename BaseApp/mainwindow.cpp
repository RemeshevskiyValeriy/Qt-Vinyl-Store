#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) //конструктор
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //Предварительные манипуляции с настрйокй внешнего вида главного окна и пользовательским интерфейсом
    ui->setupUi(this);
    this->setWindowTitle("Martinvile");
    ui->stackedWidget->setCurrentIndex(0);
    ui->pushButton_saveAnswer->setEnabled(false);
    ui->pushButton_saveChanges->setEnabled(false);
    ui->pushButton_back->hide();
    ui->pushButton_changeMP->hide();
    ui->label_avatar->hide();
    ui->label_nickname->hide();
    ui->pushButton_changeReview->hide();
    ui->pushButton_saveReview->hide();
    ui->textEdit_leaveReview->hide();
    ui->pushButton_exit->hide();

    ui->listWidget_catalog->setIconSize(QSize(108, 108));
    ui->listWidget_catalog->setFont(QFont("Franklin Gothic Book", 16));
    ui->listWidget_catalog->setStyleSheet("QListWidget {padding: 10px;} QListWidget::item {margin: 10px;}");
    ui->listWidget_catalog->setWordWrap(true);

    ui->groupBox_whoAreYou->setStyleSheet("border:0;");

    ui->textBrowser_reviews->setFont(QFont("Franklin Gothic Book", 9));

    ui->pushButton_back->setFlat(true);
    QIcon icon_back = QIcon("../BaseApp/back.svg");
    ui->pushButton_back->setIcon(icon_back);

    QIcon icon_exit = QIcon("../BaseApp/exit-cross.svg");
    ui->pushButton_exit->setIconSize(QSize(15, 15));
    ui->pushButton_exit->setIcon(icon_exit);

    //Формируем контейнер с радиокнопками музыкальных предпочтений (их много!)
    for(int i=0; i<ui->verticalLayout_2->count(); ++i) //проход по всем элементам выравнивания
    {
        //Пытаемся преобразовать виджет-элемент в groupBox
        QGroupBox* group = qobject_cast<QGroupBox*>(ui->verticalLayout_2->itemAt(i)->widget());
        if(!group)
        {
            //Если не удалось, переходим к следующему элементу
            continue;
        }
        auto widgets = group->layout(); //определение геометрии выравнивания внутри groupBox`а
        for(int j = 0; j < widgets->count(); ++j) //проход по всем элементам groupBox`а
        {
            QRadioButton* btn = qobject_cast<QRadioButton*>(widgets->itemAt(j)->widget());
            if(!btn)
            {
                //Проверка, если элемент groupBox`а оказался не радиокнопкой
                continue;
            }
            radioBtnList.push_back(btn); //добавляем в контейнер радиокнопку
            connect(btn, SIGNAL(clicked()), this, SLOT(checkMPSelected()));
        }
    }

    //Создаем и проверяем подключение к базе данных
    db=QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("127.0.0.1");
    db.setDatabaseName("...");
    db.setUserName("...");
    db.setPassword("...");
    db.setPort(5432);

    if(!db.open()) //если не удалось установить соединение с БД
    {
        qDebug() << "Connection failed";
        QMessageBox::warning(this, "Error", db.lastError().text()); //вывод на экран ошибки соединения
        QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection); //закрытие окна главного приложения
    }
    else
    {
        qDebug() << "Connection successful";
        query = new QSqlQuery(db);
    }

    //Установка целочисленного валидатора на поле ввода возраста пользователя
    ui->lineEdit_age->setValidator(new QIntValidator(12, 99, this));

    //Установка режима ввода пароля
    ui->lineEdit_password->setEchoMode(QLineEdit::Password);
    ui->lineEdit_passwordEmployee->setEchoMode(QLineEdit::Password);

    //Валидатор имени пользователя
    QRegExp LoginRegex("^[a-zA-Z][a-zA-Z0-9_.]{1,50}$"); //создаем регулярное выражение логина
    QRegExpValidator *my_login_Validator = new QRegExpValidator(LoginRegex,this); //создаем валидатор регулярного выражения
    ui->lineEdit_login->setValidator(my_login_Validator);

    //Валидатор почты пользователя
    QRegExp MailRegex("^[-\\w.]+@([A-z0-9][-A-z0-9]+\\.)+[A-z]{2,4}$"); //создаем регулярное выражение логина
    QRegExpValidator *my_mail_Validator = new QRegExpValidator(MailRegex,this); //создаем валидатор регулярного выражения
    ui->lineEdit_mail->setValidator(my_mail_Validator);

    cart = new Cart; //создаем экземпляр корзины
    connect(this, &MainWindow::signal, cart, &Cart::slot); //связываем класс основного окна приложения и класс корзины
    connect(cart, &Cart::signalCart, this, &MainWindow::slotCart); //при удалении товара из корзины передаем данные из класса корзины в основной класс
    connect(cart, &Cart::signalOfPurchase, this, &MainWindow::slotSetPurchase); //открываем страницу покупки
    connect(this, &MainWindow::signalClearCart, cart, &Cart::slorClearCart); //очистка корзины после завершения покупки

    //Добавляем на страницу корзины виджет корзины с компоновкой по сетке
    lt_cart = new QGridLayout;
    lt_cart->addWidget(cart);
    ui->stackedWidget->widget(6)->setLayout(lt_cart);

    //Создаем экземпляр класса покупки и соединяем с классом основного окна
    purchase = new Purchase;
    connect(this, &MainWindow::signalPurchasePage, purchase, &Purchase::slotPurchase);
    connect(purchase, &Purchase::signalOfPurchaseCompletion, this, &MainWindow::slotPurchaseIsCompleted); //обновляем данные после завершения покупки
    connect(purchase, &Purchase::signalBackToCatalog, this, &MainWindow::slotBackToCatalog);

    //Добавляем на страницу покупки виджет покупки с компоновкой по сетке
    lt_purchase = new QGridLayout;
    lt_purchase->addWidget(purchase);
    ui->stackedWidget->widget(7)->setLayout(lt_purchase);

    model = new QSqlQueryModel(this); //создаем экземпляр класса QSqlQueryModel для отображения таблиц базы данных

    //Создаем экземпляр класса подтверждения заказа
    confirm = new Confirm_order;
    connect(this, &MainWindow::signalConfirm, confirm, &Confirm_order::slotConfirm); //передаем модель таблицы в класс подтверждения заказов
    connect(confirm, &Confirm_order::signalConfirmation, this, &MainWindow::slotConfirmedOrder); //узнаем идентификатор подтвержденного заказа

    //Создаем экземпляр класса добавления товара
    add = new add_product;
    connect(add, &add_product::signalProductsAdded, this, &MainWindow::slotAddedProducts); //получаем информацию о добавленных продуктах

    flag_auto = true;

    //Списки стран по регионам
    NA << "США" << "Канада";

    Latin << "Мексика" << "Пуэрто-Рико" << "Ямайка" << "Колумбия" << "Барбадос"
          << "Бразилия" << "Аргентина" << "Чили";

    EUAustralia << "Великобритания" << "Франция" << "Испания" << "Германия" << "Италия"
          << "Белоруссия" << "Украина" << "Россия" <<"Норвегия" << "Швеция" <<"Австралия";

    Asia << "Япония" << "Китай" << "Южная Корея" << "Турция" << "Казахстан" << "Индия";

}

MainWindow::~MainWindow() //деструктор
{
    delete ui;
    delete query;
}

bool MainWindow::checkRegDataFill() //функция-проверка заполнения данных регистрации
{
    if (!ui->lineEdit_login->text().isEmpty() && !ui->lineEdit_password->text().isEmpty()
        && !ui->lineEdit_mail->text().isEmpty() && !ui->lineEdit_age->text().isEmpty()
        && (ui->radioButton_male->isChecked() || ui->radioButton_female->isChecked()))
    {
        ui->pushButton_go->setEnabled(true);
        return true;
    }
    else
    {
        ui->pushButton_go->setEnabled(false);
        return false;
    }
}

bool MainWindow::checkAuthDataFill() //функция-проверка заполнения данных авторизации
{
    if (!ui->lineEdit_login->text().isEmpty() && !ui->lineEdit_password->text().isEmpty())
    {
        ui->pushButton_auth->setEnabled(true);
        globalLogin = ui->lineEdit_login->text();
        return true;
    }
    else
    {
        ui->pushButton_auth->setEnabled(false);
        return false;
    }
}

bool MainWindow::checkMPSelected() //функция-проверка выбора музыкальных предпочтений пользователя
{
    int checked = 0; //проверка выбора опций предпочтений

    for (int i=0; i<radioBtnList.size(); i++)
    {
        if (radioBtnList.value(i)->isChecked())
        {
            checked++;
        }
    }

    if (checked == 3) //если выбраны все опции предпочтений, то происходит разблокировка кнопки сохранения ответа
    {
        ui->pushButton_saveAnswer->setEnabled(true);
        ui->pushButton_saveChanges->setEnabled(true);
        return true;
    }
    else
    {
        ui->pushButton_saveAnswer->setEnabled(false);
        ui->pushButton_saveChanges->setEnabled(false);
        return false;
    }
}

void MainWindow::catalogFilling() //заполнение каталога с товарами
{
    productInfo.clear();
    ui->listWidget_catalog->clear();

    QSqlQuery queryMP;
    queryMP.prepare("SELECT genre_name, music_era, music_region "
                    "FROM music_preferences "
                    "JOIN genre ON music_preferences.genre_id = genre.genre_id "
                    "WHERE user_id = ?");
    queryMP.addBindValue(user_id);

    if (!queryMP.exec())
    {
        queryMP.lastError().text();
    }

    QString fav_genre;
    QString fav_era;
    QString fav_region;
    while (queryMP.next())
    {
        fav_genre=queryMP.value(0).toString();
        fav_era=queryMP.value(1).toString();
        fav_region=queryMP.value(2).toString();
    }

    QString item_info; //информация, отображаемая в каталоге
    QListWidgetItem *pitem = 0; //объявляем переменную для хранения элемента ListWidget (каталога)

    //Выполняем SQL-запрос, возвращающий таблицу с информацией об имеющихся в базе товарах
    if(!query->exec("SELECT product_id, artist.artist_name, record.record_name, record.record_release_date, "
                    "product_price, product_quantity, record.record_country, genre.genre_name "
                    "FROM product_catalog "
                    "JOIN record ON product_catalog.record_id=record.record_id "
                    "JOIN artist ON product_catalog.artist_id=artist.artist_id "
                    "JOIN genre ON record.genre_id=genre.genre_id "
                    "ORDER BY product_id ASC;"))
    {
        qDebug() << "Unable to execute query - exiting";
    }

    while (query->next()) //считываем построково результирующую таблицу SQL-запроса
    {
        QVector<QString> tempVector; //вектор для хранения необходимых атрибутов товара (строки)

        //Добавление атрибутов в вектор
        tempVector.push_back(query->value(0).toString());
        tempVector.push_back(query->value(1).toString());
        tempVector.push_back(query->value(2).toString());
        tempVector.push_back(query->value(3).toString());
        tempVector.push_back(query->value(4).toString());
        tempVector.push_back(query->value(5).toString());
        tempVector.push_back(query->value(6).toString());
        tempVector.push_back(query->value(7).toString());

        int mp_count = 0;
        if (query->value(7).toString() == fav_genre)
        {
            mp_count++;
        }

        if (fav_era == "70-е" && query->value(3).toInt() >= 1970 && query->value(3).toInt() <= 1979)
        {
            mp_count++;
        }
        else if (fav_era == "80-е" && query->value(3).toInt() >= 1980 && query->value(3).toInt() <= 1989)
        {
            mp_count++;
        }
        else if (fav_era == "90-е" && query->value(3).toInt() >= 1990 && query->value(3).toInt() <= 1999)
        {
            mp_count++;
        }
        else if (fav_era == "00-е" && query->value(3).toInt() >= 2000 && query->value(3).toInt() <= 2009)
        {
            mp_count++;
        }
        else if (fav_era == "10-е" && query->value(3).toInt() >= 2010 && query->value(3).toInt() <= 2019)
        {
            mp_count++;
        }
        else if (fav_era == "20-е" && query->value(3).toInt() >= 2020 && query->value(3).toInt() <= 2029)
        {
            mp_count++;
        }

        if (fav_region == "Северная Америка")
        {
            for (int i=0; i<NA.size(); i++)
            {
                if (query->value(6).toString() == NA[i])
                {
                    mp_count++;
                }
            }
        }
        else if (fav_region == "Европа и Австралия")
        {
            for (int i=0; i<EUAustralia.size(); i++)
            {
                if (query->value(6).toString() == EUAustralia[i])
                {
                    mp_count++;
                }
            }
        }
        else if (fav_region == "Латинская Америка")
        {
            for (int i=0; i<Latin.size(); i++)
            {
                if (query->value(6).toString() == Latin[i])
                {
                    mp_count++;
                }
            }
        }
        else if (fav_region == "Азия")
        {
            for (int i=0; i<Asia.size(); i++)
            {
                if (query->value(6).toString() == Asia[i])
                {
                    mp_count++;
                }
            }
        }

        if (mp_count == 0)
        {
            item_info = tempVector.value(1) + " - " + tempVector.value(2) +
                        "    Стоимость: " + tempVector.value(4) + " ₽"; //информация, отображаемая в каталоге
        }
        else if (mp_count == 1)
        {
            item_info = tempVector.value(1) + " - " + tempVector.value(2) +
                        "    Стоимость: " + tempVector.value(4) + " ₽" + "     Вам подходит: ✓"; //информация, отображаемая в каталоге
        }
        else if (mp_count == 2)
        {
            item_info = tempVector.value(1) + " - " + tempVector.value(2) +
                        "    Стоимость: " + tempVector.value(4) + " ₽" + "     Вам подходит: ✓✓"; //информация, отображаемая в каталоге
        }
        else if (mp_count == 3)
        {
            item_info = tempVector.value(1) + " - " + tempVector.value(2) +
                        "    Стоимость: " + tempVector.value(4) + " ₽" + "     Вам подходит: ✓✓✓"; //информация, отображаемая в каталоге
        }

        //Формируем ячейку (товар) каталога
        pitem = new QListWidgetItem(item_info);
        QIcon icon = QIcon("../BaseApp/AlbumCovers/" + tempVector.value(2) + ".jpg");
        pitem->setIcon(icon);
        ui->listWidget_catalog->addItem(pitem);

        productInfo.push_back(tempVector); //добавляем информацию в контейнер об одном товаре имеющихся в базе товаров
    }
}

void MainWindow::on_radioButton_user_clicked() //используем приложение в качестве пользователя
{
    ui->pushButton_back->show();
    ui->pushButton_go->setEnabled(true);
    ui->stackedWidget->setCurrentIndex(1);
    previousPage = 0;
}


void MainWindow::on_pushButton_registration_clicked() //настройка интерфейса для создания новой учетной записи
{
    ui->label_invitation->setText("Введите данные для регистрации");
    ui->lineEdit_login->show();
    ui->lineEdit_login->clear();
    ui->lineEdit_password->show();
    ui->lineEdit_password->clear();
    ui->lineEdit_mail->show();
    ui->lineEdit_mail->clear();
    ui->lineEdit_age->show();
    ui->lineEdit_age->clear();
    ui->groupBox_sex->show();
    ui->pushButton_go->show();
    ui->pushButton_go->setEnabled(false);
    ui->pushButton_auth->hide();

    ui->radioButton_male->setAutoExclusive(false);
    ui->radioButton_male->setChecked(false);
    ui->radioButton_male->setAutoExclusive(true);

    ui->radioButton_female->setAutoExclusive(false);
    ui->radioButton_female->setChecked(false);
    ui->radioButton_female->setAutoExclusive(true);

    ui->stackedWidget->setCurrentIndex(2);
    previousPage = 1;
}


void MainWindow::on_pushButton_autorization_clicked() //настройка интерфейса для авторизации учетной записи
{
    ui->label_invitation->setText("Введите данные для авторизации");
    ui->lineEdit_login->show();
    ui->lineEdit_login->clear();
    ui->lineEdit_password->show();
    ui->lineEdit_password->clear();
    ui->lineEdit_mail->hide();
    ui->lineEdit_age->hide();
    ui->groupBox_sex->hide();
    ui->pushButton_auth->show();
    ui->pushButton_auth->setEnabled(false);
    ui->pushButton_go->hide();

    ui->stackedWidget->setCurrentIndex(2);
    previousPage = 1;
}


void MainWindow::on_pushButton_go_clicked() //регистрация новой учетной записи
{
    if (!ui->lineEdit_login->hasAcceptableInput())
    {
        QMessageBox::warning(this, "Error", "Введен недопустимый логин");
    }
    else if (ui->lineEdit_password->text().length() < 5)
    {
        QMessageBox::warning(this, "Error", "Пароль должен состоять минимум из 5 символов");
    }
    else if (!ui->lineEdit_mail->hasAcceptableInput())
    {
        QMessageBox::warning(this, "Error", "Адрес почтового ящика введен некорректно");
    }
    else if (!ui->lineEdit_age->hasAcceptableInput())
    {
        QMessageBox::warning(this, "Error", "Значение возраста введено некорректно");
    }
    else
    {
        //Объявляем переменные для хранения регистрационных данных пользователя
        QString login = ui->lineEdit_login->text();
        QString password = ui->lineEdit_password->text();
        QString mail = ui->lineEdit_mail->text();
        int age = ui->lineEdit_age->text().toInt();
        QString sex;
        if (ui->radioButton_male->isChecked())
        {
            sex = "Male";
        }
        else
        {
            sex = "Female";
        }

        //Проверяем наличие в базе данных пользователя с уже существующим логином
        QString loginEx = ""; //строковая переменная для хранения логина уже существующего
        QSqlQuery querySelect;
        querySelect.prepare("SELECT user_name FROM users WHERE user_name = ?");
        querySelect.addBindValue(login); //привязываем значение введенного логина к параметру запроса
        querySelect.exec(); //выполнение запроса

        while (querySelect.next()) {
            //считываем значение логина по результатам выполнения запроса
            loginEx = querySelect.value(0).toString();
        }

        //Если был найден совпадающий логин, выводим ошибку, иначе выполняем запрос добавления нового пользователя в БД
        if (loginEx == login)
        {
            QMessageBox::warning(this, "Ошибка", "Пользователь с таким логином уже существует!");
        }
        else
        {
            query->prepare("INSERT INTO users (user_name, user_password, user_sex, user_age, user_mail)"
                           "VALUES (?,?,?,?,?);");
            query->addBindValue(login);
            query->addBindValue(password);
            query->addBindValue(sex);
            query->addBindValue(age);
            query->addBindValue(mail);

            if (!query->exec())
            {
                qDebug() << "Unable to make insert operation";
            }

            ui->stackedWidget->setCurrentIndex(3);
            previousPage = 1;

            for (int i=0; i<radioBtnList.size(); i++)
            {
                //Снятие активации радиокнопок
                radioBtnList.value(i)->setAutoExclusive(false);
                radioBtnList.value(i)->setChecked(false);
                radioBtnList.value(i)->setAutoExclusive(true);
            }

            //Настройка интерфейса
            globalLogin = ui->lineEdit_login->text();
            ui->pushButton_go->setEnabled(false);
            ui->pushButton_saveChanges->hide();
            ui->label_musicPreferences->show();
            ui->pushButton_saveAnswer->show();
            ui->pushButton_saveAnswer->setEnabled(false);

            //Запомним идентификатор пользователя
            querySelect.prepare("SELECT user_id FROM users WHERE user_name = ?");
            querySelect.addBindValue(globalLogin); //привязываем значение введенного логина к параметру запроса
            querySelect.exec(); //выполнение запроса

            while (querySelect.next()) {
                //считываем значение идентификатора пользователя по результатам выполнения запроса
                user_id = querySelect.value(0).toInt();
            }
        }
    }
}


void MainWindow::on_lineEdit_login_textChanged(const QString &arg1) //проверка заполненности данных для регистрации/авторизации
{
    Q_UNUSED(arg1);
    checkRegDataFill();
    checkAuthDataFill();
}


void MainWindow::on_lineEdit_password_textChanged(const QString &arg1) //проверка заполненности данных для регистрации/авторизации
{
    Q_UNUSED(arg1);
    checkRegDataFill();
    checkAuthDataFill();
}


void MainWindow::on_lineEdit_mail_textChanged(const QString &arg1) //проверка заполненности данных для регистрации
{
    Q_UNUSED(arg1);
    checkRegDataFill();
}


void MainWindow::on_lineEdit_age_textChanged(const QString &arg1) //проверка заполненности данных для регистрации
{
    Q_UNUSED(arg1);
    checkRegDataFill();
}


void MainWindow::on_radioButton_male_clicked() //проверка заполненности данных для регистрации
{
    checkRegDataFill();
}


void MainWindow::on_radioButton_female_clicked() //проверка заполненности данных для регистрации
{
    checkRegDataFill();
}


void MainWindow::on_pushButton_auth_clicked() //авторизация учетной записи
{
    ui->listWidget_catalog->clear();
    userCart.clear();

    //Объявляем переменные для хранения данных для авторизации пользователя
    QString login = ui->lineEdit_login->text();
    QString password = ui->lineEdit_password->text();

    //Проверяем наличие учетной записи пользователя в базе данных
    QString loginEx = ""; //строковая переменная для считывания логина из базы данных
    QString passwordEx = ""; //строковая переменная для считывания пароля из базы данных
    QSqlQuery querySelect;
    querySelect.prepare("SELECT user_name, user_password, user_id FROM users WHERE user_name = ? AND user_password = ?");
    querySelect.addBindValue(login); //привязываем значение введенного логина к параметру запроса
    querySelect.addBindValue(password); //привязываем значение введенного пароля к параметру запроса
    querySelect.exec(); //выполнение запроса

    while (querySelect.next()) {
        //считываем значение логина и пароля по результатам выполнения запроса
        loginEx = querySelect.value(0).toString();
        passwordEx = querySelect.value(1).toString();
        user_id = querySelect.value(2).toInt();
    }

    //Если логин и пароль совпали, переходим в каталог товаров, иначе выводим сообщение об ошибке
    if (loginEx == login && passwordEx == password)
    {
        globalLogin = ui->lineEdit_login->text();
        ui->stackedWidget->setCurrentIndex(4); //переходим на страницу каталога
        previousPage = 2;
        catalogFilling();
        ui->pushButton_changeMP->show();
        ui->label_avatar->show();
        QPixmap pixmap("../BaseApp/avatar.jpg");
        ui->label_avatar->setPixmap(pixmap);
        ui->label_nickname->show();
        ui->label_nickname->setText(globalLogin);
        ui->pushButton_exit->show();
        ui->pushButton_back->show();

        //С помощью SQL-запроса формируем информацию о корзине пользователя
        query->prepare("SELECT "
                       "product_catalog.product_id, artist.artist_name, record.record_name, "
                       "record.record_release_date, product_catalog.product_price, cart.quantity "
                       "FROM product_catalog "
                       "JOIN cart ON product_catalog.product_id = cart.product_id "
                       "JOIN users ON cart.user_id = users.user_id "
                       "JOIN record ON product_catalog.record_id = record.record_id "
                       "JOIN artist ON record.artist_id = artist.artist_id "
                       "WHERE users.user_name = ?");
        query->addBindValue(login);
        query->exec();

        while (query->next()) //Дополнительно организуем проверку на возможность нахождения товара (-ов) в корзине
        {
            //Находим идентификатор товара из библиотеки, чтобы в дальнейшем сравнить с ним товар из корзины
            int row = -1;
            for (int i=0; i<productInfo.size(); i++)
            {
                if (query->value(0) == productInfo.value(i).value(0).toInt())
                {
                    row = i;
                    break;
                }
            }

            //Если количесвто одной единицы товара не превышает количество имеющихся экземпляров данной единицы товара
            //И если товар из корзины все еще числится в каталоге, то оставляем данную единицу товара в корзине
            if (row != -1 && query->value(5) <= productInfo.value(row).value(5).toInt())
            {
                QVector<QString> tempVector; //контейнер для хранения информации о содержащемся в корзине товаре

                //Добавление атрибутов товара
                tempVector.push_back(query->value(0).toString());
                tempVector.push_back(query->value(1).toString());
                tempVector.push_back(query->value(2).toString());
                tempVector.push_back(query->value(3).toString());
                tempVector.push_back(query->value(4).toString());
                tempVector.push_back(query->value(5).toString());

                userCart.push_back(tempVector); //записываем товар из корзины пользователя в контейнер
            }
            else //если превышает, то удаляем все экземпляры данной единицы товара из корзины и сообщаем об этом пользователю
            {
                QMessageBox::warning(this, "Error", "В вашей корзине произошли изменения");

                QSqlQuery queryDelete;
                queryDelete.prepare("DELETE FROM cart WHERE user_id = ? AND product_id = ?");
                queryDelete.addBindValue(user_id);
                queryDelete.addBindValue(query->value(0));

                if (!queryDelete.exec())
                {
                    qDebug() << "Unable to make delete operation";
                }
            }
        }

        //Изменим информацию в библиотеке товаров (не в самой БД!) в соответствии с содержимым корзины пользователя
        if (!userCart.isEmpty())
        {
            for (int i=0; i<userCart.size(); i++)
            {
                for (int j=0; j<productInfo.size(); j++)
                {
                    if (userCart.value(i).value(0) == productInfo.value(j).value(0))
                    {
                        int tq = productInfo.value(j).value(5).toInt() - userCart.value(i).value(5).toInt();
                        productInfo[j][5] = QString::number(tq);
                    }
                }
            }
        }
    }
    else
    {
        QMessageBox::warning(this, "Ошибка", "Логин или пароль введены неверно!");
    }
}


void MainWindow::on_pushButton_saveAnswer_clicked() //Сохранение результатов опроса о музыкальных предпочтениях
{
    QString genre = "";
    QString era = "";
    QString region = "";

    //Формируем контейнер с радиокнопками музыкальных предпочтений (их много!)
    for(int i=0; i<ui->verticalLayout_2->count(); ++i) //проход по всем элементам выравнивания
    {
        //Пытаемся преобразовать виджет-элемент в groupBox
        QGroupBox* group = qobject_cast<QGroupBox*>(ui->verticalLayout_2->itemAt(i)->widget());
        if(!group)
        {
            //Если не удалось, переходим к следующему элементу
            continue;
            qDebug() << "Check";
        }
        auto widgets = group->layout(); //определение геометрии выравнивания внутри groupBox`а
        for(int j = 0; j < widgets->count(); ++j) //проход по всем элементам groupBox`а
        {
            QRadioButton* btn = qobject_cast<QRadioButton*>(widgets->itemAt(j)->widget());
            if(!btn)
            {
                //Проверка, если элемент groupBox`а оказался не радиокнопкой
                continue;
            }
            if (i==1 && btn->isChecked())
            {
                genre = btn->text();
                break;
            }
            else if (i==2 && btn->isChecked())
            {
                era = btn->text();
                break;
            }
            else if (i==3 && btn->isChecked())
            {
                region = btn->text();
                break;
            }
        }
    }

    int genreID;
    QSqlQuery querySelect_genreID;
    querySelect_genreID.prepare("SELECT genre_id FROM genre WHERE genre_name = ?");
    querySelect_genreID.addBindValue(genre);
    querySelect_genreID.exec();
    while (querySelect_genreID.next()) {
        //считываем значение идентификатора жанра по результатам выполнения запроса
        genreID = querySelect_genreID.value(0).toInt();
    }

    query->prepare("INSERT INTO music_preferences (user_id, music_era, genre_id, music_region)"
                   "VALUES (?,?,?,?);");
    query->addBindValue(user_id);
    query->addBindValue(era);
    query->addBindValue(genreID);
    query->addBindValue(region);

    if (!query->exec())
    {
        qDebug() << query->lastError().text();
        qDebug() << "Unable to make insert operation";
    }

    ui->stackedWidget->setCurrentIndex(4);
    previousPage = 1;

    catalogFilling();

    //Настройка интерфейса
    ui->pushButton_back->show();
    ui->pushButton_changeMP->show();
    ui->label_avatar->show();
    QPixmap pixmap("../BaseApp/avatar.jpg");
    ui->label_avatar->setPixmap(pixmap);
    ui->label_nickname->show();
    ui->label_nickname->setText(globalLogin);
    ui->pushButton_exit->show();
}



void MainWindow::on_listWidget_catalog_itemClicked(QListWidgetItem *item) //обработка сигнала выбора элемента каталога
{
    Q_UNUSED(item);

    ui->stackedWidget->setCurrentIndex(5); //переходим на страницу карточки товара
    previousPage = 4;
    ui->pushButton_changeMP->hide();

    //находим индекс выбранного элемента, чтобы можно было извлекать данные из нужной строки массива
    int item_index = ui->listWidget_catalog->currentRow();

    //Добавляем изображение обложки альбома
    QPixmap pixmap("../BaseApp/AlbumCovers/" + productInfo.value(item_index).value(2) + ".jpg");
    ui->label_iconItem->setPixmap(pixmap);

    //Добавляем информацию об альбоме
    ui->label_album->setText(productInfo.value(item_index).value(1) + " - " + productInfo.value(item_index).value(2) + " \n" +
                             productInfo.value(item_index).value(3));
    ui->label_genre->setText("Жанр: " + productInfo.value(item_index).value(7));
    ui->label_country->setText("Страна: " + productInfo.value(item_index).value(6));
    ui->label_quantity->setText("В наличии: " + productInfo.value(item_index).value(5));
    ui->label_price->setText("Стоимость - " + productInfo.value(item_index).value(4) + " ₽");

    if (productInfo.value(item_index).value(5) != "0")
    {
        ui->pushButton_addToCart->setEnabled(true);
        ui->pushButton_addToCart->setText("Добавить в корзину");
    }
    else
    {
        ui->pushButton_addToCart->setEnabled(false);
        ui->pushButton_addToCart->setText("Вы не можете добавить товар в корзину");
    }

    //Формируем рецензии
    ui->textBrowser_reviews->clear();
    ui->textEdit_leaveReview->clear();

    query->prepare("SELECT users.user_name, users.user_id, "
                   "review.product_id, review.review_date, review.review_text "
                   "FROM users "
                   "JOIN review ON users.user_id = review.user_id "
                   "WHERE product_id = ? "
                   "ORDER BY review.review_date ASC");
    query->addBindValue(productInfo.value(item_index).value(0).toInt());

    if (!query->exec())
    {
        qDebug() << "Unable to make selection";
    }

    while (query->next()) //считываем построково результирующую таблицу SQL-запроса
    {
        QString authorNmae = query->value(0).toString();
        QDate reviewDate = query->value(3).toDate();
        QString reviewText = query->value(4).toString();

        ui->textBrowser_reviews->append(dateRebuild(reviewDate));
        ui->textBrowser_reviews->append("Пользователь: " + authorNmae);
        ui->textBrowser_reviews->append(reviewText + '\n');

        //Проверяем, оставлял ли текущий пользователь рецензию ранее
        if (user_id == query->value(1).toInt())
        {
            ui->pushButton_leaveReview->hide();
            ui->pushButton_changeReview->show();
            ui->textEdit_leaveReview->append(reviewText);
            ui->textEdit_leaveReview->hide();
            ui->pushButton_saveReview->hide();
        }
        else
        {
            ui->pushButton_changeReview->hide();
            ui->pushButton_leaveReview->show();
            ui->textEdit_leaveReview->clear();
            ui->textEdit_leaveReview->hide();
            ui->pushButton_saveReview->hide();
        }
    }
}


void MainWindow::on_pushButton_addToCart_clicked() //добавление товара в корзину
{
    ui->stackedWidget->setCurrentIndex(4); //возврат на страницу каталога
    previousPage = 3;
    ui->pushButton_changeMP->show();

    //Считываем позицию выбранного товара
    int item_index = ui->listWidget_catalog->currentRow();

    QVector<QString> tempVector;
    for(int j=0; j<5; j++)
    {
        //Считываем информацию о добавленном в корзину товаре
        tempVector.push_back(productInfo.value(item_index).value(j));
    }

    //Уменьшаем количесвто доступных товаров на единицу
    int quantity = productInfo.value(item_index).value(5).toInt() - 1;
    QString sq = QString::number(quantity);
    productInfo[item_index][5] = sq;

    if (userCart.isEmpty())
    {
        //Если корзина пуста, то сразу добавляем товар в корзину
        tempVector.push_back("1");
        userCart.push_back(tempVector); //добавляем товар в корзину пользователя

        query->prepare("INSERT INTO cart (user_id, product_id, quantity) VALUES (?,?,?)");
        query->addBindValue(user_id);
        query->addBindValue(productInfo.value(item_index).value(0).toInt());
        query->addBindValue(1);

        if (!query->exec())
        {
            qDebug() << "Unable to make insertation";
        }
    }
    else //если не пуста, то проверяем наличие этого же товара в корзине
    {
        bool found = false; //индикатор обнаружения товара
        for (int i=0; i<userCart.size(); i++)
        {
            if (tempVector.value(0) == userCart.value(i).value(0))
            {
                //Если данный товар уже имеется, то не заносим в контейнер новую строку, а обновляем старую
                int tq = userCart.value(i).value(5).toInt();
                tq++;
                userCart[i][5] = QString::number(tq);
                found = true;

                //Обновляем информацию о количестве данной единицы товара в БД
                query->prepare("UPDATE cart "
                               "SET quantity = ? "
                               "WHERE user_id = ? AND product_id = ?");
                query->addBindValue(userCart.value(i).value(5).toInt());
                query->addBindValue(user_id);
                query->addBindValue(userCart.value(i).value(0).toInt());

                if (!query->exec())
                {
                    qDebug() << query->lastError().text();
                    qDebug() << "Unable to make update operation";
                }

                break;
            }
        }
        if (!found) //если такого товара нет в корзине, то просто добавляем
        {
            tempVector.push_back("1");
            userCart.push_back(tempVector); //добавляем товар в корзину пользователя

            query->prepare("INSERT INTO cart (user_id, product_id, quantity) VALUES (?,?,?)");
            query->addBindValue(user_id);
            query->addBindValue(productInfo.value(item_index).value(0).toInt());
            query->addBindValue(1);

            if (!query->exec())
            {
                qDebug() << "Unable to make insertation";
            }
        }
    }
}


void MainWindow::on_pushButton_cart_clicked() //просмотр карзины
{
    emit signal(userCart);

    ui->pushButton_changeMP->hide();
    previousPage = 4;
    ui->stackedWidget->setCurrentWidget(ui->page_cart);

}

void MainWindow::slotCart(int currentProduct) //удаление продукта из корзины
{
    for (int i=0; i<productInfo.size(); i++)
    {
        if (productInfo.value(i).value(0) == userCart.value(currentProduct).value(0))
        {
            int tq = productInfo.value(i).value(5).toInt() + userCart.value(currentProduct).value(5).toInt();
            productInfo[i][5] = QString::number(tq);
        }
    }

    query->prepare("DELETE FROM cart WHERE user_id = ? AND product_id = ?");
    query->addBindValue(user_id);
    query->addBindValue(userCart.value(currentProduct).value(0).toInt());

    if (!query->exec())
    {
        qDebug() << "Unable to make insertation";
    }

    userCart.removeAt(currentProduct);
}

void MainWindow::slotSetPurchase(QVector<QVector<QString>> userCart) //Переход на страницу оформления покупки
{
    query->exec("SELECT order_id FROM purchase ORDER BY order_id DESC LIMIT 1");
    while (query->next())
    {
        orderN = query->value(0).toInt() + 1;
    }

    emit signalPurchasePage(userCart, orderN);

    ui->stackedWidget->setCurrentWidget(ui->page_purchase);
    previousPage = 4;
}

void MainWindow::slotPurchaseIsCompleted(QVector<QVector<QString>> cartOfPurchase, QString format, int totalPrice) //получаем информацию после завершения покупки
{   
    ui->pushButton_back->hide();

    userCart.clear();

    QDate orderDate = QDate::currentDate();

    for (int i=0; i<productInfo.size(); i++)
    {
        //Обновляем информацию о количесвте товара в каталоге в БД
        query->prepare("UPDATE product_catalog "
                       "SET product_quantity = ? "
                       "WHERE product_id = ?");
        query->addBindValue(productInfo.value(i).value(5).toInt());
        query->addBindValue(productInfo.value(i).value(0).toInt());

        if (!query->exec())
        {
            qDebug() << query->lastError().text();
            qDebug() << "Unable to make update operation";
        }

        //Удаляем записи с товарами в корзине у пользователя в БД
        query->prepare("DELETE FROM cart WHERE user_id = ? AND product_id = ?");
        query->addBindValue(user_id);
        query->addBindValue(productInfo.value(i).value(0).toInt());

        if (!query->exec())
        {
            qDebug() << query->lastError().text();
            qDebug() << "Unable to make insertation";
        }
    }

    //Добавляем в БД информацию о заказе
    query->prepare("INSERT INTO purchase (user_id, order_format, order_date, order_state, order_sum) VALUES (?,?,?,?,?)");
    query->addBindValue(user_id);
    query->addBindValue(format);
    query->addBindValue(orderDate);
    query->addBindValue("В обработке");
    query->addBindValue(totalPrice);

    if (!query->exec())
    {
        qDebug() << query->lastError().text();
        qDebug() << "Unable to make insertation";
    }

    //Также заполняем информация в сущности-связки заказа и товаров
    for (int i=0; i<cartOfPurchase.size(); i++)
    {
        query->prepare("INSERT INTO bound (order_id, product_id) VALUES (?,?)");
        query->addBindValue(orderN);
        query->addBindValue(cartOfPurchase.value(i).value(0).toInt());

        if (!query->exec())
        {
            qDebug() << query->lastError().text();
            qDebug() << "Unable to make insertation";
        }
    }

    emit signalClearCart();
}

void MainWindow::slotBackToCatalog() //возврат на страницу каталога из страницы покупки
{
    ui->stackedWidget->setCurrentIndex(4);
    ui->pushButton_back->show();
    previousPage = 3;
}

void MainWindow::slotConfirmedOrder(int order_id) //подтвержденный заказ
{
    query->prepare("UPDATE purchase "
                   "SET order_state = 'Обработан' "
                   "WHERE order_id = ?");
    query->addBindValue(order_id);

    if (!query->exec())
    {
        qDebug() << "Unable to make update";
    }

    QString curTable = ui->comboBox_table->currentText();

    if (curTable == "Данные заказов")
    {
        model->clear();
        model->setQuery("SELECT order_id, user_name, order_format, order_date, order_state, order_sum "
                        "FROM purchase "
                        "JOIN users ON purchase.user_id = users.user_id "
                        "ORDER BY order_id");
        model->setHeaderData(0, Qt::Horizontal, "Номер заказа");
        model->setHeaderData(1, Qt::Horizontal, "Имя пользователя");
        model->setHeaderData(2, Qt::Horizontal, "Формат заказа");
        model->setHeaderData(3, Qt::Horizontal, "Дата оформления");
        model->setHeaderData(4, Qt::Horizontal, "Статус заказа");
        model->setHeaderData(5, Qt::Horizontal, "Сумма заказа");

        ui->tableView_tables->setModel(model);
        ui->tableView_tables->show();
    }
}

void MainWindow::slotAddedProducts(QVector<QVector<QString>> addedProducts) //В каталог добавлены товары
{
    //Добавление нового товара или обновление количества уже имеющегося в каталоге товара
    for (int i=0; i<addedProducts.size(); i++)
    {
        query->prepare("SELECT artist.artist_name, record.record_name, "
                       "genre.genre_id, genre.genre_name, product_catalog.product_quantity "
                       "FROM artist "
                       "JOIN record ON artist.artist_id = record.artist_id "
                       "JOIN product_catalog ON artist.artist_id = product_catalog.artist_id "
                       "JOIN genre ON record.genre_id = genre.genre_id");
        query->exec();

        bool match_artist = false;
        bool match_record = false;
        while (query->next())
        {
            if (addedProducts.value(i).value(0) == query->value(0).toString())
            {
                match_artist = true;
            }
            if (addedProducts.value(i).value(1) == query->value(1).toString())
            {
                match_record = true;
            }
        }
        if (!match_artist) //если совпадений по артисту не обнаружено, добавляем в БД
        {
            QSqlQuery queryInsert_artist;
            queryInsert_artist.prepare("INSERT INTO artist (artist_name) VALUES (?)");
            queryInsert_artist.addBindValue(addedProducts.value(i).value(0));

            if (!queryInsert_artist.exec())
            {
                qDebug() << queryInsert_artist.lastError().text();
            }

        }
        if (!match_record) //если совпадений по пластинке не обнаружено, добавляем ее в БД
        {
            //Необходим атрибут artist_id
            int artist_id;
            QSqlQuery querySelect_artistID;
            querySelect_artistID.prepare("SELECT artist_id FROM artist WHERE artist_name = ?");
            querySelect_artistID.addBindValue(addedProducts.value(i).value(0));
            querySelect_artistID.exec();
            while (querySelect_artistID.next()) {
                artist_id = querySelect_artistID.value(0).toInt();
            }

            //Необходим атрибут genre_id
            int genre_id;
            QSqlQuery querySelect_genreID;
            querySelect_genreID.prepare("SELECT genre_id FROM genre WHERE genre_name = ?");
            querySelect_genreID.addBindValue(addedProducts.value(i).value(2));
            querySelect_genreID.exec();
            while (querySelect_genreID.next()) {
                genre_id = querySelect_genreID.value(0).toInt();
            }

            //Добавляем пластинку в базу данных
            QSqlQuery queryInsert_record;
            queryInsert_record.prepare("INSERT INTO record (artist_id, record_name, genre_id, record_country, record_release_date) VALUES (?,?,?,?,?)");
            queryInsert_record.addBindValue(artist_id);
            queryInsert_record.addBindValue(addedProducts.value(i).value(1));
            queryInsert_record.addBindValue(genre_id);
            queryInsert_record.addBindValue(addedProducts.value(i).value(4));
            queryInsert_record.addBindValue(addedProducts.value(i).value(3));

            if (!queryInsert_record.exec())
            {
                qDebug() << queryInsert_record.lastError().text();
            }

            //Необходим атрибут record_id
            int record_id;
            QSqlQuery querySelect_recordID;
            querySelect_recordID.prepare("SELECT record_id FROM record WHERE record_name = ?");
            querySelect_recordID.addBindValue(addedProducts.value(i).value(1));
            querySelect_recordID.exec();
            while (querySelect_recordID.next()) {
                record_id = querySelect_recordID.value(0).toInt();
            }

            //Добавляем товар в каталог
            QSqlQuery queryInsert_product;
            queryInsert_product.prepare("INSERT INTO product_catalog (artist_id, record_id, product_price, product_quantity) VALUES (?,?,?,?)");
            queryInsert_product.addBindValue(artist_id);
            queryInsert_product.addBindValue(record_id);
            queryInsert_product.addBindValue(addedProducts.value(i).value(5).toInt());
            queryInsert_product.addBindValue(addedProducts.value(i).value(6).toInt());

            if (!queryInsert_product.exec())
            {
                qDebug() << queryInsert_product.lastError().text();
            }
        }
        if (match_artist && match_record) //если товар уже есть в каталоге, обновляем его количество
        {
            //Необходим атрибут artist_id
            int artist_id;
            QSqlQuery querySelect_artistID;
            querySelect_artistID.prepare("SELECT artist_id FROM artist WHERE artist_name = ?");
            querySelect_artistID.addBindValue(addedProducts.value(i).value(0));
            querySelect_artistID.exec();
            while (querySelect_artistID.next()) {
                artist_id = querySelect_artistID.value(0).toInt();
            }

            //Необходим атрибут record_id
            int record_id;
            QSqlQuery querySelect_recordID;
            querySelect_recordID.prepare("SELECT record_id FROM record WHERE record_name = ?");
            querySelect_recordID.addBindValue(addedProducts.value(i).value(1));
            querySelect_recordID.exec();
            while (querySelect_recordID.next()) {
                record_id = querySelect_recordID.value(0).toInt();
            }

            //Необходим атрибут quantity
            int quantity;
            QSqlQuery querySel;
            querySel.prepare("SELECT product_quantity FROM product_catalog WHERE artist_id = ? AND record_id = ?");
            querySel.addBindValue(artist_id);
            querySel.addBindValue(record_id);
            querySel.exec();
            while (querySel.next()) {
                quantity = querySel.value(0).toInt();
            }

            //Обновляем
            QSqlQuery queryUpdate;
            queryUpdate.prepare("UPDATE product_catalog "
                                "SET product_quantity = ? "
                                "WHERE artist_id = ? AND record_id = ?");
            queryUpdate.addBindValue(quantity + addedProducts.value(i).value(6).toInt());
            queryUpdate.addBindValue(artist_id);
            queryUpdate.addBindValue(record_id);

            if (!queryUpdate.exec())
            {
                qDebug() << queryUpdate.lastError().text();
            }
        }
    }
}

void MainWindow::on_pushButton_back_clicked() //возврат на предыдущую страницу
{
    if (previousPage == 0) //если дошли до начала - скрываем кнопку возврата
    {
        ui->stackedWidget->setCurrentIndex(previousPage);

        ui->pushButton_back->hide();

        //Снимаем активацию радиокнопок
        ui->radioButton_user->setAutoExclusive(false);
        ui->radioButton_user->setChecked(false);
        ui->radioButton_user->setAutoExclusive(true);

        ui->radioButton_administrator->setAutoExclusive(false);
        ui->radioButton_administrator->setChecked(false);
        ui->radioButton_administrator->setAutoExclusive(true);

        ui->pushButton_exit->hide();
        ui->label_avatar->hide();
        ui->label_nickname->hide();
    }
    else
    {
        if (previousPage == 4)
        {
            //отображаем кнопку смены предпочтений только на странице каталога
            ui->pushButton_changeMP->show();
        }
        else
        {
            ui->pushButton_changeMP->hide();
        }

        if (previousPage == 3)
        {
            //Если страница возвращается на муз. предпочтения - возвращаемся еще
            previousPage--;
        }

        if (previousPage == 2 || previousPage == 1 || previousPage == 0)
        {
            ui->label_avatar->hide();
            ui->label_nickname->hide();
            ui->pushButton_exit->hide();
        }

        //Возвратное положение
        ui->stackedWidget->setCurrentIndex(previousPage);
        previousPage--;
        if (previousPage + 1 == 8)
        {
            previousPage = 0;
        }
    }
}


void MainWindow::on_pushButton_changeMP_clicked() //переход на страницу музыкальных предпочтений для их изменения
{
    ui->stackedWidget->setCurrentIndex(3);
    ui->pushButton_back->hide();
    ui->pushButton_saveChanges->show();
    ui->pushButton_saveChanges->setEnabled(true);
    ui->pushButton_saveAnswer->hide();
    ui->pushButton_changeMP->hide();
    ui->label_musicPreferences->hide();

    query->prepare("SELECT genre_name, music_era, music_region "
                   "FROM music_preferences "
                   "JOIN genre ON music_preferences.genre_id = genre.genre_id "
                   "WHERE user_id = ?");
    query->addBindValue(user_id);

    if (!query->exec())
    {
        qDebug() << query->lastError().text();
    }

    while (query->next())
    {
        for (int i=0; i<radioBtnList.size(); i++)
        {
            if (radioBtnList.value(i)->text() == query->value(0).toString())
            {
                radioBtnList.value(i)->setChecked(true);
            }
            if (radioBtnList.value(i)->text() == query->value(1).toString())
            {
                radioBtnList.value(i)->setChecked(true);
            }
            if (radioBtnList.value(i)->text() == query->value(2).toString())
            {
                radioBtnList.value(i)->setChecked(true);
            }
        }
    }
}


void MainWindow::on_pushButton_saveChanges_clicked() //утвердить изменения музыкальных предпочтений
{
    QStringList MP;
    for (int i=0; i<radioBtnList.size(); i++)
    {
        if (radioBtnList.value(i)->isChecked())
        {
            MP << radioBtnList.value(i)->text();
        }
    }

    int genreID;
    QSqlQuery querySelect_genreID;
    querySelect_genreID.prepare("SELECT genre_id FROM genre WHERE genre_name = ?");
    querySelect_genreID.addBindValue(MP[0]);
    querySelect_genreID.exec();
    while (querySelect_genreID.next()) {
        //считываем значение идентификатора жанра по результатам выполнения запроса
        genreID = querySelect_genreID.value(0).toInt();
    }

    query->prepare("UPDATE music_preferences "
                   "SET music_era = ?, genre_id = ?, music_region = ? "
                   "WHERE user_id = ?");
    query->addBindValue(MP[1]);
    query->addBindValue(genreID);
    query->addBindValue(MP[2]);
    query->addBindValue(user_id);

    if (!query->exec())
    {
        qDebug() << query->lastError().text();
        qDebug() << "Unable to make update operation";
    }

    //Возвращаемся на страницу каталога
    catalogFilling();
    ui->pushButton_changeMP->show();
    ui->pushButton_back->show();
    ui->stackedWidget->setCurrentIndex(4);
    ui->pushButton_saveChanges->setEnabled(false);
}


void MainWindow::on_pushButton_leaveReview_clicked() //желаем оставить рецензию
{
    ui->pushButton_leaveReview->hide();
    ui->textEdit_leaveReview->clear();
    ui->textEdit_leaveReview->show();
    ui->pushButton_saveReview->show();

    isReviewLeft = false;
}


void MainWindow::on_pushButton_changeReview_clicked() //желаем изменить (или удалить) рецензию
{
    ui->pushButton_changeReview->hide();
    ui->textEdit_leaveReview->show();
    ui->pushButton_saveReview->show();

    isReviewLeft = true;
}


void MainWindow::on_pushButton_saveReview_clicked() //добавляем, изменяем или удаляем рецензию
{
    //Считываем идентификатор товара
    int item_index = ui->listWidget_catalog->currentRow();

    //Значение текущей даты
    QDate curDate = QDate::currentDate();

    if (!isReviewLeft) //если впервые оставляем рецензию
    {
        if (!ui->textEdit_leaveReview->document()->isEmpty()) //Не пустая ли рецензия?
        {
            query->prepare("INSERT INTO review (product_id, user_id, review_date, review_text) VALUES (?,?,?,?)");
            query->addBindValue(productInfo.value(item_index).value(0).toInt());
            query->addBindValue(user_id);
            query->addBindValue(curDate);
            query->addBindValue(ui->textEdit_leaveReview->toPlainText());

            if (!query->exec())
            {
                qDebug() << query->lastError().text();
                qDebug() << "Unable to make insertation";
            }

            //Изменяем интерфейс для изменения рецензии
            ui->pushButton_saveReview->hide();
            ui->textEdit_leaveReview->hide();
            ui->pushButton_changeReview->show();

            //Отображаем новую рецензию в окне рецензий
            ui->textBrowser_reviews->append(dateRebuild(curDate));
            ui->textBrowser_reviews->append("Пользователь: " + globalLogin);
            ui->textBrowser_reviews->append(ui->textEdit_leaveReview->toPlainText() + '\n');
        }
        else //Если пустая, то предупреждаем об этом
        {
            QMessageBox::warning(this, "Error", "Вы совсем ничего не написали в своей рецензии :(");
        }
    }
    else //если уже имеется рецензия
    {
        if (!ui->textEdit_leaveReview->document()->isEmpty()) //обновляем текст рецензии
        {
            query->prepare("UPDATE review "
                           "SET review_date = ?, review_text = ? "
                           "WHERE user_id = ? AND product_id = ?");
            query->addBindValue(curDate);
            query->addBindValue(ui->textEdit_leaveReview->toPlainText());
            query->addBindValue(user_id);
            query->addBindValue(productInfo.value(item_index).value(0).toInt());

            if (!query->exec())
            {
                qDebug() << query->lastError().text();
                qDebug() << "Unable to make update operation";
            }

            //Изменяем интерфейс для изменения рецензии
            ui->pushButton_saveReview->hide();
            ui->textEdit_leaveReview->hide();
            ui->pushButton_changeReview->show();
        }
        else //если изменяем рецензию и сотавлем пустоту - удаляем рецензию
        {
            query->prepare("DELETE FROM review WHERE user_id = ? AND product_id = ?");
            query->addBindValue(user_id);
            query->addBindValue(productInfo.value(item_index).value(0).toInt());

            if (!query->exec())
            {
                qDebug() << query->lastError().text();
                qDebug() << "Unable to make insertation";
            }

            QMessageBox::warning(this, "Error", "Вы удалили рецензию");

            ui->textEdit_leaveReview->hide();
            ui->pushButton_saveReview->hide();
            ui->pushButton_leaveReview->show();
        }

        //Формируем рецензии после изменений
        ui->textBrowser_reviews->clear();

        query->prepare("SELECT users.user_name, users.user_id, "
                       "review.product_id, review.review_date, review.review_text "
                       "FROM users "
                       "JOIN review ON users.user_id = review.user_id "
                       "WHERE product_id = ? "
                       "ORDER BY review.review_date ASC");
        query->addBindValue(productInfo.value(item_index).value(0).toInt());

        if (!query->exec())
        {
            qDebug() << "Unable to make selection";
        }

        while (query->next()) //считываем построково результирующую таблицу SQL-запроса
        {
            QString authorNmae = query->value(0).toString();
            QDate reviewDate = query->value(3).toDate();
            QString reviewText = query->value(4).toString();

            ui->textBrowser_reviews->append(dateRebuild(reviewDate));
            ui->textBrowser_reviews->append("Пользователь: " + authorNmae);
            ui->textBrowser_reviews->append(reviewText + '\n');
        }
    }
}

QString MainWindow::dateRebuild(QDate reviewDate) //пересборка дата в читаемый вид
{
    QString dayDate;
    if (reviewDate.day() < 10)
    {
        dayDate = "0" + QString::number(reviewDate.day());
    }
    else
    {
        dayDate = QString::number(reviewDate.day());
    }
    QString monthDate;
    if (reviewDate.month() < 10)
    {
        monthDate = "0" + QString::number(reviewDate.month());
    }
    else
    {
        monthDate = QString::number(reviewDate.month());
    }
    QString yearDate = QString::number(reviewDate.year());
    QString dateNormal = dayDate + "." + monthDate + "." + yearDate;

    return dateNormal;
}


void MainWindow::on_pushButton_exit_clicked() //выход из аккаунта
{
    ui->label_avatar->hide();
    ui->label_nickname->hide();
    ui->pushButton_exit->hide();
    ui->pushButton_changeMP->hide();

    ui->stackedWidget->setCurrentIndex(0);
    ui->pushButton_back->hide();

    //Снимаем активацию радиокнопок
    ui->radioButton_user->setAutoExclusive(false);
    ui->radioButton_user->setChecked(false);
    ui->radioButton_user->setAutoExclusive(true);

    ui->radioButton_administrator->setAutoExclusive(false);
    ui->radioButton_administrator->setChecked(false);
    ui->radioButton_administrator->setAutoExclusive(true);
}


void MainWindow::on_radioButton_administrator_clicked() //используем приложение в качестве пользователя
{
    ui->stackedWidget->setCurrentIndex(8);
    previousPage = 0;

    ui->pushButton_back->show();
    ui->lineEdit_codeEmployee->clear();
    ui->lineEdit_passwordEmployee->clear();
}


void MainWindow::on_pushButton_adminAuth_clicked() //авторизация за администратора
{
    if (ui->lineEdit_codeEmployee->text().isEmpty() || ui->lineEdit_passwordEmployee->text().isEmpty())
    {
        QMessageBox::warning(this, "Error", "Для авторизации необходимо заполнить поля ввода");
    }
    else
    {
        if (ui->lineEdit_codeEmployee->text() == "admin" && ui->lineEdit_passwordEmployee->text() == "admin")
        {
            ui->stackedWidget->setCurrentIndex(9);
            previousPage = 8;

            //Прячем псевдо-элементы
            ui->label->hide();
            ui->label_2->hide();

            //Настройка интерфейса
            ui->pushButton_exit->show();
            ui->label_avatar->show();
            ui->label_avatar->show();
            QPixmap pixmap("../BaseApp/avatar.jpg");
            ui->label_avatar->setPixmap(pixmap);
            ui->label_nickname->show();
            ui->label_nickname->setText(ui->lineEdit_codeEmployee->text());
            ui->label_table->clear();

            ui->comboBox_table->setCurrentIndex(0);
            model->clear();
            ui->tableView_tables->setModel(model);

            ui->tableWidget_editTable->clear();
            ui->tableWidget_editTable->setRowCount(0);
            ui->tableWidget_editTable->setColumnCount(0);
            ui->comboBox_editableTables->setCurrentIndex(0);
        }
        else
        {
            QMessageBox::warning(this, "Error", "Код или пароль администратора введены неверно");
        }
    }
}


void MainWindow::on_comboBox_table_currentIndexChanged(const QString &arg1) //выбираем таблицу для просмотра
{
    if (arg1 != "Просмотреть таблицу")
    {
        if (arg1 == "Пользователи")
        {
            model->setQuery("SELECT user_id, user_name, user_sex, user_age, user_mail FROM users ORDER BY user_id");
            model->setHeaderData(0, Qt::Horizontal, "Идентификатор пользователя");
            model->setHeaderData(1, Qt::Horizontal, "Имя пользователя");
            model->setHeaderData(2, Qt::Horizontal, "Пол");
            model->setHeaderData(3, Qt::Horizontal, "Возраст");
            model->setHeaderData(4, Qt::Horizontal, "Почта");
        }

        if (arg1 == "Предпочтения пользователей")
        {
            model->setQuery("SELECT user_name, genre_name, music_era, music_region "
                            "FROM music_preferences "
                            "JOIN users ON music_preferences.user_id = users.user_id "
                            "JOIN genre ON music_preferences.genre_id = genre.genre_id "
                            "ORDER BY compilation_id");
            model->setHeaderData(0, Qt::Horizontal, "Имя пользователя");
            model->setHeaderData(1, Qt::Horizontal, "Любимый жанр");
            model->setHeaderData(2, Qt::Horizontal, "Любимая музыкальная эра");
            model->setHeaderData(3, Qt::Horizontal, "Любимый музыкальный регион");
        }

        if (arg1 == "Каталог")
        {
            model->setQuery("SELECT product_id, artist_name, record_name, product_price, product_quantity "
                            "FROM product_catalog "
                            "JOIN artist ON product_catalog.artist_id = artist.artist_id "
                            "JOIN record ON product_catalog.record_id = record.record_id "
                            "ORDER BY product_id");
            model->setHeaderData(0, Qt::Horizontal, "Артикул пластинки");
            model->setHeaderData(1, Qt::Horizontal, "Исполнитель");
            model->setHeaderData(2, Qt::Horizontal, "Название альбома");
            model->setHeaderData(3, Qt::Horizontal, "Цена");
            model->setHeaderData(4, Qt::Horizontal, "Количество");
        }

        if (arg1 == "Пластинка")
        {
            model->setQuery("SELECT artist_name, record_name, genre_name, record_country, record_release_date "
                            "FROM record "
                            "JOIN artist ON record.artist_id = artist.artist_id "
                            "JOIN genre ON record.genre_id = genre.genre_id "
                            "ORDER BY record_id");
            model->setHeaderData(0, Qt::Horizontal, "Исполнитель");
            model->setHeaderData(1, Qt::Horizontal, "Название альбома");
            model->setHeaderData(2, Qt::Horizontal, "Жанр");
            model->setHeaderData(3, Qt::Horizontal, "Страна выпуска");
            model->setHeaderData(4, Qt::Horizontal, "Год выпуска");
        }

        if (arg1 == "Исполнитель")
        {
            model->setQuery("SELECT artist_name FROM artist ORDER BY artist_id");
            model->setHeaderData(0, Qt::Horizontal, "Исполнитель");
        }

        if (arg1 == "Данные заказов")
        {
            model->setQuery("SELECT order_id, user_name, order_format, order_date, order_state, order_sum "
                            "FROM purchase "
                            "JOIN users ON purchase.user_id = users.user_id "
                            "ORDER BY order_id");
            model->setHeaderData(0, Qt::Horizontal, "Номер заказа");
            model->setHeaderData(1, Qt::Horizontal, "Имя пользователя");
            model->setHeaderData(2, Qt::Horizontal, "Формат заказа");
            model->setHeaderData(3, Qt::Horizontal, "Дата оформления");
            model->setHeaderData(4, Qt::Horizontal, "Статус заказа");
            model->setHeaderData(5, Qt::Horizontal, "Сумма заказа");
        }

        if (arg1 == "Рецензии")
        {
            model->setQuery("SELECT user_name, artist_name, record_name, review_date, review_text "
                            "FROM review "
                            "JOIN users ON review.user_id = users.user_id "
                            "JOIN product_catalog ON product_catalog.product_id = review.product_id "
                            "JOIN artist ON product_catalog.artist_id = artist.artist_id "
                            "JOIN record ON product_catalog.record_id = record.record_id "
                            "ORDER BY review_id");
            model->setHeaderData(0, Qt::Horizontal, "Имя пользователя");
            model->setHeaderData(1, Qt::Horizontal, "Исполнитель");
            model->setHeaderData(2, Qt::Horizontal, "Название альбома");
            model->setHeaderData(3, Qt::Horizontal, "Дата отзыва");
            model->setHeaderData(4, Qt::Horizontal, "Текс отзыва");
        }

        if (arg1 == "Жанры")
        {
            model->setQuery("SELECT * FROM genre ORDER BY genre_id");
            model->setHeaderData(0, Qt::Horizontal, "Идентификатор жанра");
            model->setHeaderData(1, Qt::Horizontal, "Наименование жанра");
        }

        ui->tableView_tables->setModel(model);
        ui->tableView_tables->show();
        ui->label_table->setText("Таблица " + arg1);
    }
    else
    {
        model->clear();
        ui->tableView_tables->setModel(model);
        ui->label_table->clear();
    }
}


void MainWindow::on_pushButton_confirmOrder_clicked() //переход в подтвердение заказов
{
    QVector<QVector<QString>> OrdersToConfirm;

    query->exec("SELECT order_id, user_name, order_format, order_date, order_state, order_sum "
                "FROM purchase "
                "JOIN users ON purchase.user_id = users.user_id "
                "WHERE order_state = 'В обработке'");

    while (query->next())
    {
       QVector<QString> tempVector;

       tempVector.push_back(query->value(0).toString());
       tempVector.push_back(query->value(1).toString());
       tempVector.push_back(query->value(2).toString());
       tempVector.push_back(query->value(3).toString());
       tempVector.push_back(query->value(4).toString());
       tempVector.push_back(query->value(5).toString());

       OrdersToConfirm.push_back(tempVector);
    }

    emit signalConfirm(OrdersToConfirm);

    confirm->setWindowModality(Qt::ApplicationModal);
    confirm->show();
}


void MainWindow::on_pushButton_addProduct_clicked() //открываем форму для добавления товара
{
    add->show();
}


void MainWindow::on_pushButton_editData_clicked() //переходим на страницу редактирования
{
    ui->stackedWidget->setCurrentIndex(10);
    previousPage = 9;
    ui->comboBox_table->setCurrentIndex(0);
}


void MainWindow::on_comboBox_editableTables_currentIndexChanged(const QString &arg1) //выбираем таблицу для редактирования
{
    flag_auto = false;

    if (arg1 == "Выберите таблицу")
    {
        ui->label_tableName->setText("");
        ui->tableWidget_editTable->clear();
        ui->tableWidget_editTable->setRowCount(0);
        ui->tableWidget_editTable->setColumnCount(0);
    }
    else
    {
        if (arg1 == "Каталог")
        {
            ui->label_tableName->setText("Таблица " + arg1);
            ui->tableWidget_editTable->clear();
            ui->tableWidget_editTable->setColumnCount(0);
            ui->tableWidget_editTable->setRowCount(0);

            QSqlQuery queryRowCount;
            queryRowCount.exec("SELECT COUNT (*) FROM product_catalog");
            int rowCount = 1;

            while (queryRowCount.next())
            {
                rowCount = queryRowCount.value(0).toInt();
            }

            if (rowCount == 0)
            {
                ui->tableWidget_editTable->clear();
                ui->label_tableName->setText("Таблица " + arg1 + " пуста");
            }
            else
            {
                ui->tableWidget_editTable->setRowCount(rowCount);
                ui->tableWidget_editTable->setColumnCount(5);

                QStringList labels_pc;
                labels_pc << "Артикул товара" << "Исполнитель" << "Название альбома" << "Стоимость, ₽" << "Количество";
                ui->tableWidget_editTable->setHorizontalHeaderLabels(labels_pc);

                QSqlQuery querySelect;
                querySelect.exec("SELECT product_id, artist_name, record_name, product_price, product_quantity "
                                 "FROM product_catalog "
                                 "JOIN artist ON product_catalog.artist_id = artist.artist_id "
                                 "JOIN record ON product_catalog.record_id = record.record_id "
                                 "ORDER BY product_id");

                int i = 0;
                while (querySelect.next())
                {
                    QTableWidgetItem* ti_1 = new QTableWidgetItem(querySelect.value(0).toString());
                    ui->tableWidget_editTable->setItem(i, 0, ti_1);
                    ti_1->setFlags(ti_1->flags()&0xfffffffd);

                    QTableWidgetItem* ti_2 = new QTableWidgetItem(querySelect.value(1).toString());
                    ui->tableWidget_editTable->setItem(i, 1, ti_2);
                    ti_2->setFlags(ti_2->flags()&0xfffffffd);

                    QTableWidgetItem* ti_3 = new QTableWidgetItem(querySelect.value(2).toString());
                    ui->tableWidget_editTable->setItem(i, 2, ti_3);
                    ti_3->setFlags(ti_3->flags()&0xfffffffd);

                    QTableWidgetItem* ti_4 = new QTableWidgetItem(querySelect.value(3).toString());
                    ui->tableWidget_editTable->setItem(i, 3, ti_4);

                    QTableWidgetItem* ti_5 = new QTableWidgetItem(querySelect.value(4).toString());
                    ui->tableWidget_editTable->setItem(i, 4, ti_5);

                    i++;
                }
            }
        }
        if (arg1 == "Пластинка")
        {
            ui->label_tableName->setText("Таблица " + arg1);
            ui->tableWidget_editTable->clear();
            ui->tableWidget_editTable->setColumnCount(0);
            ui->tableWidget_editTable->setRowCount(0);

            QSqlQuery queryRowCount;
            queryRowCount.exec("SELECT COUNT (*) FROM record");
            int rowCount = 1;

            while (queryRowCount.next())
            {
                rowCount = queryRowCount.value(0).toInt();
            }

            if (rowCount == 0)
            {
                ui->tableWidget_editTable->clear();
                ui->label_tableName->setText("Таблица " + arg1 + " пуста");
            }
            else
            {
                ui->tableWidget_editTable->setRowCount(rowCount);
                ui->tableWidget_editTable->setColumnCount(4);

                QStringList labels_record;
                labels_record << "Идентификатор пластинки" << "Название альбома" << "Страна выпуска" << "Год выпуска";
                ui->tableWidget_editTable->setHorizontalHeaderLabels(labels_record);

                QSqlQuery querySelect;
                querySelect.exec("SELECT record_id, record_name, record_country, record_release_date FROM record ORDER BY record_id ASC");

                int i = 0;
                while (querySelect.next())
                {
                    QTableWidgetItem* ti_1 = new QTableWidgetItem(querySelect.value(0).toString());
                    ui->tableWidget_editTable->setItem(i, 0, ti_1);
                    ti_1->setFlags(ti_1->flags()&0xfffffffd);

                    QTableWidgetItem* ti_2 = new QTableWidgetItem(querySelect.value(1).toString());
                    ui->tableWidget_editTable->setItem(i, 1, ti_2);

                    QTableWidgetItem* ti_3 = new QTableWidgetItem(querySelect.value(2).toString());
                    ui->tableWidget_editTable->setItem(i, 2, ti_3);

                    QTableWidgetItem* ti_4 = new QTableWidgetItem(querySelect.value(3).toString());
                    ui->tableWidget_editTable->setItem(i, 3, ti_4);

                    i++;
                }
            }
        }
        if (arg1 == "Исполнитель")
        {
            ui->label_tableName->setText("Таблица " + arg1);
            ui->tableWidget_editTable->clear();
            ui->tableWidget_editTable->setColumnCount(0);
            ui->tableWidget_editTable->setRowCount(0);

            QSqlQuery queryRowCount;
            queryRowCount.exec("SELECT COUNT (*) FROM artist");
            int rowCount = 1;

            while (queryRowCount.next())
            {
                rowCount = queryRowCount.value(0).toInt();
            }

            if (rowCount == 0)
            {
                ui->tableWidget_editTable->clear();
                ui->label_tableName->setText("Таблица " + arg1 + " пуста");
            }
            else
            {
                ui->tableWidget_editTable->setRowCount(rowCount);
                ui->tableWidget_editTable->setColumnCount(2);

                QStringList labels_artist;
                labels_artist << "Идентификатор исполнителя" << "Псевдоним исполнителя";
                ui->tableWidget_editTable->setHorizontalHeaderLabels(labels_artist);

                QSqlQuery querySelect;
                querySelect.exec("SELECT artist_id, artist_name FROM artist ORDER BY artist_id ASC");

                int i = 0;
                while (querySelect.next())
                {
                    QTableWidgetItem* ti_1 = new QTableWidgetItem(querySelect.value(0).toString());
                    ui->tableWidget_editTable->setItem(i, 0, ti_1);
                    ti_1->setFlags(ti_1->flags()&0xfffffffd);

                    QTableWidgetItem* ti_2 = new QTableWidgetItem(querySelect.value(1).toString());
                    ui->tableWidget_editTable->setItem(i, 1, ti_2);

                    i++;
                }
            }
        }
    }

    flag_auto = true;
}


void MainWindow::on_tableWidget_editTable_cellChanged(int row, int column) //подтверждение редактирования
{
    if (flag_auto)
    {
        flag_auto = false;

        if (ui->comboBox_editableTables->currentText() == "Каталог")
        {
            if (ui->tableWidget_editTable->item(row, column)->text().isEmpty() || ui->tableWidget_editTable->item(row, column)->text().toInt() == 0)
            {
                ui->tableWidget_editTable->item(row, column)->setBackground(QBrush(Qt::red));
            }
            else
            {
                if (ui->tableWidget_editTable->item(row, column)->text().toInt() < 0)
                {
                    ui->tableWidget_editTable->item(row, column)->setBackground(QBrush(Qt::red));
                }
                else
                {
                    ui->tableWidget_editTable->item(row, column)->setBackground(QBrush(Qt::white));
                    query->prepare("UPDATE product_catalog SET product_price = ?, product_quantity = ? WHERE product_id = ?");
                    query->addBindValue(ui->tableWidget_editTable->item(row, 3)->text().toInt());
                    query->addBindValue(ui->tableWidget_editTable->item(row, 4)->text().toInt());
                    query->addBindValue(ui->tableWidget_editTable->item(row, 0)->text().toInt());

                    if (!query->exec())
                    {
                        qDebug() << query->lastError().text();
                    }
                }
            }
        }
        if (ui->comboBox_editableTables->currentText() == "Исполнитель")
        {
            if (ui->tableWidget_editTable->item(row, column)->text().isEmpty())
            {
                ui->tableWidget_editTable->item(row, column)->setBackground(QBrush(Qt::red));
            }
            else
            {
                ui->tableWidget_editTable->item(row, column)->setBackground(QBrush(Qt::white));
                query->prepare("UPDATE artist SET artist_name = ? WHERE artist_id = ?");
                query->addBindValue(ui->tableWidget_editTable->item(row, column)->text());
                query->addBindValue(ui->tableWidget_editTable->item(row, 0)->text().toInt());

                if (!query->exec())
                {
                    qDebug() << query->lastError().text();
                }
            }
        }
        if (ui->comboBox_editableTables->currentText() == "Пластинка")
        {
            if (ui->tableWidget_editTable->item(row, column)->text().isEmpty())
            {
                ui->tableWidget_editTable->item(row, column)->setBackground(QBrush(Qt::red));
            }
            else
            {
                if (column == 3 && ui->tableWidget_editTable->item(row, column)->text().toInt() == 0)
                {
                    ui->tableWidget_editTable->item(row, column)->setBackground(QBrush(Qt::red));
                }
                else
                {
                    ui->tableWidget_editTable->item(row, column)->setBackground(QBrush(Qt::white));
                    query->prepare("UPDATE record SET record_name = ?, record_country = ?, record_release_date = ? WHERE record_id = ?");
                    query->addBindValue(ui->tableWidget_editTable->item(row, 1)->text());
                    query->addBindValue(ui->tableWidget_editTable->item(row, 2)->text());
                    query->addBindValue(ui->tableWidget_editTable->item(row, 3)->text().toInt());
                    query->addBindValue(ui->tableWidget_editTable->item(row, 0)->text().toInt());

                    if (!query->exec())
                    {
                        qDebug() << query->lastError().text();
                    }
                }
            }
        }
        flag_auto = true;
    }
}
