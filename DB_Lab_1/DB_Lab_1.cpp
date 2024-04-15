#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <iomanip>

std::streampos write_pos_user = 0;
std::streampos prev_pos_user = -1;

std::streampos write_pos_loan = 0;
std::streampos prev_pos_loan = -1;

int user_records = 0;
int loan_records = 0;

uint32_t last_id_user = 0;
uint32_t last_id_loan = 0;

// Структури та їх вивід
struct User
{
    uint32_t id = 0;
    char name[16] = "";
    char password[16] = "";
    char email[32] = "";

    int64_t next = -1;
};
struct GameLoan
{
    uint32_t id = 0;
    uint32_t user_id = 0;
    uint32_t game_id = 0;
    char date[16] = "";

    int64_t next = -1;
};
std::ostream& operator<<(std::ostream& os, const User& user) {
    os << "User ID: " << user.id << ", "
        << "Name: " << user.name << ", "
        << "Password: " << user.password << ", "
        << "Email: " << user.email;/* << ", "
        << "Next: " << user.next;*/
    return os;
}
std::ostream& operator<<(std::ostream& os, const GameLoan& gameLoan) {
    os << "GameLoan ID: " << gameLoan.id << ", "
        << "User ID: " << gameLoan.user_id << ", "
        << "Game ID: " << gameLoan.game_id << ", "
        << "Date: " << gameLoan.date;/* << ", "
        << "Next: " << gameLoan.next;*/
    return os;
}

// Читання та запис у файл
bool ReadUser(User& record, std::fstream& file, const std::streampos& pos)
{
    if (!file)
        return false;

    file.seekg(pos);
    file.read(reinterpret_cast<char*>(&record), sizeof(User));

    return !file.fail();
}
bool ReadLoan(GameLoan& record, std::fstream& file, const std::streampos& pos)
{
    if (!file) {
        std::cout << "Failed to open file (PROBABLY).\n\n";
        return false;
    }

    file.seekg(pos);
    file.read(reinterpret_cast<char*>(&record), sizeof(GameLoan));

    return !file.fail();
}
bool WriteUser(const User& record, std::fstream& file, const std::streampos& pos)
{
    if (!file)
        return false;

    file.seekp(pos);
    file.write(reinterpret_cast<const char*>(&record), sizeof(User));
    file.flush();

    return !file.fail();
}
bool WriteLoan(const GameLoan& record, std::fstream& file, const std::streampos& pos)
{
    if (!file)
        return false;

    file.seekp(pos);
    file.write(reinterpret_cast<const char*>(&record), sizeof(GameLoan));
    file.flush();

    return !file.fail();
}

// Додавання нових записів
bool SetNextPtrUser(std::fstream& file, const std::streampos& record_pos, const std::streampos& next_record_pos)
{
    User tmp;

    if (!ReadUser(tmp, file, record_pos))
    {
        std::cerr << "SetNextPtrUser. Read failed.\n\n";
        return false;
    }

    tmp.next = next_record_pos;

    if (!WriteUser(tmp, file, record_pos))
    {
        std::cerr << "SetNextPtrUser. Write failed.\n\n";
        return false;
    }

    return true;
}
bool SetNextPtrLoan(std::fstream& file, const std::streampos& record_pos, const std::streampos& next_record_pos)
{
    GameLoan tmp;

    if (!ReadLoan(tmp, file, record_pos))
    {
        std::cerr << "SetNextPtrLoan. Read failed.\n\n";
        return false;
    }

    tmp.next = next_record_pos;

    if (!WriteLoan(tmp, file, record_pos))
    {
        std::cerr << "SetNextPtrLoan. Write failed.\n\n";
        return false;
    }

    return true;
}
bool AddNodeUser(const User& record, std::fstream& file, const std::streampos& pos, std::unordered_map<uint32_t, std::streampos>& index, const std::streampos& prev_record_pos = -1)
{
    if (!WriteUser(record, file, pos))
    {
        std::cout << "AddNodeUser. Write failed.\n\n";
        return false;
    }

    // додаємо позицію нового запису у файлі до індексної таблиці
    index[record.id] = pos;
    user_records++;

    if (prev_record_pos == -1)
        return true;

    return SetNextPtrUser(file, prev_record_pos, pos);
}
bool AddNodeLoan(const GameLoan& record, std::fstream& file, const std::streampos& pos, const std::streampos& prev_record_pos = -1)
{
    if (!WriteLoan(record, file, pos))
    {
        std::cout << "AddNodeLoan. Write failed.\n\n";
        return false;
    }

    loan_records++;

    if (prev_record_pos == -1)
        return true;

    return SetNextPtrLoan(file, prev_record_pos, pos);
}

