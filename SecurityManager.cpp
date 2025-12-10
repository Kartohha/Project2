#include "SecurityManager.h"
#include "FileManager.h"
#include "Utils.h"
#include <sstream>
#include <vector>

SecurityManager::SecurityManager(const std::string& cfgFile)
    : configFile(cfgFile), failedAttempts(0), lockUntil(0), maxAttempts(3), lockSeconds(300)
{
    loadConfig();
}

void SecurityManager::loadConfig() {
    auto lines = FileManager::readLines(configFile);
    for (const auto& line : lines) {
        if (line.find("master_hash=") == 0) {
            masterHash = line.substr(std::string("master_hash=").size());
        }
        else if (line.find("failed=") == 0) {
            failedAttempts = std::stoi(line.substr(std::string("failed=").size()));
        }
        else if (line.find("lock_until=") == 0) {
            lockUntil = std::stoll(line.substr(std::string("lock_until=").size()));
        }
        else if (line.find("max_attempts=") == 0) {
            maxAttempts = std::stoi(line.substr(std::string("max_attempts=").size()));
        }
        else if (line.find("lock_seconds=") == 0) {
            lockSeconds = std::stoi(line.substr(std::string("lock_seconds=").size()));
        }
    }
}

void SecurityManager::saveConfig() const {
    std::vector<std::string> lines;
    lines.push_back("master_hash=" + masterHash);
    lines.push_back("failed=" + std::to_string(failedAttempts));
    lines.push_back("lock_until=" + std::to_string(lockUntil));
    lines.push_back("max_attempts=" + std::to_string(maxAttempts));
    lines.push_back("lock_seconds=" + std::to_string(lockSeconds));
    FileManager::writeLines(configFile, lines);
}

bool SecurityManager::hasMasterPassword() const {
    return !masterHash.empty();
}

void SecurityManager::ensureDefaultMaster() {
    if (!hasMasterPassword()) {
        masterHash = Utils::hashPassword("admin");
        failedAttempts = 0;
        lockUntil = 0;
        saveConfig();
    }
}

bool SecurityManager::isLocked() const {
    long long now = Utils::currentTimeSeconds();
    return lockUntil > now;
}

bool SecurityManager::verifyMasterPassword(const std::string& candidate) {
    long long now = Utils::currentTimeSeconds();
    if (isLocked()) return false;
    if (Utils::hashPassword(candidate) == masterHash) {
        failedAttempts = 0;
        lockUntil = 0;
        saveConfig();
        return true;
    }
    else {
        failedAttempts++;
        if (failedAttempts >= maxAttempts) {
            lockUntil = now + lockSeconds;
        }
        saveConfig();
        return false;
    }
}

bool SecurityManager::setMasterPassword(const std::string& newPassword) {
    if (newPassword.empty()) return false;
    masterHash = Utils::hashPassword(newPassword);
    failedAttempts = 0;
    lockUntil = 0;
    saveConfig();
    return true;
}
