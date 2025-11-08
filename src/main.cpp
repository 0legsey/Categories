#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <climits>

using namespace std;

/*Описание алгоритма
Проблема:
При наличии нескольких каталогов для позициий может возникнуть ситуация, когда набор, который на самом деле входит в документ,
считаться входящим не будет, например:
document.txt
1 2 "A", "B"
2 2 "A", "C"
kits.txt
"A" 2
"B" 2
Кажется очевидным, что набор входит в документ, однако, при поиске вхождений каталога "A" оба продукта первой позиции будут
им "съедены", тогда каталог "B" как будто не входит в наш документ в достаточном количестве. При этом, еслы бы первым
проверялось наличии продуктов из каталога "B", этой проблемы можно было бы избежать.
Решение:
Сначала определим, хватит ли для удовлетворения набору позиций с одним каталогом, если да - выводим этот набор, если нет - 
переходим к позициям с множеством каталогов. Сначала определяем для каждого каталога, которому необходимо добрать товары,
максимальное возможное количество товаров, которые в него входят, в примере выше будет А - 4, В - 2. Затем находим минимальный
из этих максимумов, в нашем случае это будет B - запоминаем этот каталог и определяем, достаточное ли количество раз в документ
входят товары из этого каталога. При этом важно, что при нахождении товаров этого каталога, будет уменьшаться максимум товаров 
не только для текущего каталога, но и для всех каталогов в этой позиции. Если каталог с минимальным из максимумов входит в документ -
забываем про него и рассматриваем оставшиеся каталоги, если не входит - значит набор не содержится в списке.
*/

struct Position{
    int id;
    int count;
    vector<string> categories;

    void operator = (Position& other){
        id = other.id;
        count = other.count;
        categories = other.categories;
    }
};

//считывание document.txt с основным списком позиций
vector<Position> readPositionsFromFile(fstream& file){
    vector<Position> positions;
    while (!file.eof()){
        int id, count;
        vector<string> categories;
        string category;
        string categoriesString;
        file >> id >> count;

        getline(file, categoriesString);
        stringstream sstream(categoriesString);

        while (sstream >> category){
            if (category.find(",") != category.npos){
                category.erase(category.find(","), 1);
            }
            categories.push_back(category);
        }

        positions.push_back({id, count, categories});
    }
    return positions;
}

//определяем, какое количество товаров содержится в позициях с одним каталогом
unordered_map<string, int> calculateUniqueKits(vector<Position> positions){
    unordered_map<string, int> uniqueKits;
    for (auto position: positions){
        if (position.categories.size() == 1){
            uniqueKits[position.categories[0]] += position.count;
        }
    }
    return uniqueKits;
}

//считываем kits.txt с наборами
unordered_map<string, int> readKitsFromFile(fstream& file){
    unordered_map<string, int> kits;
    while (!file.eof()){
        string category;
        int count;
        file >> category >> count;

        kits[category] = count;
    }
    return kits;
}

//проверяем, достаточно ли позиций с одним каталогом
bool checkIfUniqueKitsAreEnough(unordered_map<string, int>& kits, unordered_map<string, int>& leftPositions){
    bool flag = 0;
    for (auto kit: kits){
        leftPositions[kit.first] -= kit.second;
        //если какому-то из каталогов не зватает товаров - позже вернём false, но сейчас продолжаем считать количество недостающих товаров
        if (leftPositions[kit.first] < 0){
            flag = 1;
        }
    }
    if(flag){
        return false;
    }
    return true;
}

//находим минимальное значение в словаре
string findMinValueKey(unordered_map<string, int> unord_map){
    
    string minKey = "";
    int minValue = INT_MAX;
    for (auto el: unord_map){
        if (el.second < minValue){
            minValue = el.second;
            minKey = el.first;
        }
    }
    return minKey;
}

//заполняем результирующий список, здесь учитываются только позиции с одним каталогом
vector<Position> fillResultingList(vector<Position> positions, unordered_map<string, int> uniqueKits, unordered_map<string, int> leftKits){
    vector<Position> resultingList;
    int id, count;
    string category;
    for (auto position: positions){
        id = position.id;
        count = 0;
        if (position.categories.size() == 1){
            //заполняем список пока товаров не будет хватать или они не закончатся, или пока не закончится товар на текущей позиции
            while(uniqueKits[position.categories[0]] != leftKits[position.categories[0]] && uniqueKits[position.categories[0]] > 0 && position.count != 0){
                position.count--;
                count++;
                uniqueKits[position.categories[0]]--;
            }
            if (count > 0){
                resultingList.push_back({id, count, vector<string>{position.categories[0]}});
            }
        }
    }
    return resultingList;
}

