#include <iostream>
#include <random>
#include <vector>
#include <fstream>
#include <map>
#include <algorithm>

using namespace std;
int Total = 10000000;
int spliceNumber = 100;
random_device e;
uniform_real_distribution<double> u(0, 1);

//生成时间间隔
double exponential(double lambda) {
    return log(u(e)) / (-lambda);
}

struct Pack {
    int index;
    double at{}; //到达系统时间
    double st{}; //开始服务时间
    double lt{}; //离开时间(服务结束时间)
    explicit Pack(int index) {
        this->index = index;
    }
};

struct Packstream {
    int length;//该包流的服务总包数
    //包流的到达时间序列
    double deficit = 0;
    int count = 0;//存放自己这个流处理的包的数量
    vector<Pack> Packs;
    //用一个字典存放对应的队列长度和服务时间 来计算队列长度的概率
    map<int, double> queue_l;

    int index = 0; //记录包流服务的指针
    Packstream(double lamda, int length) {
        this->length = length;
        double atime = 0;
        for (int i = 0; i < length; i++) {
            Pack p(i);
            p.at = atime;
            Packs.push_back(p);
            atime += exponential(lamda);
        }
    }

    Pack &front() {
        return Packs[index];
    }

    bool empty() {
        return index == length;
    }

    //计算平均等待时间 (lt - at)
    double avg_wait(const string &textName) {
        ofstream ofile;
        ofile.open(textName);
        double avg = 0;
        vector<double> wt(count);//需要排序
        for (int i = 0; i < count; i++) {
            double waitTime = Packs[i].st - Packs[i].at;
            wt[i] = waitTime;
            avg += waitTime;
        }
        sort(wt.begin(), wt.end());
        double span = (wt[count - 1] - wt[0]) / spliceNumber;
        cout << "区间大小为" << span << endl;
        int j = 0;
        for (int i = 0; i < spliceNumber; i++) {
            int c = 0;
            for (; j < count && wt[j] < span * (i + 1); c++, j++);
            ofile << span * (i + 1) / 2 << "    " << c / (double) count << endl;
        }
        ofile.close();
        return avg / count;
    }

    //计算平均排队时间 (st - at)
    double avg_queue_wait() {
        double avg = 0;
//        for (int i = 0; i < length; i++) {
//            avg += Packs[i].st - Packs[i].at;
//        }
        for (int i = 0; i < count; i++) {
            avg += Packs[i].st - Packs[i].at;
        }
        return avg / count;
    }

    //计算平均队列长度 （st和at来计算）
    double avg_queue_length(const string &textName) {
//        vector<double> timePoints;
//        for (auto &Pack : Packs) {
//            timePoints.push_back(Pack.at);
//            timePoints.push_back(Pack.st);
//        }
//        sort(timePoints.begin(), timePoints.end());
//        vector<double> timeInterval(2 * Packs.size());
//        for (int i = 0; i < timeInterval.size() - 1; i++) {
//            timeInterval[i] = timePoints[i + 1] - timePoints[i];
//        }
//        timeInterval[timeInterval.size() - 1] = 0;
//        vector<double> numOfPacks(2 * Packs.size());
//        int temp = 0;//st的指针
//        numOfPacks[0] = 1; //开始有一个人
//        for (int i = 1; i < timePoints.size(); i++) {
//            if (timePoints[i] == Packs[temp].st) {//有包离开队列
//                numOfPacks[i] = numOfPacks[i - 1] - 1;
//                temp++;
//            } else numOfPacks[i] = numOfPacks[i - 1] + 1;
//        }
//        //更新map 并统计总的
//        double all = 0;
//        for (int i = 0; i < timeInterval.size(); i++) {
//            //更新或插入
//            if (queue_l.find(numOfPacks[i]) != queue_l.end()) {
//                queue_l[numOfPacks[i]] += timeInterval[i];
//            } else {
//                queue_l[numOfPacks[i]] = timeInterval[i];
//            }
//            all += numOfPacks[i] * timeInterval[i];
//        }
//        //计算平均队列长度
//        return all / timePoints[timePoints.size() - 1];
        vector<double> timePoints;
        for (int i = 0; i < count; i++) {
            timePoints.push_back(Packs[i].at);
            timePoints.push_back(Packs[i].st);
        }
        sort(timePoints.begin(), timePoints.end());
        vector<double> timeInterval(2 * count);
        for (int i = 0; i < timeInterval.size() - 1; i++) {
            timeInterval[i] = timePoints[i + 1] - timePoints[i];
        }
        timeInterval[timeInterval.size() - 1] = 0;
        vector<double> numOfPacks(2 * count);
        int temp = 0;//st的指针
        numOfPacks[0] = 1; //开始有一个人
        for (int i = 1; i < timePoints.size(); i++) {
            if (timePoints[i] == Packs[temp].st) {//有包离开队列
                numOfPacks[i] = numOfPacks[i - 1] - 1;
                temp++;
            } else numOfPacks[i] = numOfPacks[i - 1] + 1;
        }
        //更新map 并统计总的
        double all = 0;
        for (int i = 0; i < timeInterval.size(); i++) {
            //更新或插入
            if (queue_l.find(numOfPacks[i]) != queue_l.end()) {
                queue_l[numOfPacks[i]] += timeInterval[i];
            } else {
                queue_l[numOfPacks[i]] = timeInterval[i];
            }
            all += numOfPacks[i] * timeInterval[i];
        }
        //计算平均队列长度

        ofstream ofile;
        ofile.open(textName);
        auto iter = queue_l.begin();
        for (; iter != queue_l.end(); iter++) {
            ofile << iter->first << "   " << iter->second / timePoints[timePoints.size() - 1] << endl;
        }
        ofile.close();
        return all / timePoints[timePoints.size() - 1];
    }
};

