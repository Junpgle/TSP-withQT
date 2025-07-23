#include "mainwindow.h"
#include <QMessageBox> // 用于显示消息的对话课
#include <QFile> // 文件流
#include <QTextStream> // 文本数据流
#include <cmath>
#include <QGraphicsTextItem> // 文本框

CityMapWidget::CityMapWidget(QWidget *parent) : QGraphicsView(parent) {
    scene = new QGraphicsScene(this);
    setScene(scene); // 使视图显示该场景的内容
    setRenderHint(QPainter::Antialiasing); // 启用抗锯齿渲染
    setBackgroundBrush(QBrush(Qt::white)); // 场景的背景为白色
}

// 依据给定的城市坐标信息，在图形场景里绘制城市以及它们之间的路径
void CityMapWidget::setCities(const QList<City>& cities) {
    this->cities = cities;
    scene->clear();
    cityItems.clear();

    if (cities.isEmpty()) return;

    // 计算坐标范围
    double minX = cities[0].x, maxX = cities[0].x;
    double minY = cities[0].y, maxY = cities[0].y;

    for (const auto& city : cities) {
        minX = qMin(minX, city.x);
        maxX = qMax(maxX, city.x);
        minY = qMin(minY, city.y);
        maxY = qMax(maxY, city.y);
    }

    double width = maxX - minX;
    double height = maxY - minY;

    if (width <= 0) width = 1.0;
    if (height <= 0) height = 1.0;

    // 视图边距
    double marginPercent = 0.1;
    double viewWidth = this->width() * (1 - 2 * marginPercent);
    double viewHeight = this->height() * (1 - 2 * marginPercent);

    // 缩放比例
    scale = qMin(viewWidth / width, viewHeight / height);

    // 计算偏移量
    double centerX = this->width() / 2;
    double centerY = this->height() / 2;
    double cityCenterX = (minX + maxX) / 2;
    double cityCenterY = (minY + maxY) / 2;
    offsetX = centerX - cityCenterX * scale;
    offsetY = centerY - cityCenterY * scale;

    // 绘制城市
    for (const auto& city : cities) {
        // 使用与路径相同的坐标变换公式
        double x = city.x * scale + offsetX;
        double y = city.y * scale + offsetY;

        // y轴反转
        y = this->height() - y;

        // 绘制城市点
        QGraphicsEllipseItem *cityItem =
            scene->addEllipse(x-5, y-5, 10, 10, QPen(Qt::black), QBrush(Qt::red));
        cityItems[city.name] = cityItem;

        // 绘制城市标签
        QGraphicsTextItem *textItem = scene->addText(city.name);
        textItem->setDefaultTextColor(Qt::black);

        // 标签放在城市点上方，右侧对齐
        double textX = x + 5;  // 右侧偏移
        double textY = y - 20; // 上方偏移

        textItem->setPos(textX, textY);
    }

    // 绘制旅行商问题的路径
    if (!path.isEmpty()) {
        QPen pen(Qt::blue);
        pen.setWidth(2);

        for (int i = 0; i < path.size() - 1; ++i) {
            double x1 = path[i].x * scale + offsetX;
            double y1 = path[i].y * scale + offsetY;
            double x2 = path[i+1].x * scale + offsetX;
            double y2 = path[i+1].y * scale + offsetY;

            // 应用 Y 轴反转
            y1 = this->height() - y1;
            y2 = this->height() - y2;

            scene->addLine(x1, y1, x2, y2, pen);
        }


        // 绘制终点到起点的连线，使路径闭合
        if (path.size() >= 2) {
            double startX = path.first().x * scale + offsetX;
            double startY = path.first().y * scale + offsetY;
            double endX = path.last().x * scale + offsetX;
            double endY = path.last().y * scale + offsetY;

            // 应用 Y 轴反转
            startY = this->height() - startY;
            endY = this->height() - endY;

            scene->addLine(endX, endY, startX, startY, pen);
        }
    }

    // 0, 0：场景的左上角坐标（原点）。
    // this->width(), this->height()：场景的宽度和高度，通常取自当前窗口（this）的尺寸。
    scene->setSceneRect(0, 0, this->width(), this->height());
    fitInView(scene->sceneRect(), Qt::KeepAspectRatio); // 保持场景的宽高比，避免内容变形
}

