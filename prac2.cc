// DNI XXXXXXXX APELLIDOS, NOMBRE
#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>
#include <sstream>
#include <ctype.h>

using namespace std;

const int kMAXSTRING = 50; // Maximum size for binary string fields

struct Player {
    unsigned int id;
    string name;
    string team;
    unsigned int dorsal;
    unsigned int position;
    vector<int> ratings;
};

struct BinPlayer {
    unsigned int id;
    char name[kMAXSTRING];
    char team[kMAXSTRING];
    unsigned int dorsal;
    unsigned int position;
    unsigned int numRatings;
};

struct Agency {
    string name;
    vector<Player> players;
    unsigned int nextId;
};

struct BinAgency {
    char name[kMAXSTRING];
    unsigned int nextId;
};

// Tipos de errores posibles
enum Error {
    ERR_ARGS,
    ERR_DORSAL,
    ERR_FILE,
    ERR_NAME,
    ERR_NAME_EXISTS,
    ERR_NO_PLAYERS_WITH_RATINGS,
    ERR_OPTION,
    ERR_PLAYER_NOT_EXISTS,
    ERR_POSITION,
    ERR_RATING,
    ERR_RATING_FORMAT
};

/*
Muestra el mensaje de error correspondiente por pantalla
e: tipo de error a mostrar
*/
void error(Error e) {
    switch (e) {
        case ERR_ARGS:
            cout << "ERROR: wrong arguments" << endl;
            break;
        case ERR_DORSAL:
            cout << "ERROR: wrong dorsal" << endl;
            break;
        case ERR_FILE:
            cout << "ERROR: cannot open file" << endl;
            break;
        case ERR_NAME:
            cout << "ERROR: wrong name" << endl;
            break;
        case ERR_NAME_EXISTS:
            cout << "ERROR: player name already exists" << endl;
            break;
        case ERR_NO_PLAYERS_WITH_RATINGS:
            cout << "ERROR: no players with ratings" << endl;
            break;
        case ERR_OPTION:
            cout << "ERROR: wrong menu option" << endl;
            break;
        case ERR_PLAYER_NOT_EXISTS:
            cout << "ERROR: player does not exist" << endl;
            break;
        case ERR_POSITION:
            cout << "ERROR: wrong position" << endl;
            break;
        case ERR_RATING:
            cout << "ERROR: wrong rating" << endl;
            break;
        case ERR_RATING_FORMAT:
            cout << "ERROR: wrong rating format" << endl;
    }
}

/*
Muestra el menú de importación/exportación por pantalla
*/
void showImportExportMenu() {
    cout << "[Import/Export]" << endl
         << "1- Import from CSV" << endl
         << "2- Export to CSV" << endl
         << "3- Load data" << endl
         << "4- Save data" << endl
         << "b- Back to main menu" << endl
         << "Option: ";
}

/*
Muestra el menú principal por pantalla
*/
void showMainMenu() {
    cout << "[Menu]" << endl
         << "1- Show players" << endl
         << "2- Add player" << endl
         << "3- Add player rating" << endl
         << "4- Delete player" << endl
         << "5- Show ranking" << endl
         << "6- Import/Export" << endl
         << "q- Quit" << endl
         << "Option: ";
}

/*
Convierte el número de posición a su nombre en texto
pos: número de posición (1-5)
return: nombre de la posición como cadena
*/
string positionToString(unsigned int pos) {
    switch (pos) {
        case 1: return "Point Guard";
        case 2: return "Shooting Guard";
        case 3: return "Small Forward";
        case 4: return "Power Forward";
        case 5: return "Center";
        default: return "";
    }
}