//проверяем, удастся ли заполнить набор с учётом позиций с множеством каталогов, заодно вписываем удовлетворяющие позиции в
//результирующий список
bool checkIfInsidePositions(vector<Position> positions, unordered_map<string, int>& uniqueKits, vector<Position>& resultingList){
    unordered_map<string, int> maxPossibleCounts;
    //учитываем только те каталоги, которым пока не хватает товаров
    for (auto kit: uniqueKits){
        if (kit.second < 0){
            maxPossibleCounts[kit.first] = 0;
        }
    }

    //определяем возможный максимум товаров для каждой категории
    for (auto position: positions){
        if (position.categories.size() > 1){
            for (string category: position.categories){
                auto search = maxPossibleCounts.find(category); 
                if (search != maxPossibleCounts.end()){
                    maxPossibleCounts[category] += position.count;
                }
            }
        }
    }

    int n = maxPossibleCounts.size();

    int id, count;

    for (int i = 0; i < n; i++){
        //находим каталог с минимальным максимумом и определяем, достаточно ли для него товаров
        string minKey = findMinValueKey(maxPossibleCounts);

        for (auto& position: positions){
            id = position.id;
            count = 0;
            for (string category: position.categories){
                if (category == minKey && position.categories.size() > 1){
                    while (uniqueKits[category] != 0 && maxPossibleCounts[category] != 0 && position.count != 0){
                        for (string category: position.categories){
                            auto search = maxPossibleCounts.find(category);
                            if (search != maxPossibleCounts.end()){
                                maxPossibleCounts[category]--;
                            }
                        }
                        count++;
                        position.count--;
                        uniqueKits[category]++;
                    }
                    break;
                }
            }

            //параллельно заполняем результирующий список
            if(count > 0){
                resultingList.push_back({id, count, vector<string>{minKey}});
            }

            //если товаров достаточно - удаляем из словаря текущий каталог и переходим к следующему
            if (uniqueKits[minKey] == 0){
                break;
            }

        }

        //если даже после переборов всех позиций товаров для текущего каталога недостаточно - набор не найден
        if (uniqueKits[minKey] < 0){
            return false;
        }

        auto hs = maxPossibleCounts.extract(minKey);
    }

    return true;
}

//сортируем получившийся список по id для удобства
void sortListById(vector<Position>& positions){
    for (int i = 0; i < positions.size(); i++){
        for (int j = 0; j < positions.size() - 1; j++){
            if (positions[j].id > positions[j + 1].id){
                auto auxPos = positions[j];
                positions[j] = positions[j + 1];
                positions[j + 1] = auxPos;
            }
        }
    }
}

int main(){
    setlocale(LC_ALL, "ru_RU.UTF-8");
    fstream file1("document.txt", fstream::in);
    fstream file2("kits.txt", fstream::in);

    if (!file1.is_open()){
        cout << "Ошибка при открытии файла " << "document.txt" << endl;
        return 0;
    }
    else if (!file2.is_open()){
        cout << "Ошибка при открытии файла " << "kits.txt" << endl;
        return 0;
    }

    //считываем позиции и наборы с файла и вычисляем количество товаров в позициях с одним каталогом
    vector<Position> positions;
    positions = readPositionsFromFile(file1);

    auto uniqueKits = calculateUniqueKits(positions);
    auto leftPositions = uniqueKits; //наборы которые ещё нужно заполнить, отрицательные значения в нём
    //свидетельствуют о недостатке товаров определённого каталога, изначально они равны количеству товаров с одним каталогом

    auto kits = readKitsFromFile(file2);

    //проверяем, хватает ли позиций с одним каталогом для удовлетворения набору
    //если да - выводим список
    if (checkIfUniqueKitsAreEnough(kits, leftPositions)){
        cout << "Набор содержится в документе" << endl;
        auto resultingList = fillResultingList(positions, uniqueKits, leftPositions);
        cout << "Состав набора:" << endl;
        for (auto position: resultingList){
            cout << position.id << " " << position.count << " " << position.categories[0] << endl;
        }
    }
    //если нет - заполняем итоговый список имеющимися позициями и переходим к позициям с множеством каталогов
    else{
        auto resultingList = fillResultingList(positions, uniqueKits, leftPositions);
        if (checkIfInsidePositions(positions, leftPositions, resultingList)){
            sortListById(resultingList);
            cout << "Набор содержится в документе" << endl;
            cout << "Состав набора:" << endl;
            for (auto position: resultingList){
                cout << position.id << " " << position.count << " " << position.categories[0] << endl;
            }
        }
        else{
            cout << "Набор не содержится в документе";
        }
    }
}