void CityMapWidget::setPath(const QList<City>& path) {
    this->path = path;
    setCities(cities); // 重新绘制地图和路径
}

void CityMapWidget::clearPath() {
    path.clear();
    setCities(cities); // 清除路径(重新绘制地图，不包含路径)
}

void CityMapWidget::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    setCities(cities); // 窗口大小变化时重绘(包括城市和路径）
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 主程序大框架垂直布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mapWidget = new CityMapWidget(this);
    mainLayout->addWidget(mapWidget, 2); // 垂直排列 1:可视化,占2/3

    // 创建选项卡组件
    QTabWidget *tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget, 1); // 垂直排列 2: 下方选项卡,占1/3

    /***********城市管理选项卡*****************/
    QWidget *manageTab = new QWidget(this);
    QHBoxLayout *manageLayout = new QHBoxLayout(manageTab);

    // 增删区域
    QVBoxLayout *addremoveLayout = new QVBoxLayout;

    // 增删功能横向布局
    QHBoxLayout *addLayout = new QHBoxLayout();
    addLayout->addWidget(new QLabel("城市名称:", this));
    cityNameEdit = new QLineEdit(this);
    addLayout->addWidget(cityNameEdit);

    addLayout->addWidget(new QLabel("X坐标:", this));
    xCoordEdit = new QLineEdit(this);
    addLayout->addWidget(xCoordEdit);

    addLayout->addWidget(new QLabel("Y坐标:", this));
    yCoordEdit = new QLineEdit(this);
    addLayout->addWidget(yCoordEdit);

    QPushButton *addButton = new QPushButton("添加城市", this);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addCity); // 将按钮控件和函数事件绑定
    addLayout->addWidget(addButton);
    addremoveLayout->addLayout(addLayout);

    QHBoxLayout *removeLayout = new QHBoxLayout();
    removeLayout->addWidget(new QLabel("删除城市:", this));
    cityCombo1 = new QComboBox(this);
    removeLayout->addWidget(cityCombo1);
    QPushButton *removeButton = new QPushButton("删除", this);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::removeCity);
    removeLayout->addWidget(removeButton);
    addremoveLayout->addLayout(removeLayout);

    manageLayout->addLayout(addremoveLayout);

    // 城市列表区域
    QVBoxLayout *cityListLayout = new QVBoxLayout;
    cityListLayout->addWidget(new QLabel("城市列表:", this));

    // 初始化城市列表控件
    cityListWidget = new QListWidget(this);
    cityListLayout->addWidget(cityListWidget);
    manageLayout->addLayout(cityListLayout);

    manageLayout->addLayout(cityListLayout);

    tabWidget->addTab(manageTab, "城市管理");



    /*************距离计算选项卡*****************/
    QWidget *distanceTab = new QWidget(this);
    QVBoxLayout *distanceLayout = new QVBoxLayout(distanceTab);

    QHBoxLayout *distLayout = new QHBoxLayout();

    distLayout->addWidget(new QLabel("城市1:", this));
    cityCombo2 = new QComboBox(this);
    distLayout->addWidget(cityCombo2);

    distLayout->addWidget(new QLabel("城市2:", this));
    cityCombo3 = new QComboBox(this);
    distLayout->addWidget(cityCombo3);

    QPushButton *calcButton = new QPushButton("计算距离", this);
    connect(calcButton, &QPushButton::clicked, this, &MainWindow::calculateDistance);
    distLayout->addWidget(calcButton);
    distanceLayout->addLayout(distLayout);

    statusLabel = new QLabel("距离将显示在这里", this);
    distanceLayout->addWidget(statusLabel);

    tabWidget->addTab(distanceTab, "距离计算");

    /***************查询范围距离内城市选项卡*****************/
    QWidget *rangeTab = new QWidget(this);
    QVBoxLayout *rangeLayout = new QVBoxLayout(rangeTab);

    QHBoxLayout *rangeQueryLayout = new QHBoxLayout();
    rangeQueryLayout->addWidget(new QLabel("中心城市:", this),0);

    cityCombo4 = new QComboBox(this);
    rangeQueryLayout->addWidget(cityCombo4);

    rangeQueryLayout->addWidget(new QLabel("范围:", this));
    rangeEdit = new QLineEdit(this);
    rangeEdit->setText("10"); // 默认值为 10
    rangeQueryLayout->addWidget(rangeEdit);

    QPushButton *rangeButton = new QPushButton("查询范围内城市", this);
    connect(rangeButton, &QPushButton::clicked, this, &MainWindow::findCitiesInRange);
    rangeQueryLayout->addWidget(rangeButton);
    rangeLayout->addLayout(rangeQueryLayout);

    tabWidget->addTab(rangeTab, "范围查询");

    /***************TSP求解选项卡*****************/
    QWidget *tspTab = new QWidget(this);

    QVBoxLayout *tspLayout = new QVBoxLayout(tspTab);

    QPushButton *tspButton = new QPushButton("使用穷举法计算最短路径(精确但慢)", this);
    connect(tspButton, &QPushButton::clicked, this, &MainWindow::solveTSP);
    tspLayout->addWidget(tspButton);

    QPushButton *simulatedAnnealingButton = new QPushButton("使用模拟退火算法计算最短路径(较快,但可能不精确)",this);
    connect(simulatedAnnealingButton,&QPushButton::clicked,this,&MainWindow::solveTSPWithSimulatedAnnealing);
    tspLayout->addWidget(simulatedAnnealingButton);

    // 日志显示区域
    logTextEdit = new QTextEdit(this);
    logTextEdit->setReadOnly(true);
    logTextEdit->setFont(QFont("Consolas", 9));
    tspLayout->addWidget(logTextEdit, 2);  // 占2份空间


    tabWidget->addTab(tspTab, "旅行商问题求解");

    /***************文件操作选项卡*****************/
    QWidget *fileTab = new QWidget(this);
    QVBoxLayout *fileLayout = new QVBoxLayout(fileTab);

    QHBoxLayout *fileButtonLayout = new QHBoxLayout();

    QPushButton *loadButton = new QPushButton("从文件加载", this);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadFromFile);
    fileButtonLayout->addWidget(loadButton);

    QPushButton *saveButton = new QPushButton("保存到文件", this);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveToFile);
    fileButtonLayout->addWidget(saveButton);
    fileLayout->addLayout(fileButtonLayout);

    tabWidget->addTab(fileTab, "文件操作");



    /***************关于我们*****************/
    QWidget *aboutTab = new QWidget(this);
    QHBoxLayout *aboutLayout = new QHBoxLayout(aboutTab);
    statusLabel2 = new QLabel("作者:Junpgle 2025.7.6初版", this);
    aboutLayout->addWidget(statusLabel2);

    tabWidget->addTab(aboutTab,"关于");

    // 加载初始城市数据
    loadFromFile();
}

