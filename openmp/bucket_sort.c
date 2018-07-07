#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <limits.h>
#include <stdbool.h>

#define DEFAULT_BUCKETS_NUM 1000


typedef struct{
    int n_elem;
    int *arr;
} Bucket;


void print_array(int *arr, int arr_len){
    printf("\n[");
    for(int i = 0; i < arr_len - 1; ++i)
        printf("%d, ", arr[i]);
    printf("%d]\n", arr[arr_len - 1]);
    return;
}


static inline void heapify(Bucket **heap, int *heap_size, int root){
    int size = *heap_size;
    if(heap[0]->n_elem < 0 && size > 1){
        size = --(*heap_size);
        heap[0] = heap[size];
    }

    while(root < size){
        int left = 2 * root + 1;
        int right = 2 * root + 2;
        int smallest = root;

        if(left < size && heap[left]->arr[heap[left]->n_elem] < heap[smallest]->arr[heap[smallest]->n_elem])
            smallest = left;
        if(right < size && heap[right]->arr[heap[right]->n_elem] < heap[smallest]->arr[heap[smallest]->n_elem])
            smallest = right;

        if(root != smallest){
            Bucket *tmp = heap[root];
            heap[root] = heap[smallest];
            heap[smallest] = tmp;
            root = smallest;
        }
        else
            break;
    }
    return;
}


static inline void build_heap(Bucket **heap, int heap_size){
    for(int i = (heap_size / 2) - 1; i >= 0; --i)
        heapify(heap, &heap_size, i);
    return;
}


static inline void merge_buckets(int *arr, Bucket **buckets, int *offsets, int threads_num, int buckets_num, int rank){
    Bucket heap_nodes[threads_num];
    Bucket *heap[threads_num];
    Bucket *bucket;

    #pragma omp for schedule(dynamic)
    for(int bucket_num = 0; bucket_num < buckets_num; ++bucket_num){
        int offset = offsets[bucket_num];
        int heap_size = 0;

        for(int i = 0; i < threads_num; ++i){
            bucket = &buckets[i][bucket_num];

            if(--bucket->n_elem >= 0){
                heap_nodes[heap_size] = *bucket;
                heap[heap_size++] = &heap_nodes[heap_size];
            }
        }

        if(heap_size > 0){
            build_heap(heap, heap_size);
            while(heap[0]->n_elem >= 0){
                arr[offset++] = heap[0]->arr[heap[0]->n_elem--];
                heapify(heap, &heap_size, 0);
            }
        }
    }
    return;
}


int cmp(const void *a, const void *b){
    return *(int*)b - *(int*)a;
}


static inline void sort_buckets(Bucket **buckets, int *offsets, int threads_num, int buckets_num){
    #pragma omp for reduction(+:offsets[:buckets_num]) schedule(dynamic)
    for(int i = 0; i < buckets_num * threads_num; ++i){
        int row = i / buckets_num;
        int col = i % buckets_num;
        Bucket *bucket = &buckets[row][col];
        int n_elems = bucket->n_elem;
        for(int j = col + 1; j < buckets_num; ++j)
            offsets[j] += n_elems;
        qsort(bucket->arr, n_elems, sizeof(int), cmp);
    }
    return;
}


static inline void fill_buckets(int *arr, int arr_len, Bucket *buckets, int buckets_num){
    Bucket *bucket;
    int bucket_range = (INT_MAX / buckets_num) + 1;

    #pragma omp for schedule(static)
    for(int i = 0; i < arr_len; ++i){
        bucket = &buckets[arr[i] / bucket_range];
        bucket->arr[bucket->n_elem++] = arr[i];
    }
    return;    
}


static inline Bucket *create_buckets(int bucket_size, int buckets_num){
    Bucket *buckets = malloc(sizeof(Bucket) * buckets_num);
    for(int i = 0; i < buckets_num; ++i)
        buckets[i] = (Bucket) {.n_elem = 0, .arr = malloc(sizeof(int) * bucket_size)};
    return buckets;
}


static inline void free_buckets(Bucket *buckets, int buckets_num){
    for(int i = 0; i < buckets_num; ++i)
        free(buckets[i].arr);
    free(buckets);
    return;
}


void bucket_sort(int *arr, int arr_len, int buckets_num){
    Bucket *all_buckets[omp_get_max_threads()];
    int offsets[buckets_num];
    for(int i = 0; i < buckets_num; ++i)
        offsets[i] = 0;

    #pragma omp parallel shared(all_buckets, arr, arr_len, buckets_num)
    {
        int rank = omp_get_thread_num();
        int threads_num = omp_get_num_threads();

        Bucket *thread_buckets = create_buckets(arr_len, buckets_num);
        all_buckets[rank] = thread_buckets;

        fill_buckets(arr, arr_len, thread_buckets, buckets_num);
        sort_buckets(all_buckets, offsets, threads_num, buckets_num);
        merge_buckets(arr, all_buckets, offsets, threads_num, buckets_num, rank);

        free_buckets(thread_buckets, buckets_num);
    }
    return;
}


static inline void fill_array(int *arr, int arr_len){
    #pragma omp parallel
    {
        int seed = time(NULL) + omp_get_thread_num();

        #pragma omp for schedule(static)
        for(int i = 0; i < arr_len; ++i)
            arr[i] = rand_r(&seed);
    }
    return;
}


int main(int argc, char *argv[]){
    if(argc < 2 || argc > 3){
        printf("Please insert only the size of array and optionally buckets number");
        return -1;
    }

    int arr_len = atoi(argv[1]);
    int buckets_num = argc == 3 ? atoi(argv[2]) : DEFAULT_BUCKETS_NUM;
    int *arr = malloc(sizeof(int) * arr_len);

    fill_array(arr, arr_len);
//     print_array(arr, arr_len);
    bucket_sort(arr, arr_len, buckets_num);
//     print_array(arr, arr_len);

//    for(int i = 0; i < arr_len - 1; ++i)
//        if(arr[i] > arr[i + 1]){
//            printf("\nFAILED!!! %d %d\n", arr[i], arr[i+1]);
//            free(arr);
//            return -1;
//        }

    free(arr);
    return 0;
}
