#include <iostream>
#include <fstream>
#include <sstream> 
#include <functional>
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
    Tree* lastFileNode = nullptr;
    int lastFileIndent = -1;
    bool readingFileContent = false;

    while (getline(file, line)) {
        // Ignorar líneas vacías
        if (line.empty() || line.find_first_not_of(" \t") == string::npos) {
            continue;
        }

        // Calcular el nivel de indentación
        size_t firstChar = line.find_first_not_of(" \t");
        int indentation = firstChar; // Número de espacios o tabulaciones al inicio

        string trimmedLine = line.substr(firstChar); // Línea sin indentación

        if (readingFileContent) {
            // Si la indentación es mayor que la del archivo, es contenido
            if (indentation > lastFileIndent) {
                File* fileData = static_cast<File*>(lastFileNode->data);
                if (!fileData->content.empty()) fileData->content += "\n";
                // Quitar posibles comillas al inicio y final
                if (trimmedLine.front() == '"' && trimmedLine.back() == '"' && trimmedLine.size() > 1)
                    trimmedLine = trimmedLine.substr(1, trimmedLine.size() - 2);
                fileData->content += trimmedLine;
                continue;
            } else {
                // Ya no es contenido, resetea el flag
                readingFileContent = false;
                lastFileNode = nullptr;
                lastFileIndent = -1;
                // No hagas continue, procesa la línea como carpeta o archivo
            }
        }

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
            // Si era contenido de archivo, resetea
            readingFileContent = false;
            lastFileNode = nullptr;
            lastFileIndent = -1;
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
                // Prepara para leer contenido de archivo
                readingFileContent = true;
                lastFileNode = newFileNode;
                lastFileIndent = indentation;
            } else {
                cerr << "Error: Archivo sin carpeta padre: " << fileName << endl;
            }
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

Tree* findFolder(Tree* node, const string& folderName) {
    if (node == nullptr) {
        return nullptr;
    }
    if (node->type == FOLDER_TYPE) {
        Folder* folder = static_cast<Folder*>(node->data);
        if (folder->name == folderName) {
            return node;
        }
    }
    Tree* found = findFolder(node->children, folderName);
    if (found) {
        return found;
    }
    return findFolder(node->next, folderName);
}

void changeDirectory(Tree*& currentFolder, Tree* root, const string& folderName) {
    if (folderName == "..") {
        if (currentFolder->father != nullptr) {
            currentFolder = currentFolder->father;
        } else {
            cout << "Ya estas en la carpeta raiz." << endl;
        }
        return;
    }
    Tree* found = findFolder(root, folderName);
    if (found) {
        currentFolder = found;
    } else {
        cout << "No se encontro la carpeta: " << folderName << endl;
    }
}

bool findNodeAndParent(Tree* node, const string& itemName, Tree*& parent, Tree*& found) {
    if (!node) return false;
    //Tree* prev = nullptr;
    Tree* curr = node->children;
    while (curr) {
        bool match = false;
        if (curr->type == FOLDER_TYPE) {
            Folder* folder = static_cast<Folder*>(curr->data);
            if (folder->name == itemName) match = true;
        } else {
            File* file = static_cast<File*>(curr->data);
            if (file->name == itemName) match = true;
        }
        if (match) {
            parent = node;
            found = curr;
            return true;
        }
        // Buscar recursivamente en hijos
        if (findNodeAndParent(curr, itemName, parent, found)) return true;
        curr = curr->next;
    }
    return false;
}

void eliminateItem(Tree*& root, const string& itemName) {
    Tree* parent = nullptr;
    Tree* found = nullptr;
    if (findNodeAndParent(root, itemName, parent, found)) {
        // Eliminar el nodo encontrado de la lista de hijos de su padre
        Tree* prev = nullptr;
        Tree* curr = parent->children;
        while (curr && curr != found) {
            prev = curr;
            curr = curr->next;
        }
        if (curr == found) {
            if (prev) prev->next = curr->next;
            else parent->children = curr->next;
            if (found->type == FOLDER_TYPE)
                cout << "Carpeta eliminada: " << itemName << endl;
            else
                cout << "Archivo eliminado: " << itemName << endl;
            // Free the data
            if (found->type == FOLDER_TYPE) {
                delete static_cast<Folder*>(found->data);
            } else {
                delete static_cast<File*>(found->data);
            }
            delete found;
        }
    } else {
        cout << "No se encontro el elemento: " << itemName << endl;
    }
}

