#include "BackupManager.hpp"
#include "Utils.hpp"
#include "logger.h"
#include "FileLogger.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <cstdio>
#include <cstring>
#include <cerrno>

BackupManager::BackupManager()
    : mTotalItems(0)
    , mProcessedItems(0)
    , mSkippedItems(0)
    , mScannedDirs(0)
    , mIsBackupInProgress(false)
    , mIsScanning(false)
    , mIsSelectiveScan(false)
    , mCopySourceFile(nullptr)
    , mCopyDestFile(nullptr)
    , mCopyFileSize(0)
    , mCopyFileCopied(0)
    , mIsCopyingFile(false)
    , mProgressCallback(nullptr)
    , mErrorCallback(nullptr) {
}

BackupManager::~BackupManager() {
    EndFileCopy();
}

bool BackupManager::StartBackup(const std::string& sourcePath, const std::string& backupPath) {
    WHBLogPrintf("开始备份: %s -> %s", sourcePath.c_str(), backupPath.c_str());
    FileLogger::GetInstance().Log("开始全量备份: %s -> %s", sourcePath.c_str(), backupPath.c_str());
    
    // 重置状态
    mTotalItems = 0;
    mProcessedItems = 0;
    mSkippedItems = 0;
    mScannedDirs = 0;
    mCurrentFile = "";
    mSourcePath = sourcePath;
    mBackupPath = backupPath;
    mIsBackupInProgress = true;
    mIsScanning = true;
    mIsSelectiveScan = false;
    
    // 清空队列
    while (!mPendingFiles.empty()) {
        mPendingFiles.pop();
    }
    while (!mPendingScans.empty()) {
        mPendingScans.pop();
    }
    
    // 创建备份目录
    if (!Utils::CreateSubfolder(backupPath)) {
        if (mErrorCallback) {
            mErrorCallback("创建备份目录失败: " + backupPath);
        }
        mIsBackupInProgress = false;
        mIsScanning = false;
        return false;
    }
    
    // 添加根目录到扫描队列（异步扫描）
    mPendingScans.push(ScanEntry(sourcePath, backupPath));
    
    return true;
}

bool BackupManager::StartSelectiveBackup(const std::string& mlcPath, const std::string& sdSourcePath, const std::string& backupPath) {
    WHBLogPrintf("开始选择性备份: MLC=%s, SD=%s -> %s", mlcPath.c_str(), sdSourcePath.c_str(), backupPath.c_str());
    FileLogger::GetInstance().Log("开始选择性备份: MLC=%s, SD=%s -> %s", mlcPath.c_str(), sdSourcePath.c_str(), backupPath.c_str());
    
    // 重置状态
    mTotalItems = 0;
    mProcessedItems = 0;
    mSkippedItems = 0;
    mScannedDirs = 0;
    mCurrentFile = "";
    mSourcePath = mlcPath;
    mBackupPath = backupPath;
    mSdSourceBasePath = sdSourcePath;
    mIsBackupInProgress = true;
    mIsScanning = true;
    mIsSelectiveScan = true;
    
    // 清空队列
    while (!mPendingFiles.empty()) {
        mPendingFiles.pop();
    }
    while (!mPendingScans.empty()) {
        mPendingScans.pop();
    }
    
    // 创建备份目录
    if (!Utils::CreateSubfolder(backupPath)) {
        if (mErrorCallback) {
            mErrorCallback("创建备份目录失败: " + backupPath);
        }
        mIsBackupInProgress = false;
        mIsScanning = false;
        return false;
    }
    
    // 添加根目录到扫描队列（异步扫描）
    mPendingScans.push(ScanEntry(mlcPath, backupPath));
    
    return true;
}

void BackupManager::ScanDirectory(const std::string& srcPath, const std::string& dstPath) {
    DIR* dir = opendir(srcPath.c_str());
    if (!dir) return;

    struct dirent* dp;
    while ((dp = readdir(dir)) != nullptr) {
        std::string name = dp->d_name;
        if (name == "." || name == "..") continue;

        std::string fullSrcPath = srcPath + "/" + name;
        std::string fullDstPath = dstPath + "/" + name;
        
        struct stat filestat;
        if (stat(fullSrcPath.c_str(), &filestat) == 0) {
            bool isDir = (filestat.st_mode & S_IFMT) == S_IFDIR;
            
            if (isDir) {
                // 递归扫描子目录
                ScanDirectory(fullSrcPath, fullDstPath);
            } else {
                // 只添加文件
                mPendingFiles.push(FileEntry(fullSrcPath, false));
                mTotalItems++;
            }
        }
    }
    closedir(dir);
}

