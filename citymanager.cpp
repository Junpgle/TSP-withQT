#include "citymanager.h"
#include <iostream>
#include "qregularexpression.h"
#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <QRegularExpression>
#include <random>

CityManager::CityManager() {
}

CityManager::~CityManager() {
}

// 添加城市
bool CityManager::addCity(const City& city) {
    int index = CityManager::hash(city.name);

    // 检查是否有同名城市
    for (Node *node = list[index]; node; node = node->next) {
        if (node->city.name == city.name) {
            std::cout << "城市已存在!无法重复添加。" << std::endl;
            return false; // 同名城市不添加
        }
    }

    // 创建新节点
    Node *newNode = new Node(city);

    if (!list[index]) {
        // 链表为空，直接插入
        list[index] = newNode;
        size++; // 城市数量+1
        return true;
    } else {
        // 尾插法
        // Node *current = list[index];
        // while (current->next) {
        //     current = current->next;
        // }
        // current->next = newNode; // 将新节点连接到尾部

        // 头插法,提高效率
        newNode->next = list[index]; // 新节点指向旧的头节点
        list[index] = newNode; // 更新头指针
        size++; // 城市数量+1
        return 1;
    }
}


// 删除城市
bool CityManager::removeCity(const QString& name) {
    int index = hash(name);
    Node *current = list[index];
    Node *prev = nullptr;

    // 遍历链表查找目标节点
    while (current != nullptr) {
        if (current->city.name == name) {
            // 找到目标节点，执行删除
            if (prev == nullptr) {
                // 删除头节点
                list[index] = current->next;
            } else {
                // 删除中间或尾节点：更新前一个节点的next指针
                prev->next = current->next;
            }
            delete current; // 释放内存
            return 1;
        }
        // 移动指针
        prev = current;
        current = current->next;
    }

    // 未找到城市
    std::cout << "城市不存在!" << std::endl;
    return 0;
}

// 按名字查找城市
City CityManager::findCity(const QString& name) const {
    City result;
    int index = hash(name);
    for (const Node *node = list[index]; node; node = node->next) {
        if (node->city.name == name) {
            result = node->city;
            return result;
        }
    }
    std::cout << "城市不存在!" << std::endl;
    return {"", 0, 0}; // 返回空城市

}

