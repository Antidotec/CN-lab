#include <iostream>
#include<vector>
#include<random>

using namespace std;
random_device r;
double time_slot = 0.5; //时间槽
int numOfStation = 5;
int max_i = 15; //最大回退次数
double run_time = 1000;
double lambda = 0.5;//包传输所占用时间槽个数 的泊松过程的参数

//生成时间间隔
double exponential() {
    uniform_real_distribution<double> u(0, 1);
    return log(u(r)) / (-lambda);
}

struct Station {
    int success = 0;
    int attempt = 0;
    int back_time_slot = 0;
    int i = 0;

    Station() = default;

    void back() {
        uniform_int_distribution<int> b(0, (1 << (i + 2)) - 1);
        attempt += 1;
        back_time_slot = b(r);
        if (i < max_i) i++;
    }
};

struct Channel {
    vector<Station> stations; //信道服务的基站集合
    int numOfStation;
    double current_time = 0;

    explicit Channel(int num) {
        numOfStation = num;
        for (int i = 0; i < numOfStation; i++)
            stations.emplace_back();
    }

    vector<int> FrontPack() {
        vector<int> packs;
        for (int i = 0; i < numOfStation; i++)
            if (stations[i].back_time_slot == 0) packs.push_back(i);
        return packs;
    }

    void run_CMSACA(double time);

    void printResult() {
        int success = 0;
        int attempt = 0;
        for (int i = 0; i < numOfStation; i++) {
            cout << "第" << i << "个站的碰撞概率为" << 1 - stations[i].success / (double) stations[i].attempt << endl;
            success += stations[i].success;
            attempt += stations[i].attempt;
        }
        cout << "系统碰撞概率为" << 1 - (double)success / attempt << endl;
        cout << "系统的在" << run_time << "运行时间里的吞吐量为" << success << endl;
        cout << "系统的性能(吞吐量/运行时间)为" << success / current_time << endl;
    }
};

void Channel::run_CMSACA(double time) {
    while (current_time < time) {
        vector<int> packs = FrontPack();
        if (packs.empty()) {//介质空闲且没有包成功到达
            current_time += time_slot;
            for (int i = 0; i < numOfStation; i++) stations[i].back_time_slot--;
        } else if (packs.size() > 1) {//发生碰撞了 回退,统计
            for (int pack : packs) {
                stations[pack].back();
                stations[pack].attempt++;
            }
            //碰撞以后不用加时间吧
        } else {//可以传包
            int transition_slot = ceil(exponential()); // 向上取整
            current_time += transition_slot * time_slot;
            stations[packs[0]].success++;
            stations[packs[0]].attempt++;
            stations[packs[0]].i = 0;
            stations[packs[0]].back();
        }
    }
    printResult();
}


int main() {
    Channel channel(numOfStation);
    channel.run_CMSACA(run_time);
    return 0;
}
