#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <limits>
#include <queue>
#include <algorithm>
#include <cmath>
#include <stdio.h> 
#include <time.h> 
#include <unordered_set>

using namespace std;

struct Node {
    string lon, lat;  // Теперь строки
    vector<pair<Node*, double>> neighbors;
};

struct Graph {
    vector<Node*> nodes;
    unordered_map<string, Node*> nodeMap; // Для быстрого поиска узлов по строковому ключу

    Graph(const string& filename) {
        loadGraphFromFile(filename);
    }

     void loadGraphFromFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Ошибка открытия файла: " << filename << endl;
            return;
        }

        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string token;

            // Парсинг родительского узла
            getline(ss, token, ':');
            string parentKey = token;  // Ключ - исходная строка координат
            Node* parentNode;

            if (nodeMap.find(parentKey) == nodeMap.end()) {
                stringstream parentCoordSS(token);
                getline(parentCoordSS, token, ',');
                string parentLon = token;
                getline(parentCoordSS, token, ',');
                string parentLat = token;

                parentNode = new Node{parentLon, parentLat};
                nodes.push_back(parentNode);
                nodeMap[parentKey] = parentNode;
            } else {
                parentNode = nodeMap[parentKey];
            }

            // Парсинг дочерних узлов и ребер
            while (getline(ss, token, ';')) {
                stringstream childSS(token);

                getline(childSS, token, ',');
                string childLon = token;
                getline(childSS, token, ',');
                string childLat = token;
                string childKey = childLon + "," + childLat; // Ключ - строка координат
                
                getline(childSS, token, ','); //теперь токен это weight
                double weight = stod(token);

                Node* childNode;
                if (nodeMap.find(childKey) == nodeMap.end()) {
                    childNode = new Node{childLon, childLat};
                    nodes.push_back(childNode);
                    nodeMap[childKey] = childNode;
                } else {
                    childNode = nodeMap[childKey];
                }

                parentNode->neighbors.push_back({childNode, weight});
            }
        }
        file.close();
     }
        Node* find_closest_node(double lat, double lon) {
            // Функция для поиска в графе узла, который ближе всего находится к указанной точке, 
            // которую вы выбрали на карте
            double min_distance = numeric_limits<double>::infinity();
            Node* node_founded = nullptr;
            for (auto node : nodes){
                double nodeLat = stod(node->lat);
                double nodeLon = stod(node->lon);
                double distance = sqrt(pow(nodeLat - lat, 2) + pow(nodeLon - lon, 2));

                if (distance < min_distance) 
                {
                    node_founded = node;
                    min_distance = distance;
                }
            }
        return node_founded;    
    }
};


unordered_map<Node*, double> dijkstra(const vector<Node*>& nodes, Node* startNode) {
    unordered_map<Node*, double> distances;
    priority_queue<pair<double, Node*>, vector<pair<double, Node*> >, greater<pair<double, Node*> > > pq;
    for (Node* node : nodes) {
        distances[node] = numeric_limits<double>::infinity();
    }
    distances[startNode] = 0.0;
    pq.push({0.0, startNode});
    while (!pq.empty()) {
        double currentDist = pq.top().first;
        Node* currentNode = pq.top().second;
        pq.pop();
        if (currentDist > distances[currentNode]) {
            continue; // Пропускаем устаревшие значения
        }

        for (const auto& neighbor : currentNode->neighbors) {
            Node* nextNode = neighbor.first;
            double weight = neighbor.second;
            double newDist = currentDist + weight;

            if (newDist < distances[nextNode]) {
                distances[nextNode] = newDist;
                pq.push({newDist, nextNode});
            }
        }
    }

    return distances;
}

double heuristic(Node* a, Node* b) {
    double lat1 = stod(a->lat);
    double lon1 = stod(a->lon);
    double lat2 = stod(b->lat);
    double lon2 = stod(b->lon);
    return sqrt(pow(lat1 - lat2, 2) + pow(lon1 - lon2, 2));
}

unordered_map<Node*, double> a_star(const vector<Node*>& nodes, Node* startNode, Node* endNode) {
    unordered_map<Node*, double> distances;
    unordered_map<Node*, double> f_score;
    priority_queue<pair<double, Node*>, vector<pair<double, Node*> >, greater<pair<double, Node*> > > queue;
    for (Node* node : nodes) {
        distances[node] = numeric_limits<double>::infinity();
        f_score[node] = numeric_limits<double>::infinity();

    }
    distances[startNode] = 0.0;
    f_score[startNode] = heuristic(startNode, endNode);
    queue.push({f_score[startNode], startNode});
    while (!queue.empty()) {
        Node* current = queue.top().second;
        queue.pop();
        if (current == endNode) {
            return distances;
        }
        for (const auto& neighbor_pair : current->neighbors) {
            Node* neighbor = neighbor_pair.first;
            double weight = neighbor_pair.second;
            double tentative_gScore = distances[current] + weight;
            if(tentative_gScore < distances[neighbor]){
                distances[neighbor] = tentative_gScore;
                f_score[neighbor] = tentative_gScore + heuristic(neighbor, endNode);
                 queue.push({f_score[neighbor], neighbor});
            }
        }
    }
     return distances;
}

