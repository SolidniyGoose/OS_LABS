#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include "../include/monte_carlo.h"

// Инициализация глобальных переменных
pthread_mutex_t mutex;
long long total_points_inside = 0;
long long total_points = 0;
double calculated_area = 0.0;
int active_threads = 0;

// Функция потока
void* monte_carlo_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    long long local_inside = 0;
    
    // Генерация случайных точек
    for (long long i = 0; i < data->points_per_thread; i++) {
        double x = ((double)rand_r(&data->seed) / RAND_MAX) * 2 * data->radius - data->radius;
        double y = ((double)rand_r(&data->seed) / RAND_MAX) * 2 * data->radius - data->radius;
        
        if (x*x + y*y <= data->radius * data->radius) {
            local_inside++;
        }
    }
    
    pthread_mutex_lock(&mutex);
    total_points_inside += local_inside;
    total_points += data->points_per_thread;
    active_threads--;
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

void print_help(void) {
    printf("Использование: ./monte_carlo -r RADIUS -p POINTS -t THREADS [опции]\n");
    printf("Опции:\n");
    printf("  -r RADIUS    Радиус окружности (по умолчанию: 1.0)\n");
    printf("  -p POINTS    Общее количество точек (по умолчанию: 1000000)\n");
    printf("  -t THREADS   Максимальное количество потоков (обязательно)\n");
    printf("  -v           Подробный вывод\n");
    printf("  -h           Эта справка\n");
    printf("Пример: ./monte_carlo -r 5.0 -p 10000000 -t 8\n");
}

int parse_arguments(int argc, char* argv[], 
                   double* radius, 
                   long long* total_points_count, 
                   int* max_threads, 
                   bool* verbose) {
    int opt;
    
    while ((opt = getopt(argc, argv, "r:p:t:vh")) != -1) {
        switch (opt) {
            case 'r':
                *radius = atof(optarg);
                if (*radius <= 0) {
                    fprintf(stderr, "Ошибка: радиус должен быть положительным числом\n");
                    return -1;
                }
                break;
            case 'p':
                *total_points_count = atoll(optarg);
                if (*total_points_count <= 0) {
                    fprintf(stderr, "Ошибка: количество точек должно быть положительным\n");
                    return -1;
                }
                break;
            case 't':
                *max_threads = atoi(optarg);
                if (*max_threads <= 0) {
                    fprintf(stderr, "Ошибка: количество потоков должно быть положительным\n");
                    return -1;
                }
                break;
            case 'v':
                *verbose = true;
                break;
            case 'h':
                print_help();
                return 1;
            default:
                fprintf(stderr, "Неизвестная опция\n");
                print_help();
                return -1;
        }
    }
    
    if (*max_threads == 0) {
        fprintf(stderr, "Ошибка: необходимо указать количество потоков (-t)\n");
        print_help();
        return -1;
    }
    
    return 0;
}

void init_threads(int max_threads, 
                 long long points_per_thread,
                 long long remaining_points,
                 double radius,
                 unsigned int main_seed,
                 pthread_t* threads,
                 ThreadData* thread_data,
                 bool verbose) {
    
    active_threads = max_threads;
    for (int i = 0; i < max_threads; i++) {
        thread_data[i].points_per_thread = points_per_thread;
        if (i == 0) {
            thread_data[i].points_per_thread += remaining_points;
        }
        thread_data[i].radius = radius;
        thread_data[i].thread_id = i;
        thread_data[i].seed = main_seed + i;
        
        if (pthread_create(&threads[i], NULL, monte_carlo_thread, &thread_data[i]) != 0) {
            fprintf(stderr, "Ошибка создания потока %d\n", i);
            exit(1);
        }
        
        if (verbose) {
            printf("Создан поток %d, точек: %lld\n", i, thread_data[i].points_per_thread);
        }
    }
}

void wait_for_threads(int max_threads, pthread_t* threads) {
    for (int i = 0; i < max_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

void calculate_and_print_results(double radius, 
                                long long total_points_count,
                                double elapsed_time) {
    double square_area = 4 * radius * radius;
    calculated_area = (double)total_points_inside / total_points * square_area;
    double theoretical_area = M_PI * radius * radius;
    double error = fabs(calculated_area - theoretical_area) / theoretical_area * 100;
    
    printf("\n=== Результаты ===\n");
    printf("Точек внутри окружности: %lld из %lld (%.2f%%)\n", 
           total_points_inside, total_points, 
           (double)total_points_inside / total_points * 100);
    printf("Вычисленная площадь: %.6f\n", calculated_area);
    printf("Теоретическая площадь (π * R²): %.6f\n", theoretical_area);
    printf("Погрешность: %.2f%%\n", error);
    printf("Время выполнения: %.3f секунд\n", elapsed_time);
    printf("Скорость обработки: %.0f точек/сек\n", total_points_count / elapsed_time);
    
    double estimated_pi = calculated_area / (radius * radius);
    printf("Оценка числа π: %.6f (ошибка: %.2f%%)\n", 
           estimated_pi, fabs(estimated_pi - M_PI) / M_PI * 100);
}

int main(int argc, char* argv[]) {
    double radius = 1.0;
    long long total_points_count = 1000000;
    int max_threads = 0;
    bool verbose = false;
    
    int parse_result = parse_arguments(argc, argv, &radius, &total_points_count, &max_threads, &verbose);
    if (parse_result == 1) return 0;
    if (parse_result == -1) return 1;
    
    int cpu_cores = get_nprocs();
    if (max_threads > cpu_cores * 2) {
        printf("Предупреждение: указано %d потоков, но система имеет только %d ядер(а)\n", 
               max_threads, cpu_cores);
    }
    
    printf("=== Вычисление площади окружности методом Монте-Карло ===\n");
    printf("Радиус: %.2f\n", radius);
    printf("Количество точек: %lld\n", total_points_count);
    printf("Максимальное количество потоков: %d\n", max_threads);
    printf("Количество ядер процессора: %d\n", cpu_cores);
    
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        fprintf(stderr, "Ошибка инициализации мьютекса\n");
        return 1;
    }
    
    long long points_per_thread = total_points_count / max_threads;
    long long remaining_points = total_points_count % max_threads;
    
    pthread_t threads[max_threads];
    ThreadData thread_data[max_threads];
    
    unsigned int main_seed = time(NULL);
    
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    init_threads(max_threads, points_per_thread, remaining_points, radius, 
                main_seed, threads, thread_data, verbose);
    
    wait_for_threads(max_threads, threads);
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + 
                         (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    
    calculate_and_print_results(radius, total_points_count, elapsed_time);
    
    pthread_mutex_destroy(&mutex);
    
    return 0;
}