// Вивід всіх записів
void PrintNodesUser(std::fstream& file, const std::streampos& record_pos) {
    User tmp;

    std::streampos read_pos = record_pos;

    std::cout << "Table Users\n";
    std::cout << "+----+----------------+----------------+--------------------------------+" << std::endl;
    std::cout << "| ID |      Name      |    Password    |             Email              |" << std::endl;
    std::cout << "+----+----------------+----------------+--------------------------------+" << std::endl;

    while (read_pos != -1) {
        if (!ReadUser(tmp, file, read_pos)) {
            std::cerr << "PrintNodesUser. Read failed\n\n";
            return;
        }

        if (tmp.id != 0) {
            std::cout << "| " << std::setw(2) << tmp.id << " | " << std::setw(14) << tmp.name << " | " << std::setw(14) << tmp.password << " | " << std::setw(30) << tmp.email << " | " << std::endl;
        }

        read_pos = tmp.next;
    }

    std::cout << "+----+----------------+----------------+--------------------------------+\n\n";
}
void PrintNodesLoan(std::fstream& file, const std::streampos& record_pos)
{
    GameLoan tmp;

    std::streampos read_pos = record_pos;

    std::cout << "Table GameLoans\n";
    std::cout << "+----+--------------+--------------+------------------+" << std::endl;
    std::cout << "| ID |   User ID    |   Game ID    |       Date       |" << std::endl;
    std::cout << "+----+--------------+--------------+------------------+" << std::endl;

    while (read_pos != -1) {
        if (!ReadLoan(tmp, file, read_pos)) {
            std::cerr << "PrintNodesLoan. Read failed.\n\n";
            return;
        }

        if (tmp.id != 0) {
            std::cout << "| " << std::setw(2) << tmp.id << " | " << std::setw(12) << tmp.user_id << " | " << std::setw(12) << tmp.game_id << " | " << std::setw(16) << tmp.date << " | " << std::endl;
        }

        read_pos = tmp.next;
    }

    std::cout << "+----+--------------+--------------+------------------+\n\n";
}
void PrintDeletedRecords(std::fstream& user_file, std::fstream& gameLoan_file) {
    user_file.clear();
    user_file.seekg(0, std::ios::beg);

    User user;
    std::cout << "Deleted User records:\n";
    while (user_file.read(reinterpret_cast<char*>(&user), sizeof(User))) {
        if (user.id == 0 && user.name[0] != '\0') {
            std::cout << "ID: " << user.id << ", Name: " << user.name << ", Password: " << user.password << ", Email: " << user.email << std::endl;
        }
    }
    std::cout << std::endl;

    gameLoan_file.clear();
    gameLoan_file.seekg(0, std::ios::beg);

    GameLoan gameLoan;
    std::cout << "Deleted GameLoan records:\n";
    while (gameLoan_file.read(reinterpret_cast<char*>(&gameLoan), sizeof(GameLoan))) {
        if (gameLoan.id == 0 && gameLoan.user_id != 0) {
            std::cout << "ID: " << gameLoan.id << ", User ID: " << gameLoan.user_id << ", Game ID: " << gameLoan.game_id << ", Date: " << gameLoan.date << std::endl;
        }
    }
    std::cout << std::endl;

    user_file.clear();
    user_file.seekg(0, std::ios::beg);

    gameLoan_file.clear();
    gameLoan_file.seekg(0, std::ios::beg);
}

