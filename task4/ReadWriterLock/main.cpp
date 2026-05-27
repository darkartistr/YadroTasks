/*ЗАДАЧА 2: Читатели-писатели с приоритетом для писателей

Условия:
1. Реализуйте класс ReadWriteLock.
2. Методы:
   - reader_lock() --- блокировка для чтения (допускает нескольких читателей).
   - reader_unlock() --- освобождение блокировки чтения.
   - writer_lock() --- исключительная блокировка для записи.
   - writer_unlock() --- освобождение блокировки записи.
3. Добавьте приоритет для писателей:
   - Если писатель ожидает, новые читатели не должны получить доступ.
   - Это предотвращает голодание (starvation) писателей.
4. Используйте std::mutex и std::condition_variable.
5. Создайте RAII-обёртки ReadGuard, WriteGuard.*/

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>


class ReadWriteLock {
private:
   std::mutex mutex;
   std::condition_variable cv_reader;
   std::condition_variable cv_writer;
   int readers = 0;
   bool writer = false;
   int waitWriters = 0;

public:

   ReadWriteLock() = default;
   ReadWriteLock(const ReadWriteLock&) = delete;
   ReadWriteLock& operator=(const ReadWriteLock&) = delete;

   void reader_lock() {
      std::unique_lock<std::mutex> lock(mutex);
      cv_reader.wait(lock, [this]() { return writer == false && waitWriters == 0;});
      readers++;
   }

   void reader_unlock() {
      std::unique_lock<std::mutex> lock(mutex);
      readers--;
      if (readers == 0) {
         cv_writer.notify_one();
      }
   }

   void writer_lock() {
      std::unique_lock<std::mutex> lock(mutex);
      waitWriters++;

      cv_writer.wait(lock, [this]() { return !writer && readers == 0; });
      --waitWriters;
      writer = true;
   }

   void writer_unlock() {
      std::unique_lock<std::mutex> lock(mutex);
      writer = false;
      if (waitWriters > 0) {
         cv_writer.notify_one();
      }
      else {
         cv_reader.notify_all();
      }
   }

};

class ReadGuard {
private:
   ReadWriteLock& lock_;

public:

   explicit ReadGuard(ReadWriteLock& lock) : lock_(lock) {
      lock_.reader_lock();
   }

   ReadGuard(const ReadGuard&) = delete;
   ReadGuard& operator=(const ReadGuard&) = delete;

   ~ReadGuard() {
      lock_.reader_unlock();
   }
};

class WriteGuard {
private:
   ReadWriteLock& lock_;

public:
   explicit WriteGuard(ReadWriteLock& lock) : lock_(lock) {
      lock_.writer_lock();
   }

   WriteGuard(const WriteGuard&) = delete;
   WriteGuard& operator=(const WriteGuard&) = delete;

   ~WriteGuard() {
      lock_.writer_unlock();
   }
};

std::mutex cout_mtx;
void print(const std::string& s) {
   std::lock_guard<std::mutex> lock(cout_mtx);
   std::cout << s << std::endl;
}

int main() {
   ReadWriteLock rw;
   int data = 0;
   std::vector<std::thread> threads;

   threads.emplace_back([&]() {
       ReadGuard g(rw);
       print("-> R1 внутри");
       std::this_thread::sleep_for(std::chrono::seconds(2));
       print("<- R1 вышел");
   });

   std::this_thread::sleep_for(std::chrono::milliseconds(100));

   threads.emplace_back([&]() {
       ReadGuard g(rw);
       print("-> R2 внутри (параллельно)");
       std::this_thread::sleep_for(std::chrono::seconds(1));
       print("<- R2 вышел");
   });

   std::this_thread::sleep_for(std::chrono::milliseconds(100));


   threads.emplace_back([&]() {
       print("!! W1 встал в очередь");
       {
           WriteGuard g(rw);
           print("==> W1 пишет");
           data = 42;
           std::this_thread::sleep_for(std::chrono::seconds(1));
       }
       print("<== W1 закончил");
   });

   std::this_thread::sleep_for(std::chrono::milliseconds(100));

   threads.emplace_back([&]() {
       print(".. R3 хочет войти");
       {
           ReadGuard g(rw);
           print("-> R3 внутри. Data = " + std::to_string(data));
       }
       print("<- R3 завершил");
   });

   for (auto& t : threads) t.join();
   return 0;
}