#include <iostream>
#include <fstream>
#include <string>
#include <vector>

const int PAGE_SIZE = 4096; // Tamaño de página en bytes

struct Field {
    std::string name;
    std::string type;
    // Otros atributos del campo si es necesario
    
    // Constructor
    Field(const std::string& fieldName, const std::string& fieldType) : name(fieldName), type(fieldType) {}
};

struct Record {
    std::vector<std::string> values;
};

struct Page {
    int pageNumber;
    std::vector<Record> records;
};

struct Table {
    std::string name;
    std::vector<Field> fields;
    std::vector<Page> pages;
    // Otros atributos de la tabla si es necesario
    
    // Constructor
    Table(const std::string& tableName) : name(tableName) {}
};

void writeRecord(std::ofstream& file, const Record& record) {
    for (const std::string& value : record.values) {
        int valueLength = value.length();
        file.write(reinterpret_cast<const char*>(&valueLength), sizeof(int));
        file.write(value.c_str(), valueLength);
    }
}

void writePage(std::ofstream& file, const Page& page) {
    for (const Record& record : page.records) {
        writeRecord(file, record);
    }
}

void writeTableToFile(const std::string& filename, const Table& table) {
    std::ofstream databaseFile(filename, std::ios::binary | std::ios::app);
    
    if (!databaseFile) {
        std::cout << "Error al abrir el archivo de base de datos." << std::endl;
        return;
    }
    
    // Escribir las páginas en el archivo
    for (const Page& page : table.pages) {
        writePage(databaseFile, page);
    }
    
    databaseFile.close();
}

void readRecord(std::ifstream& file, Record& record) {
    int valueLength;
    std::string value;
    
    while (file.read(reinterpret_cast<char*>(&valueLength), sizeof(int))) {
        char* buffer = new char[valueLength];
        file.read(buffer, valueLength);
        value.assign(buffer, valueLength);
        delete[] buffer;
        
        record.values.push_back(value);
    }
}

void readPage(std::ifstream& file, Page& page) {
    Record record;
    while (!file.eof()) {
        readRecord(file, record);
        if (!record.values.empty()) {
            page.records.push_back(record);
            record.values.clear();
        }
    }
}

void readTableFromFile(const std::string& filename, Table& table) {
    std::ifstream databaseFile(filename, std::ios::binary);
    
    if (!databaseFile) {
        std::cout << "Error al abrir el archivo de base de datos." << std::endl;
        return;
    }
    
    // Leer las páginas del archivo
    Page page;
    page.pageNumber = 1; // Supongamos que la numeración de páginas comienza en 1
    
    while (!databaseFile.eof()) {
        page.records.clear();
        readPage(databaseFile, page);
        if (!page.records.empty()) {
            table.pages.push_back(page);
            page.pageNumber++;
        }
    }
    
    databaseFile.close();
}

void addRecordToTable(Table& table, const Record& record) {
    // Encontrar la página con suficiente espacio para el nuevo registro
    for (Page& page : table.pages) {
        if (page.records.size() < PAGE_SIZE) {
            page.records.push_back(record);
            return;
        }
    }
    
    // Si no hay ninguna página disponible, crear una nueva página
    Page newPage;
    newPage.pageNumber = table.pages.size() + 1; // Asignar el número de página siguiente
    newPage.records.push_back(record);
    table.pages.push_back(newPage);
}

int main() {
    Table myTable("MyTable");
    
    // Agregar campos a la tabla
    myTable.fields.push_back(Field("ID", "INT"));
    myTable.fields.push_back(Field("Name", "VARCHAR(50)"));
    myTable.fields.push_back(Field("Email", "VARCHAR(100)"));
    
    // Agregar registros a la tabla
    Record record1;
    record1.values.push_back("1");
    record1.values.push_back("Juan Camaro");
    record1.values.push_back("camaro@email.com");
    
    addRecordToTable(myTable, record1);
    
    Record record2;
    record2.values.push_back("2");
    record2.values.push_back("Albert Camus");
    record2.values.push_back("camus@email.com");
    
    addRecordToTable(myTable, record2);
    
    // Escribir la tabla en el archivo
    writeTableToFile("database.bin", myTable);
    
    // Leer la tabla desde el archivo
    Table myTableRead("MyTable");
    readTableFromFile("database.bin", myTableRead);
    
    // Imprimir los registros de la tabla
    for (const Page& page : myTableRead.pages) {
        for (const Record& record : page.records) {
            for (const std::string& value : record.values) {
                std::cout << value << " ";
            }
            std::cout << std::endl;
        }
    }
    
    return 0;
}
