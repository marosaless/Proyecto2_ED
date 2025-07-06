#include <iostream>
#include <fstream>
#include <string>

using namespace std;

enum Type {
    FILE_TYPE,
    FOLDER_TYPE
};

struct Tree{
    Type type;
    void* data; // Puntero a los datos (puede ser Folder o File)
    Tree* next; // Punter al siguiente nodo en la lista de hermanos
    Tree* father; // Puntero al nodo padre
    Tree* children; // Puntero al primer nodo hijo
};
struct Folder {
    string name;

};
struct File {
    string name;
    string content; // Contenido del archivo
};

// Función auxiliar para crear un nuevo nodo Tree
Tree* createNode(Type type, const string& name, const string& content = "") {
    Tree* newNode = new Tree();
    newNode->type = type;
    newNode->next = nullptr;
    newNode->father = nullptr;
    newNode->children = nullptr;

    if (type == FOLDER_TYPE) {
        Folder* newFolder = new Folder();
        newFolder->name = name;
        newNode->data = newFolder;
    } else { // FILE_TYPE
        File* newFile = new File();
        newFile->name = name;
        newFile->content = content;
        newNode->data = newFile;
    }
    return newNode;
}

// Función principal para cargar el sistema de archivos
Tree* loadFileSystem(string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo de configuración: " << filename << endl;
        return nullptr;
    }

    Tree* root = nullptr;
    Tree* currentFolder = nullptr;
    int currentIndentation = -1; // Para rastrear la profundidad de la indentación

    string line;
    while (getline(file, line)) {
        // Ignorar líneas vacías
        if (line.empty() || line.find_first_not_of(" \t") == string::npos) {
            continue;
        }

        // Calcular el nivel de indentación
        size_t firstChar = line.find_first_not_of(" \t");
        int indentation = firstChar; // Número de espacios o tabulaciones al inicio

        string trimmedLine = line.substr(firstChar); // Línea sin indentación

        // Manejar el contenido de archivos (si tu formato lo permite)
        // Esto es una simplificación; un formato más robusto para el contenido de archivos
        // requeriría una lógica de estado más compleja (ej. "reading_file_content = true")
        //if (currentFolder != nullptr && currentFolder->type == FILE_TYPE && trimmedLine == "---EOF---") 

        if (trimmedLine.back() == '/') { // Es una carpeta
            string folderName = trimmedLine.substr(0, trimmedLine.length() - 1);
            Tree* newFolderNode = createNode(FOLDER_TYPE, folderName);

            if (root == nullptr) { // Es la raíz
                root = newFolderNode;
                currentFolder = root;
                currentIndentation = indentation;
            } else {
                if (indentation > currentIndentation) { // Es un hijo
                    newFolderNode->father = currentFolder;
                    if (currentFolder->children == nullptr) {
                        currentFolder->children = newFolderNode;
                    } else {
                        // Añadir al final de la lista de hermanos
                        Tree* temp = currentFolder->children;
                        while (temp->next != nullptr) {
                            temp = temp->next;
                        }
                        temp->next = newFolderNode;
                    }
                    currentFolder = newFolderNode; // Descender al nuevo directorio
                    currentIndentation = indentation;
                } else if (indentation == currentIndentation) { // Es un hermano
                    newFolderNode->father = currentFolder->father;
                    Tree* temp = currentFolder;
                    while (temp->next != nullptr) {
                        temp = temp->next;
                    }
                    temp->next = newFolderNode;
                    currentFolder = newFolderNode; // El "currentFolder" ahora es este hermano
                } else { // Subir de nivel
                    while (currentIndentation > indentation && currentFolder != nullptr && currentFolder->father != nullptr) {
                        currentFolder = currentFolder->father;
                        currentIndentation -= 4; // Asumiendo 4 espacios por indentación
                    }
                    // Ahora que estamos en el nivel correcto, añadir como hermano
                    newFolderNode->father = currentFolder->father;
                    Tree* temp = currentFolder;
                    while (temp->next != nullptr) {
                        temp = temp->next;
                    }
                    temp->next = newFolderNode;
                    currentFolder = newFolderNode;
                    currentIndentation = indentation;
                }
            }
        } else { // Es un archivo de texto plano
            std::string fileName = trimmedLine;
            Tree* newFileNode = createNode(FILE_TYPE, fileName);

            if (root == nullptr) {
                cerr << "Error: El primer elemento no puede ser un archivo." << endl;
                return nullptr;
            }

            // Un archivo siempre es un hijo de la carpeta actual
            if (currentFolder != nullptr) {
                newFileNode->father = currentFolder;
                if (currentFolder->children == nullptr) {
                    currentFolder->children = newFileNode;
                } else {
                    Tree* temp = currentFolder->children;
                    while (temp->next != nullptr) {
                        temp = temp->next;
                    }
                    temp->next = newFileNode;
                }
            } else {
                cerr << "Error: Archivo sin carpeta padre: " << fileName << endl;
            }

            // Aquí es donde leerías el contenido del archivo si tu formato lo define
            // Por ejemplo, si el contenido está en las siguientes líneas hasta "---EOF---"
            string fileContent = "";
            string contentLine;
            // if (file.peek() == '\n') { // Si la siguiente línea es un salto de línea, podría indicar el inicio del contenido
            //     while (std::getline(file, contentLine) && contentLine != "---EOF---") {
            //         fileContent += contentLine + "\n";
            //     }
            //     static_cast<File*>(newFileNode->data)->content = fileContent;
            // }
        }
    }

    file.close();
    return root;
}
void printFileSystem(Tree* node, int depth = 0) {
    if (node == nullptr) {
        return;
    }

    // Imprimir la indentación
    for (int i = 0; i < depth; ++i) {
        std::cout << "    "; // 4 espacios por nivel de indentación
    }

    if (node->type == FOLDER_TYPE) {
        Folder* folder = static_cast<Folder*>(node->data);
        cout << folder->name << "/" << std::endl;

        // Recorrer los hijos (carpetas y archivos)
        Tree* currentChild = node->children;
        while (currentChild != nullptr) {
            printFileSystem(currentChild, depth + 1); // Llamada recursiva para los hijos
            currentChild = currentChild->next;
        }
    } else { // FILE_TYPE
        File* file = static_cast<File*>(node->data);
        cout << file->name << std::endl;
        // Opcional: imprimir el contenido del archivo si es relevante para la depuración
        // std::cout << "    Content: " << file->content << std::endl;
    }
}

