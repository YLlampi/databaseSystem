#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

const int PAGE_SIZE = 4096; // Tamaño de página en bytes
const int BUFFER_POOL_SIZE = 10; // Tamaño del buffer pool

// Estructura para representar un registro
struct Record {
    int id;
    std::string name;
    int age;
};

// Estructura para representar una página
struct Page {
    int pageNumber;
    std::vector<Record> records;
    bool dirty;
};

class BufferPool {
private:
    std::vector<Page> buffer; // Buffer pool

public:
    BufferPool() {
        buffer.resize(BUFFER_POOL_SIZE);
    }

    std::vector<Record> readPage(int pageNumber) {
        // Buscar la página en el buffer pool
        auto it = std::find_if(buffer.begin(), buffer.end(), [pageNumber](const Page& page) {
            return page.pageNumber == pageNumber;
        });

        if (it != buffer.end()) {
            // Si la página está en el buffer pool, devolver los registros
            return it->records;
        } else {
            // Si la página no está en el buffer pool, leerla desde el archivo binario
            std::vector<Record> records = readPageFromStorage(pageNumber);

            // Agregar la página al buffer pool
            addPageToBufferPool(pageNumber, records);

            // Devolver los registros
            return records;
        }
    }

    void writePage(int pageNumber, const std::vector<Record>& records) {
        // Buscar la página en el buffer pool
        auto it = std::find_if(buffer.begin(), buffer.end(), [pageNumber](const Page& page) {
            return page.pageNumber == pageNumber;
        });

        if (it != buffer.end()) {
            // Si la página está en el buffer pool, actualizar los registros
            it->records = records;
            it->dirty = true;
        } else {
            // Si la página no está en el buffer pool, agregarla
            addPageToBufferPool(pageNumber, records);
        }
    }

    void flushBufferPool() {
        // Escribir todas las páginas sucias del buffer pool al archivo binario
        for (auto& page : buffer) {
            if (page.dirty) {
                writePageToStorage(page.pageNumber, page.records);
                page.dirty = false;
            }
        }
    }

private:
    void addPageToBufferPool(int pageNumber, const std::vector<Record>& records) {
        // Buscar una página vacía en el buffer pool
        auto it = std::find_if(buffer.begin(), buffer.end(), [](const Page& page) {
            return page.pageNumber == -1;
        });

        if (it != buffer.end()) {
            // Si se encontró una página vacía, asignarle el número de página y los registros
            it->pageNumber = pageNumber;
            it->records = records;
            it->dirty = false;
        } else {
            // Si no se encontró una página vacía, aplicar un algoritmo de reemplazo (p. ej., LRU, FIFO)
            // Para simplificar el ejemplo, asumimos que el buffer pool tiene suficiente espacio para todas las páginas
            // Puedes implementar un algoritmo de reemplazo específico según tus necesidades
        }
    }

    std::vector<Record> readPageFromStorage(int pageNumber) {
        // Calcular la posición de inicio de la página en el archivo binario
        std::streampos startPos = static_cast<std::streampos>(pageNumber) * PAGE_SIZE;

        // Leer la página desde el archivo y devolver los registros
        std::ifstream file("database.bin", std::ios::binary | std::ios::in);
        file.seekg(startPos);

        std::vector<Record> records;
        Record record;
        while (file.read(reinterpret_cast<char*>(&record), sizeof(Record))) {
            records.push_back(record);
        }
        file.close();

        return records;
    }

    void writePageToStorage(int pageNumber, const std::vector<Record>& records) {
        // Calcular la posición de inicio de la página en el archivo binario
        std::streampos startPos = static_cast<std::streampos>(pageNumber) * PAGE_SIZE;

        // Escribir la página en el archivo
        std::ofstream file("database.bin", std::ios::binary | std::ios::in | std::ios::out);
        file.seekp(startPos);

        for (const auto& record : records) {
            file.write(reinterpret_cast<const char*>(&record), sizeof(Record));
        }
        file.close();
    }
};

class Database {
public:
    BufferPool bufferPool;

public:
    void createRecord(const Record& record) {
        // Leer la primera página desde el buffer pool
        std::vector<Record> records = bufferPool.readPage(0);

        // Agregar el nuevo registro a la página
        records.push_back(record);

        // Escribir la página actualizada en el buffer pool
        bufferPool.writePage(0, records);
    }

    Record readRecord(int id) {
        // Leer la primera página desde el buffer pool
        std::vector<Record> records = bufferPool.readPage(0);

        // Buscar el registro con el id dado
        auto it = std::find_if(records.begin(), records.end(), [id](const Record& record) {
            return record.id == id;
        });

        if (it != records.end()) {
            // Si se encontró el registro, devolverlo
            return *it;
        } else {
            // Si no se encontró el registro, devolver un registro vacío
            return {};
        }
    }

    void updateRecord(const Record& record) {
        // Leer la primera página desde el buffer pool
        std::vector<Record> records = bufferPool.readPage(0);

        // Buscar el registro con el id dado y actualizarlo
        auto it = std::find_if(records.begin(), records.end(), [record](const Record& r) {
            return r.id == record.id;
        });

        if (it != records.end()) {
            *it = record;
            // Escribir la página actualizada en el buffer pool
            bufferPool.writePage(0, records);
        }
    }

    void deleteRecord(int id) {
        // Leer la primera página desde el buffer pool
        std::vector<Record> records = bufferPool.readPage(0);

        // Eliminar el registro con el id dado
        records.erase(std::remove_if(records.begin(), records.end(), [id](const Record& record) {
            return record.id == id;
        }), records.end());

        // Escribir la página actualizada en el buffer pool
        bufferPool.writePage(0, records);
    }
};

int main() {
    // Crear una instancia de la base de datos
    Database database;

    // Crear registros
    Record record1 { 1, "Robert", 23 };
    Record record2 { 2, "James", 52 };

    // Insertar registros
    database.createRecord(record1);
    database.createRecord(record2);

    // Leer registros
    Record retrievedRecord1 = database.readRecord(1);
    Record retrievedRecord2 = database.readRecord(2);
    std::cout << "Registros obtenidos:" << std::endl;
    std::cout << "ID: " << retrievedRecord1.id << ", Nombre: " << retrievedRecord1.name << ", Edad: " << retrievedRecord1.age << std::endl;
    std::cout << "ID: " << retrievedRecord2.id << ", Nombre: " << retrievedRecord2.name << ", Edad: " << retrievedRecord2.age << std::endl;

    // Actualizar un registro
    retrievedRecord1.age = 36;
    database.updateRecord(retrievedRecord1);
    
    // Eliminar un registro
    database.deleteRecord(2);

    // Guardar los cambios en el archivo binario
    database.bufferPool.flushBufferPool();

    return 0;
}
