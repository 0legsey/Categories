#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <climits>

using namespace std;

/*Описание первого - нерабочего алгоритма
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

/*Описанный выше алгоритм имеет контрпример:
document.txt
1 3 "A", "B"
2 2 "B", "C"
3 1 "A", "C"
kits.txt
"B" 4
"C" 2
Сначала С заберёт все товары из второй позиции, хотя могло забрать из третьей и для В не останется товаров, хотя набор входит в документ

Для решения этой проблемы была использована новая метрика, по которой определяется приоритет отбора товаров каталогами - разница между 
"предложением" - максимально доступным количеством товара для категории и "спросом" - количеством необходимого товара в наборе.
Теперь в первую очередь выбирает каталог с наименьшим значением этой метрики. Он забирает один товар и далее метрики пересчитываются
чтобы понять, кто забирает следующим. При этом сначала отбираются товары из позиций, относящихся к одной категории, далее относящиеся
к двум категоряим, к трём и так далее.
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

//проверяем, нужны ли ещё товары
bool checkIfKitsAreNotEmpty(unordered_map<string, int> kits){
    bool flag = false;
    for (auto kit: kits){
        if (kit.second > 0){
            flag = true;
        }
    }
    return flag;
}

//основная функция, в которой проверяется вхождение набора в документ
bool checkIfKitIsInPositions(vector<Position> positions, unordered_map<string, int> kits, vector<Position>& resultPositions){
    //выполняем алгоритм, пока нам нужны товары
    while (checkIfKitsAreNotEmpty(kits)){
        //определяем для каких товаров нам нужно предложение
        unordered_map<string, int> offers;
        for (auto kit: kits){
            if (kit.second != 0){
                offers[kit.first] = 0;
            }
        }

        //заполнеяем предложение
        for (auto position: positions){
            for (auto category: position.categories){
                auto search = offers.find(category);
                if (search != offers.end()){
                    offers[category] += position.count;
                }
            }
        }

        unordered_map<string, int> metrics;

        //вычисляем метрику
        for (auto offer: offers){
            metrics[offer.first] = offer.second - kits[offer.first];
            if (metrics[offer.first] < 0){
                //если спрос больше предложения - набор не входит в документ
                return false;
            }
        }

        //находим минимальное значение метрики
        string minKey = findMinValueKey(metrics);

        bool stop = false;
        //максимально возможное количество каталогов в позиции не может быть больше количества уникальных каталогов в наборе
        for (int i = 1; i < kits.size() + 1; i++){
            for (auto& position: positions){
                if (position.count != 0){
                    if (position.categories.size() == i){
                        for (string category: position.categories){
                            if (category == minKey){
                                //меняем значения для последующего пересчёта метрик
                                position.count--;
                                kits[category]--;
                                stop = true;

                                //заполняем результирующий список
                                bool existsInResultList = false;
                                for (auto& resultPos: resultPositions){
                                    if (resultPos.id == position.id && resultPos.categories[0] == category){
                                        existsInResultList = true;
                                        resultPos.count++;
                                        break;
                                    }
                                }

                                if (!existsInResultList){
                                    resultPositions.push_back({position.id, 1, vector<string>{category}});
                                }

                                break;
                            }
                        }
                    }
                }
                if (stop) break;
            }
            if (stop) break;
        }
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
    vector<Position> positions, resultPositions;
    positions = readPositionsFromFile(file1);
    auto kits = readKitsFromFile(file2);

    if (checkIfKitIsInPositions(positions, kits, resultPositions)){
        cout << "Набор содержится в документе" << endl;
        cout << "Состав набора:" << endl;
        sortListById(resultPositions);
        for (auto resultPos: resultPositions){
            cout << resultPos.id << " " << resultPos.count << " " << resultPos.categories[0] << endl;
        }
    }
    else{
        cout << "Набор не содержится в документе" << endl;
    }

}