string getCurrentPath(Tree* node) {
    if (!node) return "";
    string path = "";
    while (node){
        Folder* folder = static_cast<Folder*>(node->data);
        path = "/" + folder->name + path;
        node = node->father;
    }
    return path;
}

void changeDirectory(Tree*& currentFolder, const string& folderName) {
    if (currentFolder == nullptr || currentFolder->children == nullptr) {
        cout << "No se puede cambiar de directorio. No hay carpetas disponibles." << endl;
        return;
    }
    Tree* child = currentFolder->children;
    while (child != nullptr) {
        if (child->type == FOLDER_TYPE) {
            Folder* folder = static_cast<Folder*>(child->data);
            if (folder->name == folderName) {
                currentFolder = child;
                return;
            }
        }
        child = child->next;
    }
    cout << "No se encontró la carpeta: " << folderName << endl;
}

int main() {
    string filename = "Prueba.txt"; 
    Tree* fileSystem = loadFileSystem(filename);
    Tree* root = fileSystem;
    int option = -1; // Variable para controlar el bucle de opciones
    string comando;

    if (fileSystem) {
        while(option){
            cout << getCurrentPath(root) << " ";
            cin >> comando;
            if (comando == "cd") {
                string folderName;
                cin >> folderName;
                changeDirectory(root, folderName);
            }
            else if (comando == "ls") {
                if (root) {
                    printFileSystem(root);
                } else {
                    cout << "No hay sistema de archivos cargado." << endl;
                }   
            }
            else if (comando == "exit") {
                option = 0; // Salir del bucle
            } else {
                cout << "Comando no reconocido: " << comando << endl;
            }

        }
        
    } else {
        cout << "Error al cargar el sistema de archivos." << endl;
    }

    // Liberar memoria (no implementado aquí, pero deberías hacerlo)
    return 0;
}