MainWindow::~MainWindow() {
}

void MainWindow::addCity() {
    QString name = cityNameEdit->text();
    bool okX, okY;
    double x = xCoordEdit->text().toDouble(&okX);
    double y = yCoordEdit->text().toDouble(&okY);

    if (name.isEmpty() || !okX || !okY) {
        QMessageBox::warning(this, "输入错误", "请输入有效的城市名称和坐标");
        return;
    }

    City city;
    city.name = name;
    city.x = x;
    city.y = y;

    if (cityManager.addCity(city)) { 
        cityNameEdit->clear();
        xCoordEdit->clear();
        yCoordEdit->clear();

        // 更新下拉列表
        cityCombo1->clear();
        cityCombo2->clear();
        cityCombo3->clear();
        cityCombo4->clear();
        Q_FOREACH (const auto& c, cityManager.getAllCities()) {
            cityCombo1->addItem(c.name);
            cityCombo2->addItem(c.name);
            cityCombo3->addItem(c.name);
            cityCombo4->addItem(c.name);
        }

        // 更新地图
        mapWidget->clearPath();
        mapWidget->setCities(cityManager.getAllCities());
        updateCityList(); // 更新城市列表
        QMessageBox::information(this, "成功", "城市添加成功");
    } else {
        QMessageBox::warning(this, "失败", "城市添加失败，可能名称已存在");
    }
}

void MainWindow::removeCity() {
    QString name = cityCombo1->currentText();
    if (name.isEmpty()) return;

    if (cityManager.removeCity(name)) {
        // 更新下拉列表
        cityCombo1->clear();
        cityCombo2->clear();
        cityCombo3->clear();
        cityCombo4->clear();
        Q_FOREACH (const auto& c, cityManager.getAllCities()) {
            cityCombo1->addItem(c.name);
            cityCombo2->addItem(c.name);
            cityCombo3->addItem(c.name);
            cityCombo4->addItem(c.name);
        }

        // 更新地图
        mapWidget->clearPath();
        mapWidget->setCities(cityManager.getAllCities());
        updateCityList(); // 更新城市列表
        QMessageBox::information(this, "成功", "城市删除成功");
    } else {
        QMessageBox::warning(this, "失败", "城市删除失败");
    }
}