/*
Comprueba si una cadena es un nombre válido (jugador o equipo).
Debe tener exactamente dos palabras separadas por un espacio, cada una
con al menos 2 letras, primera en mayúscula y el resto en minúsculas.
s: cadena a validar
return: true si es válida, false en caso contrario
*/
bool isValidName(const string &s) {
    if (s.empty()) {
        return false;
    }

    // Buscar exactamente un espacio
    int spacePos = -1;
    for (unsigned int i = 0; i < s.length(); i++) {
        if (s[i] == ' ') {
            if (spacePos != -1) {
                return false; // más de un espacio
            }
            spacePos = (int)i;
        }
    }
    if (spacePos == -1) {
        return false; // sin espacio
    }

    string first = s.substr(0, spacePos);
    string second = s.substr(spacePos + 1);

    if (first.length() < 2 || second.length() < 2) {
        return false;
    }

    // Primera palabra: primera letra mayúscula, resto minúsculas
    if (!isupper(first[0])) {
        return false;
    }
    for (unsigned int i = 1; i < first.length(); i++) {
        if (!islower(first[i])) {
            return false;
        }
    }

    // Segunda palabra: primera letra mayúscula, resto minúsculas
    if (!isupper(second[0])) {
        return false;
    }
    for (unsigned int i = 1; i < second.length(); i++) {
        if (!islower(second[i])) {
            return false;
        }
    }

    return true;
}

/*
Comprueba si una cadena representa un entero no negativo
s: cadena a comprobar
return: true si solo contiene dígitos y no está vacía, false en caso contrario
*/
bool isUnsignedInt(const string &s) {
    if (s.empty()) {
        return false;
    }
    for (unsigned int i = 0; i < s.length(); i++) {
        if (!isdigit(s[i])) {
            return false;
        }
    }
    return true;
}

/*
Comprueba si ya existe un jugador con el mismo nombre en la agencia
agency: agencia donde buscar
playerName: nombre a buscar
return: true si el nombre ya existe, false en caso contrario
*/
bool nameExists(const Agency &agency, const string &playerName) {
    for (unsigned int i = 0; i < agency.players.size(); i++) {
        if (agency.players[i].name == playerName) {
            return true;
        }
    }
    return false;
}

/*
Lee y valida el nombre de un jugador desde la entrada estándar.
Repite la solicitud hasta obtener un nombre válido y no duplicado.
agency: agencia para comprobar nombres duplicados
playerName: variable donde almacenar el nombre válido
*/
void readPlayerName(const Agency &agency, string &playerName) {
    bool valid = false;
    while (!valid) {
        cout << "Enter player name: ";
        getline(cin, playerName);
        if (!isValidName(playerName)) {
            error(ERR_NAME);
        } else if (nameExists(agency, playerName)) {
            error(ERR_NAME_EXISTS);
        } else {
            valid = true;
        }
    }
}

/*
Lee y valida el nombre de un equipo desde la entrada estándar.
Repite la solicitud hasta obtener un nombre válido.
teamName: variable donde almacenar el nombre válido
*/
void readTeamName(string &teamName) {
    bool valid = false;
    while (!valid) {
        cout << "Enter team name: ";
        getline(cin, teamName);
        if (!isValidName(teamName)) {
            error(ERR_NAME);
        } else {
            valid = true;
        }
    }
}

/*
Lee y valida el dorsal desde la entrada estándar (entre 0 y 99).
Repite la solicitud hasta obtener un valor válido.
dorsal: variable donde almacenar el dorsal válido
*/
void readDorsal(unsigned int &dorsal) {
    bool valid = false;
    while (!valid) {
        cout << "Enter dorsal: ";
        string line;
        getline(cin, line);
        if (!isUnsignedInt(line)) {
            error(ERR_DORSAL);
        } else {
            int val = stoi(line);
            if (val < 0 || val > 99) {
                error(ERR_DORSAL);
            } else {
                dorsal = (unsigned int)val;
                valid = true;
            }
        }
    }
}

