/*
 * PROJECT: Intelligent Transport Network Management System (ITNMS)
 * COURSE: CS221 - Data Structures & Algorithms
 * MODIFIED: 2025-12-14 by Copilot for Mu6745ty
 *
 * FINAL UPDATED SINGLE FILE:
 * - Preserves original program structure and functionality.
 * - Adds: Add/Delete Station (dynamic), Cycle Detection, Busiest Station, Busiest Route, Prim's MST.
 * - Adds BST removal and clear to keep station directory consistent after station deletes.
 * - Adds history entry for Add Station and Undo support for it.
 * - Uses only <iostream> (no other headers).
 *
 * NOTES:
 * - All data structures implemented manually (no STL except iostream).
 * - Input buffer clearing uses cin.ignore(10000, '\n').
 * - The rest of the previously provided functionality is retained.
 */

#include <iostream>
using namespace std;

// ==========================================
//      PART 0: MANUAL STRING HELPERS
// ==========================================

int my_strlen(const char* str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

void my_strcpy(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int my_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void my_strcat(char* dest, const char* src) {
    int dest_len = my_strlen(dest);
    int i = 0;
    while (src[i] != '\0') {
        dest[dest_len + i] = src[i];
        i++;
    }
    dest[dest_len + i] = '\0';
}

// Manual integer-to-string (handles negatives)
void my_itoa(int num, char* str) {
    int i = 0;
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    bool negative = false;
    long long n = num;
    if (n < 0) { negative = true; n = -n; }
    while (n != 0) {
        int rem = n % 10;
        str[i++] = rem + '0';
        n /= 10;
    }
    if (negative) str[i++] = '-';
    str[i] = '\0';
    int start = 0, end = i - 1;
    while (start < end) {
        char t = str[start]; str[start] = str[end]; str[end] = t;
        start++; end--;
    }
}

// ==========================================
//        PART 1: DATA STRUCTURES
// ==========================================

// --- Ticket Queue ---

struct Passenger {
    int id;
    char name[50];
};

struct QNode {
    Passenger data;
    QNode* next;
};

class TicketQueue {
private:
    QNode* front;
    QNode* rear;
public:
    TicketQueue() : front(nullptr), rear(nullptr) {}
    ~TicketQueue() {
        while (!isEmpty()) dequeue();
    }

    bool isEmpty() { return front == nullptr; }

    void enqueue(int id, const char* name) {
        QNode* newNode = new QNode;
        newNode->data.id = id;
        my_strcpy(newNode->data.name, name);
        newNode->next = nullptr;
        if (isEmpty()) front = rear = newNode;
        else { rear->next = newNode; rear = newNode; }
        cout << "[Queue] Passenger added: " << name << endl;
    }

    void dequeue() {
        if (isEmpty()) { cout << "[Queue] No passengers waiting.\n"; return; }
        QNode* temp = front;
        cout << "[Queue] Serving passenger: " << temp->data.name << " (ID: " << temp->data.id << ")\n";
        front = front->next;
        if (front == nullptr) rear = nullptr;
        delete temp;
    }

    void removeLast() {
        if (isEmpty()) return;
        if (front == rear) {
            cout << "[Queue] Removing last passenger: " << front->data.name << " (ID: " << front->data.id << ")\n";
            delete front;
            front = rear = nullptr;
            return;
        }
        QNode* cur = front;
        while (cur->next != rear) cur = cur->next;
        cout << "[Queue] Removing last passenger: " << rear->data.name << " (ID: " << rear->data.id << ")\n";
        delete rear;
        rear = cur;
        rear->next = nullptr;
    }

    void display() {
        if (isEmpty()) { cout << "Queue is Empty.\n"; return; }
        QNode* t = front;
        cout << "--- Passenger Queue ---\n";
        while (t) {
            cout << "ID: " << t->data.id << " | Name: " << t->data.name << endl;
            t = t->next;
        }
        cout << "-----------------------\n";
    }
};

// --- History Stack (Undo) ---

struct StackNode {
    int actionType; // 1=AddVehicle,2=AddPassenger,3=AddRoute,4=AddStation
    int relatedId;
    int src, dest, weight;
    StackNode* next;
};

class HistoryStack {
private:
    StackNode* top;
public:
    HistoryStack() : top(nullptr) {}
    ~HistoryStack() {
        while (top) {
            StackNode* t = top; top = top->next; delete t;
        }
    }
    bool isEmpty() { return top == nullptr; }
    void push(int type, int id, int s = -1, int d = -1, int w = -1) {
        StackNode* n = new StackNode;
        n->actionType = type; n->relatedId = id; n->src = s; n->dest = d; n->weight = w;
        n->next = top; top = n;
    }
    StackNode* pop() {
        if (!top) return nullptr;
        StackNode* t = top; top = top->next; return t;
    }
};

// --- Vehicle Hash Table ---

struct Vehicle {
    int id;
    char type[30];
    int capacity;
};

struct HashNode {
    Vehicle data;
    HashNode* next;
};

class VehicleHashTable {
private:
    HashNode** table;
    int size;
    int hashFunction(int key) {
        if (key < 0) key = -key;
        return key % size;
    }
public:
    VehicleHashTable(int tableSize) : size(tableSize) {
        table = new HashNode*[size];
        for (int i = 0; i < size; ++i) table[i] = nullptr;
    }
    ~VehicleHashTable() {
        for (int i = 0; i < size; ++i) {
            HashNode* cur = table[i];
            while (cur) { HashNode* nx = cur->next; delete cur; cur = nx; }
        }
        delete[] table;
    }

    void insert(int id, const char* type, int cap) {
        int idx = hashFunction(id);
        HashNode* n = new HashNode;
        n->data.id = id; my_strcpy(n->data.type, type); n->data.capacity = cap;
        n->next = table[idx]; table[idx] = n;
        cout << "[Hash] Vehicle " << id << " inserted.\n";
    }

    void remove(int id) {
        int idx = hashFunction(id);
        HashNode* cur = table[idx]; HashNode* prev = nullptr;
        while (cur) {
            if (cur->data.id == id) {
                if (!prev) table[idx] = cur->next; else prev->next = cur->next;
                delete cur; cout << "[Hash] Vehicle " << id << " removed.\n"; return;
            }
            prev = cur; cur = cur->next;
        }
        cout << "[Hash] Vehicle " << id << " not found.\n";
    }

    void search(int id) {
        int idx = hashFunction(id);
        HashNode* cur = table[idx];
        while (cur) {
            if (cur->data.id == id) {
                cout << "Vehicle Found: ID=" << cur->data.id
                     << ", Type=" << cur->data.type
                     << ", Cap=" << cur->data.capacity << endl;
                return;
            }
            cur = cur->next;
        }
        cout << "Vehicle " << id << " not found.\n";
    }

    void display() {
        cout << "--- Vehicle Database ---\n";
        for (int i = 0; i < size; ++i) {
            cout << "[" << i << "]: ";
            HashNode* cur = table[i];
            while (cur) {
                cout << "{" << cur->data.id << "-" << cur->data.type << "} -> ";
                cur = cur->next;
            }
            cout << "NULL\n";
        }
    }

    // Utility to find vehicle with max capacity (used for analytics)
    bool findMaxCapacityVehicle(int &outId, int &outCap) {
        bool found = false; outCap = -1; outId = -1;
        for (int i = 0; i < size; ++i) {
            HashNode* cur = table[i];
            while (cur) {
                if (!found || cur->data.capacity > outCap) {
                    outCap = cur->data.capacity;
                    outId = cur->data.id;
                    found = true;
                }
                cur = cur->next;
            }
        }
        return found;
    }
};

// --- BST for Stations ---

struct TreeNode {
    int stationId;
    char stationName[50];
    TreeNode* left;
    TreeNode* right;
};

class BST {
private:
    TreeNode* root;

    TreeNode* insertRec(TreeNode* node, int id, const char* name) {
        if (!node) {
            TreeNode* n = new TreeNode;
            n->stationId = id; my_strcpy(n->stationName, name);
            n->left = n->right = nullptr; return n;
        }
        if (id < node->stationId) node->left = insertRec(node->left, id, name);
        else if (id > node->stationId) node->right = insertRec(node->right, id, name);
        return node;
    }

    void inorderRec(TreeNode* node) {
        if (!node) return;
        inorderRec(node->left);
        cout << "Station ID: " << node->stationId << " | Name: " << node->stationName << endl;
        inorderRec(node->right);
    }

    TreeNode* findMin(TreeNode* node) {
        while (node && node->left) node = node->left;
        return node;
    }

    TreeNode* removeRec(TreeNode* node, int id) {
        if (!node) return node;
        if (id < node->stationId) node->left = removeRec(node->left, id);
        else if (id > node->stationId) node->right = removeRec(node->right, id);
        else {
            if (!node->left) {
                TreeNode* temp = node->right; delete node; return temp;
            } else if (!node->right) {
                TreeNode* temp = node->left; delete node; return temp;
            }
            TreeNode* temp = findMin(node->right);
            node->stationId = temp->stationId;
            my_strcpy(node->stationName, temp->stationName);
            node->right = removeRec(node->right, temp->stationId);
        }
        return node;
    }

    void clearRec(TreeNode* node) {
        if (!node) return;
        clearRec(node->left);
        clearRec(node->right);
        delete node;
    }

public:
    BST() : root(nullptr) {}
    ~BST() { clear(); }

    void insert(int id, const char* name) { root = insertRec(root, id, name); }

    void remove(int id) { root = removeRec(root, id); }

    void display() {
        cout << "--- Station Directory (BST Inorder) ---\n";
        inorderRec(root);
        cout << "---------------------------------------\n";
    }

    void clear() {
        clearRec(root);
        root = nullptr;
    }
};

// --- MinHeap for Dijkstra (unchanged) ---

struct HeapNode { int v; int dist; };

class MinHeap {
private:
    HeapNode* array;
    int* pos;
    int size;
    int capacity;
public:
    MinHeap(int cap) : size(0), capacity(cap) {
        array = new HeapNode[cap];
        pos = new int[cap];
    }
    ~MinHeap() { delete[] array; delete[] pos; }

    void swapNodes(int i, int j) {
        HeapNode t = array[i]; array[i] = array[j]; array[j] = t;
        pos[array[i].v] = i; pos[array[j].v] = j;
    }

    void minHeapify(int idx) {
        int smallest = idx;
        int l = 2*idx + 1, r = 2*idx + 2;
        if (l < size && array[l].dist < array[smallest].dist) smallest = l;
        if (r < size && array[r].dist < array[smallest].dist) smallest = r;
        if (smallest != idx) { swapNodes(smallest, idx); minHeapify(smallest); }
    }

    bool isEmpty() { return size == 0; }

    HeapNode extractMin() {
        if (isEmpty()) { HeapNode e = {-1,-1}; return e; }
        HeapNode rootN = array[0];
        HeapNode last = array[size-1];
        array[0] = last; pos[last.v] = 0; size--; minHeapify(0);
        pos[rootN.v] = capacity; // mark removed
        return rootN;
    }

    void decreaseKey(int v, int dist) {
        int i = pos[v];
        if (i >= capacity) return;
        array[i].dist = dist;
        while (i > 0 && array[i].dist < array[(i-1)/2].dist) {
            swapNodes(i, (i-1)/2);
            i = (i-1)/2;
        }
    }

    bool isInMinHeap(int v) { return pos[v] < size; }

    void initForDijkstra(int numVertices) {
        size = numVertices;
        for (int v = 0; v < numVertices; ++v) {
            array[v].v = v; array[v].dist = 2147483647; pos[v] = v;
        }
    }
};

// ==========================================
//        PART 2: GRAPH MODULE (extended)
// ==========================================

struct AdjNode {
    int dest;
    int weight;
    AdjNode* next;
};

class Graph {
private:
    int numVertices;
    AdjNode** adjList;
    char** stationNames;

public:
    Graph(int v) : numVertices(v) {
        adjList = new AdjNode*[v];
        stationNames = new char*[v];
        for (int i = 0; i < v; ++i) {
            adjList[i] = nullptr;
            stationNames[i] = new char[50];
            char defaultName[30] = "Station ";
            char numStr[10];
            my_itoa(i, numStr);
            my_strcat(defaultName, numStr);
            my_strcpy(stationNames[i], defaultName);
        }
    }

    ~Graph() {
        for (int i = 0; i < numVertices; ++i) {
            AdjNode* cur = adjList[i];
            while (cur) { AdjNode* nx = cur->next; delete cur; cur = nx; }
            delete[] stationNames[i];
        }
        delete[] adjList;
        delete[] stationNames;
    }

    int getNumVertices() { return numVertices; }

    void getStationName(int id, char* buffer) {
        if (id < 0 || id >= numVertices) { buffer[0] = '\0'; return; }
        my_strcpy(buffer, stationNames[id]);
    }

    void setStationName(int id, const char* name) {
        if (id >= 0 && id < numVertices) my_strcpy(stationNames[id], name);
    }

    void addEdge(int src, int dest, int weight) {
        if (src < 0 || src >= numVertices || dest < 0 || dest >= numVertices) {
            cout << "[Graph] Invalid station IDs. Valid range: 0-" << (numVertices - 1) << endl;
            return;
        }
        AdjNode* newNode = new AdjNode; newNode->dest = dest; newNode->weight = weight; newNode->next = adjList[src]; adjList[src] = newNode;
        newNode = new AdjNode; newNode->dest = src; newNode->weight = weight; newNode->next = adjList[dest]; adjList[dest] = newNode;
        cout << "[Graph] Route added: " << src << " <-> " << dest << " (Wt: " << weight << ")\n";
    }

    void removeEdge(int src, int dest) {
        if (src < 0 || src >= numVertices || dest < 0 || dest >= numVertices) return;

        AdjNode* temp = adjList[src]; AdjNode* prev = nullptr;
        while (temp) {
            if (temp->dest == dest) {
                if (!prev) adjList[src] = temp->next; else prev->next = temp->next;
                delete temp; break;
            }
            prev = temp; temp = temp->next;
        }

        temp = adjList[dest]; prev = nullptr;
        while (temp) {
            if (temp->dest == src) {
                if (!prev) adjList[dest] = temp->next; else prev->next = temp->next;
                delete temp; break;
            }
            prev = temp; temp = temp->next;
        }
    }

    void displayNetwork() {
        cout << "--- Transport Network Map ---\n";
        for (int i = 0; i < numVertices; ++i) {
            cout << stationNames[i] << " connects to: ";
            AdjNode* temp = adjList[i];
            while (temp) {
                cout << "[" << stationNames[temp->dest] << "|" << temp->weight << "] ";
                temp = temp->next;
            }
            cout << endl;
        }
    }

    void BFS(int startVertex) {
        if (startVertex < 0 || startVertex >= numVertices) { cout << "Invalid start vertex for BFS.\n"; return; }
        bool* visited = new bool[numVertices];
        for (int i = 0; i < numVertices; ++i) visited[i] = false;
        int* queue = new int[numVertices]; int front = 0, rear = 0;
        visited[startVertex] = true; queue[rear++] = startVertex;
        cout << "BFS Traversal: ";
        while (front < rear) {
            int curr = queue[front++]; cout << stationNames[curr] << " ";
            AdjNode* temp = adjList[curr];
            while (temp) {
                if (!visited[temp->dest]) { visited[temp->dest] = true; queue[rear++] = temp->dest; }
                temp = temp->next;
            }
        }
        cout << endl;
        delete[] visited; delete[] queue;
    }

    void DFSUtil(int v, bool visited[]) {
        visited[v] = true;
        cout << stationNames[v] << " ";
        AdjNode* temp = adjList[v];
        while (temp) {
            if (!visited[temp->dest]) DFSUtil(temp->dest, visited);
            temp = temp->next;
        }
    }

    void DFS(int startVertex) {
        if (startVertex < 0 || startVertex >= numVertices) { cout << "Invalid start vertex for DFS.\n"; return; }
        bool* visited = new bool[numVertices];
        for (int i = 0; i < numVertices; ++i) visited[i] = false;
        cout << "DFS Traversal: ";
        DFSUtil(startVertex, visited);
        cout << endl;
        delete[] visited;
    }

    void Dijkstra(int src, int dest) {
        if (src < 0 || src >= numVertices || dest < 0 || dest >= numVertices) { cout << "Invalid source/destination for Dijkstra.\n"; return; }
        int* dist = new int[numVertices]; int* parent = new int[numVertices];
        MinHeap minHeap(numVertices);
        minHeap.initForDijkstra(numVertices);
        for (int i = 0; i < numVertices; ++i) { dist[i] = 2147483647; parent[i] = -1; }
        dist[src] = 0; minHeap.decreaseKey(src, 0);
        while (!minHeap.isEmpty()) {
            HeapNode minNode = minHeap.extractMin();
            int u = minNode.v;
            if (u < 0 || u >= numVertices) continue;
            AdjNode* crawl = adjList[u];
            while (crawl) {
                int v = crawl->dest;
                if (minHeap.isInMinHeap(v) && dist[u] != 2147483647 &&
                    crawl->weight + dist[u] < dist[v]) {
                    dist[v] = dist[u] + crawl->weight;
                    parent[v] = u;
                    minHeap.decreaseKey(v, dist[v]);
                }
                crawl = crawl->next;
            }
        }
        if (dist[dest] == 2147483647) {
            cout << "No path exists between " << stationNames[src] << " and " << stationNames[dest] << endl;
        } else {
            cout << "Shortest Distance: " << dist[dest] << endl;
            cout << "Path: "; printPath(parent, dest); cout << endl;
        }
        delete[] dist; delete[] parent;
    }

    void printPath(int parent[], int j) {
        if (parent[j] == -1) { cout << stationNames[j]; return; }
        printPath(parent, parent[j]);
        cout << " -> " << stationNames[j];
    }

    // --- NEW: Add Station dynamically (appends to end) ---
    int addStationDynamic(const char* name) {
        int oldN = numVertices;
        int newN = oldN + 1;
        AdjNode** newAdj = new AdjNode*[newN];
        char** newNames = new char*[newN];

        // Copy existing pointers
        for (int i = 0; i < oldN; ++i) {
            newAdj[i] = adjList[i];
            newNames[i] = stationNames[i];
        }
        // Initialize new station
        newAdj[oldN] = nullptr;
        newNames[oldN] = new char[50];
        if (name && my_strlen(name) > 0) my_strcpy(newNames[oldN], name);
        else {
            char defaultName[30] = "Station ";
            char numStr[10]; my_itoa(oldN, numStr); my_strcat(defaultName, numStr);
            my_strcpy(newNames[oldN], defaultName);
        }

        // Replace arrays (do not delete individual strings/nodes since we transferred them)
        delete[] adjList;
        delete[] stationNames;
        adjList = newAdj;
        stationNames = newNames;
        numVertices = newN;
        cout << "[Graph] Added station ID " << (newN - 1) << " Name: " << stationNames[newN - 1] << endl;
        return newN - 1;
    }

    // --- NEW: Delete Station (removes vertex and reindexes) ---
    void deleteStationDynamic(int id) {
        if (id < 0 || id >= numVertices) {
            cout << "[Graph] Invalid station ID for deletion.\n";
            return;
        }

        // 1) Remove all edges from this station and to this station
        AdjNode* cur = adjList[id];
        while (cur) { AdjNode* nx = cur->next; delete cur; cur = nx; }
        adjList[id] = nullptr;

        // Remove references to id in other adjacency lists
        for (int i = 0; i < numVertices; ++i) {
            if (i == id) continue;
            AdjNode* prev = nullptr;
            AdjNode* node = adjList[i];
            while (node) {
                if (node->dest == id) {
                    AdjNode* toDelete = node;
                    if (!prev) adjList[i] = node->next;
                    else prev->next = node->next;
                    node = node->next;
                    delete toDelete;
                } else {
                    prev = node;
                    node = node->next;
                }
            }
        }

        // 2) Build new arrays size-1 and adjust indices > id
        int newN = numVertices - 1;
        AdjNode** newAdj = new AdjNode*[newN];
        char** newNames = new char*[newN];

        // Transfer and reindex
        int idx = 0;
        for (int i = 0; i < numVertices; ++i) {
            if (i == id) { delete[] stationNames[i]; continue; } // skip removed station
            AdjNode* head = adjList[i];
            // Adjust dest indices in linked list
            AdjNode* node = head;
            while (node) {
                if (node->dest > id) node->dest = node->dest - 1;
                node = node->next;
            }
            newAdj[idx] = head;
            newNames[idx] = stationNames[i];
            idx++;
        }

        // Clean up old arrays (but don't delete nodes/strings already moved)
        delete[] adjList;
        delete[] stationNames;

        adjList = newAdj;
        stationNames = newNames;
        numVertices = newN;
        cout << "[Graph] Deleted station ID " << id << " and reindexed remaining stations.\n";
    }

    // --- NEW: Detect cycle in undirected graph ---
    bool isCyclicUtil(int v, bool visited[], int parent) {
        visited[v] = true;
        AdjNode* cur = adjList[v];
        while (cur) {
            if (!visited[cur->dest]) {
                if (isCyclicUtil(cur->dest, visited, v)) return true;
            } else if (cur->dest != parent) {
                return true;
            }
            cur = cur->next;
        }
        return false;
    }

    bool hasCycle() {
        bool* visited = new bool[numVertices];
        for (int i = 0; i < numVertices; ++i) visited[i] = false;
        for (int u = 0; u < numVertices; ++u) {
            if (!visited[u]) {
                if (isCyclicUtil(u, visited, -1)) {
                    delete[] visited; return true;
                }
            }
        }
        delete[] visited; return false;
    }

    // --- NEW: Degree of a vertex ---
    int degree(int v) {
        if (v < 0 || v >= numVertices) return -1;
        int deg = 0;
        AdjNode* cur = adjList[v];
        while (cur) { deg++; cur = cur->next; }
        return deg;
    }

    // --- NEW: Busiest station (highest degree) ---
    int busiestStation() {
        if (numVertices == 0) return -1;
        int best = 0; int bestDeg = degree(0);
        for (int i = 1; i < numVertices; ++i) {
            int d = degree(i);
            if (d > bestDeg) { bestDeg = d; best = i; }
        }
        return best;
    }

    // --- NEW: Busiest route (max weight edge) ---
    void busiestRoute(int &outU, int &outV, int &outW) {
        outU = outV = -1; outW = -1;
        for (int u = 0; u < numVertices; ++u) {
            AdjNode* cur = adjList[u];
            while (cur) {
                int v = cur->dest;
                if (u < v) {
                    if (cur->weight > outW) {
                        outW = cur->weight; outU = u; outV = v;
                    }
                }
                cur = cur->next;
            }
        }
    }

    // --- NEW: Prim's MST (O(V^2) simple implementation) ---
    void PrimMST() {
        if (numVertices <= 0) { cout << "No vertices in graph.\n"; return; }

        int V = numVertices;
        int* parent = new int[V];
        int* key = new int[V];
        bool* inMST = new bool[V];

        for (int i = 0; i < V; ++i) {
            key[i] = 2147483647; // INF
            inMST[i] = false;
            parent[i] = -1;
        }

        key[0] = 0;
        parent[0] = -1;

        for (int count = 0; count < V - 1; ++count) {
            int u = -1;
            int minVal = 2147483647;
            for (int v = 0; v < V; ++v) {
                if (!inMST[v] && key[v] < minVal) {
                    minVal = key[v];
                    u = v;
                }
            }

            if (u == -1) break;
            inMST[u] = true;

            AdjNode* crawl = adjList[u];
            while (crawl) {
                int v = crawl->dest;
                int w = crawl->weight;
                if (!inMST[v] && w < key[v]) {
                    key[v] = w;
                    parent[v] = u;
                }
                crawl = crawl->next;
            }
        }

        cout << "--- Minimum Spanning Tree (Prim) ---\n";
        int totalWeight = 0;
        bool disconnected = false;
        for (int i = 1; i < V; ++i) {
            if (parent[i] == -1) {
                cout << stationNames[i] << " is not connected to MST (parent=-1)\n";
                disconnected = true;
            } else {
                int w = -1;
                AdjNode* cur = adjList[i];
                while (cur) {
                    if (cur->dest == parent[i]) { w = cur->weight; break; }
                    cur = cur->next;
                }
                if (w == -1) {
                    cur = adjList[parent[i]];
                    while (cur) {
                        if (cur->dest == i) { w = cur->weight; break; }
                        cur = cur->next;
                    }
                }
                if (w == -1) w = key[i];
                cout << stationNames[parent[i]] << " - " << stationNames[i] << " : " << w << endl;
                if (w > -1 && w != 2147483647) totalWeight += w;
            }
        }
        if (!disconnected) cout << "Total MST Weight: " << totalWeight << endl;
        else cout << "Graph is disconnected: MST covers only connected components.\n";

        delete[] parent; delete[] key; delete[] inMST;
    }
};

// ==========================================
//      PART 3: SORTING & SEARCHING MODULE
// ==========================================

class SortingModule {
public:
    static void printArray(int arr[], int size) {
        for (int i = 0; i < size; ++i) cout << arr[i] << " ";
        cout << endl;
    }

    static void swap(int* a, int* b) {
        int t = *a; *a = *b; *b = t;
    }

    static int linearSearch(int arr[], int size, int x) {
        for (int i = 0; i < size; ++i) if (arr[i] == x) return i;
        return -1;
    }

    static int binarySearch(int arr[], int l, int r, int x) {
        if (r >= l) {
            int mid = l + (r - l) / 2;
            if (arr[mid] == x) return mid;
            if (arr[mid] > x) return binarySearch(arr, l, mid - 1, x);
            return binarySearch(arr, mid + 1, r, x);
        }
        return -1;
    }

    static void bubbleSort(int arr[], int n) {
        for (int i = 0; i < n - 1; ++i)
            for (int j = 0; j < n - i - 1; ++j)
                if (arr[j] > arr[j + 1]) swap(&arr[j], &arr[j + 1]);
    }

    static void selectionSort(int arr[], int n) {
        for (int i = 0; i < n - 1; ++i) {
            int min_idx = i;
            for (int j = i + 1; j < n; ++j) if (arr[j] < arr[min_idx]) min_idx = j;
            swap(&arr[min_idx], &arr[i]);
        }
    }

    static void insertionSort(int arr[], int n) {
        for (int i = 1; i < n; ++i) {
            int key = arr[i]; int j = i - 1;
            while (j >= 0 && arr[j] > key) { arr[j + 1] = arr[j]; j--; }
            arr[j + 1] = key;
        }
    }

    static void merge(int arr[], int l, int m, int r) {
        int n1 = m - l + 1; int n2 = r - m;
        int* L = new int[n1]; int* R = new int[n2];
        for (int i = 0; i < n1; ++i) L[i] = arr[l + i];
        for (int j = 0; j < n2; ++j) R[j] = arr[m + 1 + j];
        int i = 0, j = 0, k = l;
        while (i < n1 && j < n2) {
            if (L[i] <= R[j]) arr[k++] = L[i++]; else arr[k++] = R[j++];
        }
        while (i < n1) arr[k++] = L[i++]; while (j < n2) arr[k++] = R[j++];
        delete[] L; delete[] R;
    }

    static void mergeSort(int arr[], int l, int r) {
        if (l < r) {
            int m = l + (r - l) / 2;
            mergeSort(arr, l, m);
            mergeSort(arr, m + 1, r);
            merge(arr, l, m, r);
        }
    }

    static int partition(int arr[], int low, int high) {
        int pivot = arr[high]; int i = low - 1;
        for (int j = low; j <= high - 1; ++j) {
            if (arr[j] < pivot) { i++; swap(&arr[i], &arr[j]); }
        }
        swap(&arr[i + 1], &arr[high]);
        return i + 1;
    }

    static void quickSort(int arr[], int low, int high) {
        if (low < high) {
            int pi = partition(arr, low, high);
            quickSort(arr, low, pi - 1);
            quickSort(arr, pi + 1, high);
        }
    }

    static void heapify(int arr[], int n, int i) {
        int largest = i; int l = 2 * i + 1; int r = 2 * i + 2;
        if (l < n && arr[l] > arr[largest]) largest = l;
        if (r < n && arr[r] > arr[largest]) largest = r;
        if (largest != i) { swap(&arr[i], &arr[largest]); heapify(arr, n, largest); }
    }

    static void heapSort(int arr[], int n) {
        for (int i = n / 2 - 1; i >= 0; --i) heapify(arr, n, i);
        for (int i = n - 1; i > 0; --i) {
            swap(&arr[0], &arr[i]); heapify(arr, i, 0);
        }
    }
};

// ==========================================
//          PART 4: MAIN SYSTEM (extended)
// ==========================================

class ITNMS {
private:
    Graph* cityGraph;
    TicketQueue ticketQ;
    VehicleHashTable* vehicleDB;
    HistoryStack history;
    BST stationDirectory;
    int stationCount;

    void clearInputBuffer() { cin.ignore(10000, '\n'); }

    void readLine(char* buffer, int bufSize) {
        clearInputBuffer();
        cin.getline(buffer, bufSize);
    }

    // Rebuild BST station directory from current graph station names
    void rebuildStationDirectory() {
        stationDirectory.clear();
        int n = cityGraph->getNumVertices();
        char buf[50];
        for (int i = 0; i < n; ++i) {
            cityGraph->getStationName(i, buf);
            stationDirectory.insert(i, buf);
        }
    }

public:
    ITNMS(int stations) : stationCount(stations) {
        cityGraph = new Graph(stationCount);
        vehicleDB = new VehicleHashTable(10);
    }

    ~ITNMS() { delete cityGraph; delete vehicleDB; }

    void preloadDefaultStations() {
        if (stationCount > 0) cityGraph->setStationName(0, "Central");
        if (stationCount > 1) cityGraph->setStationName(1, "North");
        if (stationCount > 2) cityGraph->setStationName(2, "East");
        if (stationCount > 3) cityGraph->setStationName(3, "West");
        if (stationCount > 4) cityGraph->setStationName(4, "Airport");

        for (int i = 0; i < stationCount && i < 5; ++i) {
            char buf[50];
            if (i == 0) my_strcpy(buf, "Central");
            else if (i == 1) my_strcpy(buf, "North");
            else if (i == 2) my_strcpy(buf, "East");
            else if (i == 3) my_strcpy(buf, "West");
            else my_strcpy(buf, "Airport");
            stationDirectory.insert(i, buf);
        }
    }

    void sortingDemo() {
        int n;
        cout << "Enter number of elements for sorting demo: ";
        if (!(cin >> n) || n <= 0) { cout << "Invalid number\n"; cin.clear(); clearInputBuffer(); return; }
        int* arr = new int[n];
        cout << "Enter " << n << " integers separated by spaces/newlines:\n";
        for (int i = 0; i < n; ++i) cin >> arr[i];
        int sortCh;
        cout << "Choose Sort: 1.Bubble 2.Selection 3.Insertion 4.Merge 5.Quick 6.Heap: ";
        cin >> sortCh;
        int* temp = new int[n];
        for (int i = 0; i < n; ++i) temp[i] = arr[i];
        if (sortCh == 1) SortingModule::bubbleSort(temp, n);
        else if (sortCh == 2) SortingModule::selectionSort(temp, n);
        else if (sortCh == 3) SortingModule::insertionSort(temp, n);
        else if (sortCh == 4) SortingModule::mergeSort(temp, 0, n - 1);
        else if (sortCh == 5) SortingModule::quickSort(temp, 0, n - 1);
        else if (sortCh == 6) SortingModule::heapSort(temp, n);
        else cout << "Invalid choice, showing original array.\n";
        cout << "Sorted Array: "; SortingModule::printArray(temp, n);
        cout << "Enter value to search (Binary Search on sorted array): ";
        int target; cin >> target;
        int res = SortingModule::binarySearch(temp, 0, n - 1, target);
        if (res != -1) cout << "Found at index " << res << "\n"; else cout << "Not Found\n";
        delete[] arr; delete[] temp;
    }

    void undo() {
        StackNode* lastAction = history.pop();
        if (!lastAction) { cout << "Nothing to Undo.\n"; return; }

        if (lastAction->actionType == 1) {
            cout << "Undoing Add Vehicle ID: " << lastAction->relatedId << endl;
            vehicleDB->remove(lastAction->relatedId);
        } else if (lastAction->actionType == 2) {
            cout << "Undoing Add Passenger ID: " << lastAction->relatedId << endl;
            ticketQ.removeLast();
        } else if (lastAction->actionType == 3) {
            cout << "Undoing Route " << lastAction->src << "-" << lastAction->dest << endl;
            cityGraph->removeEdge(lastAction->src, lastAction->dest);
        } else if (lastAction->actionType == 4) {
            int sid = lastAction->relatedId;
            if (sid == cityGraph->getNumVertices() - 1) {
                cityGraph->deleteStationDynamic(sid);
                rebuildStationDirectory();
                cout << "Undid Add Station ID: " << sid << endl;
            } else {
                cout << "Cannot undo Add Station: station index changed since addition.\n";
            }
        }
        delete lastAction;
    }

    void run() {
        preloadDefaultStations();
        int choice;
        do {
            cout << "\n========== ITNMS MENU ==========\n";
            cout << "1.  Add Vehicle\n";
            cout << "2.  Remove Vehicle\n";
            cout << "3.  Find Vehicle\n";
            cout << "4.  Add Passenger to Queue\n";
            cout << "5.  Serve Passenger\n";
            cout << "6.  Add Route (Edge)\n";
            cout << "7.  View Map (Adjacency List)\n";
            cout << "8.  Graph Traversals (BFS/DFS)\n";
            cout << "9.  Shortest Path (Dijkstra)\n";
            cout << "10. View Station Directory (BST)\n";
            cout << "11. Sorting & Searching Demo\n";
            cout << "12. Undo Last Action\n";
            cout << "13. Generate Minimum Spanning Tree (MST - Prim)\n";
            cout << "14. Display Passenger Queue\n";
            cout << "15. Display Vehicle DB\n";
            cout << "16. Add Station\n";
            cout << "17. Delete Station\n";
            cout << "18. Detect Cycles in Network\n";
            cout << "19. Busiest Station (highest degree)\n";
            cout << "20. Busiest Route (max weight)\n";
            cout << "0.  Exit\n";
            cout << "Enter Choice: ";

            if (!(cin >> choice)) {
                if (cin.eof()) { cout << "\nInput closed. Exiting.\n"; break; }
                cin.clear(); clearInputBuffer(); cout << "Invalid input. Enter a number.\n"; continue;
            }

            switch (choice) {
            case 1: {
                int id, cap; char type[30];
                cout << "Enter Vehicle ID: "; cin >> id;
                cout << "Enter Vehicle Type (may include spaces) -> press Enter: ";
                readLine(type, 30);
                if (my_strlen(type) == 0) my_strcpy(type, "Unknown");
                cout << "Enter Capacity: "; cin >> cap;
                vehicleDB->insert(id, type, cap);
                history.push(1, id);
                break;
            }
            case 2: {
                int id; cout << "Enter Vehicle ID to remove: "; cin >> id;
                vehicleDB->remove(id);
                break;
            }
            case 3: {
                int id; cout << "Enter Vehicle ID to search: "; cin >> id;
                vehicleDB->search(id);
                break;
            }
            case 4: {
                int id; char name[50];
                cout << "Enter Passenger ID: "; cin >> id;
                cout << "Enter Name (can include spaces) -> press Enter: ";
                readLine(name, 50);
                if (my_strlen(name) == 0) my_strcpy(name, "Anonymous");
                ticketQ.enqueue(id, name);
                history.push(2, id);
                break;
            }
            case 5: {
                ticketQ.dequeue();
                break;
            }
            case 6: {
                int u, v, w;
                cout << "Enter Source ID (0-" << cityGraph->getNumVertices() - 1 << "): "; cin >> u;
                cout << "Enter Dest ID (0-" << cityGraph->getNumVertices() - 1 << "): "; cin >> v;
                cout << "Enter Weight: "; cin >> w;
                cityGraph->addEdge(u, v, w);
                history.push(3, 0, u, v, w);
                break;
            }
            case 7: {
                cityGraph->displayNetwork();
                break;
            }
            case 8: {
                int start; cout << "Enter Start Node: "; cin >> start;
                cityGraph->BFS(start);
                cityGraph->DFS(start);
                break;
            }
            case 9: {
                int s, d; cout << "Enter Source: "; cin >> s; cout << "Enter Destination: "; cin >> d;
                cityGraph->Dijkstra(s, d);
                break;
            }
            case 10: {
                stationDirectory.display();
                break;
            }
            case 11: {
                sortingDemo();
                break;
            }
            case 12: {
                undo();
                break;
            }
            case 13: {
                cityGraph->PrimMST();
                break;
            }
            case 14: {
                ticketQ.display();
                break;
            }
            case 15: {
                vehicleDB->display();
                break;
            }
            case 16: {
                char name[50];
                cout << "Enter new station name (may include spaces) -> press Enter: ";
                readLine(name, 50);
                int newId = cityGraph->addStationDynamic(name);
                stationDirectory.insert(newId, name);
                history.push(4, newId);
                break;
            }
            case 17: {
                int id; cout << "Enter Station ID to delete: "; cin >> id;
                cityGraph->deleteStationDynamic(id);
                rebuildStationDirectory();
                break;
            }
            case 18: {
                bool cyc = cityGraph->hasCycle();
                if (cyc) cout << "Graph contains a cycle.\n"; else cout << "Graph is acyclic.\n";
                break;
            }
            case 19: {
                int bs = cityGraph->busiestStation();
                if (bs == -1) cout << "No stations available.\n";
                else {
                    char buf[50]; cityGraph->getStationName(bs, buf);
                    cout << "Busiest Station: ID " << bs << " Name: " << buf << " Degree: " << cityGraph->degree(bs) << endl;
                }
                break;
            }
            case 20: {
                int u,v,w; cityGraph->busiestRoute(u,v,w);
                if (u == -1) cout << "No routes present.\n";
                else {
                    char a[50], b[50]; cityGraph->getStationName(u, a); cityGraph->getStationName(v, b);
                    cout << "Busiest Route: " << a << " <-> " << b << " Weight: " << w << endl;
                }
                break;
            }
            case 0: {
                cout << "Exiting...\n"; break;
            }
            default: cout << "Invalid Choice\n";
            }
        } while (choice != 0);
    }
};

// ==========================================
//              PROGRAM ENTRY
// ==========================================

int main() {
    cout << "===== Intelligent Transport Network Management System (ITNMS) =====\n";
    int stations = 5;
    cout << "Enter number of stations to initialize (default 5): ";
    if (!(cin >> stations) || stations <= 0) {
        cout << "Invalid input; defaulting to 5 stations.\n";
        cin.clear();
        cin.ignore(10000, '\n');
        stations = 5;
    }
    ITNMS system(stations);
    system.run();
    return 0;
}