void MainWindow::calculateDistance() {
    QString name1 = cityCombo2->currentText();
    QString name2 = cityCombo3->currentText();

    City city1 = cityManager.findCity(name1);
    City city2 = cityManager.findCity(name2);

    if (city1.name.isEmpty() || city2.name.isEmpty()) {
        QMessageBox::warning(this, "错误", "找不到指定的城市");
        return;
    }

    mapWidget->drawDistanceLine(city1,city2);

    double dist = cityManager.distance(city1, city2);
    statusLabel->setText(QString("从 %1 到 %2 的距离是: %3").arg(name1,name2,QString::number(dist)));
}

void CityMapWidget::drawDistanceLine(const City &city1,const City &city2){
    clearPath();
    double startX = city1.x * scale + offsetX;
    double startY = city1.y * scale + offsetY;
    double endX = city2.x * scale + offsetX;
    double endY = city2.y * scale + offsetY;

    // 应用 Y 轴反转
    startY = this->height() - startY;
    endY = this->height() - endY;

    QPen pen(Qt::blue);
    pen.setWidth(2);
    setCities(cities);
    scene->addLine(endX, endY, startX, startY, pen);
}


void MainWindow::findCitiesInRange() {
    QString name = cityCombo4->currentText();
    bool ok;
    double range = rangeEdit->text().toDouble(&ok);

    if (!ok || range <= 0) {
        QMessageBox::warning(this, "输入错误", "请输入有效的范围值");
        return;
    }

    City center = cityManager.findCity(name);
    if (center.name.isEmpty()) {
        QMessageBox::warning(this, "错误", "找不到指定的城市");
        return;
    }

    QList<City> citiesInRange = cityManager.getCitiesWithinRange(name, range);

    QString result = QString("距离 %1 (范围 <= %2) 的城市有:\n").arg(name).arg(range);
    Q_FOREACH(const auto& city, citiesInRange) {
        double dist = cityManager.distance(center, city);
        result += QString("%1 (距离: %2)\n").arg(city.name).arg(dist);
    }



    if (citiesInRange.isEmpty()) {
        result += "没有找到符合条件的城市";
    }

    mapWidget->drawRangeCircle(center,range);
    QMessageBox::information(this, "查询结果", result);


}

void CityMapWidget::drawRangeCircle(const City&center,double range){
    clearPath();
    double circleX = center.x * scale + offsetX;
    double circleY = center.y * scale + offsetY;

    circleY = this->height() - circleY;

    double circleRadius = range * scale;
    QPen pen(Qt::blue);
    pen.setWidth(2);
    scene->addEllipse(
        circleX - circleRadius,
        circleY - circleRadius,
        2 * circleRadius,
        2 * circleRadius,
        pen
        );
}

void MainWindow::solveTSP() {
    if (cityManager.getCityCount() < 2) {
        QMessageBox::information(this, "提示", "至少需要两个城市来求解旅行商问题");
        return;
    }

    // 清空日志
    logTextEdit->clear();
    logTextEdit->append("穷举法求解开始...");

    // 收集步骤
    QList<BruteForceStep> steps;
    QList<City> path = cityManager.solveTSP(&steps);

    // 显示每一步日志
    Q_FOREACH (const auto& step, steps) {
        QString logLine = QString("[%1/%2] %3 | 距离: %4")
                              .arg(step.iteration)
                              .arg(step.totalPermutations)
                              .arg(step.message)
                              .arg(step.currentDistance, 8, 'f', 3);
        logTextEdit->append(logLine);
    }

    if (!path.isEmpty()) {
        mapWidget->setPath(path);

        // 构建路径字符串（包含回到起点）
        QString pathStr = "最优路径（闭合回路）:\n";
        for (int i = 0; i < path.size() - 1; ++i) {  // 减去1以避免重复显示起点
            pathStr += QString("%1. %2\n").arg(i+1).arg(path[i].name);
        }
        pathStr += QString("%1. %2（回到起点）\n").arg(path.size()).arg(path.last().name);

        // 计算总距离（包含回到起点）
        double totalDistance = cityManager.calculateTotalDistance(path);
        pathStr += QString("\n总距离: %1").arg(totalDistance);

        QMessageBox::information(this, "求解结果", pathStr);
    } else {
        logTextEdit->append("求解失败，无法找到有效的路径");
        QMessageBox::warning(this, "求解失败", "无法找到有效的路径");
    }
}