/*
Lee y valida la posición desde la entrada estándar (entre 1 y 5).
Repite la solicitud hasta obtener un valor válido.
position: variable donde almacenar la posición válida
*/
void readPosition(unsigned int &position) {
    bool valid = false;
    while (!valid) {
        cout << "Enter position: ";
        string line;
        getline(cin, line);
        if (!isUnsignedInt(line)) {
            error(ERR_POSITION);
        } else {
            int val = stoi(line);
            if (val < 1 || val > 5) {
                error(ERR_POSITION);
            } else {
                position = (unsigned int)val;
                valid = true;
            }
        }
    }
}

/*
Muestra todos los jugadores de la agencia con sus valoraciones por partido
agency: agencia con los jugadores a mostrar
*/
void showPlayers(const Agency &agency) {
    for (unsigned int i = 0; i < agency.players.size(); i++) {
        const Player &p = agency.players[i];
        cout << "Id: " << p.id << endl;
        cout << "Name: " << p.name << endl;
        cout << "Team: " << p.team << endl;
        cout << "Dorsal: " << p.dorsal << endl;
        cout << "Position: " << positionToString(p.position) << endl;
        if (!p.ratings.empty()) {
            cout << "Game | Rating" << endl;
            for (unsigned int j = 0; j < p.ratings.size(); j++) {
                cout << j + 1 << " | " << p.ratings[j] << endl;
            }
        }
        if (i < agency.players.size() - 1) {
            cout << endl;
        }
    }
}

/*
Añade un nuevo jugador a la agencia solicitando sus datos al usuario.
El id se asigna automáticamente a partir del campo nextId de la agencia.
agency: agencia donde se añadirá el jugador
*/
void addPlayer(Agency &agency) {
    Player p;
    readPlayerName(agency, p.name);
    readTeamName(p.team);
    readDorsal(p.dorsal);
    readPosition(p.position);
    p.id = agency.nextId;
    agency.nextId++;
    agency.players.push_back(p);
}

/*
Busca un jugador por su identificador en la agencia
agency: agencia donde buscar
id: identificador del jugador
return: índice del jugador en el vector, o -1 si no existe
*/
int findPlayerById(const Agency &agency, unsigned int id) {
    for (unsigned int i = 0; i < agency.players.size(); i++) {
        if (agency.players[i].id == id) {
            return (int)i;
        }
    }
    return -1;
}

/*
Lee un id de jugador desde la entrada estándar y busca al jugador.
Muestra ERR_PLAYER_NOT_EXISTS si la entrada es inválida o el jugador no existe.
agency: agencia donde buscar
return: índice del jugador en el vector, o -1 si no existe o la entrada es inválida
*/
int readAndFindPlayerId(const Agency &agency) {
    cout << "Enter player id: ";
    string idLine;
    getline(cin, idLine);

    if (!isUnsignedInt(idLine)) {
        error(ERR_PLAYER_NOT_EXISTS);
        return -1;
    }

    unsigned int id = (unsigned int)stoi(idLine);
    int idx = findPlayerById(agency, id);
    if (idx == -1) {
        error(ERR_PLAYER_NOT_EXISTS);
    }
    return idx;
}

/*
Parsea y valida una cadena de valoraciones separadas por comas.
line: cadena con las valoraciones
ratings: vector donde se almacenan las valoraciones parseadas
return: 0 si correcto, 1 si error de formato, 2 si error de rango
*/
int parseRatings(const string &line, vector<int> &ratings) {
    if (line.empty()) {
        return 1;
    }
    if (line[line.length() - 1] == ',') {
        return 1;
    }

    ratings.clear();
    stringstream ss(line);
    string token;
    while (getline(ss, token, ',')) {
        if (token.empty()) {
            return 1;
        }
        // Validar token: signo opcional seguido de dígitos
        unsigned int start = 0;
        if (token[0] == '-') {
            start = 1;
        }
        if (start >= token.length()) {
            return 1;
        }
        for (unsigned int i = start; i < token.length(); i++) {
            if (!isdigit(token[i])) {
                return 1;
            }
        }
        int val = stoi(token);
        if (val < -50 || val > 50) {
            return 2;
        }
        ratings.push_back(val);
    }
    return 0;
}

