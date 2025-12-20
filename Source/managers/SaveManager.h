#pragma once

#include "../Includes.hpp"
#include <sqlite3.h>
#include <zlib.h>
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace cosmiccities {

struct SaveSlotInfo {
    int slotId;
    bool exists;
    std::string playerName;
    int64_t lastSaveTime;
    int chapter;
    int level;
    float playtime; // in seconds
};

class SaveManager {
public:
    static SaveManager& instance();

    // Initialize the save system
    bool initialize(const std::string& saveDirectory);

    // Slot management
    bool doesSlotExist(int slotId) const;
    SaveSlotInfo getSlotInfo(int slotId) const;
    std::vector<SaveSlotInfo> getAllSlots() const;
    
    // Load/Save operations
    bool loadSlot(int slotId);
    bool saveSlot(int slotId);
    bool deleteSlot(int slotId);
    bool createNewSlot(int slotId, const std::string& playerName);

    // Current slot operations
    int getCurrentSlot() const { return m_currentSlot; }
    bool hasLoadedSlot() const { return m_currentSlot >= 0; }

    // Data access methods (key-value store)
    bool setString(const std::string& key, const std::string& value);
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    
    bool setInt(const std::string& key, int64_t value);
    int64_t getInt(const std::string& key, int64_t defaultValue = 0) const;
    
    bool setFloat(const std::string& key, double value);
    double getFloat(const std::string& key, double defaultValue = 0.0) const;
    
    bool setBool(const std::string& key, bool value);
    bool getBool(const std::string& key, bool defaultValue = false) const;

    // Batch operations for better performance
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // Utility
    void closeCurrentSlot();
    std::string getSlotFilePath(int slotId) const;

private:
    SaveManager() = default;
    ~SaveManager();

    // Prevent copying
    SaveManager(const SaveManager&) = delete;
    SaveManager& operator=(const SaveManager&) = delete;

    // Internal helpers
    bool compressSlotToFile(int slotId, const std::vector<uint8_t>& dbData);
    std::vector<uint8_t> decompressSlotFromFile(int slotId);
    
    bool createDatabase();
    bool openSlotDatabase(int slotId);
    void closeDatabase();
    
    bool executeSQL(const std::string& sql);
    bool prepareStatement(const std::string& sql, sqlite3_stmt** stmt) const;

    std::string m_saveDirectory;
    sqlite3* m_db = nullptr;
    int m_currentSlot = -1;
    bool m_inTransaction = false;
};

} // namespace cosmiccities
