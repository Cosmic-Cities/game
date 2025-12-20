#include "SaveManager.h"
#include <zlib.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <filesystem>
#include <ctime>

namespace cosmiccities {

namespace {
    constexpr int MAX_SAVE_SLOTS = 3;
    constexpr const char* SLOT_EXTENSION = ".ccsave";
    constexpr uint32_t SAVE_MAGIC = 0x43435356; // "CCSV"
    constexpr uint32_t SAVE_VERSION = 1;

    struct SaveFileHeader {
        uint32_t magic;
        uint32_t version;
        uint32_t compressedSize;
        uint32_t uncompressedSize;
    };
}

SaveManager& SaveManager::instance() {
    static SaveManager inst;
    return inst;
}

SaveManager::~SaveManager() {
    closeDatabase();
}

bool SaveManager::initialize(const std::string& saveDirectory) {
    m_saveDirectory = saveDirectory;
    
    std::error_code ec;
    std::filesystem::create_directories(m_saveDirectory, ec);
    if (ec) {
        spdlog::error("SaveManager: Failed to create save directory '{}': {}", 
                     m_saveDirectory, ec.message());
        return false;
    }
    
    spdlog::info("SaveManager: Initialized with save directory '{}'", m_saveDirectory);
    return true;
}

std::string SaveManager::getSlotFilePath(int slotId) const {
    if (slotId < 0 || slotId >= MAX_SAVE_SLOTS) {
        return "";
    }
    return m_saveDirectory + "/slot" + std::to_string(slotId) + SLOT_EXTENSION;
}

bool SaveManager::doesSlotExist(int slotId) const {
    if (slotId < 0 || slotId >= MAX_SAVE_SLOTS) {
        return false;
    }
    return std::filesystem::exists(getSlotFilePath(slotId));
}

SaveSlotInfo SaveManager::getSlotInfo(int slotId) const {
    SaveSlotInfo info;
    info.slotId = slotId;
    info.exists = doesSlotExist(slotId);
    
    if (!info.exists) {
        return info;
    }
    
    // Temporarily open the slot to read metadata
    auto dbData = const_cast<SaveManager*>(this)->decompressSlotFromFile(slotId);
    if (dbData.empty()) {
        info.exists = false;
        return info;
    }
    
    sqlite3* tempDb = nullptr;
    int rc = sqlite3_open_v2(":memory:", &tempDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_MEMORY, nullptr);
    if (rc != SQLITE_OK) {
        return info;
    }
    
    // Deserialize the database
    rc = sqlite3_deserialize(tempDb, "main", dbData.data(), dbData.size(), dbData.size(),
                             SQLITE_DESERIALIZE_READONLY);
    if (rc != SQLITE_OK) {
        sqlite3_close(tempDb);
        return info;
    }
    
    // Read metadata
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT key, value FROM metadata WHERE key IN ('player_name', 'last_save', 'chapter', 'level', 'playtime')";
    
    if (sqlite3_prepare_v2(tempDb, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string key = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            
            if (key == "player_name") info.playerName = value;
            else if (key == "last_save") info.lastSaveTime = std::stoll(value);
            else if (key == "chapter") info.chapter = std::stoi(value);
            else if (key == "level") info.level = std::stoi(value);
            else if (key == "playtime") info.playtime = std::stof(value);
        }
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(tempDb);
    return info;
}

std::vector<SaveSlotInfo> SaveManager::getAllSlots() const {
    std::vector<SaveSlotInfo> slots;
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        slots.push_back(getSlotInfo(i));
    }
    return slots;
}

bool SaveManager::createDatabase() {
    const char* schema = R"(
        CREATE TABLE IF NOT EXISTS metadata (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL
        );
        
        CREATE TABLE IF NOT EXISTS game_data (
            key TEXT PRIMARY KEY,
            value_type TEXT NOT NULL,
            value TEXT NOT NULL
        );
        
        CREATE INDEX IF NOT EXISTS idx_game_data_key ON game_data(key);
    )";
    
    return executeSQL(schema);
}

bool SaveManager::createNewSlot(int slotId, const std::string& playerName) {
    if (slotId < 0 || slotId >= MAX_SAVE_SLOTS) {
        spdlog::error("SaveManager: Invalid slot ID {}", slotId);
        return false;
    }
    
    if (doesSlotExist(slotId)) {
        spdlog::warn("SaveManager: Slot {} already exists", slotId);
        return false;
    }
    
    // Close current slot if any
    closeDatabase();
    
    // Create in-memory database
    int rc = sqlite3_open(":memory:", &m_db);
    if (rc != SQLITE_OK) {
        spdlog::error("SaveManager: Failed to create in-memory database: {}", sqlite3_errmsg(m_db));
        return false;
    }
    
    if (!createDatabase()) {
        closeDatabase();
        return false;
    }
    
    // Set initial metadata
    auto now = std::time(nullptr);
    setString("player_name", playerName);
    setInt("last_save", now);
    setInt("chapter", 1);
    setInt("level", 1);
    setFloat("playtime", 0.0);
    
    m_currentSlot = slotId;
    
    // Save to file
    return saveSlot(slotId);
}