struct Server {
    double u; //服务器服务
    double timePiece;

    Server(double u, double timePiece) {
        this->u = u;
        this->timePiece = timePiece;
    }

    //返回服务的包数
    void serveOneTimePiece1(Packstream &p, double &currentTime, double &sT, int &numOfPackages) {
        int count = 0;
        double serveTime = 0; //已经运行的时间
        double myServerTime = timePiece + p.deficit; //我这次拥有的运行总时间
        while (numOfPackages < Total && serveTime < myServerTime) {
            double time = exponential(u);
            //包到达处理,包不到达就直接离开了
            if (p.front().at <= currentTime) {
                //处理，更新相关的参数
                if (time + serveTime > myServerTime) { //已经处理不完了
                    p.deficit += myServerTime - serveTime;
                    break;
                }
                p.front().st = currentTime;//开始处理时间
                p.front().lt = currentTime + time;//离开时间
                serveTime += time;
                currentTime += time;
                sT += time;
                numOfPackages++;
                p.count++;
                p.index++;
            } else {
                p.deficit += myServerTime - serveTime;
                break;
            }
        }
    }
};

void run1(Packstream &p1, Packstream &p2, Packstream &p3, Server server);

int main() {
    Packstream p1(2, Total);
    Packstream p2(3, Total);
    Packstream p3(4, Total);
    Server server(10, 1);
    run1(p1, p2, p3, server);
    return 0;
}

double timeSkip(const vector<double> &a, double currentTime) {
}

//第一种方法赤字轮询调(时间片剩余需要进行存储)
void run1(Packstream &p1, Packstream &p2, Packstream &p3, Server server) {
    //系统运行指针
    int count = 0;
    //服务的总数量
    int numOfPackages = 0;
    //当前运行时间
    double currentTime = 0;
    //服务器服务时间
    double sT = 0;
    while (numOfPackages < Total) {
        if (count % 3 == 0) server.serveOneTimePiece1(p1, currentTime, sT, numOfPackages);
        else if (count % 3 == 1) server.serveOneTimePiece1(p2, currentTime, sT, numOfPackages);
        else server.serveOneTimePiece1(p3, currentTime, sT, numOfPackages);
        //这里更新currentTime防止死循环
        currentTime += 0.01;
        count++;
    }
    cout << "utilization:" << endl;
    cout << sT / currentTime << endl;
    cout << "平均等待时间:" << endl;
    cout << "p1: " << p1.avg_wait("d:\\mt\\p1_avg_wait.txt") << endl;
    cout << "p2: " << p2.avg_wait("d:\\mt\\p2_avg_wait.txt") << endl;
    cout << "p3: " << p3.avg_wait("d:\\mt\\p3_avg_wait.txt") << endl;
    cout << "平均队列长度:" << endl;
    cout << "p1: " << p1.avg_queue_length("d:\\mt\\p1_avg_queue_l.txt") << endl;
    cout << "p2: " << p2.avg_queue_length("d:\\mt\\p2_avg_queue_l.txt") << endl;
    cout << "p3: " << p3.avg_queue_length("d:\\mt\\p3_avg_queue_l.txt") << endl;
    cout << "分别服务的包数:" << endl;
    cout << "p1: " << p1.count << endl;
    cout << "p2: " << p2.count << endl;
    cout << "p3: " << p3.count << endl;

}