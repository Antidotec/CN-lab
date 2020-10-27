/**************************
 *  M/M/1 核心类 Controller
 **************************/

import java.util.*;
import java.util.Random;

public class Controller {

    public static final int BIRTH = 0;
    public static final int DEATH = 1;

    LinkedList<Event> schedule;     // 事件队列
    double clock;                    // 时钟
    double endTime;                    // 时钟终止时间
    double nextArrival;                // 服务开始时间
    double nextDeparture;            // 服务结束时间
    double ta;                        // 平均到达率
    double ts;                        // 平均服务率
    ArrayList<Double> checkpoints;    // 监控测点
    int numChecks = 0;                    // 监控测点数

    double Tq, Tw, Ts;
    int w, q;
    int requests, serviced;

    /* 初始化 */
    public Controller(double ta, double ts, double endTime) {
        this.ta = ta;
        this.ts = ts;
        this.endTime = endTime;

        checkpoints = new ArrayList<Double>();
        double n = 0;
        while (n < endTime) {
            numChecks++;
            n += exponential(ta);
            checkpoints.add(n); //时间检查表
        }

        clock = 0;
        schedule = new LinkedList<Event>();
        nextArrival = exponential(ta);
        nextDeparture = Double.POSITIVE_INFINITY;

        Tq = 0;
        Tw = 0;
        Ts = 0;
        w = 0;
        q = 0;
        requests = 0;
        serviced = 0;
    }

    /* 下一个顾客到达 */
    public void birthHandler(double time) {
        if (schedule.isEmpty()) { // 队列为空，执行服务
            scheduleDeath(time);
        } else { // 队列不为空，添加此顾客到队列中
            schedule.add(new Event(time, BIRTH));
        }
        nextArrival += exponential(ta);
    }


    /* 当前服务结束 */
    public void deathHandler() {
        schedule.remove(); // 事件移除
        if (!schedule.isEmpty()) { // 队列不为空：弹出执行队列中的任务
            Event next = schedule.remove();
            scheduleDeath(next.getTime());
        } else {
            // 队列为空，此时服务器空闲，等待下一任务
            nextDeparture = Double.POSITIVE_INFINITY;
            nextArrival += exponential(ta);
        }
    }

    /* 服务执行，计算相关参数 */
    public void scheduleDeath(double arrivalTime) {
        nextDeparture = clock + exponential(ts);  // 本次服务结束时间
        schedule.addFirst(new Event(nextDeparture, DEATH));
        serviced++;  // 总服务数
        Tq += (nextDeparture - arrivalTime); // 总逗留时间
        Tw += (clock - arrivalTime);  // 总等待时间？？
        Ts += (nextDeparture - clock);  // 总服务时间
    }

    /* 监控输出 */
    public void monitorHandler() {
        int cur_q = schedule.size();
        int cur_w = (cur_q > 0) ? (schedule.size() - 1) : 0;
        w += cur_w;
        q += cur_q;
        checkpoints.remove(0);
    }

    /* 指数分布生成函数 */
    public static double exponential(double lambda) {
        Random r = new Random();
        double x = Math.log(1 - r.nextDouble()) / (-lambda);
        return x;
    }

    /* 主函数 */
    public void run() {
        clock = exponential(ta);
               while (clock < endTime) {

            if (checkpoints.get(0) < nextArrival && checkpoints.get(0) < nextDeparture) {
                // 监控时刻位于服务发生时
                clock = checkpoints.get(0);
                monitorHandler();
            } else if (nextArrival <= nextDeparture) { // 下一个顾客的到来
                clock = nextArrival;
                birthHandler(nextArrival);
                requests++;
            } else {    // 服务结束
                clock = nextDeparture;
                deathHandler();
            }
        }
        printStats();
    }

    public void printStats() {
        // 输出相关参数
        System.out.println("相关计算参数输出：");
        System.out.println("顾客数目requests: " + requests);
        System.out.println("服务顾客数serviced: " + serviced);
        System.out.println("队列中平均等待客户数q = " + q / numChecks);
        System.out.println("平均等待时间Tw = " + Tw / requests);
        System.out.println("平均逗留时间Tq = " + Tq / serviced);
        System.out.println("平均服务时间Ts = " + Ts / serviced);
        System.out.println("服务器利用率 = " + Ts / endTime);
    }
}