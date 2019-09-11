# emhash
a very fast and efficient open address c++ flat hash map, you can bench it and compare the result with third party hash map.

some new feature:
1. the default load factor is 0.8, also be set to 1.0 by open compile marco set(average 5% performance loss)

2. head only with c++11/14/17 without any depency, interface is highly compatible with std::unordered_map

3. it's the fastest hash map for find performance(100% hit), and good inserting performacne if no rehash(call reserve before insert)

4. more memory efficient if the key.vlaue size is not aligned (size(key) % 8 != size(value) % 8) than other's implemention
for exmaple hash_map<long, int> can save 1/3 memoery the  hash_map<long, long>.

5. it's only one array allocted, a simple and smart collision algorithm.

6. it's can use a second hash if the input hash is bad with high collision.

7. lru is also used in main bucket if compile marco set

8. can dump hash collsion statics

9. no tombstones is used in my hash map. performance will not lost if high inserting and erasion.

10.more than 5 different flat hash map implemention to choose, each of them is some tiny different can be used in many case.


# bench

some simple benchmark (code in bench/martin_bench.cpp) compraed with std::unordered_map/std::unordered_set

1. add 1 - 12345678 one by one
emap unique time = 88 ms loadf = 0.736
emap insert time = 88 ms loadf = 0.736
umap insert time = 956 ms loadf = 0.942
vec reserve time = 144 ms
vec emplace time = 72 ms
vec resize time = 16 ms
vec push time = 72 ms
eset time = 68 ms
uset time = 952 ms


2. random_shuffle 1 - 12345678
emap insert time = 440 ms
emap unique time = 312 ms
umap insert time = 1628 ms loadf = 0.942
vec    time = 296 ms
eset unique time = 220 ms
uset insert time = 1728 ms


3. random data
emap insert time = 568 ms loadf = 0.734/size 12310422
umap insert time = 2912 ms loadf = 0.939
eset insert range time = 1412 ms loadf = 0.734/size 12310422
uset insert time = 2916 ms loadf = 0.939/size 12310422

