#include <iostream>
#include <fstream>
#include <string>
using namespace std;

struct RegistroVariable {
    string nombre;
    int edad;
    double salario;
};

void escribirRegistroVariable(const RegistroVariable& registro, ofstream& archivo) {
    int longitudNombre = registro.nombre.length();
    archivo.write(reinterpret_cast<const char*>(&longitudNombre), sizeof(int));
    archivo.write(registro.nombre.c_str(), longitudNombre);
    archivo.write(reinterpret_cast<const char*>(&registro.edad), sizeof(int));
    archivo.write(reinterpret_cast<const char*>(&registro.salario), sizeof(double));
}

RegistroVariable leerRegistroVariable(ifstream& archivo) {
    RegistroVariable registro;
    int longitudNombre;
    archivo.read(reinterpret_cast<char*>(&longitudNombre), sizeof(int));
    char* nombreBuffer = new char[longitudNombre + 1];
    archivo.read(nombreBuffer, longitudNombre);
    nombreBuffer[longitudNombre] = '\0';
    registro.nombre = nombreBuffer;
    delete[] nombreBuffer;
    archivo.read(reinterpret_cast<char*>(&registro.edad), sizeof(int));
    archivo.read(reinterpret_cast<char*>(&registro.salario), sizeof(double));
    return registro;
}

void almacenarRegistroVariable(const RegistroVariable& registro, const string& archivoNombre) {
    ofstream archivo(archivoNombre, ios::binary | ios::app);
    if (archivo.is_open()) {
        escribirRegistroVariable(registro, archivo);
        archivo.close();
        cout << "Registro almacenado con éxito." << endl;
    } else {
        cout << "No se pudo abrir el archivo." << endl;
    }
}

void recuperarRegistrosVariables(const string& archivoNombre) {
    ifstream archivo(archivoNombre, ios::binary);
    if (archivo.is_open()) {
        RegistroVariable registro;
        while (!archivo.eof()) {
            registro = leerRegistroVariable(archivo);
            if (!archivo.eof()) {
                cout << "Nombre: " << registro.nombre << endl;
                cout << "Edad: " << registro.edad << endl;
                cout << "Salario: " << registro.salario << endl;
                cout << endl;
            }
        }
        archivo.close();
    } else {
        cout << "No se pudo abrir el archivo." << endl;
    }
}

void eliminarRegistroVariable(const string& nombre, const string& archivoNombre) {
    ifstream archivoEntrada(archivoNombre, ios::binary);
    ofstream archivoTemporal("temp.bin", ios::binary);
    if (archivoEntrada.is_open() && archivoTemporal.is_open()) {
        RegistroVariable registro;
        bool encontrado = false;
        while (!archivoEntrada.eof()) {
            registro = leerRegistroVariable(archivoEntrada);
            if (!archivoEntrada.eof()) {
                if (registro.nombre != nombre) {
                    escribirRegistroVariable(registro, archivoTemporal);
                } else {
                    encontrado = true;
                }
            }
        }
        archivoEntrada.close();
        archivoTemporal.close();
        remove(archivoNombre.c_str());
        rename("temp.bin", archivoNombre.c_str());
        if (encontrado) {
            cout << "Registro eliminado con éxito." << endl;
        } else {
            cout << "No se encontró el registro." << endl;
        }
    } else {
        cout << "No se pudo abrir el archivo." << endl;
    }
}