bool SaveManager::loadSlot(int slotId) {
    if (slotId < 0 || slotId >= MAX_SAVE_SLOTS) {
        spdlog::error("SaveManager: Invalid slot ID {}", slotId);
        return false;
    }
    
    if (!doesSlotExist(slotId)) {
        spdlog::error("SaveManager: Slot {} does not exist", slotId);
        return false;
    }
    
    closeDatabase();
    
    auto dbData = decompressSlotFromFile(slotId);
    if (dbData.empty()) {
        return false;
    }
    
    int rc = sqlite3_open(":memory:", &m_db);
    if (rc != SQLITE_OK) {
        spdlog::error("SaveManager: Failed to open in-memory database: {}", sqlite3_errmsg(m_db));
        return false;
    }
    
    // Deserialize the database
    unsigned char* buffer = static_cast<unsigned char*>(sqlite3_malloc64(dbData.size()));
    if (!buffer) {
        closeDatabase();
        return false;
    }
    
    std::memcpy(buffer, dbData.data(), dbData.size());
    
    rc = sqlite3_deserialize(m_db, "main", buffer, dbData.size(), dbData.size(),
                             SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE);
    if (rc != SQLITE_OK) {
        spdlog::error("SaveManager: Failed to deserialize database: {}", sqlite3_errmsg(m_db));
        sqlite3_free(buffer);
        closeDatabase();
        return false;
    }
    
    m_currentSlot = slotId;
    spdlog::info("SaveManager: Loaded slot {}", slotId);
    return true;
}

bool SaveManager::saveSlot(int slotId) {
    if (slotId < 0 || slotId >= MAX_SAVE_SLOTS) {
        spdlog::error("SaveManager: Invalid slot ID {}", slotId);
        return false;
    }
    
    if (!m_db) {
        spdlog::error("SaveManager: No database open");
        return false;
    }
    
    // Update last save time
    setInt("last_save", std::time(nullptr));
    
    // Serialize the database
    sqlite3_int64 size = 0;
    unsigned char* data = sqlite3_serialize(m_db, "main", &size, 0);
    if (!data) {
        spdlog::error("SaveManager: Failed to serialize database");
        return false;
    }
    
    std::vector<uint8_t> dbData(data, data + size);
    sqlite3_free(data);
    
    bool success = compressSlotToFile(slotId, dbData);
    if (success) {
        spdlog::info("SaveManager: Saved slot {} ({} bytes -> {} bytes compressed)", 
                    slotId, dbData.size(), std::filesystem::file_size(getSlotFilePath(slotId)));
    }
    
    return success;
}

bool SaveManager::deleteSlot(int slotId) {
    if (slotId < 0 || slotId >= MAX_SAVE_SLOTS) {
        return false;
    }
    
    if (m_currentSlot == slotId) {
        closeDatabase();
    }
    
    std::error_code ec;
    std::filesystem::remove(getSlotFilePath(slotId), ec);
    
    if (ec) {
        spdlog::error("SaveManager: Failed to delete slot {}: {}", slotId, ec.message());
        return false;
    }
    
    spdlog::info("SaveManager: Deleted slot {}", slotId);
    return true;
}

bool SaveManager::compressSlotToFile(int slotId, const std::vector<uint8_t>& dbData) {
    std::string filePath = getSlotFilePath(slotId);
    
    // Compress with zlib
    uLongf compressedSize = compressBound(dbData.size());
    std::vector<uint8_t> compressed(compressedSize);
    
    int result = compress2(
        compressed.data(),
        &compressedSize,
        dbData.data(),
        dbData.size(),
        9  // Maximum compression level
    );
    
    if (result != Z_OK) {
        spdlog::error("SaveManager: zlib compression failed");
        return false;
    }
    
    // Write to file
    std::ofstream file(filePath, std::ios::binary);
    if (!file) {
        spdlog::error("SaveManager: Failed to open file for writing: {}", filePath);
        return false;
    }
    
    SaveFileHeader header;
    header.magic = SAVE_MAGIC;
    header.version = SAVE_VERSION;
    header.compressedSize = static_cast<uint32_t>(compressedSize);
    header.uncompressedSize = static_cast<uint32_t>(dbData.size());
    
    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    file.write(reinterpret_cast<const char*>(compressed.data()), compressedSize);
    
    return file.good();
}

