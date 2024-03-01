

#include "weatherwindow.h"
#include "ui_weatherwindow.h"

weatherwindow::weatherwindow(QWidget *parent) : QMainWindow(parent),
                                                ui(new Ui::weatherwindow)
{
    uiSetting();
    initSidebar();

    // 初始化
    GPSmanager = new QNetworkAccessManager(this);
    manager = new QNetworkAccessManager(this);

    // 设置背景
    this->background = new QPixmap;//创建一个QPixmap对象
    this->palette = new QPalette;//创建一个QPalette对象

    // 信号与槽
    connect(ui->searchline, SIGNAL(returnPressed()), this, SLOT(onEnterPress()));
    connect(GPSmanager, SIGNAL(finished(QNetworkReply *)), this, SLOT(GPSreplyFinished(QNetworkReply *)));
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));
    connect(this, SIGNAL(GPSfinished()), this, SLOT(getWeatherInfo()));
    // 侧边栏按钮点击信号与槽
    connect(this, SIGNAL(sidebarButtonClicked(QString)), this, SLOT(on_searchbutton_3_clicked(QString)));
    connect(this, SIGNAL(sidebarButtonDelete(int, QString)), this, SLOT(on_deleteButton_clicked(int, QString)));

    // 获取当前城市信息
    QString url = "https://api.map.baidu.com/location/ip?&coor=bd09ll&ak=" + myAK;
    GPSmanager->get(QNetworkRequest(QUrl(url)));

    // 对qlabel进行赋值
    initLable();
}

void weatherwindow::initSidebar () {
    sidebarWidget = new QWidget(this);
    QFile file1("cityname.txt");
    if (!file1.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        qDebug() << "Open failed.";
        return;
    }
    else {
        qDebug() << "Open succeed.";
    }
    file1.close();//关闭文件
    QPushButton *hideButton = new QPushButton("隐藏边栏", sidebarWidget);//隐藏侧边栏按钮
    count++;// 侧边栏按钮数量加一
    qDebug() << "当前数量" << count;
    hideButton->setGeometry(5 , 10 * count, 100, 30);//设置按钮的位置和大小
    sidebarButtons.push_back(hideButton);//将按钮加入到侧边栏按钮列表中
    QFile file2("cityname.txt");
    if (!file2.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Open failed.";
        return;
    }
    else {
        QTextStream in(&file2);
        while (!in.atEnd()) {
            QString line = in.readLine();
            addButtonToSidebar(line);
        }
    }
    file2.close();
    connect(hideButton, SIGNAL(clicked()), this, SLOT(toggleSidebar()));
    sidebarDock = new QDockWidget("Sidebar", this);
    sidebarDock->setWidget(sidebarWidget);
    addDockWidget(Qt::RightDockWidgetArea, sidebarDock);  // 修改这里
    sidebarDock->setAllowedAreas(Qt::RightDockWidgetArea);  // 修改这里
    sidebarDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    sidebarDock->setMinimumWidth(120);
    sidebarDock->setMaximumWidth(120);
    sidebarDock->hide();
}

// 添加按钮到侧边栏,按钮名为城市名，点击后主界面显示该城市的天气
void weatherwindow::addButtonToSidebar(QString name)
{
    QPushButton *button = new QPushButton(name, sidebarWidget);
    QPushButton *deleteButton = new QPushButton("X", sidebarWidget);
    count++;
    qDebug() << "当前数量" << count;
    //设置按钮的位置和大小
    button->setGeometry(5 , 10 * count + 30 * (count - 1),  100, 30);
    deleteButton->setGeometry(105, 10 * count + 30 * (count - 1), 15, 30);

    sidebarButtons.push_back(button);
    deleteButtons.push_back(deleteButton);

    qDebug() << "加按钮了:" << name;
    //显示按钮
    button->show();
    deleteButton->show();

    // 信号与槽
    connect(button, &QPushButton::clicked, [=]() {
        emit sidebarButtonClicked(name);
    });

    connect(deleteButton, &QPushButton::clicked, [=]() {
        int i = 0;
        for (auto tmp : deleteButtons) {
            if (tmp == deleteButton) {
                break;
            }
            i++;
        }
        QString name = sidebarButtons[i + 1]->text();
        emit sidebarButtonDelete(i + 1, name);
    });
}