void actualizarRegistroVariable(const string& nombre, const RegistroVariable& nuevoRegistro, const string& archivoNombre) {
    ifstream archivoEntrada(archivoNombre, ios::binary);
    ofstream archivoTemporal("temp.bin", ios::binary);
    if (archivoEntrada.is_open() && archivoTemporal.is_open()) {
        RegistroVariable registro;
        bool encontrado = false;
        while (!archivoEntrada.eof()) {
            registro = leerRegistroVariable(archivoEntrada);
            if (!archivoEntrada.eof()) {
                if (registro.nombre != nombre) {
                    escribirRegistroVariable(registro, archivoTemporal);
                } else {
                    escribirRegistroVariable(nuevoRegistro, archivoTemporal);
                    encontrado = true;
                }
            }
        }
        archivoEntrada.close();
        archivoTemporal.close();
        remove(archivoNombre.c_str());
        rename("temp.bin", archivoNombre.c_str());
        if (encontrado) {
            cout << "Registro actualizado con éxito." << endl;
        } else {
            cout << "No se encontró el registro." << endl;
        }
    } else {
        cout << "No se pudo abrir el archivo." << endl;
    }
}

void buscarRegistroVariable(const string& nombre, const string& archivoNombre) {
    ifstream archivo(archivoNombre, ios::binary);
    if (archivo.is_open()) {
        RegistroVariable registro;
        bool encontrado = false;
        while (!archivo.eof()) {
            registro = leerRegistroVariable(archivo);
            if (!archivo.eof()) {
                if (registro.nombre == nombre) {
                    cout << "Registro encontrado:" << endl;
                    cout << "Nombre: " << registro.nombre << endl;
                    cout << "Edad: " << registro.edad << endl;
                    cout << "Salario: " << registro.salario << endl;
                    encontrado = true;
                    break;
                }
            }
        }
        archivo.close();
        if (!encontrado) {
            cout << "No se encontró el registro." << endl;
        }
    } else {
        cout << "No se pudo abrir el archivo." << endl;
    }
}

int main() {
    int opcion;
    string archivoNombre = "registros_variables.bin";

    do {
        cout << "------ Menú ------" << endl;
        cout << "1. Almacenar registro" << endl;
        cout << "2. Recuperar registros" << endl;
        cout << "3. Eliminar registro" << endl;
        cout << "4. Actualizar registro" << endl;
        cout << "5. Buscar registro" << endl;
        cout << "6. Salir" << endl;
        cout << "Ingrese una opción: ";
        cin >> opcion;

        switch (opcion) {
            case 1: {
                RegistroVariable registro;
                cout << "Ingrese el nombre: ";
                cin.ignore();
                getline(cin, registro.nombre);
                cout << "Ingrese la edad: ";
                cin >> registro.edad;
                cout << "Ingrese el salario: ";
                cin >> registro.salario;

                almacenarRegistroVariable(registro, archivoNombre);
                break;
            }
            case 2:
                recuperarRegistrosVariables(archivoNombre);
                break;
            case 3: {
                string nombreEliminar;
                cout << "Ingrese el nombre del registro a eliminar: ";
                cin.ignore();
                getline(cin, nombreEliminar);

                eliminarRegistroVariable(nombreEliminar, archivoNombre);
                break;
            }
            case 4: {
                string nombreActualizar;
                cout << "Ingrese el nombre del registro a actualizar: ";
                cin.ignore();
                getline(cin, nombreActualizar);

                RegistroVariable nuevoRegistro;
                cout << "Ingrese el nuevo nombre: ";
                getline(cin, nuevoRegistro.nombre);
                cout << "Ingrese la nueva edad: ";
                cin >> nuevoRegistro.edad;
                cout << "Ingrese el nuevo salario: ";
                cin >> nuevoRegistro.salario;

                actualizarRegistroVariable(nombreActualizar, nuevoRegistro, archivoNombre);
                break;
            }
            case 5: {
                string nombreBuscar;
                cout << "Ingrese el nombre del registro a buscar: ";
                cin.ignore();
                getline(cin, nombreBuscar);

                buscarRegistroVariable(nombreBuscar, archivoNombre);
                break;
            }
            case 6:
                cout << "¡Hasta luego!" << endl;
                break;
            default:
                cout << "Opción inválida. Intente nuevamente." << endl;
                break;
        }

        cout << endl;
    } while (opcion != 6);

    return 0;
}
