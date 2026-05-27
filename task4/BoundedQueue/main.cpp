/*ЗАДАЧА 1: Потокобезопасная очередь ограниченного размера

Условия:
1. Реализуйте класс BoundedQueue<T> с фиксированной максимальной ёмкостью.
2. Методы:
   - push(value) --- добавляет элемент, блокируется, если очередь полна.
   - pop() --- извлекает элемент, блокируется, если очередь пуста.
   - try_push(value) --- неблокирующая версия.
   - try_pop() --- неблокирующая версия.
3. Используйте std::mutex и std::condition_variable.
4. Реализуйте корректную обработку исключений.*/

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <optional>

template <typename T>
class BoundedQueue {
private:
    std::mutex mutex;
    std::condition_variable cv;
    std::queue<T> elements;
    size_t capacity;

public:
    BoundedQueue(size_t capacity) : capacity(capacity) {}

    void push(T value) {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this]() { return elements.size() < capacity; });
        elements.push(std::move(value));
        cv.notify_one();
    }

    bool try_push(T value) {
        std::unique_lock<std::mutex> lock(mutex);
        if (elements.size() < capacity) {
            elements.push(std::move(value));
            cv.notify_one();
            return true;
        }
        return false;
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this]() { return !elements.empty(); });
        T value = std::move(elements.front());
        elements.pop();
        cv.notify_one();
        return value;
    }

    std::optional<T> try_pop() {
        std::unique_lock<std::mutex> lock(mutex);
        if (!elements.empty()) {
            T value = std::move(elements.front());
            elements.pop();
            cv.notify_one();
            return value;
        }
        return std::nullopt;
    }

};

int main() {
    BoundedQueue<int> queue(3);

    std::thread producer([&queue]() {
        for (int i = 1; i <= 5; ++i) {
            queue.push(i);
            std::cout << "-> push: " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    if (!queue.try_push(999)) {
        std::cout << "try_push: очередь полна (false)" << std::endl;
    }

    std::thread consumer([&queue]() {
        for (int i = 1; i <= 5; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            int val = queue.pop();
            std::cout << "<- pop: " << val << std::endl;
        }
    });

    producer.join();
    consumer.join();

    auto popped = queue.try_pop();
    if (!popped.has_value()) {
        std::cout << "try_pop: очередь пуста" << std::endl;
    }

    return 0;
}
