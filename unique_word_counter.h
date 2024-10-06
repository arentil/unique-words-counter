#include <iostream>
#include <cstdint>
#include <thread>
#include <condition_variable>
#include <cstring>
#include <vector>
#include <unordered_set>
#include <sstream>
#include <atomic>

#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

class unique_word_counter
{
public:
    [[nodiscard]] static uint64_t count(char filename[])
    {
        const auto is_file_exists = [](char filename_str[]) -> bool
        {
            struct stat buffer;
            return stat(filename_str, &buffer) == 0;
        };

        if (!is_file_exists(filename))
        {
            throw std::invalid_argument("Invalid or not existing filename!");
        }

        const int file_descriptor = open(filename, O_RDONLY);           // get file descriptor
        const long int file_size = lseek(file_descriptor, 0, SEEK_END); // get file size
        const long int FILE_PAGE_BYTES = sysconf(_SC_PAGE_SIZE);        // get system file page in bytes
        const long int full_pages_count = file_size / FILE_PAGE_BYTES;  // calculate how many full pages does file contains
        const size_t total_threads_to_be_spawned = file_size % FILE_PAGE_BYTES == 0 ? full_pages_count : full_pages_count + 1;
        const uint hardware_concurrency = std::thread::hardware_concurrency(); // get system available threats

        struct thread_data
        {
            std::thread thread;
            std::string front_data; // this will be added to the beginning of next buffer
            char *buffer;
            long long buffer_end_index;
        };
        std::vector<thread_data> threads;

        std::string front_data_for_next_thread = "";
        for (size_t i = 0; i < total_threads_to_be_spawned; i++)
        {
            // Loop delegates mapped buffers for threads
            // mmap might split the buffer at position that divides word, to avoid loosing potential data,
            // the buffer will be read until last space character.
            // Then the remaining buffer will be merged with next buffer span and calculated in next thread.

            char *buffer = reinterpret_cast<char *>(
                mmap(NULL, FILE_PAGE_BYTES, PROT_READ, MAP_PRIVATE, file_descriptor, FILE_PAGE_BYTES * i));

            if (buffer == MAP_FAILED)
                throw std::runtime_error("Memory mapping failed! Cannot continue...");

            const std::string temp_front_data_for_next_thread = front_data_for_next_thread;

            long long buffer_end_index = FILE_PAGE_BYTES - 1;
            if (i + 1 < total_threads_to_be_spawned) // until its not last loop run
            {
                front_data_for_next_thread.clear();
                for (; buffer_end_index >= 0; buffer_end_index--)
                {
                    // read characters from the end to beginning until space character occured
                    if (buffer[buffer_end_index] == ' ')
                        break;
                    front_data_for_next_thread.insert(0, 1, buffer[buffer_end_index]);
                }
            }
            else
            {
                buffer_end_index = strlen(buffer) - 1;
            }

            thread_data new_thread_data;
            new_thread_data.front_data = temp_front_data_for_next_thread;
            new_thread_data.buffer = buffer;
            new_thread_data.buffer_end_index = buffer_end_index;
            threads.emplace_back(std::move(new_thread_data));
            // do not initialize std::thread yet
        }

        std::atomic_uint available_threads(hardware_concurrency);
        std::unordered_set<std::string> unique_word_map;
        std::mutex mutex;

        for (size_t i = 0; i < total_threads_to_be_spawned; i++)
        {
            // Spawn threads.
            // Maximum number of spawned threads at once is defined by `std::thread::hardware_concurrency();`
            // If there are already 16 working threads, program will wait with spawning more threads

            available_threads.wait(available_threads == 0); // wait until available threads will be other than 0
            available_threads--;                            // if unblocked "reserve" available thread to spawn

            threads.at(i).thread = std::thread([&thread_data = threads.at(i), &available_threads, &mutex, &unique_word_map]()
                                               {
                                                   const std::string string_to_count_words = thread_data.front_data + std::string(thread_data.buffer, thread_data.buffer_end_index + 1);

                                                //    std::unordered_map<std::string, unsigned long long> word_counter;
                                                   std::unordered_set<std::string> local_unique_word_map;
                                                   std::istringstream f(string_to_count_words);
                                                   std::string s;
                                                   while (std::getline(f, s)) // break by line
                                                   {
                                                      std::stringstream ss(s);
                                                      while (std::getline(ss, s, ' ')) // break by space char
                                                        if (s != "")
                                                            local_unique_word_map.emplace(s);
                                                   }

                                                   std::unique_lock ul(mutex);
                                                   unique_word_map.insert(local_unique_word_map.begin(), local_unique_word_map.end());
                                                   ul.unlock();
                                                   available_threads++;
                                                   available_threads.notify_one(); });
        }

        // Wait for threads to be joined.
        // Unmap memory(should be done without when program execution ends)
        for (size_t i = 0; i < total_threads_to_be_spawned; i++)
        {
            threads.at(i).thread.join();
            if (munmap(threads.at(i).buffer, FILE_PAGE_BYTES) != 0)
                throw std::runtime_error("Memory unmapping failed! Cannot continue...");
        }

        close(file_descriptor);

        return unique_word_map.size();
    }
};