void PrintCommandList() {
    std::cout << "List of commands:\n"
        << "get-m    / get-s\n"
        << "del-m    / del-s\n"
        << "update-m / update-s\n"
        << "insert-m / insert-s\n"
        << "calc-m   / calc-s\n"
        << "ut-m     / ut-s\n\n";
}

// Пошук і вивід записів
bool FindRecordByID_User(uint32_t id, std::fstream& file, const std::unordered_map<uint32_t, std::streampos>& index) {
    auto it = index.find(id);
    if (it == index.end())
    {
        std::cerr << "Record with ID " << id << " not found.\n\n";
        return false;
    }

    User record;
    if (!ReadUser(record, file, it->second))
    {
        std::cerr << "Unable to read record.\n\n";
        return false;
    }

    std::cout << "Found record with ID: " << id << std::endl;
    std::cout << record << "\n\n";

    return true;
}
bool FindRecordByID_Loan(uint32_t id, std::fstream& file) {
    std::streampos current_pos = 0;

    while (current_pos != -1) {
        GameLoan record;
        if (!ReadLoan(record, file, current_pos)) {
            std::cerr << "Failed to read record.\n\n";
            return false;
        }

        if (record.id == id) {
            std::cout << "Found record with ID: " << id << std::endl;
            std::cout << record << "\n\n";
            return true;
        }

        current_pos = record.next;
    }

    std::cerr << "Record with ID " << id << " not found.\n\n";
    return false;
}

// Знайти юзера але не виводити
bool UserExist(uint32_t id, std::fstream& file, const std::unordered_map<uint32_t, std::streampos>& index) {
    auto it = index.find(id);
    if (it == index.end())
        return false;

    User record;
    if (!ReadUser(record, file, it->second))
        return false;

    return true;
}

void CalcLoansByUser(uint32_t id, std::fstream& user_file, std::fstream& gameLoan_file,
    const std::unordered_map<uint32_t, std::streampos>& index)
{
    if (!UserExist(id, user_file, index)) {
        std::cout << "User record with ID " << id << " not found.\n\n";
        return;
    }

    GameLoan tmp;
    std::streampos read_pos = 0;
    int record_count = 0;

    while (read_pos != -1) {
        if (!ReadLoan(tmp, gameLoan_file, read_pos)) {
            std::cerr << "PrintNodesLoan. Read failed.\n\n";
            return;
        }

        if (tmp.user_id == id) {
            record_count++;
        }

        read_pos = tmp.next;
    }

    if (record_count == 0) {
        std::cout << "User with ID " << id << " has no subrecords.\n\n";
        return;
    }

    if (record_count == 1) {
        std::cout << "User with ID " << id << " has " << record_count << " subrecord.\n\n";
        return;
    }

    std::cout << "User with ID " << id << " has (" << record_count << ") GameLoan records.\n\n";
}

// Видалення записів
bool RemoveLoansByUserID(uint32_t user_id, std::fstream& file, std::unordered_map<uint32_t, std::streampos>& index);