/*
Añade valoraciones a un jugador existente solicitando el id y los valores al usuario.
agency: agencia con los jugadores
*/
void addPlayerRating(Agency &agency) {
    int idx = readAndFindPlayerId(agency);
    if (idx == -1) {
        return;
    }

    vector<int> newRatings;
    bool valid = false;
    while (!valid) {
        cout << "Enter ratings (comma-separated): ";
        string line;
        getline(cin, line);
        int result = parseRatings(line, newRatings);
        if (result == 1) {
            error(ERR_RATING_FORMAT);
        } else if (result == 2) {
            error(ERR_RATING);
        } else {
            valid = true;
        }
    }

    for (unsigned int i = 0; i < newRatings.size(); i++) {
        agency.players[idx].ratings.push_back(newRatings[i]);
    }
}

/*
Elimina un jugador de la agencia según el id introducido por el usuario
agency: agencia de la que se eliminará el jugador
*/
void deletePlayer(Agency &agency) {
    int idx = readAndFindPlayerId(agency);
    if (idx == -1) {
        return;
    }
    agency.players.erase(agency.players.begin() + idx);
}

/*
Calcula la valoración media de un jugador multiplicada por 10 (para evitar floats),
redondeada al entero más cercano.
ratings: vector de valoraciones del jugador
return: media * 10 redondeada como entero
*/
int calcAvgTimes10(const vector<int> &ratings) {
    int sum = 0;
    for (unsigned int i = 0; i < ratings.size(); i++) {
        sum += ratings[i];
    }
    int n = (int)ratings.size();
    int quotient = (sum * 10) / n;
    int remainder = (sum * 10) % n;

    // Redondeo hacia el entero más cercano
    if (remainder < 0) {
        remainder = -remainder;
    }
    if (remainder * 2 >= n) {
        if (sum >= 0) {
            quotient++;
        } else {
            quotient--;
        }
    }
    return quotient;
}

/*
Imprime una media almacenada como entero*10 con formato de 1 decimal.
Si el decimal es 0, no se muestra el punto.
avgTimes10: valor de la media multiplicado por 10
*/
void printAvg(int avgTimes10) {
    int intPart = avgTimes10 / 10;
    int decPart = avgTimes10 % 10;
    if (decPart < 0) {
        decPart = -decPart;
    }
    if (decPart == 0) {
        cout << intPart;
    } else {
        cout << intPart << "." << decPart;
    }
}

/*
Muestra el ranking de jugadores con al menos una valoración,
ordenados de mayor a menor valoración media.
agency: agencia con los jugadores
*/
void showRanking(const Agency &agency) {
    // Recoger índices de jugadores con al menos una valoración
    vector<unsigned int> indices;
    for (unsigned int i = 0; i < agency.players.size(); i++) {
        if (!agency.players[i].ratings.empty()) {
            indices.push_back(i);
        }
    }
    if (indices.empty()) {
        error(ERR_NO_PLAYERS_WITH_RATINGS);
        return;
    }

    // Ordenar por media descendente con algoritmo de burbuja
    for (unsigned int i = 0; i < indices.size() - 1; i++) {
        for (unsigned int j = 0; j < indices.size() - 1 - i; j++) {
            int avgA = calcAvgTimes10(agency.players[indices[j]].ratings);
            int avgB = calcAvgTimes10(agency.players[indices[j + 1]].ratings);
            if (avgA < avgB) {
                unsigned int tmp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = tmp;
            }
        }
    }

    cout << "Ranking | Id | Name | Dorsal | Position | Rating" << endl;
    for (unsigned int r = 0; r < indices.size(); r++) {
        const Player &p = agency.players[indices[r]];
        cout << r + 1 << " | " << p.id << " | " << p.name << " | "
             << p.dorsal << " | " << positionToString(p.position) << " | ";
        printAvg(calcAvgTimes10(p.ratings));
        cout << endl;
    }
}