bool BackupManager::UpdateScan() {
    if (mPendingScans.empty()) {
        // 扫描完成
        mIsScanning = false;
        WHBLogPrintf("扫描完成: 总计 %d 个项目, 扫描了 %d 个目录", mTotalItems, mScannedDirs);
        FileLogger::GetInstance().Log("扫描完成: 总计 %d 个项目, 扫描了 %d 个目录", mTotalItems, mScannedDirs);
        return false;
    }
    
    // 每帧只处理一个目录
    ScanEntry entry = mPendingScans.front();
    mPendingScans.pop();
    mScannedDirs++;  // 增加已扫描目录计数
    
    DIR* dir = opendir(entry.srcPath.c_str());
    if (!dir) {
        // 无法打开目录,记录错误
        if (mErrorCallback) {
            mErrorCallback("无法打开目录: " + entry.srcPath);
        }
        // 继续处理下一个目录
        return true;
    }

    struct dirent* dp;
    while ((dp = readdir(dir)) != nullptr) {
        std::string name = dp->d_name;
        if (name == "." || name == "..") continue;

        std::string fullSrcPath = entry.srcPath + "/" + name;
        std::string fullDstPath = entry.dstPath + "/" + name;
        
        struct stat filestat;
        if (stat(fullSrcPath.c_str(), &filestat) == 0) {
            bool isDir = (filestat.st_mode & S_IFMT) == S_IFDIR;
            
            if (isDir) {
                if (mIsSelectiveScan) {
                    // 选择性备份：检查SD卡中是否存在同名目录
                    std::string relativePath = fullSrcPath;
                    if (relativePath.length() > mSourcePath.length()) {
                        relativePath = relativePath.substr(mSourcePath.length());
                    }
                    std::string sdPath = mSdSourceBasePath + relativePath;
                    
                    struct stat sdStat;
                    if (stat(sdPath.c_str(), &sdStat) == 0) {
                        bool sdIsDir = (sdStat.st_mode & S_IFMT) == S_IFDIR;
                        if (sdIsDir) {
                            // SD卡中存在同名目录，添加到队列并继续扫描
                            WHBLogPrintf("[选择性] 包含目录: %s", relativePath.c_str());
                            mPendingFiles.push(FileEntry(fullSrcPath, true));
                            mTotalItems++;
                            mPendingScans.push(ScanEntry(fullSrcPath, fullDstPath));
                        } else {
                            WHBLogPrintf("[选择性] 跳过目录(SD是文件): %s", relativePath.c_str());
                        }
                    } else {
                        WHBLogPrintf("[选择性] 跳过目录(SD不存在): %s", relativePath.c_str());
                    }
                } else {
                    // 全量备份：添加所有目录
                    mPendingFiles.push(FileEntry(fullSrcPath, true));
                    mTotalItems++;
                    mPendingScans.push(ScanEntry(fullSrcPath, fullDstPath));
                }
            } else {
                if (mIsSelectiveScan) {
                    // 选择性备份：检查SD卡中是否存在同名文件
                    std::string relativePath = fullSrcPath;
                    if (relativePath.length() > mSourcePath.length()) {
                        relativePath = relativePath.substr(mSourcePath.length());
                    }
                    std::string sdPath = mSdSourceBasePath + relativePath;
                    
                    struct stat sdStat;
                    if (stat(sdPath.c_str(), &sdStat) == 0) {
                        bool sdIsFile = (sdStat.st_mode & S_IFMT) != S_IFDIR;
                        if (sdIsFile) {
                            // SD卡中存在同名文件，需要备份
                            WHBLogPrintf("[选择性] 包含文件: %s", relativePath.c_str());
                            mPendingFiles.push(FileEntry(fullSrcPath, false));
                            mTotalItems++;
                        } else {
                            WHBLogPrintf("[选择性] 跳过文件(SD是目录): %s", relativePath.c_str());
                        }
                    } else {
                        WHBLogPrintf("[选择性] 跳过文件(SD不存在): %s", relativePath.c_str());
                    }
                } else {
                    // 全量备份：添加所有文件
                    mPendingFiles.push(FileEntry(fullSrcPath, false));
                    mTotalItems++;
                }
            }
        } else {
            // stat 失败,可能是权限问题或特殊文件
            // 静默跳过,避免中断备份
        }
    }
    closedir(dir);
    
    return true;
}

