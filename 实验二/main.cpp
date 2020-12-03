#include <iostream>
#include<vector>
#include<random>
#include <ctime>

using namespace std;
default_random_engine r(time(nullptr));
double time_slot = 0.5; //时间槽
int numOfStation = 5;
int max_binary_i = 15; //最大回退次数
int random_i = 6;// 随机回退的最大时间槽数
int default_i = 5; //初始回退的指数
double run_time = 10000000;
double lambda = 1;// 平均一个时间槽内 传输的包的数量

//用泊松过程模拟包传递的间隔时间
double exponential() {
    uniform_real_distribution<double> u(0, 1);
    return log(u(r)) / (-lambda);
}

struct Station {
    int success = 0;
    int attempt = 0;
    int back_time_slot = 0;
    int binary_i{};

    //在一开始设定一个back—_time_slot；可以避免起初全部碰撞
    Station() = default;;

    void reset_binary_back() {
        binary_i = default_i;
        uniform_int_distribution<int> b(0, (1 << binary_i) - 1);
        back_time_slot = b(r);
    }

    void binary_back() {
        uniform_int_distribution<int> b(0, (1 << binary_i) - 1);
        back_time_slot = b(r);
        if (binary_i < max_binary_i) binary_i++;
    }

    void random_back() {
        uniform_int_distribution<int> b(0, random_i);
        back_time_slot = b(r);
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

    void run_P_persistence(double time, double p);

    void printResult() {
        int success = 0;
        int attempt = 0;
        for (int i = 0; i < numOfStation; i++) {
            cout << "第" << i << "个站的碰撞概率为" << 1 - stations[i].success / (double) stations[i].attempt << endl;
//            cout << "第" << i << "个站的性能为" << stations[i].success / current_time << endl;
            success += stations[i].success;
            attempt += stations[i].attempt;
        }
        cout << "系统碰撞概率为" << 1 - (double) success / attempt << endl;
        cout << "系统的在" << run_time << "运行时间里的吞吐量为" << success << endl;
        cout << "系统的性能(吞吐量/运行时间)为" << success / current_time << endl<<endl;
    }

    //重置信道服务的基站
    void reset() {
        current_time = 0;
        stations.clear();
        for (int i = 0; i < numOfStation; i++)
            stations.emplace_back();
    }
};

void Channel::run_CMSACA(double time) {
    //一开始都想发送都回退
    for (Station &s :stations)
        s.reset_binary_back();

    while (current_time < time) {
        vector<int> packs = FrontPack();
        if (packs.empty()) {//介质空闲且没有包成功到达
            current_time += time_slot;
            for (int i = 0; i < numOfStation; i++) stations[i].back_time_slot--;
        } else if (packs.size() > 1) {//发生碰撞了 回退,统计
            for (int pack : packs) {
                stations[pack].attempt++;
                stations[pack].binary_back();
                //在这里忽略了碰撞以后确认的时间！！！！
            }
        } else {//可以传包
            int transition_slot = ceil(exponential()); // 向上取整
            current_time += transition_slot * time_slot;
            // 包传输的时候不减等待时间！
//            for (int i = 0; i < numOfStation; i++) {
//                stations[i].back_time_slot = stations[i].back_time_slot < transition_slot ? 0 :
//                                             stations[i].back_time_slot - transition_slot;
//            }
            Station &s = stations[packs[0]];
            s.attempt++;
            s.success++;
            s.reset_binary_back();//饱和状态包瞬间到达要发送，继续回退
        }
    }
    //打印结果
    cout<<"CMSACA的效率:"<<endl<<endl;
    printResult();
}

void Channel::run_P_persistence(double time, double p) {
    //初始信道空闲根据p来初始化back_slot
    uniform_real_distribution<double> b(0, 1);
    for (Station &s: stations) {
        if ((b(r)) > p)//不发送推迟一个time_slot
            s.back_time_slot = 1;
    }
    while (current_time < time) {
        vector<int> packs = FrontPack();
        if (packs.empty()) {//介质空闲且没有包成功到达
            current_time += time_slot;
            for (int i = 0; i < numOfStation; i++) stations[i].back_time_slot--;
        } else if (packs.size() > 1) {//发生碰撞了 回退,统计
            for (int pack : packs) {
                stations[pack].attempt++;
                stations[pack].random_back();
            }
        } else {//可以传包
            Station &s = stations[packs[0]];
            if (b(r) > p){
                s.back_time_slot = 1;//推迟一个时间槽
                continue;
            }
            int transition_slot = ceil(exponential()); // 向上取整
            current_time += transition_slot * time_slot;
            // 包传输的时候减等待时间！
            for (int i = 0; i < numOfStation; i++) {
                stations[i].back_time_slot = stations[i].back_time_slot < transition_slot ? 0 :
                                             stations[i].back_time_slot - transition_slot;
            }
            s.attempt++;
            s.success++;
            s.back_time_slot = 0;
        }
    }
    cout<<"p="<<p<<"的p坚持效率如下:"<<endl<<endl;
    printResult();
}

int main() {
    Channel channel(numOfStation);
//    channel.run_CMSACA(run_time);
//    channel.reset();
    channel.run_P_persistence(run_time,0.6);

    return 0;
}
