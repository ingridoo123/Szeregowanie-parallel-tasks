#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <chrono>
#include <limits>

struct Task {
    int id; // job number
    int submit; // r_j
    int runtime; // p_j
    int proc; // size_j
};

struct Task_Completed {
    int id;
    int start;
    int stop;
    std::vector<int> processors;
};

bool arrivalSort(Task t1, Task t2) {
    return t1.submit < t2.submit;
}

class Data {
private:
    std::vector<Task> data;
    std::vector<Task_Completed> output;
    int maxProcs; // liczba procesorów
    int maxJobs; // liczba zadań
    int data_size;

    // Pomocnicze do algorytmu genetycznego
    std::random_device rd;
    std::mt19937 gen;

public:
    Data() : gen(rd()) {}

    void getData(const std::string& path_name, long maxTaskNumber) {
        int n;
        data_size = 0;
        std::vector<int> reader;
        std::ifstream file;
        std::string line;
        file.open(path_name, std::ios::in);
        while (!file.eof()) {
            getline(file, line);
            if (line.empty()) continue;
            if (line[0] == ';') {
                std::string tmp;
                int value;
                char c;
                std::stringstream ss(line);
                ss >> c >> tmp >> value;
                if (tmp == "MaxProcs:") maxProcs = value;
                else if (tmp == "MaxJobs:") maxJobs = value;
            } else {
                Task task;
                std::stringstream stream(line);
                while (stream >> n) {
                    reader.push_back(n);
                }
                task.id = reader[0];
                task.submit = reader[1];
                task.runtime = reader[3];
                task.proc = reader[4];

                reader.clear();

                if (task.id != -1 && task.submit != -1 && task.runtime > 0 && task.proc != -1) {
                    data.push_back(task);
                    data_size++;
                    if (data_size == maxTaskNumber) break;
                } else maxJobs--;
            }
        }
        file.close();
    }

    void showData() {
        std::cout << maxProcs << std::endl;
        std::cout << maxJobs << std::endl;
        for (int i = 0; i < data_size; i++) {
            std::cout << data[i].id << "\t" << data[i].submit << "\t" << data[i].runtime << "\t" << data[i].proc << std::endl;
        }
    }

    long long int calculateCost(const std::vector<Task>& schedule) {
        std::vector<int> procTab(maxProcs, 0);
        int time = 0;
        long long int sumCJ = 0;
        output.clear();

        for (const auto& task : schedule) {
            if (task.submit > time) time = task.submit;

            std::vector<int> readyProcs;
            for (int j = 0; j < maxProcs; j++) {
                if (time >= procTab[j]) {
                    readyProcs.push_back(j);
                }
            }

            if (readyProcs.size() >= task.proc) {
                std::vector<int> outputProcs;
                for (int k = 0; k < task.proc; k++) {
                    int procIndex = readyProcs[k];
                    procTab[procIndex] = time + task.runtime;
                    outputProcs.push_back(procIndex + 1);
                }

                Task_Completed completedTask;
                completedTask.id = task.id;
                completedTask.start = time;
                completedTask.stop = time + task.runtime;
                completedTask.processors = outputProcs;
                output.push_back(completedTask);

                sumCJ += completedTask.stop;
            }
        }

        return sumCJ;
    }

    std::vector<Task> mutate(const std::vector<Task>& individual) {
        std::vector<Task> mutated = individual;
        int size = mutated.size();
        if (size > 1) {
            std::uniform_int_distribution<> dis(0, size - 1);
            int idx1 = dis(gen);
            int idx2 = dis(gen);
            std::swap(mutated[idx1], mutated[idx2]);
        }
        return mutated;
    }

    std::vector<Task> crossover(const std::vector<Task>& parent1, const std::vector<Task>& parent2) {
        int size = parent1.size();
        std::uniform_int_distribution<> dis(0, size - 1);
        int crossoverPoint = dis(gen);

        std::vector<Task> child(parent1.begin(), parent1.begin() + crossoverPoint);
        for (const auto& task : parent2) {
            if (std::find_if(child.begin(), child.end(),
                             [&task](const Task& t) { return t.id == task.id; }) == child.end()) {
                child.push_back(task);
            }
        }
        return child;
    }

    std::vector<Task> initializeRandomIndividual() {
        std::vector<Task> individual = data;
        std::shuffle(individual.begin(), individual.end(), gen);
        return individual;
    }

    long long int geneticAlgorithm(int populationSize, int generations) {
    std::vector<std::vector<Task>> population;

    // Inicjalizacja populacji
    for (int i = 0; i < populationSize; ++i) {
        population.push_back(initializeRandomIndividual());
    }

    long long int bestCost = std::numeric_limits<long long int>::max();
    std::vector<Task> bestIndividual;

    for (int generation = 0; generation < generations; ++generation) {
        std::vector<std::pair<long long int, std::vector<Task>>> fitness;

        // Obliczanie kosztów dla każdego osobnika w populacji
        for (const auto& individual : population) {
            long long int cost = calculateCost(individual);
            fitness.emplace_back(cost, individual);
        }

        // Sortowanie osobników według ich kosztów
        std::sort(fitness.begin(), fitness.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });

        // Aktualizacja najlepszego kosztu
        if (fitness[0].first < bestCost) {
            bestCost = fitness[0].first;
            bestIndividual = fitness[0].second;
        }

        // Wyświetlenie informacji o generacji
        std::cout << "Generation: " << generation + 1
                  << " | Best Cost in Generation: " << fitness[0].first
                  << " | Best Overall Cost: " << bestCost << std::endl;

        // Tworzenie nowej populacji przez krzyżowanie i mutacje
        std::vector<std::vector<Task>> newPopulation;

        for (int i = 0; i < populationSize / 2; ++i) {
            auto parent1 = fitness[i].second;
            auto parent2 = fitness[(i + 1) % (populationSize / 2)].second;

            auto child1 = crossover(parent1, parent2);
            auto child2 = crossover(parent2, parent1);

            newPopulation.push_back(mutate(child1));
            newPopulation.push_back(mutate(child2));
        }

        population = std::move(newPopulation);
    }

    // Ostateczne obliczenie najlepszego rozwiązania i przypisanie do output
    calculateCost(bestIndividual);
    return bestCost;
}


    std::vector<Task_Completed> getOutput() {
        return output;
    }
};

int main(int argc, char* argv[]) {
    std::cout.sync_with_stdio(false);

    if (argc > 1) {
        std::cout << "Max Task Number: " << argv[1] << std::endl;
    }
    Data d;

    d.getData(argv[2], long(atoi(argv[1])));

    int populationSize = 10;
    int generations = 20;

    long long int bestCost = d.geneticAlgorithm(populationSize, generations);
    std::vector<Task_Completed> bestOutput = d.getOutput();

    std::ofstream outputFile("wyniki.txt");
    if (outputFile.is_open()) {
        for (const auto& task : bestOutput) {
            outputFile << task.id << " " << task.start << " " << task.stop << " ";
            for (int proc : task.processors) {
                outputFile << proc << " ";
            }
            outputFile << std::endl;
        }
        outputFile.close();
    }

    std::cout << "Min SumCj: " << bestCost << std::endl;

    return 0;
}


