#include <iostream> // Para usar cout (imprimir en pantalla)
#include <vector>   // Para usar arreglos dinámicos (vectores)
#include <list>     // Para listas doblemente enlazadas
#include <utility>  // Para usar 'pair' (pares de datos)
#include <queue>    // Para usar colas y colas de prioridad
#include <climits>  // Para usar INT_MAX (representar el "infinito")
#include <stack>    // Para usar pilas (LIFO)
#include <map>      // Para diccionarios (clave-valor)
#include <string>   // Para usar cadenas de texto

using namespace std; // Evita escribir std:: repetidamente

class GrafoDP {

private:
    int numVertices; // Cantidad total de estaciones en la ciudad
    map<pair<int, int>, string> nombresRutas; // Diccionario que guarda: Par (Origen, Destino) -> Nombre de la calle
    vector<list<pair<int, int>>> adjList; // Lista de adyacencia: Guarda las conexiones y pesos de cada vértice
    
    // Método privado recursivo para buscar todas las rutas posibles (Backtracking)
    void dfsRutas(int current, int fin, vector<int>& path, vector<vector<int>>& rutas, vector<bool>& visited) const;

public:
    GrafoDP(int vertices); // Constructor: inicializa el grafo
    void agregarArista(int origen, int destino, int peso); // Añade una calle entre dos estaciones
    void imprimirGrafo(); // Muestra las conexiones en consola
    void caminoMinimoDijkstra(int origen, int destino); // Calcula la ruta más rápida (Furgón)
    void arbolExpansionMinimaPrim(int inicio); // Diseña la red eléctrica más barata
    int gradoEntrada(int v) const; // Cuenta cuántas calles llegan a una estación
    int gradoSalida(int v) const; // Cuenta cuántas calles salen de una estación
    vector<pair<int, int>> getConexiones(int v) const; // Devuelve las conexiones de una estación
    vector<vector<int>> todasLasRutas(int inicio, int fin) const; // Inicia la búsqueda de rutas (llama a dfsRutas)
    void registrarNombreRuta(int origen, int destino, const string& nombre); // Asigna un nombre a una calle
    string obtenerNombreRuta(int origen, int destino) const; // Devuelve el nombre de una calle

};

int main() {
    GrafoDP g(7); // Crea un grafo con 7 estaciones (E0 a E6)
    
    // Definimos las calles (Origen, Destino, Tiempo/Coste)
    g.agregarArista(0, 1, 4);
    g.agregarArista(0, 2, 6);
    g.agregarArista(1, 3, 7);
    g.agregarArista(2, 4, 8);
    g.agregarArista(3, 4, 3);
    g.agregarArista(4, 5, 10);
    g.agregarArista(4, 6, 12);
    g.agregarArista(5, 6, 2);

    cout << "--- GRAFO DE LA CIUDAD (Estaciones y Calles) ---\n";
    for (int i = 0; i < 7; ++i) { // Recorremos todas las estaciones para imprimirlas
        cout << "Estacion E" << i << ":" << endl;
        cout << "  Conexiones:" << endl;
        auto conexiones = g.getConexiones(i); // Obtenemos a dónde está conectada
        for (auto const& arista : conexiones) { // Imprimimos cada conexión
            cout << "    -> E" << arista.first << " (coste/distancia: " << arista.second << ")" << endl;
        }
        cout << endl;
    }
    
    // Nombramos algunas calles importantes
    g.registrarNombreRuta(0, 1, "Avenida Principal");
    g.registrarNombreRuta(1, 3, "Calle de la Industria");
    g.registrarNombreRuta(3, 4, "Boulevard Central");
    g.registrarNombreRuta(4, 6, "Autovia Norte");
    
    cout << "Rutas posibles del furgon de mantenimiento (E0 a E6):" << endl;
    auto rutas = g.todasLasRutas(0, 6); // Buscamos TODOS los caminos posibles sin repetir
    if (rutas.empty()) {
        cout << "No hay rutas disponibles." << endl;
    } else {
        for (size_t i = 0; i < rutas.size(); ++i) { // Imprimimos ruta por ruta
            cout << "Ruta " << i + 1 << ": ";
            for (size_t j = 0; j < rutas[i].size(); ++j) {
                cout << "E" << rutas[i][j];
                if (j < rutas[i].size() - 1) cout << " -> ";
            }
            cout << endl;
        }
    }
    cout << endl;

    // Ejecutamos algoritmo de Dijkstra (Ruta rápida)
    g.caminoMinimoDijkstra(0, 6);

    // Ejecutamos algoritmo de Prim (Cableado barato)
    g.arbolExpansionMinimaPrim(0);

    return 0; // Fin del programa
}

