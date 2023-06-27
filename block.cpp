#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

struct Record {
    int id;
    std::string name;
    int age;
};

class PaginationManager {
public:
    std::vector<Record> readPage(int pageNumber) {
        std::ifstream file("database.bin", std::ios::binary | std::ios::in);
        std::vector<Record> pageRecords;

        if (file) {
            file.seekg(pageNumber * pageSize * sizeof(Record), std::ios::beg);
            for (int i = 0; i < pageSize; ++i) {
                Record record;
                file.read(reinterpret_cast<char*>(&record), sizeof(Record));
                pageRecords.push_back(record);
            }
            file.close();
        }

        return pageRecords;
    }

    void writePage(int pageNumber, const std::vector<Record>& records) {
        std::fstream file("database.bin", std::ios::binary | std::ios::out | std::ios::in);
        file.seekp(pageNumber * pageSize * sizeof(Record), std::ios::beg);
        for (const auto& record : records) {
            file.write(reinterpret_cast<const char*>(&record), sizeof(Record));
        }
        file.close();
    }

private:
    const int pageSize = 5;
};

class BufferPool {
public:
    BufferPool() {
        buffer.resize(bufferSize);
    }

    std::vector<Record> readPage(int pageNumber) {
        auto it = std::find_if(buffer.begin(), buffer.end(), [pageNumber](const Page& page) {
            return page.pageNumber == pageNumber;
        });

        if (it != buffer.end()) {
            it->lastUsed = 0;
            return it->records;
        } else {
            PaginationManager paginationManager;
            std::vector<Record> records = paginationManager.readPage(pageNumber);
            addPageToBufferPool(pageNumber, records);
            return records;
        }
    }

    void writePage(int pageNumber, const std::vector<Record>& records) {
        auto it = std::find_if(buffer.begin(), buffer.end(), [pageNumber](const Page& page) {
            return page.pageNumber == pageNumber;
        });

        if (it != buffer.end()) {
            it->records = records;
            it->dirty = true;
        } else {
            addPageToBufferPool(pageNumber, records);
            markPageAsDirty(pageNumber);
        }
    }

private:
    struct Page {
        int pageNumber;
        std::vector<Record> records;
        bool dirty;
        int lastUsed;
    };

    const int bufferSize = 3;
    std::vector<Page> buffer;

    void addPageToBufferPool(int pageNumber, const std::vector<Record>& records) {
        auto it = std::find_if(buffer.begin(), buffer.end(), [](const Page& page) {
            return page.pageNumber == -1;
        });

        if (it != buffer.end()) {
            it->pageNumber = pageNumber;
            it->records = records;
            it->dirty = false;
        } else {
            replacePageLRU(pageNumber, records);
        }
    }

    void replacePageLRU(int pageNumber, const std::vector<Record>& records) {
        auto lruPageIt = std::min_element(buffer.begin(), buffer.end(), [](const Page& page1, const Page& page2) {
            return page1.lastUsed < page2.lastUsed;
        });

        if (lruPageIt != buffer.end()) {
            if (lruPageIt->dirty) {
                PaginationManager paginationManager;
                paginationManager.writePage(lruPageIt->pageNumber, lruPageIt->records);
            }

            lruPageIt->pageNumber = pageNumber;
            lruPageIt->records = records;
            lruPageIt->dirty = false;
            lruPageIt->lastUsed = 0;
        }
    }

    void markPageAsDirty(int pageNumber) {
        auto it = std::find_if(buffer.begin(), buffer.end(), [pageNumber](const Page& page) {
            return page.pageNumber == pageNumber;
        });

        if (it != buffer.end()) {
            it->dirty = true;
        }
    }
};

class DatabaseManager {
public:
    void createRecord(const Record& newRecord) {
        std::vector<Record> records = bufferPool.readPage(0);
        records.push_back(newRecord);
        bufferPool.writePage(0, records);
    }

    Record readRecord(int recordId) {
        std::vector<Record> records = bufferPool.readPage(0);

        auto it = std::find_if(records.begin(), records.end(), [recordId](const Record& record) {
            return record.id == recordId;
        });

        if (it != records.end()) {
            return *it;
        } else {
            return Record{-1, "", -1};
        }
    }

    void updateRecord(const Record& updatedRecord) {
        std::vector<Record> records = bufferPool.readPage(0);

        auto it = std::find_if(records.begin(), records.end(), [updatedRecord](const Record& record) {
            return record.id == updatedRecord.id;
        });

        if (it != records.end()) {
            *it = updatedRecord;
            bufferPool.writePage(0, records);
        }
    }

    void deleteRecord(int recordId) {
        std::vector<Record> records = bufferPool.readPage(0);
        records.erase(std::remove_if(records.begin(), records.end(), [recordId](const Record& record) {
            return record.id == recordId;
        }), records.end());
        bufferPool.writePage(0, records);
    }

    void saveToFile() {
        std::vector<Record> records = bufferPool.readPage(0);

        std::ofstream file("database.bin", std::ios::binary | std::ios::out | std::ios::trunc);
        for (const auto& record : records) {
            file.write(reinterpret_cast<const char*>(&record), sizeof(Record));
        }
        file.close();
    }

private:
    BufferPool bufferPool;
};

int main() {
    // Leer los registros desde el archivo binario existente o crear uno nuevo
    std::vector<Record> initialRecords;
    std::ifstream file("database.bin", std::ios::binary | std::ios::in);
    if (file) {
        Record record;
        while (file.read(reinterpret_cast<char*>(&record), sizeof(Record))) {
            initialRecords.push_back(record);
        }
        file.close();
    } else {
        std::ofstream newFile("database.bin", std::ios::binary | std::ios::out);
        newFile.close();
    }

    // Crear el gestor de la base de datos
    DatabaseManager database;

    // Cargar los registros iniciales al buffer pool
    if (!initialRecords.empty()) {
        database.createRecord(initialRecords[0]);
    }

    // Crear un nuevo registro
    Record newRecord{1, "John", 25};

    // Agregar el registro a la base de datos
    database.createRecord(newRecord);

    // Leer el registro recién agregado
    Record retrievedRecord = database.readRecord(1);
    std::cout << "Retrieved Record: ID=" << retrievedRecord.id << ", Name=" << retrievedRecord.name << ", Age=" << retrievedRecord.age << std::endl;

    // Actualizar el registro
    retrievedRecord.name = "Jane";
    database.updateRecord(retrievedRecord);
    retrievedRecord = database.readRecord(1);
    std::cout << "Updated Record: ID=" << retrievedRecord.id << ", Name=" << retrievedRecord.name << ", Age=" << retrievedRecord.age << std::endl;

    // Eliminar el registro
    database.deleteRecord(1);

    // Guardar los cambios en el archivo binario
    database.saveToFile();

    return 0;
}