bool RemoveRecordByID_User(uint32_t id, std::fstream& file, std::fstream& game_file, std::unordered_map<uint32_t, std::streampos>& index) {
    auto it = index.find(id);
    if (it == index.end()) {
        std::cerr << "User record with ID " << id << " not found.\n\n";
        return false;
    }

    std::streampos record_pos = it->second;

    index.erase(it);

    User record;
    if (!ReadUser(record, file, record_pos)) {
        std::cerr << "Unable to read record.\n\n";
        return false;
    }

    // позначаємо запис як пустий з айді 0
    record.id = 0;
    if (!WriteUser(record, file, record_pos)) {
        std::cerr << "Failed to write empty record.\n\n";
        return false;
    }

    if (!RemoveLoansByUserID(id, game_file, index)) {
        std::cout << "Failed to delete this User`s GameLoan records.\n\n";
        return false;
    }

    std::cout << "User record with ID " << id << " removed successfully.\n\n";
    user_records--;
    return true;
}
bool RemoveRecordByID_Loan(uint32_t id, std::fstream& file, std::streampos& start_pos) {
    GameLoan record;
    std::streampos current_pos = start_pos;
    std::streampos last_pos = -1;

    while (ReadLoan(record, file, current_pos)) {
        if (record.id == id) {
            std::streampos delete_start_pos = current_pos;

            // Позначаємо запис як пустий, присвоюючи йому айді 0
            record.id = 0;
            WriteLoan(record, file, delete_start_pos);

            std::cout << "GameLoan record with ID " << id << " removed successfully.\n\n";
            loan_records--;
            return true;
        }

        current_pos = record.next;
        last_pos = current_pos;
    }

    std::cerr << "GameLoan record with ID " << id << " not found.\n\n";

    if (last_pos != -1) {
        start_pos = last_pos;
    }

    return false;
}
bool RemoveLoansByUserID(uint32_t user_id, std::fstream& file, std::unordered_map<uint32_t, std::streampos>& index) {
    // Викликається коли видаляється юзер
    bool removed_at_least_one = false;
    std::streampos current_pos = 0;
    std::streampos prev_pos = -1;

    while (current_pos != -1) {
        GameLoan loan;
        if (!ReadLoan(loan, file, current_pos)) {
            std::cerr << "Failed to read game loan record.\n\n";
            return false;
        }

        if (loan.user_id == user_id) {

            if (prev_pos != -1) {
                GameLoan prev_loan;
                ReadLoan(prev_loan, file, prev_pos);
                prev_loan.next = loan.next;
                WriteLoan(prev_loan, file, prev_pos);
            }
            else {
                // якщо це перший запис, оновити початкову позицію
                index[loan.id] = loan.next;
            }

            if (!RemoveRecordByID_Loan(loan.id, file, current_pos)) {
                std::cerr << "Failed to remove GameLoan record with ID " << loan.id << "\n";
                return false;
            }
            removed_at_least_one = true;
        }
        else {
            prev_pos = current_pos;
        }

        current_pos = loan.next;
    }

    if (!removed_at_least_one) {
        std::cerr << "No game loan records found for user with ID " << user_id << "\n\n";
    }
    return true;
}

// Оновлення записів
bool UpdateRecordFieldByID_User(uint32_t id, std::fstream& file, std::unordered_map<uint32_t, std::streampos>& index, const std::string& field, const std::string& value) {
    auto it = index.find(id);
    if (it == index.end()) {
        std::cerr << "Record with ID " << id << " not found.\n\n";
        return false;
    }

    std::streampos record_pos = it->second;

    User record;
    if (!ReadUser(record, file, record_pos)) {
        std::cerr << "Unable to read record.\n\n";
        return false;
    }

    if (record.id == 0) {
        std::cerr << "Record with ID " << id << " not found.\n\n";
        return false;
    }

    if (field == "name") {
        if (value.length() >= sizeof(record.name)) {
            std::cerr << "New name is too long.\n\n";
            return false;
        }
        strcpy_s(record.name, value.c_str());
    }
    else if (field == "password") {
        if (value.length() >= sizeof(record.password)) {
            std::cerr << "New password is too long.\n\n";
            return false;
        }
        strcpy_s(record.password, value.c_str());
    }
    else if (field == "email") {
        if (value.length() >= sizeof(record.email)) {
            std::cerr << "New email is too long.\n\n";
            return false;
        }
        strcpy_s(record.email, value.c_str());
    }
    else {
        std::cout << "Wrong field input.\n\n";
        return false;
    }

    if (!WriteUser(record, file, record_pos)) {
        std::cerr << "Failed to update record.\n\n";
        return false;
    }

    std::cout << "Record with ID " << id << " updated successfully.\n\n";
    return true;
}
bool UpdateRecordFieldByID_Loan(uint32_t id, std::fstream& file, const std::string& field, const std::string& value) {
    GameLoan record;
    std::streampos current_pos = 0;

    while (ReadLoan(record, file, current_pos)) {
        if (record.id == id) {
            if (field == "user_id") {
                record.user_id = std::stoi(value);
            }
            else if (field == "game_id") {
                record.game_id = std::stoi(value);
            }
            else if (field == "date") {
                if (value.length() >= sizeof(record.date)) {
                    std::cerr << "Invalid date input.\n\n";
                    return false;
                }
                strcpy_s(record.date, value.c_str());
            }
            else {
                std::cout << "Wrong field input.\n\n";
                return false;
            }

            if (!WriteLoan(record, file, current_pos)) {
                std::cerr << "Failed to update record.\n\n";
                return false;
            }

            std::cout << "Record with ID " << id << " updated successfully.\n\n";
            return true;
        }

        current_pos = record.next;
    }

    std::cerr << "Record with ID " << id << " not found.\n\n";
    return false;
}

