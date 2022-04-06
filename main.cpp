#include <iostream>
#include <queue>
#include <list>
#include <cstdlib>
#include <cmath>
#include <unistd.h>

using namespace std;

unsigned int q_id = 0;                // 队列进程号的全局变量
unsigned int stime = 0;               // 进程开始执行的时间
const unsigned int Q_NUM = 3;         // 总就绪队列数
const unsigned int TOTAL_PRO_NUM = 5; // 总进程数
const unsigned int INIT_PRO_NUM = 3;  // 初始就绪进程数
unsigned int pro_num = 3;             // 当前进程个数

// 进程信息
struct Pro {
    unsigned int pid;        // 进程ID
    unsigned int start_time; // 开始执行时间
    unsigned int need_time;  // 预计执行时间
    unsigned int end_time;   // 完成时间
    unsigned int run_time;   // 已经运行时间
    unsigned int count;      // 计数器
};

// 多级就绪队列节点信息
struct Node {
    queue<Pro> q;          // 就绪队列
    unsigned int priority; // 队列优先级
    unsigned int cap;      // 时间片
};

// 进程调度类
class Scheduling {
private:
    queue<Pro> qp;    // 队列
    list<Pro> lp;     // 链表
    list<Node> ln;    // 链表队列
    unsigned int cap; // 时间片
public:
    Scheduling(/* args */);

    ~Scheduling();

    void create_q_pro(); // 创建进程queue的函数
    void create_l_pro(); // 创建进程list的函数
    void create_node();  // 创建node队列
    void fcfs();         // FCFS ： 先来先服务调度算法
    void rr();           // RR : 轮转调度算法
    void mfq();          // MFQ : 多级反馈队列调度算法
};

Scheduling::Scheduling(/* args */) {
    cap = 10; // 初始化时间片
}

Scheduling::~Scheduling()
= default;

void Scheduling::create_q_pro() {
    Pro item;
    item.pid = (++q_id);
    item.count = 0;
    item.need_time = rand() % 100 + 10; // 预计需求时间 在 10 ～ 100 之间
    item.run_time = 0;
    //
    qp.push(item);
    printf("创建进程 PID= %d:  执行所需时间= %d\n", item.pid, item.need_time);
}

void Scheduling::create_l_pro() {
    Pro item;
    item.pid = ++q_id;
    item.count = 0;
    item.need_time = rand() % 200 + 10; // 预计需求时间 在 10 ～ 100 之间
    item.run_time = 0;
    //
    lp.push_back(item);
    printf("创建进程 PID = %d:   执行所需时间 = %d\n", item.pid, item.need_time);
}

void Scheduling::create_node() {
    for (int j = 0; j < Q_NUM; j++) {
        Node node;
        node.priority = 1 + j;     // 初始队列最高优先级1
        node.cap = 20 * pow(2, j); // 初始化时间片 20
        int i;
        // 在最高优先队列中创建 3 个进程
        for (i = 0; j == 0 && i < INIT_PRO_NUM; i++) {
            Pro item;
            item.pid = (++q_id);
            item.count = 0;
            item.need_time = rand() % 100 + 10; // 预计需求时间 在 10 ～ 100 之间
            item.run_time = 0;
            //
            node.q.push(item);
            printf("创建进程 PID= %d:  执行所需时间 = %d\n", item.pid, item.need_time);
            printf("\n");
        }
        //
        ln.push_back(node);
    }
}

// 先来先服务调度算法
void Scheduling::fcfs() {
    int i, rd;
    printf("-------先来先服务调度算法-------\n");
    for (i = 0; i < 5; i++) {
        create_q_pro();
        printf("\n");
    }
    while (!qp.empty()) {
        Pro *p = &qp.front();
        p->start_time = stime;
        printf("创建进程 PID= %d:  执行所需时间 = %d\n", p->pid, p->need_time);
        sleep(p->end_time);
        p->end_time = p->start_time + stime;
        printf("结束时间%d\n", p->end_time);
        printf("\n");
        qp.pop();
        stime = p->end_time;
        //
        // rd = rand() % 10;
        // if (rd > 6)
        // {
        //     create_q_pro();
        //     printf("\n");
        // }
    }
}