// 城市距离
double CityManager::distance(const City& a, const City& b) const {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

// 城市数量
int CityManager::getCityCount() const {
    return size;
}

// 获取所有城市
QList<City> CityManager::getAllCities() const {
    QList<City> allCities;
    for (int i = 0; i < HASH_SIZE; ++i) {
        Node *current = list[i];
        while (current) {
            allCities.append(current->city);
            current = current->next;
        }
    }
    return allCities;
}

// 获取某城市一定范围内所有城市
QList<City> CityManager::getCitiesWithinRange(const QString& targetCityName, double range) const {
    QList<City> result;

    City target = findCity(targetCityName);
    if (target.name.isEmpty()) {
        return result;
    }

    QList<City> allCities = getAllCities();
    for (const auto& city : allCities) {
        if (city.name == target.name) continue;

        double dist = distance(target, city);
        if (dist <= range) {
            result.append(city);
        }
    }

    return result;
}


// 根据索引列表构建路径
QList<City> CityManager::buildPathFromIndices(const QList<City>& cityList, const QList<int>& indices) const {
    QList<City> path;
    foreach(int idx, indices) {
        path.append(cityList[idx]);
    }
    // 添加回到起点的城市
    if (!path.isEmpty()) {
        path.append(path.first());
    }
    return path;
}

// 计算阶乘
long long CityManager::factorial(int n) const {
    long long result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

// 穷举法解决旅行商问题
QList<City> CityManager::solveTSP(QList<BruteForceStep>* steps) const {
    QList<City> result;
    int n = getCityCount(); // 获取所有城市数量

    if (n < 2) return result;

    // 创建城市列表
    QList<City> cityList = getAllCities();

    // 初始排列 [0, 1, 2, ..., n-1]
    QList<int> indices;
    for (int i = 0; i < n; ++i) {
        indices.append(i);
    }

    double minDistance = std::numeric_limits<double>::max(); // 使用double最大值作为初始值
    QList<int> optimalPath; // 选择的路径
    int iteration = 0; // 初始化迭代次数为0
    int totalPermutations = factorial(n);

    // 初始化日志
    if (steps) {
        steps->clear();
        BruteForceStep initStep;
        initStep.iteration = iteration;
        initStep.currentPath = QList<City>(); // 初始为空
        initStep.currentDistance = 0;
        initStep.totalPermutations = totalPermutations;
        initStep.bestDistance = minDistance;
        initStep.message = QString("开始穷举法 (城市数: %1, 总排列数: %2)")
                               .arg(n).arg(initStep.totalPermutations);
        steps->append(initStep);
    }

    // 生成所有排列并计算路径长度
    do {
        double currentDistance = 0;

        // 计算当前路径的总距离
        for (int i = 0; i < n - 1; ++i) {
            currentDistance += distance(cityList[indices[i]], cityList[indices[i+1]]);
        }
        // 回到起点
        currentDistance += distance(cityList[indices[n-1]], cityList[indices[0]]);


        bool isNewBest = false;
        // 更新最优路径
        if (currentDistance < minDistance) {
            minDistance = currentDistance;
            optimalPath = indices;
            isNewBest = true;
        }

        // 记录当前步骤 (每一千步)
        if (steps && (iteration++ % 1000 == 0)) {
            BruteForceStep step;
            step.iteration = iteration++;
            step.totalPermutations = totalPermutations;
            step.currentPath = buildPathFromIndices(cityList, indices);
            step.currentDistance = currentDistance;
            step.bestDistance = minDistance;

            if (isNewBest) {
                step.message = QString("找到更优解: 距离=%1")
                                   .arg(currentDistance, 8, 'f', 3); // 数值占用的最小字符数8,浮点数表示,保留到小数点后三位
            } else {
                step.message = QString("当前路径距离=%1 (最优=%2)")
                                   .arg(currentDistance, 8, 'f', 3)
                                   .arg(minDistance, 8, 'f', 3);
            }

            if(step.iteration<=step.totalPermutations)
                steps->append(step);
        }

    } while (std::next_permutation(indices.begin(), indices.end())); // 生成字典序的下一种排列组合

    // 构建最优路径的城市列表
    if (!optimalPath.isEmpty()) {
        Q_FOREACH (int idx, optimalPath) {
            result.append(cityList[idx]);
        }
        // 添加回到起点的城市（闭合路径）
        result.append(cityList[optimalPath[0]]);
    }

    // 添加最终结果日志
    if (steps) {
        BruteForceStep finalStep;
        finalStep.iteration = totalPermutations;
        finalStep.currentPath = result;
        finalStep.currentDistance = minDistance;
        finalStep.totalPermutations = totalPermutations;
        finalStep.bestDistance = minDistance;
        finalStep.message = QString("穷举完成，找到最优解: 距离=%1")
                                .arg(minDistance, 8, 'f', 3);
        steps->append(finalStep);
    }

    return result;
}

/***************************模拟退火算法********************************/

// 生成初始解
QList<City> CityManager::generateInitialSolution(const QList<City>& cities) {
    int n = cities.size();
    if (n <= 2) return cities; // 直接返回

    // 贪心算法：从随机城市开始，每次选择最近且能高效回到起点的下一个城市
    QList<City> path;
    QSet<QString> visited;

    // 随机选择起点
    std::random_device rd; // 获取随机种子的设备对象
    std::mt19937 gen(rd()); // 基于梅森旋转算法的伪随机数生成器
    std::uniform_int_distribution<> dis(0, n-1); // 指定生成随机整数的范围是 [0, n - 1]
    int startIdx = dis(gen); // 生成一个在 [0, n - 1] 范围内的随机整数，作为要选择城市的索引
    path.append(cities[startIdx]); // 加入路径
    visited.insert(cities[startIdx].name); // 标记已访问

    // 构建路径
    while (path.size() < n) {
        City last = path.last(); // 当前路径的最后一个城市
        City bestNext; // 存储最优的下一城市
        double minCost = std::numeric_limits<double>::max(); // 定义默认成本为double的最大值

        // 寻找下一个城市，同时考虑最终回到起点的成本
        for (const auto& city : cities) {
            if (visited.contains(city.name)) continue; // 若已访问则跳过

            // 计算从last到city的距离 + city回到起点的距离
            double cost = distance(last, city) + distance(city, path.first());

            if (cost < minCost) {
                minCost = cost;
                bestNext = city;
            }
        }

        // 筛选出的 “最优下一个城市” 加入路径
        path.append(bestNext);
        visited.insert(bestNext.name);
    }

    return path;
}

// 简化的 2-opt 优化
QList<City> CityManager::generateNeighbor(const QList<City>& solution) {
    QList<City> neighbor = solution;
    int n = neighbor.size();
    if (n <= 2) return neighbor;

    // 随机交换两个索引
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, n-1);
    int i = dis(gen);
    int j = dis(gen);
    while (i == j) j = dis(gen);
    std::swap(neighbor[i], neighbor[j]);

    return neighbor;
}

// 根据城市数量自适应调整参数
void CityManager::adjustParameters(int cityCount, double& initialTemp, double& coolingRate, int& iterationsPerTemp) {
    if (cityCount <= 10) {
        initialTemp = 1000.0;       // 小规模问题用较低温度
        coolingRate = 0.99;         // 降温快
        iterationsPerTemp = 200;    // 较少迭代
    } else if (cityCount <= 50) {
        initialTemp = 2000.0;       // 中等规模
        coolingRate = 0.996;        // 降温适中
        iterationsPerTemp = 70 * cityCount;
    } else {
        initialTemp = 10000.0;      // 大规模问题用较高温度
        coolingRate = 0.999;        // 降温慢
        iterationsPerTemp = 100 * cityCount; // 平衡效率
    }
}

// 接受程度计算
bool CityManager::acceptNewSolution(double energyDiff, double temperature) {
    // 温度接近于0,算法已收敛，拒绝所有新解
    if (temperature <= 1e-10) return false;

    // 新解路径长度比当前解小,直接接受
    if (energyDiff < 0) return true;

    // 新解更差,计算接受概率 (允许更多坏解,增加全局搜索能力)
    return std::exp(-energyDiff / temperature) > (static_cast<double>(rand()) / RAND_MAX);
}

// 路径总距离计算
double CityManager::calculateTotalDistance(const QList<City>& path) {
    if (path.size() < 2) return 0.0; // 城市数量过少时直接返回
    double total = 0.0;
    int n = path.size();
    for (int i = 0; i < n - 1; ++i) {
        total += distance(path[i], path[i + 1]);
    }
    // 计算终点到起点的距离，让路径闭合
    total += distance(path.last(), path.first());
    return total;
}

// 模拟退火算法
QList<City> CityManager::solveTSPWithSimulatedAnnealing(QList<AnnealingStep>* steps) {
    QList<City> allCities = getAllCities();
    int n = allCities.size();
    if (n <= 1) return QList<City>();

    // 清空历史步骤
    if (steps) steps->clear();

    // 自适应参数
    double initialTemp, coolingRate;
    int iterationsPerTemp;
    adjustParameters(n, initialTemp, coolingRate, iterationsPerTemp);

    // 定义 终止温度,越低精度越高
    double finalTemp = 1e-4;

    // 生成初始解
    QList<City> currentSolution = generateInitialSolution(allCities);
    double currentEnergy = calculateTotalDistance(currentSolution);

    // 记录最优解
    QList<City> bestSolution = currentSolution;
    double bestEnergy = currentEnergy;

    // 记录初始状态
    if (steps) {
        AnnealingStep step;
        step.iteration = 0;
        step.temperature = initialTemp;
        step.currentEnergy = currentEnergy;
        step.bestEnergy = bestEnergy;
        step.message = "初始化完成";
        steps->append(step);
    }


    int stagnationCount = 0; // 记录连续未改进解(停滞)的次数
    int maxStagnation = 100 * n; // 最大允许停滞次数
    double temperature = initialTemp;
    int iterationCount = 0; // 迭代次数

    // 模拟退火主循环
    while (temperature > finalTemp && stagnationCount < maxStagnation) {
        bool improved = false;
        int acceptedCount = 0; // 记录接受次数
        int rejectedCount = 0; // 记录拒绝次数

        // 开始同一温度下的迭代循环
        for (int i = 0; i < iterationsPerTemp; ++i) {
            QList<City> newSolution = generateNeighbor(currentSolution);
            double newEnergy = calculateTotalDistance(newSolution);
            double delta = newEnergy - currentEnergy;

            bool accepted = acceptNewSolution(delta, temperature);

            if (accepted) {
                currentSolution = newSolution;
                currentEnergy = newEnergy;
                acceptedCount++;

                if (currentEnergy < bestEnergy) {
                    bestSolution = currentSolution;
                    bestEnergy = currentEnergy;
                    improved = true;
                    stagnationCount = 0; // 停滞次数归零

                    // 记录找到新最优解
                    if (steps) {
                        AnnealingStep step;
                        step.iteration = ++iterationCount;
                        step.temperature = temperature;
                        step.currentEnergy = currentEnergy;
                        step.bestEnergy = bestEnergy;
                        step.message = QString("找到新最优解: %1").arg(bestEnergy);
                        steps->append(step);
                    }
                }
            } else {
                rejectedCount++;
            }
        }

        // 记录每个温度周期的统计信息
        if (steps) {
            AnnealingStep step;
            step.iteration = ++iterationCount;
            step.temperature = temperature;
            step.currentEnergy = currentEnergy;
            step.bestEnergy = bestEnergy;

            if (improved) {
                step.message = QString("温度 %1: 找到更优解，接受 %2 次，拒绝 %3 次")
                                   .arg(temperature, 8, 'f', 3) // 数值占用的最小字符数8,浮点数表示,保留到小数点后三位
                                   .arg(acceptedCount)
                                   .arg(rejectedCount);
            } else {
                step.message = QString("温度 %1: 未找到更优解，接受 %2 次，拒绝 %3 次，停滞计数 %4/%5")
                                   .arg(temperature, 8, 'f', 3)
                                   .arg(acceptedCount)
                                   .arg(rejectedCount)
                                   .arg(stagnationCount)
                                   .arg(maxStagnation);
            }

            steps->append(step);
        }

        if (!improved) {
            stagnationCount++;
        }

        temperature *= coolingRate; // 执行降温
    }

    // 记录最终结果
    if (steps) {
        AnnealingStep step;
        step.iteration = iterationCount + 1;
        step.temperature = temperature;
        step.currentEnergy = currentEnergy;
        step.bestEnergy = bestEnergy;
        step.message = QString("算法终止，最终最优解: %1").arg(bestEnergy);
        steps->append(step);
    }

    return bestSolution;
}

// 从文件中读取城市
bool CityManager::loadFromFile(const QString& filename) {
    QFile file(filename);
    // 以只读、文本模式打开文件
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    // 清空现有城市
    for (int i = 0; i < HASH_SIZE; ++i) {
        Node *current = list[i];
        while (current) {
            Node *next = current->next;
            delete current;
            current = next;
        }
        list[i] = nullptr;
    }
    size = 0;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(QRegularExpression("\\s+")); // 按空白字符分割成多个子字符串

        if (parts.size() >= 3) {
            City city;
            city.name = parts[0];
            city.x = parts[1].toDouble();
            city.y = parts[2].toDouble();

            addCity(city);
        }
    }

    file.close();
    return true;
}

// 保存城市列表到文件
bool CityManager::saveToFile(const QString& filename) const {
    QFile file(filename);
    // 以只写文本的方式打开文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    QList<City> allCities = getAllCities();
    Q_FOREACH(const auto& city, allCities) {
        out << city.name << " "
            << city.x << " " << city.y << "\n";
    }

    file.close();
    return true;
}
