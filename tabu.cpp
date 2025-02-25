#include <iostream>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <random>
#include <string>
#include <vector>
#include <sstream>
#include <limits>



struct Task {
    int id; //job number
    int submit; //r_j
    int runtime; //p_j
    int proc; //size_j

    bool operator==(const Task& other) const {
        return id == other.id && submit == other.submit && runtime == other.runtime && proc == other.proc;
    }

    bool operator!=(const Task& other) const {
        return !(*this == other);
    }
};

struct Task_Completed {
    int id;
    int start;
    int stop;
    std::vector <int> processors;
};


bool arrivalSort(Task t1, Task t2) {
    return t1.submit < t2.submit;
}


class Data {
private:
    std::vector<Task> data;
    std::vector<Task_Completed> output;
    std::vector<Task_Completed> bestOutput;
    int maxProcs; // ==maxProcs
    int maxJobs;	// ==maxRecords
    int data_size;
    int lowerBound;
    unsigned int sumTime = 0;



public:
    bool operator==(const Data& other) const {
        return data == other.data;  // Porównanie danych
    }

    void getData(const std::string& path_name, int maxTaskNumber)
    {
        int n;
        data_size = 0;
        std::vector <int> reader;
        std::ifstream file;
        std::string line;
        file.open(path_name, std::ios::in);
        while (!file.eof())
        {
            if (data_size == maxTaskNumber) break;
            getline(file, line); // Czyta jedna linie z pliku i zapisuje do zmiennej line
            if (line.empty()) {
                continue;
            }
            if (line[0] == ';')
            {
                std::string tmp;
                int value;
                char c;
                std::stringstream ss(line);
                ss >> c >> tmp >> value; // C przechowuje ";". tmp przechowuje stringi "MaxProcs:" i "MaxJobs:", a value przechowuje ich wartości
                if (tmp == "MaxProcs:") maxProcs = value;
                else if (tmp == "MaxJobs:") maxJobs = value;
            }

            else
            {
                Task task;
                std::stringstream stream(line);
                while (stream >> n) // Wyciaga number jeden po drugim z line i potem zapisuje w n
                {
                    reader.push_back(n); // Zapisywane w vectorze reader
                }
                task.id = reader[0];
                task.submit = reader[1];
                task.runtime = reader[3];
                task.proc = reader[4];
                reader.clear();

                // Walidacja danych przed dodaniem ich do data
                if (task.id != -1 && task.submit != -1 && task.runtime > 0 && task.proc != -1)
                {
                    data.push_back(task);
                    data_size++;

                }
                else maxJobs--;
            }
        }
        file.close();
    }

    long long int calculateCost(const std::vector<Task>& solution) {
        std::vector<int> procTab(maxProcs, 0);
        int time = 0;
        int finish, max = 0;
        int solutionIndex = 0;
        long long int sumCJ = 0;

        output.clear();
        for (int i = 0; i < solution.size(); i++) {
            if (solution[i].submit > time)
                time = solution[i].submit;

            // Znajdź gotowe procesory
            std::vector<int> readyProcs;
            for (int j = 0; j < maxProcs; j++) {
                if (time >= procTab[j]) {
                    readyProcs.push_back(j);  // Dodaj indeks procesora do listy gotowych procesorów
                }
            }

            // Sprawdź, czy jest wystarczająco dużo gotowych procesorów do wykonania bieżącego zadania
            if (readyProcs.size() >= solution[i].proc) {
                // Przypisz do tego zadania tylko wymaganą liczbę procesorów
                std::vector<int> outputProcs;
                for (int k = 0; k < solution[i].proc; k++) {
                    int procIndex = readyProcs[k];
                    procTab[procIndex] = time + solution[i].runtime;
                    outputProcs.push_back(procIndex + 1); // Zaktualizuj, kiedy ten procesor będzie wolny
                }

                // Aktualizacja ogólnego czasu schedule dla tego zadania
                finish = time + solution[i].runtime;
                Task_Completed task;
                task.id = solution[i].id;
                task.start = time;
                task.stop = time + solution[i].runtime;
                task.processors = outputProcs;
                output.push_back(task);
                sumCJ += task.stop;
            } else {
                // Gdy za mało dostępnych procesorów; znajdź następny raz, kiedy będzie wystarczająco dużo wolnych procesorów
                while (readyProcs.size() < solution[i].proc) {
                    readyProcs.clear();
                    int nextFreeTime = INT_MAX;
                    for (int j = 0; j < maxProcs; j++) {
                        if (procTab[j] > time) {
                            nextFreeTime = std::min(nextFreeTime, procTab[j]);
                        }
                    }
                    time = nextFreeTime;

                    for (int j = 0; j < maxProcs; j++) {
                        if (time >= procTab[j]) {
                            readyProcs.push_back(j);
                        }
                    }
                }

                std::vector<int> outputProcs;
                for (int k = 0; k < solution[i].proc; k++) {
                    int procIndex = readyProcs[k];
                    procTab[procIndex] = time + solution[i].runtime;
                    outputProcs.push_back(procIndex + 1);
                }

                finish = time + solution[i].runtime;

                Task_Completed task;
                task.id = solution[i].id;
                task.start = time;
                task.stop = time + solution[i].runtime;
                task.processors = outputProcs;
                output.push_back(task);
                solutionIndex++;
                sumCJ += task.stop;
            }

            if (finish > max) max = finish;
        }
        // std::cout<<sumCJ<<std::endl;
        return sumCJ;
    }