// Inicializa el grafo dándole tamaño a la lista de adyacencia
GrafoDP::GrafoDP(int vertices) {
    numVertices = vertices;
    adjList.resize(vertices); 
}

// Crea la conexión en ambos sentidos (es una ciudad transitable y un cable eléctrico)
void GrafoDP::agregarArista(int origen, int destino, int peso) {
    adjList[origen].push_back(make_pair(destino, peso)); // Ida
    adjList[destino].push_back(make_pair(origen, peso)); // Vuelta
}

// Recorre el grafo e imprime cada vértice con sus vecinos
void GrafoDP::imprimirGrafo() {
    cout << "**** GRAFO DE LA CIUDAD ****\n";
    for (int i = 0; i < numVertices; ++i) {
        cout << "ESTACION : " << i << "\t Adyacentes : ";
        for (auto const& arista : adjList[i]) {
            cout << arista.first << "(" << arista.second << ") ";
        }
        cout << "\n";
    }
}

// Encuentra el camino de menor coste entre 'origen' y 'destino'
void GrafoDP::caminoMinimoDijkstra(int origen, int destino) {
    vector<int> dist(numVertices, INT_MAX); // Distancias inicializadas a "infinito"
    vector<int> parent(numVertices, -1);    // Rastrea de dónde venimos para reconstruir la ruta
    // Cola de prioridad que ordena automáticamente de menor a mayor coste
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

    dist[origen] = 0; // La distancia al origen es 0
    pq.push(make_pair(0, origen)); // Metemos el origen a la cola

    while (!pq.empty()) { // Mientras haya nodos por revisar
        int u = pq.top().second; // Extraemos el nodo actual
        int d = pq.top().first;  // Extraemos la distancia hasta él
        pq.pop(); // Lo quitamos de la cola

        if (d > dist[u]) continue; // Si encontramos una distancia peor que la ya registrada, la ignoramos

        for (auto const& vecino : adjList[u]) { // Revisamos los vecinos del nodo actual
            int v = vecino.first; 
            int peso = vecino.second;

            // Si ir a través del nodo actual es más barato que la ruta conocida al vecino
            if (dist[u] + peso < dist[v]) {
                dist[v] = dist[u] + peso; // Actualizamos la distancia
                parent[v] = u;            // Decimos que llegamos a 'v' a través de 'u'
                pq.push(make_pair(dist[v], v)); // Lo encolamos para revisarlo luego
            }
        }
    }

    // Imprimir el resultado
    cout << "--- RUTA MAS RAPIDA PARA EL FURGON [E" << origen << " -> E" << destino << "] ---\n";
    if (dist[destino] == INT_MAX) { // Si la distancia sigue siendo "infinito", no hay camino
        cout << "No hay camino disponible.\n\n";
        return;
    }
    
    // Reconstruimos el camino usando una pila (LIFO) para invertir el orden (del destino al origen -> origen al destino)
    stack<int> camino;
    int actual = destino;
    while (actual != -1) {
        camino.push(actual);
        actual = parent[actual]; // Retrocedemos paso a paso
    }

    cout << "ESTACIONES : ";
    while (!camino.empty()) { // Vaciamos la pila imprimiendo
        cout << "E" << camino.top();
        camino.pop();
        if (!camino.empty()) cout << " -> ";
    }
    cout << "\nTIEMPO/COSTE : " << dist[destino] << " min\n\n";
}

