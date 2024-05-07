//
// Created by koro on 24-5-5.
//

#ifndef TICK_SCHEDULER_H
#define TICK_SCHEDULER_H

#include <cstdint>
#include <utility>

using tick_callback_t = std::function<void(void)>;
#define MAX_CALLBACKS 10

typedef struct {
    tick_callback_t callback;
    uint32_t delay_ticks;
    uint32_t current_ticks;
    int active; // 标记任务是否仍在激活状态
} tick_task_t;

static tick_task_t tasks[MAX_CALLBACKS];
static int num_tasks = 0;

void register_callback(tick_callback_t callback, uint32_t delay_ticks) {
    if (num_tasks >= MAX_CALLBACKS) {
        return;
    }
    tasks[num_tasks].callback = std::move(callback);
    tasks[num_tasks].delay_ticks = delay_ticks;
    tasks[num_tasks].current_ticks = 0;
    tasks[num_tasks].active = 1;
    num_tasks++;
}

void tick_increment(void) {
    for (int i = 0; i < num_tasks; i++) {
        if (tasks[i].active) {
            tasks[i].current_ticks++;
            if (tasks[i].current_ticks >= tasks[i].delay_ticks) {
                tasks[i].callback();
                tasks[i].active = 0; // 标记为已完成
            }
        }
    }

    // 移除已完成的任务
    int write_index = 0;
    for (int i = 0; i < num_tasks; i++) {
        if (tasks[i].active) {
            tasks[write_index++] = tasks[i];
        }
    }
    num_tasks = write_index;
}

#endif //TICK_SCHEDULER_H



