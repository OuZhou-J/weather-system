#ifndef weatherwindow_H
#define weatherwindow_H

#include <QMainWindow>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QVariantMap>
#include <QVariantList>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPixmap>
#include <QFile>
#include <QDebug>
#include <QPalette>
#include <QDockWidget>
#include <Qdir>
#include <QFile>
#include <QTextStream>
#include <QVector>

namespace Ui {
class weatherwindow;
}

class weatherwindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit weatherwindow(QWidget *parent = nullptr);
    QString getCityCode(const QString& cityName, const QString& filePath);
    void drivenotice(QString weatherConditon);
    void initLable();
    void changeBackground(QString weatherCondtion);
    void uiSetting();
    ~weatherwindow();

    //sidebar
    void initSidebar();

private slots:
    void getWeatherInfo();
    void on_back_clicked();    //返回按钮
    void replyFinished(QNetworkReply *);    //对返回的数据进行json解析处理
    void GPSreplyFinished(QNetworkReply *);    //对返回的数据进行json解析处理
    void on_searchbutton_clicked();         //search
    void on_searchbutton_2_clicked();
    void on_searchbutton_3_clicked(QString cityname);
    void onEnterPress() {
        on_searchbutton_clicked();
    }
    void toggleSidebar();//侧边栏
    void on_openSideBar_clicked() {
        toggleSidebar();
    }
    void addButtonToSidebar(QString name);
    void addButtonToSidebarAndFile(QString name);

    void on_add_clicked();

    void on_deleteButton_clicked(int index, QString name);

signals:
    void GPSfinished();    //GPS定位完成
    void sidebarButtonClicked(QString name);// 侧边栏按钮点击信号
    void sidebarButtonDelete(int index, QString name);// 侧边栏删除按钮点击信号
protected:

private:
    QString myAK = "mrnKOfPOXofTCwJWCauSbsHRpuAruiwI";
    Ui::weatherwindow *ui;
    QNetworkAccessManager *manager;         //请求
    QNetworkAccessManager *GPSmanager;        //请求
    QLabel *dw[6];                          //日期
    QLabel *tpe1[6];                        //天气类型
    QLabel *hg[6];                          //最高温
    QLabel *lw[6];                          //最低温

    QString currentCity;                    //当前城市
    QString currentCityCode;                //当前城市代码
    bool currentLocation = true;            //是否是当前位置
    QPixmap *background;                    //背景图片
    QPalette *palette;                      //背景调色板

    //侧边栏sidebar
    QDockWidget *sidebarDock;// 侧边栏
    QWidget *sidebarWidget;//
    QVector<QPushButton*> sidebarButtons;// 侧边栏按钮
    QVector<QPushButton*> deleteButtons;// 侧边栏删除按钮
    int count = 0;// 侧边栏按钮数量
};


#endif // weatherwindow_H
