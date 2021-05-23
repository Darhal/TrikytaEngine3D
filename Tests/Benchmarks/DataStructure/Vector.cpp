#include <random>
#include <benchmark/benchmark.h>
#include <Core/DataStructure/Vector/Vector.hpp>

void VectorEmplaceBack(benchmark::State& state)
{
    TRE::Vector<int> vec;
    // vec.Reserve(vec.DEFAULT_CAPACITY);

    for (auto _ : state) {
        // This code gets timed
        vec.EmplaceBack(5);
    }
}

void StdVectorEmplaceBack(benchmark::State& state)
{
    state.iterations();
    std::vector<int> vec;

    for (auto _ : state) {
        // This code gets timed
        vec.emplace_back(5);
    }
}

void VectorEmplaceFront(benchmark::State& state)
{
    TRE::Vector<int> vec;
    // vec.Reserve(vec.DEFAULT_CAPACITY);

    for (auto _ : state) {
        // This code gets timed
        vec.EmplaceFront(5);
    }
}

void StdVectorEmplaceFront(benchmark::State& state)
{
    state.iterations();
    std::vector<int> vec;

    for (auto _ : state) {
        // This code gets timed
        vec.insert(vec.begin(), 5);
    }
}

std::vector<usize> insertPos;
std::vector<usize> erasePos;
std::vector<std::pair<usize, usize>> ereaseRange;

void VectorInsert(benchmark::State& state)
{
    constexpr auto NB = 100'000'000;
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, NB); // define the range
    TRE::Vector<int> vec;
    // vec.Reserve(vec.DEFAULT_CAPACITY);
    int pos = 0;

    for (auto _ : state) {
        vec.Insert(pos, pos);

        state.PauseTiming();
        pos = distr(gen) % vec.Size();
        insertPos.push_back(pos);
        state.ResumeTiming();
    }
}

void StdVectorInsert(benchmark::State& state)
{
    constexpr auto NB = 100'000'000;
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, NB); // define the range
    std::vector<int> vec;
    usize pos = 0;
    usize index = 0;

    for (auto _ : state) {
        vec.insert(vec.begin() + pos, (int)pos);

        state.PauseTiming();
        if (index < insertPos.size()) {
            pos = insertPos[index++];
        } else {
            pos = distr(gen) % vec.size();
        }
        
        state.ResumeTiming();
    }
}

void VectorErease(benchmark::State& state)
{
    constexpr auto NB = 100;
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, NB); // define the range
    TRE::Vector<int> vec;
    vec.Reserve(vec.DEFAULT_CAPACITY);
    usize pos1 = 0;

    for (auto _ : state) {
        state.PauseTiming();

        for (auto i = 0; i < NB; i++) {
            vec.EmplaceBack(i);
        }

        pos1 = distr(gen) % vec.Size();
        erasePos.emplace_back(pos1);

        state.ResumeTiming();
        vec.Erease(pos1);
    }
}

void StdVectorErease(benchmark::State& state)
{
    constexpr auto NB = 100;
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, NB); // define the range
    std::vector<int> vec;
    usize pos1 = 0;
    usize index = 0;

    for (auto _ : state) {
        state.PauseTiming();

        for (auto i = 0; i < NB; i++) {
            vec.emplace_back(i);
        }

        if (index < erasePos.size()) {
            pos1 = erasePos[index++];
        } else {
            pos1 = distr(gen) % vec.size();
        }
        
        state.ResumeTiming();
        vec.erase(vec.begin() + pos1);
    }
}

void VectorEreaseRange(benchmark::State& state)
{
    constexpr auto NB = 100;
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, NB); // define the range
    TRE::Vector<int> vec;
    vec.Reserve(vec.DEFAULT_CAPACITY);
    usize pos1 = 0;
    usize pos2 = 0;

    for (auto _ : state) {
        state.PauseTiming();

        for (auto i = 0; i < NB; i++) {
            vec.EmplaceBack(i);
        }

        pos1 = distr(gen) % vec.Size();
        pos2 = distr(gen) % vec.Size();
        ereaseRange.emplace_back(pos1, pos2);

        if (pos1 > pos2)
            std::swap(pos1, pos2);

        state.ResumeTiming();
        vec.Erease(pos1, pos2);
    }
}

void StdVectorEreaseRange(benchmark::State& state)
{
    constexpr auto NB = 100;
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, NB); // define the range
    std::vector<int> vec;
    usize pos1 = 0;
    usize pos2 = 0;
    usize index = 0;

    for (auto _ : state) {
        state.PauseTiming();

        for (auto i = 0; i < NB; i++) {
            vec.emplace_back(i);
        }

        if (index < ereaseRange.size()) {
            auto [p1, p2] = ereaseRange[index++];
            pos1 = p1;
            pos2 = p2;
        } else {
            pos1 = distr(gen) % vec.size();
            pos2 = distr(gen) % vec.size();
        }
        
        if (pos1 > pos2)
            std::swap(pos1, pos2);

        state.ResumeTiming();
        vec.erase(vec.begin() + pos1, vec.begin() + pos2);
    }
}

// Register the function as a benchmark
BENCHMARK(VectorEmplaceBack);
BENCHMARK(StdVectorEmplaceBack);

BENCHMARK(VectorEmplaceFront);
BENCHMARK(StdVectorEmplaceFront);

BENCHMARK(VectorInsert);
BENCHMARK(StdVectorInsert);

BENCHMARK(VectorErease);
BENCHMARK(StdVectorErease);

BENCHMARK(VectorEreaseRange);
BENCHMARK(StdVectorEreaseRange);