// IMPLEMENTACION
void renameFolder(Tree* currentFolder, const string& oldName, const string& newName) {
    Tree* child = currentFolder->children;
    while (child != nullptr) {
        if (child->type == FOLDER_TYPE) {
            Folder* folder = static_cast<Folder*>(child->data);
            if (folder->name == oldName) {
                folder->name = newName;
                cout << "Carpeta '" << oldName << "' renombrada a '" << newName << "'" << endl;
                return;
            }
        }
        child = child->next;
    }
    cout << "No se encontro la carpeta '" << oldName << "' en el directorio actual." << endl;
}

// IMPLEMENTACION
void renameFile(Tree* currentFolder, const string& oldName, const string& newName) {
    Tree* child = currentFolder->children;
    while (child != nullptr) {
        if (child->type == FILE_TYPE) {
            File* file = static_cast<File*>(child->data);
            if (file->name == oldName) {
                file->name = newName;
                cout << "Archivo '" << oldName << "' renombrado a '" << newName << "'" << endl;
                return;
            }
        }
        child = child->next;
    }
    cout << "No se encontro el archivo '" << oldName << "' en el directorio actual." << endl;
}
Tree* findChildNode(Tree* parent, const string& itemName) {
    if (!parent || !parent->children) return nullptr;
    Tree* currentChild = parent->children;
    while (currentChild) {
        if (currentChild->type == FOLDER_TYPE) {
            Folder* folder = static_cast<Folder*>(currentChild->data);
            if (folder->name == itemName) {
                return currentChild;
            }
        } else { // FILE_TYPE
            File* file = static_cast<File*>(currentChild->data);
            if (file->name == itemName) {
                return currentChild;
            }
        }
        currentChild = currentChild->next;
    }
    return nullptr;
}
void editFileContent(Tree* currentFolder, const string& fileName) {
    Tree* targetNode = findChildNode(currentFolder, fileName);
    if (targetNode && targetNode->type == FILE_TYPE) {
        File* file = static_cast<File*>(targetNode->data);
        cout << "Contenido actual de '" << file->name << "':" << endl;
        cout << "---START_CONTENT---" << endl;
        cout << file->content << endl;
        cout << "---END_CONTENT---" << endl;
        cout << "Ingrese el nuevo contenido (escriba '---EOF---' en una linea nueva para guardar):" << endl;

        string newContent = "";
        string line;
        cin.ignore(); // Consume the leftover newline character after reading fileName
        while (getline(cin, line)) {
            if (line == "---EOF---") {
                break;
            }
            newContent += line + "\n";
        }
        // Remove the last newline character if it exists
        if (!newContent.empty()) {
            newContent.pop_back();
        }
        file->content = newContent;
        cout << "Contenido de '" << file->name << "' actualizado." << endl;
    } else {
        cout << "No se encontro el archivo '" << fileName << "' en el directorio actual." << endl;
    }
}