# test case
static void basic_test(int n)
{
    printf("1. add 1 - %d one by one\n", n);
    {
        {
            auto ts = now2ms();
            emilib2::HashMap<int, int> emap(n);
            for (int i = 0; i < n; i++)
                emap.insert_unique(i, i);
            printf("emap unique time = %ld ms loadf = %.3lf\n", now2ms() - ts, emap.load_factor());
        }

        {
            auto ts = now2ms();
            emilib2::HashMap<int, int> emap(n);
            for (int i = 0; i < n; i++)
                emap.insert(i, i);
            printf("emap insert time = %ld ms loadf = %.3lf\n", now2ms() - ts, emap.load_factor());
        }

        {
            auto ts = now2ms();
            std::unordered_map<int, int> umap(n);
            for (int i = 0; i < n; i++)
                umap.emplace(i, i);
            printf("umap insert time = %ld ms loadf = %.3lf\n", now2ms() - ts, umap.load_factor());
        }

        if (0)
        {
            auto ts = now2ms();
            std::map<int, int> mmap;
            for (int i = 0; i < n; i++)
                mmap.emplace(i, i);
            printf("map time = %ld ms\n", now2ms() - ts);
        }

        {
            auto ts = now2ms();
            std::vector<int> v;
            v.reserve(n);
            for (int i = 0; i < n; i++)
                v.emplace_back(i);
            printf("vec reserve time = %ld ms\n", now2ms() - ts);
        }

        {
            auto ts = now2ms();
            std::vector<int> v;
            for (int i = 0; i < n; i++)
                v.emplace_back(i);
            printf("vec emplace time = %ld ms\n", now2ms() - ts);
        }
        {
            auto ts = now2ms();
            std::vector<int> v(n);
            for (int i = 0; i < n; i++)
                v[i] = i;
            printf("vec resize time = %ld ms\n", now2ms() - ts);
        }
        {
            auto ts = now2ms();
            std::vector<int> v;
            for (int i = 0; i < n; i++)
                v.push_back(i);
            printf("vec push time = %ld ms\n", now2ms() - ts);
        }
        {
            auto ts = now2ms();
            emilib9::HashSet<int> eset(n);
            for (int i = 0; i < n; i++)
                eset.insert_unique(i);
            printf("eset time = %ld ms\n", now2ms() - ts);
        }

        {
            auto ts = now2ms();
            std::unordered_set<int> uset(n);
            for (int i = 0; i < n; i++)
                uset.insert(i);
            printf("uset time = %ld ms\n", now2ms() - ts);
        }

        puts("\n");
    }

    printf("2. random_shuffle 1 - %d\n", n);
    {
        std::vector <int> data(n);
        for (int i = 0; i < n; i ++)
            data[i] = i;

        myrandom_shuffle(data.begin(), data.end());
        {
            auto ts = now2ms();
            emilib2::HashMap<int, int> emap(n);
            for (auto v: data)
                emap.insert(v, 0);
            printf("emap insert time = %ld ms\n", now2ms() - ts);
        }

        {
            auto ts = now2ms();
            emilib2::HashMap<int, int> emap(n);
            for (auto v: data)
                emap.insert_unique(v, 0);
            printf("emap unique time = %ld ms\n", now2ms() - ts);
        }

        {
            auto ts = now2ms();
            std::unordered_map<int, int> umap(n);
            for (auto v: data)
                umap.emplace(v, 0);
            printf("umap insert time = %ld ms loadf = %.3lf\n", now2ms() - ts, umap.load_factor());
        }

        {
            auto ts = now2ms();
            std::vector<int> v(n);
            for (auto d: data)
                v[d] = 0;
            printf("vec    time = %ld ms\n", now2ms() - ts);
        }

        {
            auto ts = now2ms();
            emilib9::HashSet<int> eset(n);
            for (auto v: data)
                eset.insert_unique(v);
            printf("eset unique time = %ld ms\n", now2ms() - ts);
        }

        {
            auto ts = now2ms();
            std::unordered_set<int> eset(n);
            for (auto v: data)
                eset.insert(v);
            printf("uset insert time = %ld ms\n", now2ms() - ts);
        }
        puts("\n");
    }

    puts("3. random data");
    {
        std::vector <int> data(n);
        for (int i = 0; i < n; i ++)
            data[i] = (uint32_t)(rand() * rand()) % n + rand();
        myrandom_shuffle(data.begin(), data.end());

        {
            auto ts = now2ms();
            emilib2::HashMap<int, int> emap(n);
            for (auto v: data)
                emap.insert(v, 0);
            printf("emap insert time = %ld ms loadf = %.3lf/size %zd\n", now2ms() - ts, emap.load_factor(), emap.size());
        }

        {
            auto ts = now2ms();
            std::unordered_map<int, int> umap(n);
            for (auto v: data)
                umap.emplace(v, 0);
            printf("umap insert time = %ld ms loadf = %.3lf\n", now2ms() - ts, umap.load_factor());
        }

        if (0)
        {
            auto ts = now2ms();
            std::vector<int> vec(n);
            for (auto d: data)
                vec[(unsigned int)d % n] = 0;
            printf("vec time = %ld ms\n", now2ms() - ts);
        }

        {
            auto ts = now2ms();
            emilib9::HashSet<int> eset(n);
            eset.insert(data.begin(), data.end());
            printf("eset insert range time = %ld ms loadf = %.3lf/size %zd\n", now2ms() - ts, eset.load_factor(), eset.size());
        }

        {
            auto ts = now2ms();
            std::unordered_set<int> uset(n);
            uset.insert(data.begin(), data.end());
            printf("uset insert time = %ld ms loadf = %.3lf/size %zd\n", now2ms() - ts, uset.load_factor(), uset.size());
        }

        puts("\n");
    }
}