void MainWindow::solveTSPWithSimulatedAnnealing() {
    if (cityManager.getCityCount() < 2) {
        QMessageBox::information(this, "提示", "至少需要两个城市");
        return;
    }

    // 清空日志
    logTextEdit->clear();
    logTextEdit->append("模拟退火算法开始...");

    // 收集步骤
    QList<AnnealingStep> steps;
    QList<City> path = cityManager.solveTSPWithSimulatedAnnealing(&steps);

    // 显示每一步
    for (const auto& step : steps) {
        QString logLine = QString("[%1] T=%2 | 当前路径=%3 | 最优路径=%4 | %5")
                              .arg(step.iteration, 4)
                              .arg(step.temperature, 8, 'f', 3)
                              .arg(step.currentEnergy, 8, 'f', 3)
                              .arg(step.bestEnergy, 8, 'f', 3)
                              .arg(step.message);

        logTextEdit->append(logLine);
    }

    double totalDistance = cityManager.calculateTotalDistance(path);

    QString pathInfo = "最优路径:\n";
    for (int i = 0; i < path.size(); ++i) {
        pathInfo += QString("%1. %2\n")
        .arg(i + 1)
            .arg(path[i].name);
    }
    pathInfo += QString("\n总距离: %1").arg(totalDistance);

    // 显示最终结果
    if (!path.isEmpty()) {
        mapWidget->setPath(path);
        logTextEdit->append("\n=== 最终结果 ===");
        logTextEdit->append(QString("最优路径长度: %1").arg(totalDistance));
        QMessageBox::information(this, "求解结果", pathInfo);
    } else {
        logTextEdit->append("求解失败，无法找到有效路径");
    }
}

void MainWindow::loadFromFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "打开城市文件", "", "文本文件 (*.txt)");
    if (fileName.isEmpty()) return;

    if (cityManager.loadFromFile(fileName)) {
        QMessageBox::information(this, "成功", "文件加载成功");

        // 更新下拉列表
        cityCombo1->clear();
        cityCombo2->clear();
        cityCombo3->clear();
        cityCombo4->clear();
        Q_FOREACH (const auto& c, cityManager.getAllCities()) {
            cityCombo1->addItem(c.name);
            cityCombo2->addItem(c.name);
            cityCombo3->addItem(c.name);
            cityCombo4->addItem(c.name);
        }

        // 设置 cityCombo3 的初始值为 cityCombo2 的下一个城市
        if (cityCombo2->count() > 1) { // 确保有足够的城市
            int nextIndex = (cityCombo2->currentIndex() + 1) % cityCombo2->count();
            cityCombo3->setCurrentIndex(nextIndex);
        }

        // 更新地图
        mapWidget->clearPath();
        mapWidget->setCities(cityManager.getAllCities());
        updateCityList(); // 更新城市列表
    } else {
        QMessageBox::warning(this, "失败", "文件加载失败");
    }
}

void MainWindow::saveToFile() {
    QString fileName = QFileDialog::getSaveFileName(this, "保存城市文件", "", "文本文件 (*.txt)");
    if (fileName.isEmpty()) return;

    if (cityManager.saveToFile(fileName)) {
        QMessageBox::information(this, "成功", "文件保存成功");
    } else {
        QMessageBox::warning(this, "失败", "文件保存失败");
    }
}

void MainWindow::updateCityList()
{
    cityListWidget->clear(); // 先清空原有内容
    // 从 cityManager 获取所有城市数据
    QList<City> allCities = cityManager.getAllCities();
    Q_FOREACH (const auto& city, allCities) {
        // 拼接城市名、x、y 坐标为字符串，格式如“城市名 x:100 y:200”
        QString cityInfo = QString("%1 x:%2 y:%3")
                               .arg(city.name)
                               .arg(city.x)
                               .arg(city.y);
        cityListWidget->addItem(cityInfo);
    }
}
