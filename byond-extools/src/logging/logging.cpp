#include <list>

#include <fstream>

#include <sstream>

#include <iomanip>

#include <ctime>

#include <mutex>

#include <string>

#include <thread>

#include <condition_variable>

struct FilePayload
{
    std::string file_name;
    std::string message;
    bool format = true;
};
std::list<FilePayload> file_queue;
std::mutex log_mutex;
std::condition_variable log_cv;
bool done_writing_files = false;
void extools_log_writer()
{
    std::unique_lock lock(log_mutex);
    log_cv.wait(lock);
    lock.unlock();
    while(!done_writing_files || !file_queue.empty())
    {
        if(!done_writing_files && file_queue.empty())
        {
            lock.lock();
            log_cv.wait(lock);
            lock.unlock();
        }
        if(!file_queue.empty() && lock.try_lock())
        {
            auto next = file_queue.front();
            file_queue.pop_front();
            lock.unlock();
            std::ofstream file(next.file_name,std::ios_base::app);
            if(next.format)
            {
                std::stringstream stream(next.message);
                std::string line;
                while(std::getline(stream,line))
                {
                    time_t now = time(0);
                    tm* local = localtime(&now);
                    file << std::put_time(local,"%F %T ") << line << '\n';
                }
            }
            else
            {
                file << next.message;
            }
            file.close();
        }
    }
}

std::thread log_thread;

#include "../core/core.h"

trvh write_log(unsigned int args_len, Value* args, Value src)
{
    if(args_len < 2)
    {
        return Value::Null();
    }
    {
        std::scoped_lock lock(log_mutex);
        file_queue.push_back({args[0],args[1],args_len >= 2 ? args[2].valuef != 0 : true});
    }
    log_cv.notify_all();
    return Value::Null();
}

trvh finalize_logs(unsigned int args_len,Value* args,Value src)
{
    if(!done_writing_files)
    {
        done_writing_files = true;
        log_cv.notify_all();
        log_thread.join();
    }
    return Value::Null();
}

const char* enable_logging()
{
    Core::get_proc("/proc/extools_log_write").hook(write_log);
    Core::get_proc("/proc/extools_finalize_logging").hook(finalize_logs);
    log_thread = std::thread(extools_log_writer);
    return "ok";
}