// Для індексної таблиці
std::unordered_map<uint32_t, std::streampos> BuildIndex(std::fstream& file, const std::streampos& record_pos)
{
    std::unordered_map<uint32_t, std::streampos> index;

    User tmp;
    std::streampos read_pos = record_pos;

    std::cout << "BuildIndex: Creating index table...\n\n";
    while (read_pos != -1)
    {
        if (!ReadUser(tmp, file, read_pos))
        {
            if (tmp.id == 0) {
                std::cout << "BuildIndex: Empty delivery.bin file\n\n";
                return index;
            }
            std::cerr << "BuildIndex: Unable to build index. Error: read failed.\n\n";
            return index;
        }

        index[tmp.id] = read_pos;
        read_pos = tmp.next;
    }

    return index;
}
std::unordered_map<uint32_t, std::streampos> ReadIndexFile(const std::string& filename) {
    std::unordered_map<uint32_t, std::streampos> index;
    std::ifstream indexFile(filename);
    if (!indexFile.is_open()) {
        std::cerr << "Failed to open index file: " << filename << "\n\n";
        return index;
    }
    uint32_t id;
    long long pos;
    while (indexFile >> id >> pos) {
        index[id] = static_cast<std::streampos>(pos);
    }
    return index;
}
bool WriteIndexFile(const std::unordered_map<uint32_t, std::streampos>& index, const std::string& filename) {
    std::ofstream indexFile(filename);
    if (!indexFile.is_open()) {
        std::cerr << "Failed to open index file for writing: " << filename << "\n\n";
        return false;
    }
    for (const auto& pair : index) {
        indexFile << pair.first << " " << pair.second << std::endl;
    }
    return true;
}
void PrintIndexTable(const std::unordered_map<uint32_t, std::streampos>& index) {
    if (index.size() == 0) {
        std::cout << "Index table is empty.\n\n";
        return;
    }

    std::cout << "Index Table:\n";
    for (const auto& pair : index) {
        std::cout << "ID: " << pair.first << ", Position: " << pair.second << std::endl;
    }
    std::cout << std::endl;
}

// Команди
enum class CommandType {
    GetUser,
    GetLoan,
    DeleteUser,
    DeleteLoan,
    UpdateUser,
    UpdateLoan,
    InsertUser,
    InsertLoan,
    CalculateUser,
    CalculateLoan,
    CalculateLoansByUser,
    PrintUser,
    PrintLoan,
    Exit,
    Index,
    Deleted,
    Unknown
};

// Для Insert`ів
void GetUserInput(User& user) {
    std::cin.ignore();

    std::cout << "Enter name (up to 15 characters): ";
    std::cin.getline(user.name, 16);

    std::cout << "Enter password (up to 15 characters): ";
    std::cin.getline(user.password, 16);

    std::cout << "Enter email (up to 31 characters): ";
    std::cin.getline(user.email, 32);

    std::cout << std::endl;
}
void GetGameLoanInput(GameLoan& gameLoan) {
    std::cin.ignore();

    std::cout << "Enter user ID: ";
    std::cin >> gameLoan.user_id;

    std::cout << "Enter game ID: ";
    std::cin >> gameLoan.game_id;

    std::cin.ignore();

    std::cout << "Enter date (up to 10 characters): ";
    std::cin.getline(gameLoan.date, 11);

    std::cout << std::endl;
}