void guardarArchivo(Tree* node, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "No se pudo abrir el archivo para guardar." << endl;
        return;
    }

    // Función recursiva auxiliar
    function<void(Tree*, int)> saveNode = [&](Tree* n, int depth) {
        if (!n) return;
        string indent(depth * 4, ' '); // 4 espacios por nivel

        if (n->type == FOLDER_TYPE) {
            Folder* folder = static_cast<Folder*>(n->data);
            file << indent << folder->name << "/" << endl;
            Tree* child = n->children;
            while (child) {
                saveNode(child, depth + 1);
                child = child->next;
            }
        } else { // FILE_TYPE
            File* f = static_cast<File*>(n->data);
            file << indent << f->name << endl;
            if (!f->content.empty()) {
                istringstream iss(f->content);
                string line;
                while (getline(iss, line)) {
                    file << indent << "    ";
                    // Si la línea contiene espacios, comillas o está vacía, la ponemos entre comillas
                    bool needsQuotes = line.find(' ') != string::npos || line.empty() || line.front() == '"' || line.back() == '"';
                    if (needsQuotes) file << '"';
                    file << line;
                    if (needsQuotes) file << '"';
                    file << endl;
                }
            }
        }
    };

    saveNode(node, 0);
    file.close();
}


int main() {
    string filename = "directorio.txt"; 
    Tree* fileSystem = loadFileSystem(filename);
    Tree* root = fileSystem;
    int option = -1; // Variable para controlar el bucle de opciones
    string comando;

    if (fileSystem) {
        while(option){
            cout << "Ingrese un comando (help para ver opciones): "<<endl;
            cout << getCurrentPath(root) << " ";
            cin >> comando;
            if (comando == "help") {
                cout << "Comandos disponibles:" << endl;
                cout << "cd <nombre_carpeta> - Cambiar a la carpeta especificada" << endl;
                cout << "cd .. - Subir un nivel en el directorio" << endl; // Added help for "cd .."
                cout << "ls - Listar archivos y carpetas" << endl;
                cout << "mkdir <nombre_carpeta> - Crear una nueva carpeta" << endl;
                cout << "rm <nombre_archivo_o_carpeta> - Eliminar un archivo o carpeta" << endl;
                cout << "touch <nombre_archivo> - Crear un nuevo archivo" << endl;
                cout << "cnfolder <nombre_carpeta_a_cambiar> <nuevo_nombre> - Cambiar el nombre de una carpeta" << endl;
                cout << "cnfile <nombre_archivo_a_cambiar> <nuevo_nombre> - Cambiar el nombre de un archivo" << endl;
                cout << "exit - Salir del programa" << endl;
            } 
            else if (comando == "cd") {
                string folderName;
                cin >> folderName;
                changeDirectory(root,fileSystem, folderName);
            }
            else if (comando == "ls") {
                if (root) {
                    printFileSystem(root);
                } else {
                    cout << "No hay sistema de archivos cargado." << endl;
                }   
            }
            else if (comando == "mkdir") {
                string folderName;
                cin >> folderName;
                Tree* newFolder = createNode(FOLDER_TYPE, folderName);
                if (root) {
                    // Agregar la nueva carpeta como hijo de la carpeta actual
                    newFolder->father = root;
                    newFolder->next = root->children;
                    root->children = newFolder;
                }
                
            } 
            else if (comando == "rm") {
                string itemName;
                cin >> itemName;
                eliminateItem(root, itemName);
            }
            else if(comando == "touch") {
                string fileName;
                cin >> fileName;   
                Tree* newFile = createNode(FILE_TYPE, fileName);
                if (root) {
                    // Agregar el nuevo archivo como hijo de la carpeta actual
                    newFile->father = root;
                    newFile->next = root->children;
                    root->children = newFile;
                }
            }   
            // IMPLEMENTACION
            else if (comando == "edit") {
                string fileName;
                cin >> fileName;
                editFileContent(root, fileName); // 'root' here is the current working directory
            }
            // IMPLEMENTACION
            else if (comando == "cnfolder") {
                string oldName, newName;
                cin >> oldName >> newName;
                renameFolder(fileSystem, oldName, newName);
            }
            // IMPLEMENTACION
            else if (comando == "cnfile") {
                string oldName, newName;
                cin >> oldName >> newName;
                renameFile(fileSystem, oldName, newName);
            }
            else if (comando == "exit") {
                guardarArchivo(fileSystem, filename); // Implementa esta función para guardar el sistema de archivos
                cout << "Directorio guardado como: " << filename << endl;
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