    std::vector<std::vector<Task>> generateNeighbors(std::vector<Task>& solution, int num_neighbors) {
        std::vector<std::vector<Task>> neighbors;
        for (int i = 0; i < num_neighbors; i++) {
            // Kopiowanie bieżącego rozwiązania
            std::vector<Task> neighbor = solution;
            // Losowa zamiana dwóch zadań
            int idx1 = rand() % solution.size();
            int idx2 = rand() % solution.size();
            while (idx1 == idx2) {
                idx2 = rand() % solution.size();  // Upewniamy się, że idx1 != idx2
            }
            // Zamiana miejscami
            std::swap(neighbor[idx1], neighbor[idx2]);
            neighbors.push_back(neighbor);
        }
        return neighbors;
    }


    std::vector<Task> generateInitialSolution(std::vector<Task>& solution, long long int cost, int windowSize) {
        std::random_device rd;
        std::mt19937 gen(rd());
        long long int minSumCJ = cost;
        long long int currSumCJ;
        std::vector<Task> minSolution = solution;
        // Zastosowanie randomizacji RCL
        std::vector<Task> result;

        auto start = std::chrono::steady_clock::now();
        auto end = start + std::chrono::seconds(10);
        while(std::chrono::steady_clock::now() < end) {
            std::vector<Task> initialSolution = solution;
            result.clear();
            while(!initialSolution.empty()) {
                int rclSize = std::min(windowSize, static_cast<int>(initialSolution.size()));
                std::vector<Task> RCL(initialSolution.begin(), initialSolution.begin() + rclSize);
                std::uniform_int_distribution<> dis(0, rclSize - 1);
                int randomIndex = dis(gen);
                result.push_back(RCL[randomIndex]);
                initialSolution.erase(initialSolution.begin() + randomIndex);
            }
            currSumCJ = calculateCost(result);
            if (currSumCJ < minSumCJ) {
                minSumCJ = currSumCJ;
                minSolution = result;
            }
        }
        std::cout <<"2. Best cost (after RCL): "<< minSumCJ << std::endl;
        return minSolution;
    }

    long long int tabuSearch(char* argv[]) {
        int num_neighbors = 30;
        int tabu_list_size = 50;
        int max_iterations = 100000;
        std::vector<Task> initial_solution = data;  // Tworzymy kopię danych
        std::sort(initial_solution.begin(), initial_solution.end(), arrivalSort);  // Sortujemy tę kopię
        std::vector<std::vector<Task>> tabu_list;
        long long int best_cost = calculateCost(initial_solution);
        bestOutput = output;
        std::cout << "1. Best cost (before RCL): " << best_cost << std::endl;
        std::vector<Task> best_solution = generateInitialSolution(initial_solution, best_cost, 3);
        best_cost = calculateCost(best_solution);
        auto start_time = std::chrono::steady_clock::now();  // Start time tracking

        for(int iter = 0; iter < max_iterations; iter++) {
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);
            if (elapsed_time.count() >= 30) {
                std::cout << "Time limit reached! Best cost found: " << best_cost << std::endl;
                break;
            }
            std::vector<std::vector<Task>> neighbors = generateNeighbors(best_solution, num_neighbors);
            std::vector<Task> best_neighbor = best_solution;
            long long int best_neighbor_cost = std::numeric_limits<long long>::max();
            for (int i = 0; i < neighbors.size(); ++i) {
                const auto& neighbor = neighbors[i];  // Uzyskanie dostępu do sąsiada przez indeks

                // Sprawdzanie, czy sąsiad nie jest na liście tabu
                if (std::find(tabu_list.begin(), tabu_list.end(), neighbor) == tabu_list.end()) {
                    // Obliczanie kosztu sąsiada
                    long long int neighbor_cost = calculateCost(neighbor);  // Liczymy koszt dla sąsiada

                    // Sprawdzamy, czy ten sąsiad jest lepszy
                    if (neighbor_cost < best_neighbor_cost) {
                        best_neighbor = neighbor;
                        best_neighbor_cost = neighbor_cost;
                    }
                }
            }
            // Aktualizacja najlepszego rozwiązania, jeśli znaleziono lepszego sąsiada
            if (best_neighbor_cost < best_cost) {
                best_solution = best_neighbor;
                best_cost = best_neighbor_cost;
                calculateCost(best_solution);
                bestOutput = output; // Zapisz poprawny harmonogram dla najlepszego rozwiązania
                std::cout<<"Tabu search: " << best_cost <<std::endl;
            }
            // Dodajemy najlepszy sąsiad do listy tabu
            tabu_list.push_back(best_neighbor);
            // Usuwamy najstarsze rozwiązanie z listy tabu, jeśli lista przekroczyła rozmiar
            if (tabu_list.size() > tabu_list_size) {
                tabu_list.erase(tabu_list.begin());
            }
        }
        return best_cost;
    }




    std::vector<Task_Completed> getOutput() {
        return bestOutput;
    }

    std::vector<Task_Completed> getLastOutput() {
        return output;
    }
};


int main(int argc, char* argv[]) {
    if (argc > 1) {
        //std::cout << "Optional argument (window size): " << argv[1] << std::endl;
        std::cout << "Max Task Number: " << argv[1] << std::endl;
    }
    Data d;
    std::vector<Task_Completed> bestOutput;
    std::vector<Task_Completed> lastOutput;

    d.getData(argv[2], int(atoi(argv[1])));

    auto start_time = std::chrono::high_resolution_clock::now();
    long long int bestCost =  d.tabuSearch(argv);
    auto end_time = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed_seconds = end_time - start_time;

    bestOutput = d.getOutput();

    // Zapis wyników
    std::ofstream outputFile("wyniki-tabu-cygwin.txt");
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

    // Wyświetlanie wyniku
    std::cout << "Min SumCj: " << bestCost << std::endl;
    std::cout << "Elapsed time: " << elapsed_seconds.count() << " seconds" << std::endl;

    return 0;
}