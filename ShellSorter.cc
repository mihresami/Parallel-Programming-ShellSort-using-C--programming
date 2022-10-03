#include "Sorters.hh"
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <semaphore.h>

using namespace std;
/*
void ShellSorter::sort(uint64_t* array, int array_size)
{
    int gap = 1;
    while (gap < array_size / 3) {
        gap = 3 * gap + 1;
    }
    for (; gap > 0; gap /= 3) {
        for (int j = gap; j < array_size; j++) {
            for (int k = j; k >= gap && array[k] < array[k - gap]; k -= gap) {
                swap(array[k], array[k - gap]);
            }
        }
    }
}
*/
//Optimized ShellSOrter
void ShellSorter::sort(uint64_t* array,int array_size){
int i,j,increment;
uint64_t temp ;
for(increment = array_size / 2; increment > 0; increment /= 2){
	for(i = increment; i <array_size ; i++){
		temp = array[i];
		for(j = i; j >= increment; j-= increment){
			if (temp < array[j-increment])
				array[j] = array[j-increment];
			else
			break;
			}
			array[j] = temp;
		}
	}
}

struct Param1 {
    uint64_t *array;
    int array_size;
    int index;
    int task_sz;
    int gap;
};

void *sub_sort1(void* param) {
    int index = ((Param1*)param)->index;
    int gap = ((Param1*)param)->gap;
    int array_size = ((Param1*)param)->array_size;
    uint64_t* array = ((Param1*)param)->array;
    int task_sz = ((Param1*)param)->task_sz;
    for (int t = gap; t < array_size; t += gap) {
        int ub = index * task_sz + t + task_sz;
        for (int j = index * task_sz + t; j < ub && j < array_size; j++) {
            for (int k = j; k >= gap && array[k] < array[k - gap]; k -= gap) {
                swap(array[k], array[k - gap]);
            }
        }
    }
}

struct Param2 {
    uint64_t *array;
    int array_size;
    int gap;
    sem_t* sem;
};

void *sub_sort2(void * param) {
    sem_wait(((Param2*)param)->sem);
    int gap = ((Param2*)param)->gap;
    int array_size = ((Param2*)param)->array_size;
    uint64_t* array = ((Param2*)param)->array;
    for (int j = gap; j < array_size; j++) {
        for (int k = j; k >= gap && array[k] < array[k - gap]; k -= gap) {
            swap(array[k], array[k - gap]);
        }
    }
    sem_post(((Param2*)param)->sem);
}

void ParallelShellSorter::sort(uint64_t* array, int array_size) {
    int gap = 1;
    while (gap < array_size / 3) {
        gap = 3 * gap + 1;
    }
    if (array_size < 10000) {
        for (; gap > 0; gap /= 3) {
            for (int j = gap; j < array_size; j++) {
                for (int k = j; k >= gap && array[k] < array[k - gap]; k -= gap) {
                    swap(array[k], array[k - gap]);
                }
            }
        }
        return ;
    }
    long thread;
    Param1* params1 = (Param1*) malloc((m_nthreads + 1) * sizeof (Param1));
    Param2* params2 = (Param2*) malloc((m_nthreads + 1) * sizeof (Param2));
    for (int i = 0; i <= m_nthreads; i++) {
        params1[i].array_size = array_size;
        params1[i].array = array;
    }
    pthread_t* thread_handles = (pthread_t*) malloc((m_nthreads + 1) * sizeof (pthread_t));
    sem_t* sems = (sem_t*) malloc((m_nthreads + 2) * sizeof(sem_t));
    for (int i = 0; i <= m_nthreads + 1; i++) {
        sem_init(sems + i, 0, 1);
    }
    int act_nthreads = m_nthreads + 1;
    int task_sz;
    // printf("init task_sz: %d\n", array_size / act_nthreads);
    for (; gap > 0; gap /= 3) {
        // start = timer.get_time_ns();
        int mod;
        task_sz = array_size / act_nthreads;
        if (act_nthreads << 3 > gap) {
            mod = 2;
        } else {
            mod = 1;
            task_sz = gap / act_nthreads;
        }
        if (mod == 1) {
            // printf("proc 1  gap:%9d   sub_thr:%3d   ", gap, act_nthreads - 1);
            for (thread = 1; thread < act_nthreads; thread++) {
                params1[thread].index = thread;
                params1[thread].gap = gap;
                params1[thread].task_sz = task_sz;
            }
            for (thread = 1; thread < act_nthreads; thread++) {
                pthread_create(&thread_handles[thread], NULL, sub_sort1, &params1[thread]);
            }
            params1[0].index = 0;
            params1[0].gap = gap;
            params1[0].task_sz = task_sz;
            sub_sort1(&params1[0]);
            // hrtime_t sum_wait = timer.get_time_ns();
            for (thread = 0; thread < act_nthreads - 1; thread++) {
                pthread_join(thread_handles[thread], NULL);
            }
            // sum_wait = timer.get_time_ns() - sum_wait;
            // end = timer.get_time_ns();
            // printf("time:%11llu ns   wait_time:%10llu\n", end - start, sum_wait);
        } else {
            // start = timer.get_time_ns();
            // printf("proc 2  gap:%9d   sub_thr:%3d   ", gap, m_nthreads);
            task_sz = array_size / (m_nthreads + 1) + 1;
            uint64_t* arr = array + task_sz;
            int leftsz = array_size - task_sz;
            for (thread = 0; thread < m_nthreads; thread++, arr += task_sz, leftsz -= task_sz) {
                params2[thread].array_size = min(task_sz, leftsz);
                params2[thread].array = arr;
                params2[thread].gap = gap;
                params2[thread].sem = sems + thread + 1;
            }
            for (thread = 0; thread < m_nthreads; thread++) {
                pthread_create(&thread_handles[thread], NULL, sub_sort2, &params2[thread]);
            }
            int j;
            for (j = gap; j < task_sz; j++) {
                for (int k = j; k >= gap && array[k] < array[k - gap]; k -= gap) {
                    swap(array[k], array[k - gap]);
                }
            }
            // hrtime_t sum_wait = 0, ws, we;
            for (int i = 0; i < m_nthreads; i++) {
                // ws = timer.get_time_ns();
                sem_wait(sems + i + 1);
                // we = timer.get_time_ns();
                // printf("i = %2d, time = %10llu\n", i, we - ws);
                for (int _j = 0; _j < task_sz; _j++, j++) {
                    for (int k = j; k >= gap && array[k] < array[k - gap]; k-= gap) {
                        swap(array[k], array[k - gap]);
                    }
                }
                sem_post(sems + i + 1);
            }
            // end = timer.get_time_ns();
            // printf("time:%11llu ns   wait_time:%10llu\n", end - start, sum_wait);
        }
    }
    for (int i = 0; i <= m_nthreads; i++) {
        sem_destroy(sems + i);
    }
    free(sems);
    free(thread_handles);
    free(params1);
    free(params2);
}

