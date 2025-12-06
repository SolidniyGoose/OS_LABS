#ifndef MONTE_CARLO_H
#define MONTE_CARLO_H

#include <pthread.h>
#include <stdbool.h>

typedef struct {
    long long points_per_thread;
    double radius;
    long long points_inside;
    int thread_id;
    unsigned int seed;
} ThreadData;

extern pthread_mutex_t mutex;
extern long long total_points_inside;
extern long long total_points;
extern double calculated_area;
extern int active_threads;

void* monte_carlo_thread(void* arg);
void print_help(void);
int parse_arguments(int argc, char* argv[], 
                   double* radius, 
                   long long* total_points_count, 
                   int* max_threads, 
                   bool* verbose);
void calculate_and_print_results(double radius, 
                                long long total_points_count,
                                double elapsed_time);
void init_threads(int max_threads, 
                 long long points_per_thread,
                 long long remaining_points,
                 double radius,
                 unsigned int main_seed,
                 pthread_t* threads,
                 ThreadData* thread_data,
                 bool verbose);
void wait_for_threads(int max_threads, pthread_t* threads);

#endif