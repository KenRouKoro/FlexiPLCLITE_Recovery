/**
 * @file tick_scheduler.h
 * @brief 基于Tick的定时任务调度器，支持最多MAX_CALLBACKS个并发任务
 */
#ifndef TICK_SCHEDULER_H
#define TICK_SCHEDULER_H

#include <cstdint>
#include <utility>

#define MAX_CALLBACKS 10  // 最大支持的任务数量

using tick_callback_t = std::function<void(void)>;

typedef struct {
    tick_callback_t callback;  // 需执行的回调函数
    uint32_t delay_ticks;     // 需延迟的tick数
    uint32_t current_ticks;   // 当前累计的tick数
    int active;               // 活动状态(1:计时中, 0:已完成)
} tick_task_t;

// 静态任务数组，保存所有注册的任务
static tick_task_t tasks[MAX_CALLBACKS];
// 当前活跃任务数量
static int num_tasks = 0;

/**
 * 注册新的定时任务
 * @param callback 需注册的回调函数
 * @param delay_ticks 触发前需等待的tick数
 * @note 若任务队列已满则注册失败
 */
void register_callback(tick_callback_t callback, uint32_t delay_ticks) {
    if (num_tasks >= MAX_CALLBACKS) return;
    tasks[num_tasks].callback = std::move(callback);
    tasks[num_tasks].delay_ticks = delay_ticks;
    tasks[num_tasks].current_ticks = 0;
    tasks[num_tasks].active = 1;
    num_tasks++;
}

/**
 * 执行Tick计时并处理到期任务
 * 1. 递增所有活动任务的计时器
 * 2. 触发已到期的回调函数
 * 3. 移除已完成的任务
 */
void tick_increment() {
    int write_index = 0;  // 未完成任务的新起始位置

    for (int i = 0; i < num_tasks; i++) {
        auto& task = tasks[i];
        if (!task.active) continue;

        task.current_ticks++;
        if (task.current_ticks >= task.delay_ticks) {
            task.callback();   // 触发回调
            task.active = 0;  // 标记完成
        } else {
            // 将未完成任务移动到数组前部
            tasks[write_index++] = task;
        }
    }
    num_tasks = write_index;  // 更新活跃任务数量
}

#endif // TICK_SCHEDULER_H

