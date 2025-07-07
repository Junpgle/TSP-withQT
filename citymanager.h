#ifndef CITYMANAGER_H
#define CITYMANAGER_H

#include <QList>
#include <QString>
#include <cmath>
#include <random>

struct City {
    QString name;
    double x;
    double y;

    // 运算符重载 QList::removeOne()需要比较两个City对象是否相等
    bool operator==(const City& other) const {
        return name == other.name && x == other.x && y == other.y;
    }
};

// 穷举法步骤信息
struct BruteForceStep {
    int iteration;            // 当前迭代次数
    QList<City> currentPath;  // 当前路径
    double currentDistance;   // 当前路径距离
    double bestDistance;      // 已知最优距离
    int totalPermutations=0; // 需要穷举的总次数
    QString message;          // 步骤描述
};

// 记录模拟退火算法日志
struct AnnealingStep {
    int iteration;        // 当前迭代次数
    double temperature;   // 当前温度
    double currentEnergy; // 当前路径长度
    double bestEnergy;    // 历史最优路径长度
    QString message;      // 步骤描述
};

class CityManager {
protected:
    int size = 0; // 城市数量
    static const int HASH_SIZE = 100; // 哈希表大小

    // 拉链法解决哈希冲突
    struct Node {
        City city;
        Node *next{};

        Node(const City &c) : city(c) {
        }
    };

    // 哈希表数组
    Node *list[HASH_SIZE] = {};

    // 哈希值用城市名称的ASCII的码和求余数
    int hash(const QString &name) const {
        int sum = 0;
        Q_FOREACH (QChar c, name) {
            // 使用unsigned char,避免负数
            sum += c.unicode();
        }
        return sum % HASH_SIZE;
    }


    // 模拟退火需要的随机数生成器
    std::random_device rd;
    std::mt19937 rng;

public:
    CityManager();
    ~CityManager();

    // 增加城市
    bool addCity(const City& city);

    // 删除城市
    bool removeCity(const QString& name);

    // 查找城市
    City findCity(const QString& name) const;

    // 城市距离
    double distance(const City& a, const City& b) const;

    // 城市数量
    int getCityCount() const;

    // 所有城市
    QList<City> getAllCities() const;

    // 找出与指定城市距离在给定范围内的所有城市
    QList<City> getCitiesWithinRange(const QString& targetCityName, double range) const;

    /****************穷举法求解旅行商问题起点************/

    // 计算阶乘
    long long factorial(int n) const;

    // 根据索引列表构建路径
    QList<City> buildPathFromIndices(const QList<City>& cityList, const QList<int>& indices) const;

    // 穷举法求解旅行商问题
    QList<City> solveTSP(QList<BruteForceStep>* steps) const;

    /****************模拟退火算法起点********************/
    // 模拟退火算法求解旅行商问题
    QList<City> solveTSPWithSimulatedAnnealing(QList<AnnealingStep>* steps);

    // 生成初始解
    QList<City> generateInitialSolution(const QList<City>& cities);

    // 2-opt优化
    QList<City> generateNeighbor(const QList<City>& solution);

    // 根据城市数量自适应调整参数
    void adjustParameters(int cityCount, double& initialTemp, double& coolingRate, int& iterationsPerTemp);

    // 接受程度计算
    bool acceptNewSolution(double energyDiff, double temperature);

    // 路径总距离计算
    double calculateTotalDistance(const QList<City>& path);

    /****************模拟退火算法终点********************/

    // 从文件中加载
    bool loadFromFile(const QString& filename);

    // 保存到文件
    bool saveToFile(const QString& filename) const;


};

#endif // CITYMANAGER_H
