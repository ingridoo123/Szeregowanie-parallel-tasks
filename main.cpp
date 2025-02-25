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
    int maxProcs; // ==maxProcs
    int maxJobs;	// ==maxRecords
    int data_size;
    int lowerBound;
    unsigned int sumTime = 0;


public:

    void getData(const std::string& path_name, long maxTaskNumber)
    {
        int n;
        data_size = 0;
        std::vector <int> reader;
        std::ifstream file;
        std::string line;
        file.open(path_name, std::ios::in);
        while (!file.eof())
        {
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
                    if (data_size == maxTaskNumber) break;
                }
                else maxJobs--;
            }
        }
        file.close();
    }
    void showData()
    {
        std::cout << maxProcs << std::endl;
        std::cout << maxJobs << std::endl;
        for (int i = 0; i < data_size; i++)
            std::cout << data[i].id << "\t" << data[i].submit << "\t" << data[i].runtime << "\t" << data[i].proc << std::endl;
    }

    long long int calculateCost() {
        std::vector<int> procTab(maxProcs, 0);
        int time = 0;
        int finish, max = 0;
        int solutionIndex = 0;
        long long int sumCJ = 0;

        output.clear();
        for (int i = 0; i < data.size(); i++) {
            if (data[i].submit > time)
                time = data[i].submit;

            // Znajdź gotowe procesory
            std::vector<int> readyProcs;
            for (int j = 0; j < maxProcs; j++) {
                if (time >= procTab[j]) {
                    readyProcs.push_back(j);  // Dodaj indeks procesora do listy gotowych procesorów
                }
            }

            // Sprawdź czy jest wystarczająco dużo gotowych procesorów do wykonania bieżącego zadania
            if (readyProcs.size() >= data[i].proc) {
                // Przypisz do tego zadania tylko wymaganą liczbę procesorów (data[i].proc)
                std::vector<int> outputProcs;
                for (int k = 0; k < data[i].proc; k++) {
                    int procIndex = readyProcs[k];
                    procTab[procIndex] = time + data[i].runtime;
                    outputProcs.push_back(procIndex+1); // Zaktualizuj, kiedy ten procesor będzie wolny
                }

                // Aktualizacja ogólnego czasu schedule dla tego zadania
                finish = time + data[i].runtime;
                Task_Completed task;
                task.id = data[i].id;
                task.start = time;
                task.stop = time + data[i].runtime;
                task.processors = outputProcs;
                output.push_back(task);
                sumCJ = sumCJ + task.stop;
            } else {
                // Gdy za mało dostępnych procesorów; znajdź następny raz, kiedy będzie wystarczająco dużo wolnych procesorów
                while (readyProcs.size() < data[i].proc) {
                    // Zresetuj liczbę gotowych procesorów
                    readyProcs.clear();

                    //Znajdź następny dostępny czas wśród wszystkich procesorów

                    int nextFreeTime = INT_MAX;
                    for (int j = 0; j < maxProcs; j++) {
                        if (procTab[j] > time) {
                            nextFreeTime = std::min(nextFreeTime, procTab[j]); // Znajdź najwcześniejszy wolny czas
                        }
                    }

                    //Zaktualizuj aktualny czas do następnego dostępnego wolnego czasu
                    time = nextFreeTime;

                    // Sprawdź ponownie gotowe procesory
                    for (int j = 0; j < maxProcs; j++) {
                        if (time >= procTab[j]) {
                            readyProcs.push_back(j);  //Aktualizuj listę gotowych procesorów
                        }
                    }
                }

                std::vector<int> outputProcs;
                for (int k = 0; k < data[i].proc; k++) {
                    int procIndex = readyProcs[k];
                    procTab[procIndex] = time + data[i].runtime;  //Aktualizuj kiedy proces będzie wolny
                    outputProcs.push_back(procIndex+1);
                }

                // Aktualizacja ogólnego czasu schedule dla tego zadania
                finish = time + data[i].runtime;

                Task_Completed task;
                task.id = data[i].id;
                task.start = time;
                task.stop = time + data[i].runtime;
                task.processors = outputProcs;
                output.push_back(task);
                solutionIndex++;
                sumCJ += task.stop;
            }
            if (finish > max) max = finish;
        }

        return sumCJ;
    }




    void greedyRCL(std::vector<Task>& data, int windowSize = 5) {


        std::random_device rd;
        std::mt19937 gen(rd());
        // std::uniform_int_distribution<> disWindow(100, 1000);
        // windowSize = disWindow(gen); // Ustawienie windowSize na losową wartość w zakresie
        // //Sortowanie przy użyciu arrivalSort
        // std::sort(data.begin(), data.end(), arrivalSort);





        // Zastosowanie randomizacji RCL
        std::vector<Task> result;
        while (!data.empty()) {
            int rclSize = std::min(windowSize, static_cast<int>(data.size()));

            // Utwórz RCL z elementem windowSize
            std::vector<Task> RCL(data.begin(), data.begin() + rclSize);

            //  Wybieranie losowego elementu z RCL
            std::uniform_int_distribution<> dis(0, rclSize - 1);
            int randomIndex = dis(gen);
            result.push_back(RCL[randomIndex]);

            // Usunięcie wybranego elementu z data
            data.erase(data.begin() + randomIndex);
        }
        // Przeniesienie losowo posortowanego wyniku z powrotem do oryginalnego wektora danych
        data = std::move(result);
    }


    long long int grasp() {

        greedyRCL(data);

        long long int cost = calculateCost();
        return cost;
    }

    std::vector<Task_Completed> getOutput() {
        return output;
    }
};



int main(int argc, char* argv[]) {
    std::cout.sync_with_stdio(false);

    // Maksymlana liczba zadań oraz wielkość opcjonalego argumentu windows size

    if (argc > 1) {
        //std::cout << "Optional argument (window size): " << argv[1] << std::endl;
        std::cout << "Max Task Number: " << argv[1] << std::endl;
    }
    Data d;


    d.getData(argv[2], long(atoi(argv[1])));

    long long int bestCost = std::numeric_limits<long long int>::max();
    std::vector<Task_Completed> bestOutput;
    auto start = std::chrono::steady_clock::now();
    auto end = start + std::chrono::seconds(10);
    int count = 0;


    while (std::chrono::steady_clock::now() < end) {
        long long int currentCost = d.grasp();
        std::cout << currentCost << std::endl;

        if (currentCost < bestCost) {
            bestCost = currentCost;
            bestOutput = d.getOutput();
        }
        count++;
    }

    // Zapis wyników
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

    // Wyświetlanie wyniku
    std::cout << "Min SumCj: " << bestCost << std::endl;


    return 0;
}