/*
Valida un campo de dorsal leído desde CSV
dorsalStr: cadena con el dorsal a validar
dorsal: variable donde se almacena el valor si es válido
return: true si es válido, false en caso contrario
*/
bool validateCsvDorsal(const string &dorsalStr, unsigned int &dorsal) {
    if (!isUnsignedInt(dorsalStr)) {
        error(ERR_DORSAL);
        return false;
    }
    int val = stoi(dorsalStr);
    if (val < 0 || val > 99) {
        error(ERR_DORSAL);
        return false;
    }
    dorsal = (unsigned int)val;
    return true;
}

/*
Valida un campo de posición leído desde CSV
posStr: cadena con la posición a validar
position: variable donde se almacena el valor si es válido
return: true si es válido, false en caso contrario
*/
bool validateCsvPosition(const string &posStr, unsigned int &position) {
    if (!isUnsignedInt(posStr)) {
        error(ERR_POSITION);
        return false;
    }
    int val = stoi(posStr);
    if (val < 1 || val > 5) {
        error(ERR_POSITION);
        return false;
    }
    position = (unsigned int)val;
    return true;
}

/*
Valida y parsea las valoraciones de una línea CSV a partir del índice dado
fields: campos de la línea CSV
startIdx: índice del primer campo de valoración
ratings: vector donde se almacenan las valoraciones válidas
return: true si todas las valoraciones son válidas, false en caso contrario
*/
bool validateCsvRatings(const vector<string> &fields,
                         unsigned int startIdx,
                         vector<int> &ratings) {
    for (unsigned int i = startIdx; i < fields.size(); i++) {
        const string &rStr = fields[i];
        unsigned int start = 0;
        if (!rStr.empty() && rStr[0] == '-') {
            start = 1;
        }
        if (rStr.empty() || start >= rStr.length()) {
            error(ERR_RATING);
            return false;
        }
        for (unsigned int j = start; j < rStr.length(); j++) {
            if (!isdigit(rStr[j])) {
                error(ERR_RATING);
                return false;
            }
        }
        int rVal = stoi(rStr);
        if (rVal < -50 || rVal > 50) {
            error(ERR_RATING);
            return false;
        }
        ratings.push_back(rVal);
    }
    return true;
}

/*
Importa jugadores desde un fichero CSV a la agencia.
Los jugadores con datos inválidos se ignoran y se muestra el error correspondiente.
agency: agencia donde se añaden los jugadores
filename: nombre del fichero CSV a importar
*/
void importFromCSV(Agency &agency, const string &filename) {
    if (filename.empty()) {
        error(ERR_FILE);
        return;
    }
    ifstream file(filename);
    if (!file.is_open()) {
        error(ERR_FILE);
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        // Separar campos por coma
        vector<string> fields;
        stringstream ss(line);
        string token;
        while (getline(ss, token, ',')) {
            fields.push_back(token);
        }
        if (fields.size() < 4) {
            continue;
        }

        // Validar nombre del jugador
        string pName = fields[0];
        if (!isValidName(pName)) {
            error(ERR_NAME);
            continue;
        }
        if (nameExists(agency, pName)) {
            error(ERR_NAME_EXISTS);
            continue;
        }

        // Validar nombre del equipo
        string pTeam = fields[1];
        if (!isValidName(pTeam)) {
            error(ERR_NAME);
            continue;
        }

        // Validar dorsal y posición
        unsigned int dorsal = 0;
        if (!validateCsvDorsal(fields[2], dorsal)) {
            continue;
        }
        unsigned int position = 0;
        if (!validateCsvPosition(fields[3], position)) {
            continue;
        }

        // Validar valoraciones (opcionales)
        vector<int> ratings;
        if (!validateCsvRatings(fields, 4, ratings)) {
            continue;
        }

        Player p;
        p.name = pName;
        p.team = pTeam;
        p.dorsal = dorsal;
        p.position = position;
        p.ratings = ratings;
        p.id = agency.nextId;
        agency.nextId++;
        agency.players.push_back(p);
    }
    file.close();
}

