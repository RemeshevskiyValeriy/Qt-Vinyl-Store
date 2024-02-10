# Qt-Vinyl-Store
### **Demo project of Vinyl Store**
Desktop application for buying vinyl records using the Qt library and PostgreSQL DBMS. The application can be used by both a common user and an administrator.

In this project I mastered:
* The basis of database design
* Normalization of relations in the database
* SQL
* Qt Classes for interacting with databases (QSql, QSqlDatabase, QSqlQuery, QSqlQueryModel, QSqlRecord)

All components are in the `BaseApp` folder (excepting backup-file)

It is needed to run the application locally:
*  Clone branch `main` with `git clone https://github.com/RemeshevskiyValeriy/Qt-Vinyl-Store.git` or fork it and then clone it from your forked repository
*  Open the file `BaseAppp.pro` using the Qt Creator. You can read how to build a Qt project in Visual Studio [here](https://doc.qt.io/qtvstools-2/qtvstools-importing-and-exporting-projects.html)
* You must install the latest version of the [PostgreSQL](https://www.postgresql.org/download/) if it has not been installed before. The installation configuration includes the pgAdmin 4 software product for database administration and development, which also needs to be installed
* In pgAdmin 4, you need to open a local server (remember your username and password). To restore the original database, right-click on the raw database on the local server to open the context menu and select `Restore...`, and then specify the backup-file `Database_backup.sql`
* By opening the file `ВаѕеАрр.рго` in the lines of code numbered **67**, **68** and **69**, the *setDatabaseName*, *setUserName* and *setPassword* methods must be passed as arguments the *database name* (where the original database was restored), *username* and *password* (which were specified when opening the local server) accordingly

**It is worth mentioning** - when working with an application for authorization as an administrator, the administrator code and password are the keyword "admin".