void weatherwindow::on_deleteButton_clicked (int index, QString name) {
    qDebug() << "delete";
    qDebug() << index;
    qDebug() << name;
    //删除文件cityname.txt中的城市名
    QFile file("cityname.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Open failed.";
        return;
    }
    else {
        QTextStream in(&file);
        QStringList list;
        while (!in.atEnd()) {
            QString line = in.readLine();
            list.append(line);
        }
        file.close();
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        for (int i = 0; i < list.size(); i++) {
            if (i != index - 1) {
                out << list[i] << "\n";
            }
        }
    }
    file.close();
    count--;// 侧边栏按钮数量减一
    qDebug() << "当前数量" << count;
    // 删除侧边栏按钮
    sidebarButtons[index]->deleteLater();//删除按钮
    deleteButtons[index - 1]->deleteLater();//删除删除按钮
    sidebarButtons.remove(index);
    deleteButtons.remove(index - 1);

    // 调整下面侧边栏按钮位置
    for (int i = index; i < sidebarButtons.size(); i++) {
        //i + 1是按钮的序号
        sidebarButtons[i]->setGeometry(5, 10 * ( 1+i) + 30 * i, 100, 30);
        deleteButtons[i - 1]->setGeometry(105, 10 *( 1+i) + 30 * i, 15, 30);
    }

}

void weatherwindow::addButtonToSidebarAndFile(QString name) {

    //打开文件cityname.txt,将城市名写入
    QFile file("cityname.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        qDebug() << "Open failed.";
        return;
    }
    else {
        QTextStream out(&file);
        out << name << "\n";
    }
    qDebug() << name << "Write succeed.";
    file.close();
    addButtonToSidebar(name);
}


void weatherwindow::on_add_clicked()
{
    QString city = ui->searchline->text();
    if (city.isEmpty())
    {
        QMessageBox::warning(this, "警告", "城市名不能为空");
        return;
    }
    QString filePath = ":/search/cities.json";
    QString cityCode = getCityCode(city, filePath);

    if (cityCode.isEmpty())
    {
        QMessageBox::warning(this, "警告", "城市信息不存在");
        ui->searchline->clear();
        return;
    }

    if (sidebarButtons.size() >= 11)
    {
        QMessageBox::warning(this, "警告", "侧边栏已满");
        return;
    }

    for (auto x : sidebarButtons)
    {
        if (x->text() == city)
        {
            QMessageBox::warning(this, "警告", "该城市已存在");
            return;
        }
    }
    addButtonToSidebarAndFile(city);
}

// 侧边栏的显示与隐藏
void weatherwindow::toggleSidebar()
{
    if (sidebarDock->isHidden())
    {
        sidebarDock->show();
    }
    else
    {
        sidebarDock->hide();
    }
}

// ui 设置
void weatherwindow::uiSetting() {
    ui->setupUi(this);
    ui->noticeIcon->setPixmap(QPixmap(QString::fromUtf8(":/icon/weather/notice.png")));
    ui->windIcon->setPixmap(QPixmap(QString::fromUtf8(":/weather/windIcon.png")));
    ui->searchline->setFocus();
    ui->back->setIcon(QIcon(":/icon/weather/back.png"));
    ui->searchbutton->setIcon(QIcon(":/icon/weather/search.png"));
    ui->searchbutton_2->setIcon(QIcon(":/icon/weather/GPS.png"));
    ui->openSideBar->setIcon(QIcon(":/icon/weather/more.png"));
    ui->add->setIcon(QIcon(":/icon/weather/add.png"));
    //设置窗口标题
    this->setWindowTitle("weather");
}

// 回车键
void weatherwindow::getWeatherInfo () {
    manager->get(QNetworkRequest(QUrl("http://t.weather.itboy.net/api/weather/city/" + currentCityCode)));
}

weatherwindow::~weatherwindow()
{
    delete ui;
}

// 返回按钮
void weatherwindow::on_back_clicked()
{
    this->parentWidget()->show();
    this->hide();
}

