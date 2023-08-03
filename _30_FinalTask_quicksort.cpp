#include "Pool.h"

//using namespace std;

mutex coutLocker;
bool make_thread = true;

void arrayFormation(vector <int>&, int);
void ckeckArray(vector <int>&);
void quicksort(vector<int>&, int, int);


int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    int arr_size = 10000000;
    vector<int> array(arr_size);

    arrayFormation(array, arr_size);
    auto start = chrono::high_resolution_clock::now();
    RequestHandler rh;
    rh.push_task(quicksort, array, 0, arr_size - 1);
    auto end = chrono::high_resolution_clock::now();
    auto time_span = end - start;
    cout << "Сортировка заняла " << (double)time_span.count()/1000000000 << " секунд, а точнее: " << time_span.count() << " наносекунды.\n";
    ckeckArray(array);

    arrayFormation(array, arr_size);
    cout << "Монопоточная сортировка.\n";
    make_thread = false;
    start = chrono::high_resolution_clock::now();
    quicksort(array, 0, arr_size - 1);
    end = chrono::high_resolution_clock::now();
    time_span = end - start;
    cout << "Сортировка заняла " << (double)time_span.count() / 1000000000 << " секунд, а точнее: " << time_span.count() << " наносекунды.\n";
    ckeckArray(array);
    return 0;
}

void arrayFormation(vector <int>& array, int arr_size)
{
    for (int i = 0; i < arr_size; i++)
        array[i]=(rand() % 50000);
    cout << "Массив сформирован.\n";
}

void ckeckArray(vector<int>& array)
{
    bool res = true;
    for (int i = 0; i < array.size() - 1; i++)
    {
        if (array[i] > array[i + 1])
        {
            res = false;
            break;
        }
    }
    if (res) cout <<  "Отсортировано.\n\n"; else cout << "Не отсортировано.\n\n";
}

void quicksort(vector<int>& array, int left, int right)
{
    shared_mutex m;
    if (left >= right) return;
    int left_bound = left;
    int right_bound = right;

    m.lock_shared();
    long middle = array[(left_bound + right_bound) / 2];
    m.unlock_shared();

    do {
        m.lock_shared();
        while (array[left_bound] < middle) {
            left_bound++;
        }
        while (array[right_bound] > middle) {
            right_bound--;
        }
        m.unlock_shared();

        if (left_bound <= right_bound) {
            m.lock();
            std::swap(array[left_bound], array[right_bound]);
            m.unlock();
            left_bound++;
            right_bound--;
        }
    } while (left_bound <= right_bound);

    if (make_thread && (right_bound - left > 100000))
    {
        // если элементов в левой части больше чем 100000
        // вызываем для нее рекурсию асинхронно 
        auto f = async(launch::async, [&]() {
            quicksort(array, left, right_bound);
            });
        quicksort(array, left_bound, right);
    }
    else {
        quicksort(array, left, right_bound);
        quicksort(array, left_bound, right);
    }
}