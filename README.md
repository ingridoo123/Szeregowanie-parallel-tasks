Poradnik do poprawnego włączenia programu w terminalu (Cygwin lub Windowsy):
-skompilować program
-dodać do folderu z plikem .exe pliki z danymi wejściowymi
-jako pierwszy argument podać maksymalną liczbę zadań, a jako drugi argument nazwę pliku z danymi wejściowymi (musi być w tym samym folderze co pilk .exe)
-plik z wynikami (wyniki.txt) utworzy się w folderze, w którym znajduje się kod źródłowy
W kodzie w linii 267 (auto end = start + std::chrono::seconds(5);) jest możliwość zmiany czasu wykonywania się programu.