// GPS定位完成
void weatherwindow::GPSreplyFinished (QNetworkReply *arg) // 对返回的数据进行json解析处理
{
    QString js = arg->readAll();                                       // 取出返回的数据
    QJsonParseError error;                                             // 定义json解析错误
    QJsonDocument json = QJsonDocument::fromJson(js.toUtf8(), &error); // 解析json
    if (error.error != QJsonParseError::NoError)
    {
        qDebug() << error.errorString();
    }
    else
        qDebug() << "init GPSjson succeed!";
    QJsonObject obj = json.object(); // 取出json对象
    // 对象里的data
    QString value_city = obj.take("content").toObject().take("address_detail").toObject().take("city").toString();
    value_city.remove("市");
    currentCity = value_city;
    QString filePath = ":/search/cities.json";
    currentCityCode = getCityCode(currentCity, filePath);
    currentLocation = true;
    //debug info
    qDebug() << value_city;
    qDebug() << currentCityCode;



    value_city = "";

    if (value_city != "") {
        emit GPSfinished();
    }
    else {
        //QMessageBox::warning(this, "警告", "定位失败");
        currentCity = "长春";
        currentCityCode = "101060101";
        emit GPSfinished();
    }
}

// 对返回的数据进行json解析处理
void weatherwindow::replyFinished(QNetworkReply *arg) // 对返回的数据进行json解析处理
{
    QString js = arg->readAll();                                       // 取出返回的数据
    QJsonParseError error;                                             // 定义json解析错误
    QJsonDocument json = QJsonDocument::fromJson(js.toUtf8(), &error); // 解析json
    if (error.error != QJsonParseError::NoError)
        qDebug() << error.errorString();
    else
        qDebug() << "init json succeed!";
    QJsonObject obj = json.object(); // 取出json对象
    QJsonValue value1 = obj.take("data"); // 取出data里的值
    QJsonObject obj1 = value1.toObject(); // 取出data里的对象
    QJsonValue value2 = obj.take("cityInfo"); // 取出cityInfo里的值
    QJsonObject obj2 = value2.toObject();     // 取出cityInfo里的对象
    QJsonValue value_city = obj2.take("city"); // 取出cityInfo里的city的值
    ui->city->setText(value_city.toString()); // 将city的值赋给city
    if (value_city.toString().remove("市") == currentCity) currentLocation = true;
    QJsonValue value_centg = obj1.take("wendu"); // 取出data里的wendu的值
    ui->centigrade->setText(value_centg.toString() + "℃"); // 将wendu的值赋给centigrade
    QJsonValue value_apilevel = obj1.take("quality"); // 取出data里的quality的值
    ui->aqilevel->setText("级别 " + value_apilevel.toString()); // 将quality的值赋给aqilevel
    QJsonValue value_humid = obj1.take("shidu"); // 取出data里的shidu的值
    ui->humid->setText("湿度 " + value_humid.toString()); // 将shidu的值赋给humid
    QJsonValue value = obj1.take("forecast"); // 取出data里的forecast的值
    QJsonArray arr = value.toArray();         // 将forecast的值转换为数组
    QVariantList list = arr.toVariantList();      // 将数组转换为列表
    QVariantMap map = list[0].toMap();            // 将列表转换为map
    ui->weekday->setText(map["week"].toString()); // 将map里的week的值赋给weekday
    ui->date_2->setText(map["ymd"].toString());   // 将map里的ymd的值赋给date_2
    ui->aqi->setText("空气质量指数 " + QString::number(map["aqi"].toInt()));
    ui->wind->setText("风速 " + map["fx"].toString() + " " + map["fl"].toString());
    ui->type->setText(map["type"].toString());
    if (currentLocation)
    {
        ui->noticeIcon->setVisible(true);
        drivenotice(map["type"].toString());
    }
    else {
        ui->noticeIcon->setVisible(false);
        ui->warningPanel->setText("");
    }
    if (map["type"].toString() == "晴")
        ui->icon->setPixmap(QPixmap(QString::fromUtf8(":/weather/sunny.png")));
    if (map["type"].toString() == "多云")
        ui->icon->setPixmap(QPixmap(QString::fromUtf8(":/weather/cloudy.png")));
    if (map["type"].toString() == "阴")
        ui->icon->setPixmap(QPixmap(QString::fromUtf8(":/weather/duoyun.png")));
    if (map["type"].toString() == "小雨")
        ui->icon->setPixmap(QPixmap(QString::fromUtf8(":/weather/xiaoyu.png")));
    if (map["type"].toString() == "中雨")
        ui->icon->setPixmap(QPixmap(QString::fromUtf8(":/weather/zhongyu.png")));
    if (map["type"].toString() == "大雨")
        ui->icon->setPixmap(QPixmap(QString::fromUtf8(":/weather/dayu.png")));
    if (map["type"].toString() == "小雪" || map["type"].toString() == "中雪" || map["type"].toString() == "大雪")
        ui->icon->setPixmap(QPixmap(QString::fromUtf8(":/weather/snowy.png")));
    if (map["type"].toString() == "霾")
        ui->icon->setPixmap(QPixmap(QString::fromUtf8(":/weather/mai.png")));
    changeBackground(map["type"].toString());
    for (int i = 0; i < 6; i++)
    {
        QVariantMap map = list[i].toMap();
        dw[i]->setText(map["date"].toString() + "(" + map["week"].toString() + ")");
        hg[i]->setText(map["high"].toString().mid(3).remove("℃") + "°");
        lw[i]->setText(map["low"].toString().mid(3).remove("℃") + "°");
        if (map["type"].toString() == "晴")
        {
            tpe1[i]->setPixmap(QPixmap(QString::fromUtf8(":/weather/sunny.png")));
        }
        if (map["type"].toString() == "多云")
        {
            tpe1[i]->setPixmap(QPixmap(QString::fromUtf8(":/weather/cloudy.png")));
        }
        if (map["type"].toString() == "阴")
        {
            tpe1[i]->setPixmap(QPixmap(QString::fromUtf8(":/weather/duoyun.png")));
        }
        if (map["type"].toString() == "小雨")
        {
            tpe1[i]->setPixmap(QPixmap(QString::fromUtf8(":/weather/xiaoyu.png")));
        }
        if (map["type"].toString() == "中雨")
        {
            tpe1[i]->setPixmap(QPixmap(QString::fromUtf8(":/weather/zhongyu.png")));
        }
        if (map["type"].toString() == "大雨")
        {
            tpe1[i]->setPixmap(QPixmap(QString::fromUtf8(":/weather/dayu.png")));
        }
        if (map["type"].toString() == "小雪" || map["type"].toString() == "中雪" || map["type"].toString() == "大雪")
        {
            tpe1[i]->setPixmap(QPixmap(QString::fromUtf8(":/weather/snowy.png")));
        }
        if (map["type"].toString() == "霾")
        {
            tpe1[i]->setPixmap(QPixmap(QString::fromUtf8(":/weather/mai.png")));
        }
    }
}