// 轮转调度算法
void Scheduling::rr() {
    int i, rd;
    stime = 0;
    printf("-------时间片轮转法(时间片 = %d)-------\n", cap);
    for (i = 0; i < 5; i++) {
        create_q_pro();
        printf("\n");
    }
    while (!qp.empty()) {
        Pro *p = &qp.front();
        p->start_time = stime;
        printf("进程PID=%d: 执行还需时间%d 开始执行时间%d ", p->pid, p->need_time, p->start_time);
        if (p->need_time > cap) {
            sleep(cap);
            // sleep(3);
            p->need_time = p->need_time - cap;
            p->end_time = cap;
            p->run_time = p->run_time + cap;
            stime = stime + cap;
            ++(p->count);
            printf("第 %d 次执行，已执行时间 = %d\n", p->count, p->run_time);
            qp.push(qp.front());
            qp.pop();
        } else {
            sleep(p->end_time);
            // sleep(3);
            stime = stime + p->end_time;
            p->end_time = stime;
            p->run_time = p->run_time + p->need_time;
            ++(p->count);
            printf("第 %d 次执行，已执行时间 = %d 结束时间 = %d 执行完毕\n", p->count, p->run_time, p->end_time);
            p->end_time = 0;
            qp.pop();
        }
        printf("\n");
        // rd = rand() % 10;
        // if (rd > 6)
        // {
        //     create_q_pro();
        //     printf("\n");
        // }
    }
}

// 多级反馈队列调度算法
void Scheduling::mfq() {
    int rd;  // 随机数，随机产生新进程
    int flag = 0; // flag标志是否有新进程进入初级队列
    stime = 0; // 系统的时间
    printf("-------多级反馈队列调度(时间片 = %d)-------\n\n", cap);
    create_node();
    // 遍历每个就绪队列
    for (list<Node>::iterator iter = ln.begin(); iter != ln.end();) {
        printf("队列优先级 = %d 队列时间片 = %d\n", iter->priority, iter->cap);
        // 遍历每个就绪队列的每个进程
        while (!iter->q.empty()) {
            // iter 指向当前队列; iter1 指向下一队列
            list<Node>::iterator iter1 = iter;
            Pro *p = &iter->q.front();
            p->start_time = stime;
            printf("进程PID=%d: 执行还需时间%d 开始执行时间%d ", p->pid, p->need_time, p->start_time);
            // 进程无法在此队列一次完成
            if (p->need_time > iter->cap) {
                // sleep(cap);
                sleep(1);
                p->need_time = p->need_time - cap;
                p->end_time = cap;
                p->run_time = p->run_time + cap;
                stime = stime + cap;
                ++(p->count);
                printf("第 %d 次执行，已执行时间 = %d\n", p->count, p->run_time);
                if (++iter1 == ln.end()) // 如果当前进程正在最后一个队列
                {
                    iter->q.push(iter->q.front());
                    iter->q.pop();
                } else // 把当前进程放到下一队列队尾
                {
                    iter1->q.push(iter->q.front());
                }
                // 从当前队列移除
                iter->q.pop();
            } else // 进程可以在此队列完成
            {
                // sleep(p->need_time);
                sleep(1);
                stime = stime + p->need_time;
                p->end_time = stime;
                p->run_time = p->run_time + p->need_time;
                ++(p->count);
                printf("第 %d 次执行，已执行时间 = %d 结束时间 = %d 执行完毕\n", p->count, p->run_time, p->end_time);
                p->end_time = 0;
                iter->q.pop();
            }
            printf("\n");
            rd = rand() % 10;
            if (rd > 7 && pro_num < TOTAL_PRO_NUM) //有新进程进入高优先级队列
            {
                list<Node>::iterator iter2 = ln.begin();
                Pro item;
                item.pid = ++q_id;
                item.count = 0;
                item.need_time = rand() % 200 + 10; // 预计需求时间 在 10 ～ 100 之间
                item.run_time = 0;
                iter2->q.push(item);
                pro_num++;
                printf("创建进程 PID= %d:  执行所需时间 = %d\n", item.pid, item.need_time);
                printf("\n");
                if (iter2->priority < iter->priority) // 若当前队列优先级不是最高优先级
                {
                    flag = 1;
                    break;
                }
            }
        }
        if (flag == 1) {
            iter = ln.begin();
        } else {
            ++iter;
        }
        flag = 0;
    }
}

int main(int argc, char const *argv[]) {
    Scheduling sc;
    sc.mfq();
    return 0;
}