void BackupManager::ScanDirectorySelective(const std::string& mlcPath, const std::string& sdSourcePath, const std::string& backupPath) {
    DIR* dir = opendir(mlcPath.c_str());
    if (!dir) return;

    struct dirent* dp;
    while ((dp = readdir(dir)) != nullptr) {
        std::string name = dp->d_name;
        if (name == "." || name == "..") continue;

        std::string fullMlcPath = mlcPath + "/" + name;
        std::string fullSdPath = sdSourcePath + "/" + name;
        std::string fullBackupPath = backupPath + "/" + name;
        
        struct stat mlcStat;
        if (stat(fullMlcPath.c_str(), &mlcStat) == 0) {
            bool isDir = (mlcStat.st_mode & S_IFMT) == S_IFDIR;
            
            if (isDir) {
                // 递归扫描子目录
                ScanDirectorySelective(fullMlcPath, fullSdPath, fullBackupPath);
            } else {
                // 检查SD卡中是否存在同名文件（且也是文件，不是目录）
                struct stat sdStat;
                if (stat(fullSdPath.c_str(), &sdStat) == 0) {
                    bool sdIsFile = (sdStat.st_mode & S_IFMT) != S_IFDIR;
                    if (sdIsFile) {
                        // SD卡中存在同名文件，说明会被覆盖，需要备份MLC中的文件
                        mPendingFiles.push(FileEntry(fullMlcPath, false));
                        mTotalItems++;
                    }
                }
            }
        }
    }
    closedir(dir);
}

bool BackupManager::UpdateBackup() {
    if (!mIsBackupInProgress) {
        return false;
    }
    
    // 如果正在扫描，继续扫描
    if (mIsScanning) {
        UpdateScan();
        return true;  // 继续扫描
    }
    
    // 如果正在复制文件，继续复制
    if (mIsCopyingFile) {
        if (!ContinueFileCopy()) {
            if (mErrorCallback) {
                mErrorCallback("备份文件失败");
            }
            mIsBackupInProgress = false;
            return false;
        }
        
        // 如果文件复制完成
        if (!mIsCopyingFile) {
            mProcessedItems++;
            if (mProgressCallback) {
                mProgressCallback(mProcessedItems, mTotalItems, mCurrentFile);
            }
        }
        return true;
    }
    
    // 处理下一个文件或目录
    if (mPendingFiles.empty()) {
        // 备份完成
        WHBLogPrintf("备份完成: 总计 %d 项, 已处理 %d 项, 跳过 %d 项", 
                     mTotalItems, mProcessedItems, mSkippedItems);
        FileLogger::GetInstance().Log("备份完成: 总计 %d 项, 已处理 %d 项, 跳过 %d 项", 
                                          mTotalItems, mProcessedItems, mSkippedItems);
        mIsBackupInProgress = false;
        return false;
    }
    
    FileEntry entry = mPendingFiles.front();
    mPendingFiles.pop();
    
    // 如果是目录，只需创建目录
    if (entry.isDirectory) {
        // 计算目标路径
        std::string relativePath = entry.path;
        if (relativePath.length() > mSourcePath.length()) {
            relativePath = relativePath.substr(mSourcePath.length());
        }
        std::string dstPath = mBackupPath + relativePath;
        
        // 创建目录
        if (!Utils::CreateSubfolder(dstPath)) {
            if (mErrorCallback) {
                mErrorCallback("创建目录失败: " + dstPath);
            }
        }
        
        // 目录创建完成，立即处理下一项
        mProcessedItems++;
        return true;
    }
    
    // 处理文件
    // 计算目标路径
    std::string relativePath = entry.path;
    if (relativePath.length() > mSourcePath.length()) {
        relativePath = relativePath.substr(mSourcePath.length());
    }
    std::string dstPath = mBackupPath + relativePath;
    
    // 更新当前文件
    if (relativePath.length() > 0 && (relativePath[0] == '/' || relativePath[0] == '\\')) {
        mCurrentFile = relativePath.substr(1);
    } else {
        mCurrentFile = relativePath;
    }
    
    // 确保父目录存在
    size_t lastSlash = dstPath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        std::string parentDir = dstPath.substr(0, lastSlash);
        if (!Utils::CreateSubfolder(parentDir)) {
            if (mErrorCallback) {
                mErrorCallback("创建备份父目录失败: " + parentDir);
            }
            mIsBackupInProgress = false;
            return false;
        }
    }
    
    // 开始复制文件
    if (!StartFileCopy(entry.path, dstPath)) {
        // 文件复制失败,记录错误但继续处理其他文件
        WHBLogPrintf("跳过无法访问的文件: %s", entry.path.c_str());
        FileLogger::GetInstance().LogError("跳过无法访问的文件: %s", entry.path.c_str());
        mSkippedItems++;  // 增加跳过计数
        // 跳过这个文件,继续处理下一个
        mProcessedItems++;
        return true;
    }
    
    // 记录详细的文件操作日志(如果启用)
    FileLogger::GetInstance().LogFileOperation(entry.path.c_str(), dstPath.c_str());
    
    return true;
}

