// task_manager.sl
struct Task {
    name: string,
    priority: int,
    completed: bool
}

fn main() {
    Var:type:priority:[1, 2];
    let mut tasks: [Task; 3] = [
        Task { name: "High priority task".to_string(), priority: 2, completed: false },
        Task { name: "Medium priority task".to_string(), priority: 1, completed: false },
        Task { name: "Low priority task".to_string(), priority: 0, completed: false }
    ];

    // タスクの処理
    for task in &mut tasks {
        if task.priority == 2 {
            log!("Processing high priority task: {}", task.name);
            task.completed = true;
        }
    }

    // 完了したタスクの表示
    for task in &tasks {
        if task.completed {
            log!("Completed: {}", task.name);
        }
    }
}