double bfs(const vector<Node*>& nodes, Node* startNode, Node* endNode) {
    queue<pair<Node*, double>> q;
    unordered_set<Node*> visited;
    q.push({startNode, 0.0});
    visited.insert(startNode);
    while (!q.empty()) {
        Node* current = q.front().first;
        double currentDistance = q.front().second;
        q.pop();
        if (current == endNode) {
            return currentDistance;
        }
        for (const auto& neighbor_pair : current->neighbors) {
            Node* neighbor = neighbor_pair.first;
            double weight = neighbor_pair.second;

            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                q.push({neighbor, currentDistance + weight});
            }
        }
    }

    return -1.0;
}

double dfs(const vector<Node*>& nodes, Node* startNode, Node* endNode, unordered_set<Node*>& visited) {
    visited.insert(startNode);
    if (startNode == endNode) {
        return 0.0;
    }
    for (const auto& neighbor_pair : startNode->neighbors) {
        Node* neighbor = neighbor_pair.first;
        double weight = neighbor_pair.second;
        if (visited.find(neighbor) == visited.end()) {
            double result = dfs(nodes, neighbor, endNode, visited);
            if (result != -1.0) {
                return result + weight;
            }
        }
    }
    return -1.0;
}


int main() {
    Graph graph("spb_graph.txt");
    unordered_set<Node*> visited_dfs;
    double mylat = 59.884972; // Широта 
    double mylon = 30.368072; // Долгота 
    double finlat = 59.956248;
    double finlon = 30.309215;

    Node* closest_node = graph.find_closest_node(mylat, mylon); // Ближайшая стартовая вершина 
    Node* finish_node = graph.find_closest_node(finlat, finlon); // Ближайшая конечная вершина 

    //dfs
    clock_t start_dfs = clock(); // подсчет времени
    double distance_to_finish_dfs = dfs(graph.nodes, closest_node, finish_node, visited_dfs); // Поиск пути по алгоритму DFS
    clock_t end_dfs = clock();
    cout << "Расстояние до ИТМО по оценке DFS: " << distance_to_finish_dfs << endl;

    //bfs
    clock_t start_bfd = clock(); // подсчет времени
    double distance_to_finish_bfs = bfs(graph.nodes, closest_node, finish_node); // Поиск пути по алгоритму BFS
    clock_t end_bfs = clock();
    cout << "Расстояние до ИТМО по оценке BFS: " << distance_to_finish_bfs << endl;

    //dijkstra
    clock_t start_djkstra = clock(); // подсчет времени
    unordered_map<Node*, double> distances_dijkstra = dijkstra(graph.nodes, closest_node); // Поиск пути по алгоритму Дейкстры
    clock_t end_djkstra = clock();
    double distance_to_finish_dijkstra = distances_dijkstra[finish_node];
    cout << "Расстояние до ИТМО по оценке Дейкстры: " << distance_to_finish_dijkstra << endl;

    //a_star
    clock_t start_a_star = clock(); // подсчет времени
    unordered_map<Node*, double> distances_a_star = a_star(graph.nodes, closest_node, finish_node); // Поиск пути по алгоритму A*
    clock_t end_a_star = clock();
    double distance_to_finish_a_star = distances_a_star[finish_node];
    cout << "Расстояние до ИТМО по оценке Дейкстры с эвристикой: " << distance_to_finish_a_star << endl;

    //Вывод времени выполнения алгоритмов
    double time_djkstra = (double)(end_djkstra - start_djkstra) / CLOCKS_PER_SEC;
    double time_a_star = (double)(end_a_star - start_a_star) / CLOCKS_PER_SEC;
    double time_dfs = (double)(end_dfs - start_dfs) / CLOCKS_PER_SEC;
    double time_bfs = (double)(end_bfs - start_bfd) / CLOCKS_PER_SEC;
    cout << "Время выполнения алгоритма DFS: " << time_dfs << endl;
    cout << "Время выполнения алгоритма BFS: " << time_bfs << endl;
    cout << "Время выполнения алгоритма Дейкстры: " << time_djkstra << endl;
    cout << "Время выполнения алгоритма A*: " << time_djkstra << endl;
    
    // Вывод distances
    // for (const auto& pair : distances) {
    //     Node* node = pair.first;
    //     double distance = pair.second;
    //     if (distance != numeric_limits<double>::infinity()) {
    //         cout << "(" << node->lon << ", " << node->lat << "): " << distance << endl;
    //     }
    // }
    // Вывод graph
    // for (const auto& node : graph.nodes) {
    //     cout << node->lon << "," << node->lat << ": ";
    //     for (const auto& neighbor : node->neighbors) {
    //         cout << neighbor.first->lon << "," << neighbor.first->lat << "," << neighbor.second << "; ";
    //     }
    //     cout << endl;
    // }
    return 0;
}