// Conecta todas las estaciones con el menor coste total posible sin formar ciclos
void GrafoDP::arbolExpansionMinimaPrim(int inicio) {
    vector<int> costeMinimo(numVertices, INT_MAX); // Coste de conectar cada nodo al árbol
    vector<int> parent(numVertices, -1);           // Qué nodo conecta con qué nodo
    vector<bool> enMST(numVertices, false);        // Marca si un nodo ya es parte de la red (MST)
    // Cola de prioridad para sacar siempre el cable más barato disponible
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

    pq.push(make_pair(0, inicio)); // Empezamos por el nodo inicial con coste 0
    costeMinimo[inicio] = 0;

    while (!pq.empty()) {
        int u = pq.top().second; // Extraemos el nodo más barato de alcanzar
        pq.pop();

        if (enMST[u]) continue; // Si ya está en la red, lo ignoramos
        enMST[u] = true;        // Lo añadimos a la red

        for (auto const& vecino : adjList[u]) { // Revisamos las opciones de conexión desde este nodo
            int v = vecino.first;
            int peso = vecino.second;

            // Si el vecino no está en la red y la conexión actual es más barata que la que conocíamos
            if (!enMST[v] && peso < costeMinimo[v]) {
                costeMinimo[v] = peso;         // Actualizamos el coste
                pq.push(make_pair(costeMinimo[v], v)); // Lo metemos en la cola
                parent[v] = u;                 // Registramos la conexión
            }
        }
    }

    cout << "--- DISENO RED ELECTRICA MAS BARATA (ARBOL DE EXPANSION MINIMA - PRIM) ---\n";
    int costeTotal = 0;
    for (int i = 0; i < numVertices; ++i) { // Recorremos las conexiones establecidas
        if (parent[i] != -1) { // Imprimimos cada cable colocado
            cout << "Cablear Estacion E" << parent[i] << " con Estacion E" << i << " (Coste: " << costeMinimo[i] << ")\n";
            costeTotal += costeMinimo[i]; // Sumamos al presupuesto total
        }
    }
    cout << "COSTE TOTAL DE INFRAESTRUCTURA: " << costeTotal << "\n\n";
}

// Cuenta cuántas conexiones apuntan hacia un nodo específico
int GrafoDP::gradoEntrada(int v) const {
    int count = 0;
    for (int i = 0; i < numVertices; ++i) {
        for (auto const& arista : adjList[i]) {
            if (arista.first == v) count++;
        }
    }
    return count; // Al ser bidireccional, esto será igual al grado de salida
}

// Cuenta cuántas conexiones salen de un nodo específico
int GrafoDP::gradoSalida(int v) const {
    return adjList[v].size(); // Solo contamos el tamaño de su lista de adyacencia
}

// Devuelve las conexiones (pares de Destino-Peso) de un nodo
vector<pair<int, int>> GrafoDP::getConexiones(int v) const {
    return vector<pair<int, int>>(adjList[v].begin(), adjList[v].end());
}

// Algoritmo de Búsqueda en Profundidad (DFS) para encontrar TODOS los caminos
void GrafoDP::dfsRutas(int current, int fin, vector<int>& path, vector<vector<int>>& rutas, vector<bool>& visited) const {
    visited[current] = true; // Marcamos el nodo actual como visitado para no dar vueltas en círculos
    path.push_back(current); // Lo añadimos a la ruta que estamos construyendo

    if (current == fin) { // Si llegamos al destino final...
        rutas.push_back(path); // ...guardamos esta ruta completada en nuestra lista maestra de rutas
    } 
    else { // Si no hemos llegado...
        for (auto const& vecino : adjList[current]) { // ...miramos a dónde más podemos ir
            int next = vecino.first;
            if (!visited[next]) { // Si no hemos pasado ya por ese vecino...
                dfsRutas(next, fin, path, rutas, visited); // ...exploramos esa vía (llamada recursiva)
            }
        }
    }
    
    // Una vez explorada esta vía, hacemos "backtracking" (marcha atrás):
    path.pop_back(); // Quitamos el nodo de la ruta actual
    visited[current] = false; // Lo desmarcamos para que otras rutas puedan usarlo
}

// Prepara las variables y llama al DFS para sacar todas las rutas
vector<vector<int>> GrafoDP::todasLasRutas(int inicio, int fin) const {
    vector<vector<int>> rutas; // Almacenará la lista final de listas de rutas
    vector<int> path;          // Almacena la ruta que se está explorando en el momento
    vector<bool> visited(numVertices, false); // Lleva el control de qué nodos pisamos
    dfsRutas(inicio, fin, path, rutas, visited); // Inicia el proceso recursivo
    return rutas;
}

// Guarda un nombre string asociado a una calle (en ambos sentidos)
void GrafoDP::registrarNombreRuta(int origen, int destino, const string& nombre) {
    nombresRutas[make_pair(origen, destino)] = nombre;
    nombresRutas[make_pair(destino, origen)] = nombre; 
}

// Busca en el diccionario y te devuelve el nombre de la calle, si existe
string GrafoDP::obtenerNombreRuta(int origen, int destino) const {
    auto it = nombresRutas.find(make_pair(origen, destino));
    if (it != nombresRutas.end()) {
        return it->second;
    }
    return ""; // Retorna vacío si no tiene nombre
}