// search
void weatherwindow::on_searchbutton_clicked()
{
    this->currentLocation = false;
    QString city = ui->searchline->text();
    if (city.isEmpty())
    {
        QMessageBox::warning(this, "警告", "城市不能为空");
        return;
    }
    QString filePath = ":/search/cities.json";
    QString cityCode = getCityCode(city, filePath);

    if (cityCode.isEmpty())
    {
        QMessageBox::warning(this, "警告", "城市不存在");
        ui->searchline->clear();
        return;
    }
    qDebug() << cityCode;
    manager->get(QNetworkRequest(QUrl("http://t.weather.itboy.net/api/weather/city/" + cityCode)));
}


QString weatherwindow::getCityCode(const QString& cityName, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open the JSON file:" << file.errorString();
        return QString();  // Return an empty string indicating failure
    }
    QByteArray jsonData = file.readAll();
    file.close();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        qDebug() << "Error parsing JSON:" << parseError.errorString();
        return QString();  // Return an empty string indicating failure
    }
    QJsonObject rootObject = jsonDoc.object();

    if (rootObject.contains("城市代码"))
    {
        QJsonArray cityCodeArray = rootObject.value("城市代码").toArray();
        for (const QJsonValue& provinceValue : cityCodeArray)
        {
            QJsonObject provinceObject = provinceValue.toObject();
            QJsonArray cityArray = provinceObject.value("市").toArray();
            for (const QJsonValue& cityValue : cityArray)
            {
                QJsonObject cityObject = cityValue.toObject();
                QString currentCityName = cityObject.value("市名").toString();
                QString cityCode = cityObject.value("编码").toString();
                if (currentCityName == cityName)
                {
                    return cityCode;  // Return the city code if found
                }
            }
        }
    }
    return QString();   
}