void BackupManager::CancelBackup() {
    EndFileCopy();
    mIsBackupInProgress = false;
    while (!mPendingFiles.empty()) {
        mPendingFiles.pop();
    }
}

bool BackupManager::StartFileCopy(const std::string& srcPath, const std::string& dstPath) {
    // 打开源文件
    mCopySourceFile = fopen(srcPath.c_str(), "rb");
    if (!mCopySourceFile) {
        // 无法打开源文件,可能是权限问题或特殊文件
        WHBLogPrintf("无法打开源文件: %s (errno: %d)", srcPath.c_str(), errno);
        return false;
    }
    
    // 获取文件大小
    fseek(mCopySourceFile, 0, SEEK_END);
    mCopyFileSize = ftell(mCopySourceFile);
    fseek(mCopySourceFile, 0, SEEK_SET);
    
    WHBLogPrintf("开始复制文件: %s (%ld 字节)", mCurrentFile.c_str(), mCopyFileSize);
    
    // 打开目标文件
    mCopyDestFile = fopen(dstPath.c_str(), "wb");
    if (!mCopyDestFile) {
        WHBLogPrintf("无法打开目标文件: %s (errno: %d)", dstPath.c_str(), errno);
        fclose(mCopySourceFile);
        mCopySourceFile = nullptr;
        return false;
    }
    
    mCopyFileCopied = 0;
    mIsCopyingFile = true;
    return true;
}

bool BackupManager::ContinueFileCopy() {
    if (!mIsCopyingFile || !mCopySourceFile || !mCopyDestFile) {
        return false;
    }
    
    // 每次复制 256KB
    const size_t CHUNK_SIZE = 256 * 1024;
    char buffer[CHUNK_SIZE];
    
    size_t bytesRead = fread(buffer, 1, CHUNK_SIZE, mCopySourceFile);
    if (bytesRead > 0) {
        size_t bytesWritten = fwrite(buffer, 1, bytesRead, mCopyDestFile);
        if (bytesWritten != bytesRead) {
            EndFileCopy();
            return false;
        }
        mCopyFileCopied += bytesRead;
    }
    
    // 检查是否复制完成
    if (mCopyFileCopied >= mCopyFileSize || feof(mCopySourceFile)) {
        WHBLogPrintf("文件复制完成: %s (%ld 字节)", mCurrentFile.c_str(), mCopyFileSize);
        EndFileCopy();
        return true;
    }
    
    return true;
}

void BackupManager::EndFileCopy() {
    if (mCopySourceFile) {
        fclose(mCopySourceFile);
        mCopySourceFile = nullptr;
    }
    if (mCopyDestFile) {
        fclose(mCopyDestFile);
        mCopyDestFile = nullptr;
    }
    mIsCopyingFile = false;
    mCopyFileSize = 0;
    mCopyFileCopied = 0;
}