std::vector<uint8_t> SaveManager::decompressSlotFromFile(int slotId) {
    std::string filePath = getSlotFilePath(slotId);
    
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        spdlog::error("SaveManager: Failed to open file for reading: {}", filePath);
        return {};
    }
    
    SaveFileHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    
    if (header.magic != SAVE_MAGIC) {
        spdlog::error("SaveManager: Invalid save file magic");
        return {};
    }
    
    if (header.version != SAVE_VERSION) {
        spdlog::warn("SaveManager: Save file version mismatch (expected {}, got {})", 
                    SAVE_VERSION, header.version);
    }
    
    std::vector<uint8_t> compressed(header.compressedSize);
    file.read(reinterpret_cast<char*>(compressed.data()), header.compressedSize);
    
    if (!file.good()) {
        spdlog::error("SaveManager: Failed to read compressed data");
        return {};
    }
    
    std::vector<uint8_t> decompressed(header.uncompressedSize);
    uLongf decompressedSize = header.uncompressedSize;
    
    int result = uncompress(
        decompressed.data(),
        &decompressedSize,
        compressed.data(),
        header.compressedSize
    );
    
    if (result != Z_OK) {
        spdlog::error("SaveManager: zlib decompression failed");
        return {};
    }
    
    return decompressed;
}

void SaveManager::closeCurrentSlot() {
    closeDatabase();
}

void SaveManager::closeDatabase() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
    m_currentSlot = -1;
    m_inTransaction = false;
}

bool SaveManager::executeSQL(const std::string& sql) {
    if (!m_db) return false;
    
    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        spdlog::error("SaveManager: SQL error: {}", errMsg ? errMsg : "unknown");
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}

bool SaveManager::prepareStatement(const std::string& sql, sqlite3_stmt** stmt) const {
    if (!m_db) return false;
    
    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, stmt, nullptr);
    if (rc != SQLITE_OK) {
        spdlog::error("SaveManager: Failed to prepare statement: {}", sqlite3_errmsg(m_db));
        return false;
    }
    
    return true;
}

bool SaveManager::beginTransaction() {
    if (m_inTransaction) return false;
    bool success = executeSQL("BEGIN TRANSACTION");
    if (success) m_inTransaction = true;
    return success;
}

bool SaveManager::commitTransaction() {
    if (!m_inTransaction) return false;
    bool success = executeSQL("COMMIT");
    if (success) m_inTransaction = false;
    return success;
}

bool SaveManager::rollbackTransaction() {
    if (!m_inTransaction) return false;
    bool success = executeSQL("ROLLBACK");
    m_inTransaction = false;
    return success;
}

// Data access implementations
bool SaveManager::setString(const std::string& key, const std::string& value) {
    if (!m_db) return false;
    
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT OR REPLACE INTO game_data (key, value_type, value) VALUES (?, 'string', ?)";
    
    if (!prepareStatement(sql, &stmt)) return false;
    
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

std::string SaveManager::getString(const std::string& key, const std::string& defaultValue) const {
    if (!m_db) return defaultValue;
    
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT value FROM game_data WHERE key = ? AND value_type = 'string'";
    
    if (!prepareStatement(sql, &stmt)) return defaultValue;
    
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
    
    std::string result = defaultValue;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    }
    
    sqlite3_finalize(stmt);
    return result;
}

bool SaveManager::setInt(const std::string& key, int64_t value) {
    return setString(key, std::to_string(value));
}

int64_t SaveManager::getInt(const std::string& key, int64_t defaultValue) const {
    std::string str = getString(key, std::to_string(defaultValue));
    try {
        return std::stoll(str);
    } catch (...) {
        return defaultValue;
    }
}

bool SaveManager::setFloat(const std::string& key, double value) {
    return setString(key, std::to_string(value));
}

double SaveManager::getFloat(const std::string& key, double defaultValue) const {
    std::string str = getString(key, std::to_string(defaultValue));
    try {
        return std::stod(str);
    } catch (...) {
        return defaultValue;
    }
}

bool SaveManager::setBool(const std::string& key, bool value) {
    return setString(key, value ? "1" : "0");
}

bool SaveManager::getBool(const std::string& key, bool defaultValue) const {
    std::string str = getString(key, defaultValue ? "1" : "0");
    return str == "1" || str == "true";
}

} // namespace cosmiccities