/*
Exporta todos los jugadores de la agencia a un fichero CSV.
Formato por línea: name,team,dorsal,position[,rating1,rating2,...].
Los ids no se incluyen en la exportación.
agency: agencia con los jugadores a exportar
*/
void exportToCSV(const Agency &agency) {
    cout << "Enter filename: ";
    string filename;
    getline(cin, filename);

    if (filename.empty()) {
        error(ERR_FILE);
        return;
    }
    ofstream file(filename);
    if (!file.is_open()) {
        error(ERR_FILE);
        return;
    }

    for (unsigned int i = 0; i < agency.players.size(); i++) {
        const Player &p = agency.players[i];
        file << p.name << "," << p.team << "," << p.dorsal << "," << p.position;
        for (unsigned int j = 0; j < p.ratings.size(); j++) {
            file << "," << p.ratings[j];
        }
        file << endl;
    }
    file.close();
}

/*
Carga los datos de la agencia desde un fichero binario.
Los datos actuales se borran y se sustituyen por los del fichero.
Si el fichero no puede abrirse, los datos no se modifican.
agency: agencia donde se cargan los datos
filename: nombre del fichero binario
*/
void loadDataFromFile(Agency &agency, const string &filename) {
    if (filename.empty()) {
        error(ERR_FILE);
        return;
    }
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        error(ERR_FILE);
        return;
    }

    agency.players.clear();

    BinAgency ba;
    file.read((char*)&ba, sizeof(BinAgency));
    agency.name = ba.name;
    agency.nextId = ba.nextId;

    BinPlayer bp;
    while (file.read((char*)&bp, sizeof(BinPlayer))) {
        Player p;
        p.id = bp.id;
        p.name = bp.name;
        p.team = bp.team;
        p.dorsal = bp.dorsal;
        p.position = bp.position;
        for (unsigned int i = 0; i < bp.numRatings; i++) {
            int rating;
            file.read((char*)&rating, sizeof(int));
            p.ratings.push_back(rating);
        }
        agency.players.push_back(p);
    }
    file.close();
}

/*
Solicita confirmación al usuario y carga los datos desde un fichero binario.
Si el usuario no confirma con 'y' o 'Y', no se realiza ninguna acción.
agency: agencia donde se cargarán los datos
*/
void loadData(Agency &agency) {
    bool decided = false;
    bool confirmed = false;
    while (!decided) {
        cout << "All data will be erased. Continue? [y/n]: ";
        string line;
        getline(cin, line);
        if (line == "y" || line == "Y") {
            confirmed = true;
            decided = true;
        } else if (line == "n" || line == "N") {
            decided = true;
        }
        // Cualquier otro valor: repetir la pregunta
    }
    if (!confirmed) {
        return;
    }

    cout << "Enter filename: ";
    string filename;
    getline(cin, filename);
    loadDataFromFile(agency, filename);
}

/*
Guarda todos los datos de la agencia en un fichero binario.
Formato: BinAgency seguido de pares (BinPlayer + valoraciones) por cada jugador.
agency: agencia cuyos datos se guardarán
*/
void saveData(const Agency &agency) {
    cout << "Enter filename: ";
    string filename;
    getline(cin, filename);

    if (filename.empty()) {
        error(ERR_FILE);
        return;
    }
    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        error(ERR_FILE);
        return;
    }

    // Escribir cabecera BinAgency
    BinAgency ba;
    memset(&ba, 0, sizeof(BinAgency));
    strncpy(ba.name, agency.name.c_str(), kMAXSTRING - 1);
    ba.name[kMAXSTRING - 1] = '\0';
    ba.nextId = agency.nextId;
    file.write((char*)&ba, sizeof(BinAgency));

    // Escribir cada jugador seguido de sus valoraciones
    for (unsigned int i = 0; i < agency.players.size(); i++) {
        const Player &p = agency.players[i];
        BinPlayer bp;
        memset(&bp, 0, sizeof(BinPlayer));
        bp.id = p.id;
        strncpy(bp.name, p.name.c_str(), kMAXSTRING - 1);
        bp.name[kMAXSTRING - 1] = '\0';
        strncpy(bp.team, p.team.c_str(), kMAXSTRING - 1);
        bp.team[kMAXSTRING - 1] = '\0';
        bp.dorsal = p.dorsal;
        bp.position = p.position;
        bp.numRatings = p.ratings.size();
        file.write((char*)&bp, sizeof(BinPlayer));

        for (unsigned int j = 0; j < p.ratings.size(); j++) {
            int rating = p.ratings[j];
            file.write((char*)&rating, sizeof(int));
        }
    }
    file.close();
}

