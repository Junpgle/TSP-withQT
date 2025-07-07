#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>  // 主窗口的基础框架
#include <QGraphicsView>  // 是一个可滚动的视口，适合展示复杂的二维图形
#include <QGraphicsScene> // 管理大量的 2D 图形项
#include <QPushButton> // 创建可点击的按钮
#include <QVBoxLayout> // 垂直排列界面元素，比如按钮、标签等
#include <QHBoxLayout> // 水平排列界面元素
#include <QLineEdit> // 单行文本输入框
#include <QLabel>  // 静态文本或者图像
#include <QComboBox> // 创建下拉选择框
#include <QListWidget> // 列表
#include <QTextEdit>
#include <QFileDialog> // 文件选择对话框
#include "citymanager.h"

class CityMapWidget : public QGraphicsView {
    Q_OBJECT
public:
    CityMapWidget(QWidget *parent = nullptr);
    void setCities(const QList<City>& cities);
    void setPath(const QList<City>& path);
    void clearPath();
    void drawDistanceLine(const City &city1,const City &city2);
    void drawRangeCircle(const City &center,double range);

protected:
    void resizeEvent(QResizeEvent *event) override;


private:
    double scale;
    double offsetX;
    double offsetY;
    QGraphicsScene *scene;
    QList<City> cities;
    QList<City> path;
    QMap<QString, QGraphicsEllipseItem*> cityItems;

};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addCity();
    void removeCity();
    void calculateDistance();
    void findCitiesInRange();
    void solveTSP();
    void solveTSPWithSimulatedAnnealing();
    void loadFromFile();
    void saveToFile();
    void updateCityList();

private:
    CityManager cityManager;
    CityMapWidget *mapWidget;
    QLineEdit *cityNameEdit, *xCoordEdit, *yCoordEdit, *rangeEdit; // 文本输入框
    QComboBox *cityCombo1, *cityCombo2, *cityCombo3, *cityCombo4; // 城市下拉选择框
    QListWidget *cityListWidget; // 城市列表
    QLabel *statusLabel;
    QTextEdit *logTextEdit;
};

#endif // MAINWINDOW_H