int main()
{
    const std::string user_filename = "Users.bin";
    const std::string gameLoan_filename = "GameLoans.bin";
    const std::string index_filename = "Index.txt";

    // читаю (файл)
    std::unordered_map<uint32_t, std::streampos> index = ReadIndexFile(index_filename);

    std::fstream user_file(user_filename, std::ios::binary | std::ios::in | std::ios::out);
    std::fstream gameLoan_file(gameLoan_filename, std::ios::binary | std::ios::in | std::ios::out);
    auto err = errno;

    // Перевірка файлів
    if (err == ENOENT)
    {
        user_file = std::fstream(user_filename, std::ios::binary | std::ios::in | std::ios::out);
        gameLoan_file = std::fstream(gameLoan_filename, std::ios::binary | std::ios::in | std::ios::out);
    }
    if (!user_file) {
        std::cerr << "Unable to open file=" << user_filename << "\n\n";

        return -1;
    }
    if (!gameLoan_file) {
        std::cerr << "Unable to open file=" << gameLoan_filename << "\n\n";

        return -1;
    }

    // Додаємо записи
    AddNodeUser({ ++last_id_user, "Petro", "6g7rfyehuis", "atackhelicopter@gmail.com" }, user_file, write_pos_user, index, prev_pos_user);
    prev_pos_user = write_pos_user;
    write_pos_user = write_pos_user + static_cast<std::streamoff>(sizeof(User));
    AddNodeUser({ ++last_id_user, "Vadym", "1111", "brawlkvas1400@gmail.com" }, user_file, write_pos_user, index, prev_pos_user);
    prev_pos_user = write_pos_user;
    write_pos_user = write_pos_user + static_cast<std::streamoff>(sizeof(User));
    AddNodeUser({ ++last_id_user, "Mark", "password", "wqgfhbj@gmail.com" }, user_file, write_pos_user, index, prev_pos_user);
    prev_pos_user = write_pos_user;
    write_pos_user = write_pos_user + static_cast<std::streamoff>(sizeof(User));
    AddNodeUser({ ++last_id_user, "Steve", "heheheha", "chipichapa@patron.com" }, user_file, write_pos_user, index, prev_pos_user);
    prev_pos_user = write_pos_user;
    write_pos_user = write_pos_user + static_cast<std::streamoff>(sizeof(User));
    AddNodeUser({ ++last_id_user, "Igor", "qwerty", "john@gmail.com" }, user_file, write_pos_user, index, prev_pos_user);
    prev_pos_user = write_pos_user;
    write_pos_user = write_pos_user + static_cast<std::streamoff>(sizeof(User));

    AddNodeLoan({ ++last_id_loan, 1, 1, "29.03.2024" }, gameLoan_file, write_pos_loan, prev_pos_loan);
    prev_pos_loan = write_pos_loan;
    write_pos_loan = write_pos_loan + static_cast<std::streamoff>(sizeof(GameLoan));
    AddNodeLoan({ ++last_id_loan, 2, 2, "29.03.2024" }, gameLoan_file, write_pos_loan, prev_pos_loan);
    prev_pos_loan = write_pos_loan;
    write_pos_loan = write_pos_loan + static_cast<std::streamoff>(sizeof(GameLoan));
    AddNodeLoan({ ++last_id_loan, 3, 2, "30.03.2024" }, gameLoan_file, write_pos_loan, prev_pos_loan);
    prev_pos_loan = write_pos_loan;
    write_pos_loan = write_pos_loan + static_cast<std::streamoff>(sizeof(GameLoan));
    AddNodeLoan({ ++last_id_loan, 2, 5, "30.03.2024" }, gameLoan_file, write_pos_loan, prev_pos_loan);
    prev_pos_loan = write_pos_loan;
    write_pos_loan = write_pos_loan + static_cast<std::streamoff>(sizeof(GameLoan));
    AddNodeLoan({ ++last_id_loan, 2, 2, "30.03.2024" }, gameLoan_file, write_pos_loan, prev_pos_loan);
    prev_pos_loan = write_pos_loan;
    write_pos_loan = write_pos_loan + static_cast<std::streamoff>(sizeof(GameLoan));
    AddNodeLoan({ ++last_id_loan, 2, 3, "31.03.2024" }, gameLoan_file, write_pos_loan, prev_pos_loan);
    prev_pos_loan = write_pos_loan;
    write_pos_loan = write_pos_loan + static_cast<std::streamoff>(sizeof(GameLoan));
    AddNodeLoan({ ++last_id_loan, 3, 5, "31.03.2024" }, gameLoan_file, write_pos_loan, prev_pos_loan);
    prev_pos_loan = write_pos_loan;
    write_pos_loan = write_pos_loan + static_cast<std::streamoff>(sizeof(GameLoan));
    AddNodeLoan({ ++last_id_loan, 4, 3, "01.04.2024" }, gameLoan_file, write_pos_loan, prev_pos_loan);
    prev_pos_loan = write_pos_loan;
    write_pos_loan = write_pos_loan + static_cast<std::streamoff>(sizeof(GameLoan));
    AddNodeLoan({ ++last_id_loan, 1, 2, "03.04.2024" }, gameLoan_file, write_pos_loan, prev_pos_loan);
    prev_pos_loan = write_pos_loan;
    write_pos_loan = write_pos_loan + static_cast<std::streamoff>(sizeof(GameLoan));

    // Індексна таблиця
    index = BuildIndex(user_file, 0);

    PrintNodesUser(user_file, 0);
    PrintNodesLoan(gameLoan_file, 0);

    while (1) {
        std::string command;
        CommandType type = CommandType::Unknown;

        PrintCommandList();

        std::cout << "Enter command: ";
        //getline(std::cin, command);
        std::cin >> command;
        std::cout << std::endl;

        if (command == "get-m")
            type = CommandType::GetUser;
        else if (command == "get-s")
            type = CommandType::GetLoan;
        else if (command == "del-m")
            type = CommandType::DeleteUser;
        else if (command == "del-s")
            type = CommandType::DeleteLoan;
        else if (command == "update-m")
            type = CommandType::UpdateUser;
        else if (command == "update-s")
            type = CommandType::UpdateLoan;
        else if (command == "insert-m")
            type = CommandType::InsertUser;
        else if (command == "insert-s")
            type = CommandType::InsertLoan;
        else if (command == "calc-m")
            type = CommandType::CalculateUser;
        else if (command == "calc-s")
            type = CommandType::CalculateLoan;
        else if (command == "calc-s-user")
            type = CommandType::CalculateLoansByUser;
        else if (command == "ut-m")
            type = CommandType::PrintUser;
        else if (command == "ut-s")
            type = CommandType::PrintLoan;
        else if (command == "exit")
            type = CommandType::Exit;
        else if (command == "index")
            type = CommandType::Index;
        else if (command == "deleted")
            type = CommandType::Deleted;

        switch (type) {
        case CommandType::GetUser: {
            int id_to_find_user;
            std::cout << "Enter ID: ";
            std::cin >> id_to_find_user;
            std::cout << std::endl;
            FindRecordByID_User(id_to_find_user, user_file, index);
            break;
        }
        case CommandType::GetLoan: {
            uint32_t id_to_find_loan;
            std::cout << "Enter ID: ";
            std::cin >> id_to_find_loan;
            std::cout << std::endl;
            FindRecordByID_Loan(id_to_find_loan, gameLoan_file);
            break;
        }
        case CommandType::DeleteUser: {
            std::cout << "Enter ID: ";
            int id_to_delete_user;
            std::cin >> id_to_delete_user;
            std::cout << std::endl;
            RemoveRecordByID_User(id_to_delete_user, user_file, gameLoan_file, index);
            break;
        }
        case CommandType::DeleteLoan: {
            std::cout << "Enter ID: ";
            int id_to_delete_loan;
            std::cin >> id_to_delete_loan;
            std::cout << std::endl;
            std::streampos start_pos = 0;
            RemoveRecordByID_Loan(id_to_delete_loan, gameLoan_file, start_pos);
            break;
        }
        case CommandType::UpdateUser: {
            uint32_t id_to_update_user;
            std::string field_name, new_value;
            std::cout << "Enter ID of the record to update: ";
            std::cin >> id_to_update_user;
            std::cout << "Enter name of the field to update (name, password, email): ";
            std::cin >> field_name;
            if (field_name == "id") {
                std::cout << "Id field cannot be updated.\n\n";
                break;
            }
            std::cout << "Enter new value: ";
            std::cin >> new_value;
            std::cout << std::endl;
            UpdateRecordFieldByID_User(id_to_update_user, user_file, index, field_name, new_value);
            break;
        }
        case CommandType::UpdateLoan: {
            uint32_t id_to_update_loan;
            std::string field_name_loan, new_value;
            std::cout << "Enter ID of the record to update: ";
            std::cin >> id_to_update_loan;
            std::cout << "Enter name of the field to update (user_id, game_id, date): ";
            std::cin >> field_name_loan;
            if (field_name_loan == "id") {
                std::cout << "Id field cannot be updated.\n\n";
                break;
            }
            std::cout << "Enter new value: ";
            std::cin >> new_value;
            std::cout << std::endl;
            UpdateRecordFieldByID_Loan(id_to_update_loan, gameLoan_file, field_name_loan, new_value);
            break;
        }
        case CommandType::InsertUser: {
            User user;
            int id = ++last_id_user;
            GetUserInput(user);
            user.id = static_cast<uint32_t>(id);
            if (AddNodeUser(user, user_file, write_pos_user, index, prev_pos_user)) {
                prev_pos_user = write_pos_user;
                write_pos_user = write_pos_user + static_cast<std::streamoff>(sizeof(User));
                std::cout << "User record successfully added with ID " << user.id << ".\n\n";
            }
            break;
        }
        case CommandType::InsertLoan: {
            GameLoan gameLoan;
            int id = ++last_id_loan;
            GetGameLoanInput(gameLoan);
            gameLoan.id = static_cast<uint32_t>(id);
            if (AddNodeLoan(gameLoan, gameLoan_file, write_pos_loan, prev_pos_loan)) {
                prev_pos_loan = write_pos_loan;
                write_pos_loan = write_pos_loan + static_cast<std::streamoff>(sizeof(GameLoan));
                std::cout << "GameLoan record successfully added with ID " << gameLoan.id << ".\n\n";
            }
            break;
        }
        case CommandType::CalculateUser: {
            std::cout << "Current User records amount: " << user_records << "\n\n";
            break;
        }
        case CommandType::CalculateLoan: {
            std::cout << "Current GameLoan records amount: " << loan_records << "\n\n";
            break;
        }
        case CommandType::CalculateLoansByUser: {
            uint32_t id;
            std::cout << "Enter the ID of User record: ";
            std::cin >> id;
            std::cout << std::endl;
            std::cin.ignore();
            CalcLoansByUser(id, user_file, gameLoan_file, index);
            break;
        }
        case CommandType::PrintUser:
            PrintNodesUser(user_file, 0);
            break;
        case CommandType::PrintLoan:
            PrintNodesLoan(gameLoan_file, 0);
            break;
        case CommandType::Exit:
            goto exit_loop;
        case CommandType::Index:
            PrintIndexTable(index);
            break;
        case CommandType::Deleted:
            PrintDeletedRecords(user_file, gameLoan_file);
            break;
        case CommandType::Unknown:
            std::cout << "Invalid command. Returning to command list...\n\n";
            break;
        }
    }

exit_loop:;

    index = BuildIndex(user_file, 0);
    if (!WriteIndexFile(index, user_filename)) {
        std::cerr << "Failed to write index file.\n\n";
        return 1;
    }

    return 0;
}