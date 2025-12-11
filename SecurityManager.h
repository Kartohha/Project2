#ifndef SECURITYMANAGER_H
#define SECURITYMANAGER_H

#include <string>

class SecurityManager {
    std::string configFile;
    std::string masterHash;
    int failedAttempts;
    long long lockUntil;
    int maxAttempts;
    int lockSeconds;
    void loadConfig();
    void saveConfig() const;
public:
    SecurityManager(const std::string& cfgFile = "config.txt");
    bool hasMasterPassword() const;
    bool verifyMasterPassword(const std::string& candidate);
    bool isLocked() const;
    void ensureDefaultMaster();
    bool setMasterPassword(const std::string& newPassword);
    int getFailedAttempts() const { return failedAttempts; }
    long long getLockUntil() const { return lockUntil; }
    int getMaxAttempts() const { return maxAttempts; }
    int getLockSeconds() const { return lockSeconds; }
};

#endif 