/*
Gestiona el menú de importación/exportación y llama a la función
correspondiente según la opción elegida por el usuario.
agency: agencia sobre la que se operará
*/
void importExportMenu(Agency &agency) {
    char option;
    do {
        showImportExportMenu();
        cin >> option;
        cin.get();

        switch (option) {
            case '1': {
                cout << "Enter filename: ";
                string filename;
                getline(cin, filename);
                importFromCSV(agency, filename);
                break;
            }
            case '2':
                exportToCSV(agency);
                break;
            case '3':
                loadData(agency);
                break;
            case '4':
                saveData(agency);
                break;
            case 'b':
                break;
            default:
                error(ERR_OPTION);
        }
    } while (option != 'b');
}

/*
Procesa los argumentos de línea de comandos.
Admite -l <fichero> para cargar datos binarios y -i <fichero> para importar CSV.
Siempre se procesa primero -l y después -i, independientemente del orden dado.
argc: número de argumentos
argv: array de argumentos
agency: agencia a rellenar con los datos cargados
return: true si los argumentos son correctos, false si hay algún error
*/
bool processArgs(int argc, char *argv[], Agency &agency) {
    string fileL = "";
    string fileI = "";
    bool hasL = false;
    bool hasI = false;

    int i = 1;
    while (i < argc) {
        string arg = argv[i];
        if (arg == "-l") {
            if (hasL || i + 1 >= argc) {
                error(ERR_ARGS);
                return false;
            }
            hasL = true;
            fileL = argv[i + 1];
            i += 2;
        } else if (arg == "-i") {
            if (hasI || i + 1 >= argc) {
                error(ERR_ARGS);
                return false;
            }
            hasI = true;
            fileI = argv[i + 1];
            i += 2;
        } else {
            error(ERR_ARGS);
            return false;
        }
    }

    // Procesar primero -l y luego -i
    if (hasL) {
        loadDataFromFile(agency, fileL);
    }
    if (hasI) {
        importFromCSV(agency, fileI);
    }
    return true;
}

/*
Función principal: inicializa la agencia, procesa los argumentos de línea de
comandos y gestiona el bucle del menú principal.
argc: número de argumentos de línea de comandos
argv: array de argumentos de línea de comandos
return: 0
*/
int main(int argc, char *argv[]) {
    Agency agency;
    agency.name = "ProSports Agency";
    agency.nextId = 1;

    if (argc > 1) {
        if (!processArgs(argc, argv, agency)) {
            return 0;
        }
    }

    char option;
    do {
        showMainMenu();
        cin >> option;
        cin.get();

        switch (option) {
            case '1':
                showPlayers(agency);
                break;
            case '2':
                addPlayer(agency);
                break;
            case '3':
                addPlayerRating(agency);
                break;
            case '4':
                deletePlayer(agency);
                break;
            case '5':
                showRanking(agency);
                break;
            case '6':
                importExportMenu(agency);
                break;
            case 'q':
                break;
            default:
                error(ERR_OPTION);
        }
    } while (option != 'q');

    return 0;
}