void weatherwindow::on_searchbutton_2_clicked()
{
    currentLocation = true;
    manager->get(QNetworkRequest(QUrl("http://t.weather.itboy.net/api/weather/city/" + currentCityCode)));
}

// sidebar button clicked
void weatherwindow::on_searchbutton_3_clicked(QString cityname)
{
    currentLocation = false;
    QString filePath = ":/search/cities.json";
    QString cityCode = getCityCode(cityname, filePath);
    manager->get(QNetworkRequest(QUrl("http://t.weather.itboy.net/api/weather/city/" + cityCode)));
}


void weatherwindow::drivenotice(QString weatherConditon)
{
    // 行车提醒
    if (weatherConditon == "晴") {
        ui->warningPanel->setText("天气晴朗，适合出行\n请勿疲劳驾驶");
    }
    else if (weatherConditon == "多云") {
        ui->warningPanel->setText("天气多云，适合出行\n请勿疲劳驾驶");
    }
    else if (weatherConditon == "阴") {
        ui->warningPanel->setText("\n请勿疲劳驾驶");
    }
    else if (weatherConditon == "小雨" || weatherConditon == "中雨" || weatherConditon == "大雨") {
        ui->warningPanel->setText("雨天路滑，注意行车安全");
    }
    else if (weatherConditon == "小雪" || weatherConditon == "中雪" || weatherConditon == "大雪") {
        ui->warningPanel->setText("路面积雪结冰\n注意行车安全");
    }
    else if (weatherConditon == "霾") {
        ui->warningPanel->setText("雾霾天，能见度低\n注意行车安全");
    }
}


void weatherwindow::changeBackground(QString weatherCondtion)
{
    
    //清除background加载的图片
    background->load("");

    if (weatherCondtion == "晴")
    {
        background->load(":/back/weather/sunnyday.jpg");
    }

    if (weatherCondtion == "多云")
    {
        background->load(":/back/weather/cloudyday.jpg");
    }

    if (weatherCondtion == "阴")
    {
        background->load(":/back/weather/overcast.jpg");
    }

    if (weatherCondtion == "小雨" || weatherCondtion == "中雨" || weatherCondtion == "大雨")
    {
        background->load(":/back/weather/rainyday.jpg");
    }

    if (weatherCondtion == "小雪" || weatherCondtion == "中雪" || weatherCondtion == "大雪")
    {
        background->load(":/back/weather/snowyday.jpg");
    }

    if (weatherCondtion == "霾")
    {
        background->load(":/back/weather/foggy.jpg");
    }

    palette->setBrush(QPalette::Window, QBrush(background->scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    this->setPalette(*palette);
    this->setAutoFillBackground(true);
}

void weatherwindow::initLable() {
    // 对qlabel进行赋值
    dw[0] = ui->dateweekday;
    dw[1] = ui->dateweekday_2;
    dw[2] = ui->dateweekday_3;
    dw[3] = ui->dateweekday_4;
    dw[4] = ui->dateweekday_5;
    dw[5] = ui->dateweekday_6;

    tpe1[0] = ui->type1;
    tpe1[1] = ui->type1_2;
    tpe1[2] = ui->type1_3;
    tpe1[3] = ui->type1_4;
    tpe1[4] = ui->type1_5;
    tpe1[5] = ui->type1_6;

    hg[0] = ui->high;
    hg[1] = ui->high_2;
    hg[2] = ui->high_3;
    hg[3] = ui->high_4;
    hg[4] = ui->high_5;
    hg[5] = ui->high_6;

    lw[0] = ui->low;
    lw[1] = ui->low_2;
    lw[2] = ui->low_3;
    lw[3] = ui->low_4;
    lw[4] = ui->low_5;
    lw[5] = ui->low_6;
}




