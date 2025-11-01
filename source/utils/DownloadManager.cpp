#include "DownloadManager.hpp"
#include "Utils.hpp"
#include "FileLogger.hpp"
#include "Config.hpp"
#include "minizip/unzip.h"
#include <curl/curl.h>
#include <whb/log.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <mbedtls/sha256.h>
#include <SDL.h>  // 用于事件处理

// 临时下载目录
#define TEMP_DOWNLOAD_DIR "fs:/vol/external01/wiiu/apps/LingoBrew/"
#define EXTRACT_TARGET_PATH "fs:/vol/external01"

DownloadManager::DownloadManager()
    : mState(DOWNLOAD_IDLE)
    , mProgress(0.0f)
    , mDownloadSpeed(0)
    , mDownloadedSize(0)
    , mTotalSize(0)
    , mCurrentMirrorIndex(0)
    , mExtractPath(EXTRACT_TARGET_PATH)
    , mDownloadFile(nullptr)
    , mCancelRequested(false)
    , mCurlMulti(nullptr)
    , mCurl(nullptr)
    , mIsDownloading(false)
{
    // 初始化 CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

DownloadManager::~DownloadManager() {
    CancelDownload();
    CleanupTempFiles();
    curl_global_cleanup();
}

bool DownloadManager::StartDownload(const std::vector<std::string>& mirrorUrls, 
                                    const std::string& savePath) {
    return StartDownload(mirrorUrls, savePath, "");
}

bool DownloadManager::StartDownload(const std::vector<std::string>& mirrorUrls, 
                                    const std::string& savePath,
                                    const std::string& expectedSHA256) {
    if (mState != DOWNLOAD_IDLE && mState != DOWNLOAD_COMPLETE && mState != DOWNLOAD_ERROR) {
        WHBLogPrintf("下载已在进行中");
        return false;
    }
    
    if (mirrorUrls.empty()) {
        mErrorMessage = "没有提供下载地址";
        mState = DOWNLOAD_ERROR;
        if (mStateCallback) mStateCallback(mState, mErrorMessage);
        return false;
    }
    
    mMirrorUrls = mirrorUrls;
    mCurrentMirrorIndex = 0;
    mCancelRequested = false;
    mProgress = 0.0f;
    mDownloadedSize = 0;
    mTotalSize = 0;
    mErrorMessage = "";
    mExpectedSHA256 = expectedSHA256;
    mComputedSHA256 = "";
    
    // 从URL提取文件名
    std::string url = mirrorUrls[0];
    size_t lastSlash = url.find_last_of('/');
    if (lastSlash != std::string::npos) {
        mOriginalFileName = url.substr(lastSlash + 1);
    } else {
        mOriginalFileName = "download.zip";
    }
    
    // 构建完整路径
    mTempFilePath = std::string(TEMP_DOWNLOAD_DIR) + mOriginalFileName;
    
    WHBLogPrintf("开始下载，共 %d 个镜像源", mirrorUrls.size());
    WHBLogPrintf("文件名: %s", mOriginalFileName.c_str());
    FileLogger::GetInstance().Log("下载镜像源数量: %zu", mirrorUrls.size());
    FileLogger::GetInstance().Log("目标文件名: %s", mOriginalFileName.c_str());
    FileLogger::GetInstance().Log("保存路径: %s", mTempFilePath.c_str());
    
    // 尝试第一个镜像
    return DownloadFromUrl(mMirrorUrls[0], mTempFilePath);
}

bool DownloadManager::DownloadFromUrl(const std::string& url, const std::string& savePath) {
    mState = DOWNLOAD_CONNECTING;
    if (mStateCallback) mStateCallback(mState, "正在连接 " + url);
    
    WHBLogPrintf("尝试从镜像 %d 下载: %s", mCurrentMirrorIndex, url.c_str());
    FileLogger::GetInstance().Log("连接镜像 #%d: %s", mCurrentMirrorIndex + 1, url.c_str());
    
    // 确保临时目录存在（递归创建）
    mkdir("fs:/vol", 0777);
    mkdir("fs:/vol/external01", 0777);
    mkdir("fs:/vol/external01/wiiu", 0777);
    mkdir("fs:/vol/external01/wiiu/apps", 0777);
    mkdir("fs:/vol/external01/wiiu/apps/LingoBrew", 0777);
    
    // 打开临时文件
    mDownloadFile = fopen(savePath.c_str(), "wb");
    if (!mDownloadFile) {
        mErrorMessage = "无法创建临时文件: " + savePath;
        WHBLogPrintf("%s", mErrorMessage.c_str());
        FileLogger::GetInstance().LogError("%s", mErrorMessage.c_str());
        mState = DOWNLOAD_ERROR;
        if (mStateCallback) mStateCallback(mState, mErrorMessage);
        return false;
    }
    
    // 初始化 CURL easy handle
    mCurl = curl_easy_init();
    if (!mCurl) {
        fclose(mDownloadFile);
        mDownloadFile = nullptr;
        mErrorMessage = "无法初始化 CURL";
        mState = DOWNLOAD_ERROR;
        if (mStateCallback) mStateCallback(mState, mErrorMessage);
        return false;
    }
    
    // 设置 CURL 选项
    curl_easy_setopt(mCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, mDownloadFile);
    curl_easy_setopt(mCurl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(mCurl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
    curl_easy_setopt(mCurl, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(mCurl, CURLOPT_FOLLOWLOCATION, 1L);  // 跟随重定向
    
    // 超时设置优化 - 使用低速限制替代固定超时
    curl_easy_setopt(mCurl, CURLOPT_LOW_SPEED_LIMIT, 1024L);   // 低于1KB/s算慢速
    curl_easy_setopt(mCurl, CURLOPT_LOW_SPEED_TIME, 30L);      // 慢速持续30秒则超时
    curl_easy_setopt(mCurl, CURLOPT_CONNECTTIMEOUT, 30L);      // 30秒连接超时
    
    // SSL设置
    curl_easy_setopt(mCurl, CURLOPT_SSL_VERIFYPEER, 0L);   // 跳过SSL验证（Wii U需要）
    curl_easy_setopt(mCurl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    // 网络优化设置
    curl_easy_setopt(mCurl, CURLOPT_USERAGENT, "LingoBrew/1.0");
    curl_easy_setopt(mCurl, CURLOPT_BUFFERSIZE, 524288L);      // 512KB 接收缓冲区
    curl_easy_setopt(mCurl, CURLOPT_TCP_NODELAY, 1L);          // 禁用Nagle算法，减少延迟
    curl_easy_setopt(mCurl, CURLOPT_ACCEPT_ENCODING, "");      // 启用压缩传输(gzip/deflate)
    
    // 连接复用优化
    curl_easy_setopt(mCurl, CURLOPT_FRESH_CONNECT, 0L);        // 复用连接
    curl_easy_setopt(mCurl, CURLOPT_FORBID_REUSE, 0L);         // 允许连接复用
    
    // Socket回调函数用于底层优化
    curl_easy_setopt(mCurl, CURLOPT_SOCKOPTFUNCTION, SocketOptCallback);
    curl_easy_setopt(mCurl, CURLOPT_SOCKOPTDATA, this);
    
    // 创建 multi handle
    mCurlMulti = curl_multi_init();
    if (!mCurlMulti) {
        curl_easy_cleanup(mCurl);
        mCurl = nullptr;
        fclose(mDownloadFile);
        mDownloadFile = nullptr;
        mErrorMessage = "无法初始化 CURL multi";
        mState = DOWNLOAD_ERROR;
        if (mStateCallback) mStateCallback(mState, mErrorMessage);
        return false;
    }
    
    // 添加 easy handle 到 multi handle
    curl_multi_add_handle(mCurlMulti, mCurl);
    
    mState = DOWNLOAD_DOWNLOADING;
    mIsDownloading = true;
    if (mStateCallback) mStateCallback(mState, "下载中...");
    
    return true;
}

bool DownloadManager::ExtractZip(const std::string& zipPath, const std::string& extractPath) {
    mState = DOWNLOAD_EXTRACTING;
    if (mStateCallback) mStateCallback(mState, "正在解压...");
    
    WHBLogPrintf("开始解压: %s -> %s", zipPath.c_str(), extractPath.c_str());
    FileLogger::GetInstance().Log("========== 开始解压 ==========");
    FileLogger::GetInstance().Log("ZIP文件: %s", zipPath.c_str());
    FileLogger::GetInstance().Log("解压目标: %s", extractPath.c_str());
    
    // 打开 ZIP 文件
    unzFile zipFile = unzOpen(zipPath.c_str());
    if (!zipFile) {
        WHBLogPrintf("无法打开ZIP文件: %s", zipPath.c_str());
        FileLogger::GetInstance().LogError("无法打开ZIP文件");
        mState = DOWNLOAD_ERROR;
        if (mStateCallback) mStateCallback(mState, "无法打开ZIP文件");
        return false;
    }
    
    // 获取ZIP文件信息
    unz_global_info globalInfo;
    if (unzGetGlobalInfo(zipFile, &globalInfo) != UNZ_OK) {
        WHBLogPrintf("无法获取ZIP文件信息");
        FileLogger::GetInstance().LogError("无法读取ZIP文件信息");
        unzClose(zipFile);
        mState = DOWNLOAD_ERROR;
        if (mStateCallback) mStateCallback(mState, "无法读取ZIP文件信息");
        return false;
    }
    
    WHBLogPrintf("ZIP文件包含 %lu 个条目", globalInfo.number_entry);
    FileLogger::GetInstance().Log("ZIP文件包含 %lu 个条目", globalInfo.number_entry);
    
    // 遍历ZIP中的所有文件
    char filenameInZip[256];
    unz_file_info fileInfo;
    char buffer[8192];
    
    for (uLong i = 0; i < globalInfo.number_entry; i++) {
        // 获取当前文件信息
        if (unzGetCurrentFileInfo(zipFile, &fileInfo, filenameInZip, sizeof(filenameInZip),
                                  NULL, 0, NULL, 0) != UNZ_OK) {
            WHBLogPrintf("无法获取文件信息 (索引 %lu)", i);
            break;
        }
        
        // 构建目标路径
        std::string fullPath = extractPath;
        if (fullPath.back() != '/') fullPath += '/';
        fullPath += filenameInZip;
        
        WHBLogPrintf("解压: %s", filenameInZip);
        
        // 检查是否是目录
        if (filenameInZip[strlen(filenameInZip) - 1] == '/') {
            // 创建目录
            WHBLogPrintf("创建目录: %s", fullPath.c_str());
            if (Config::GetInstance().IsVerboseLogging()) {
                FileLogger::GetInstance().Log("创建目录: %s", filenameInZip);
            }
            mkdir(fullPath.c_str(), 0777);
        } else {
            // 创建父目录
            std::string dirPath = fullPath.substr(0, fullPath.find_last_of('/'));
            mkdir(dirPath.c_str(), 0777);
            
            // 打开ZIP中的文件
            if (unzOpenCurrentFile(zipFile) != UNZ_OK) {
                WHBLogPrintf("无法打开ZIP中的文件: %s", filenameInZip);
                FileLogger::GetInstance().LogError("无法打开ZIP中的文件: %s", filenameInZip);
                continue;
            }
            
            // 创建目标文件
            FILE* outFile = fopen(fullPath.c_str(), "wb");
            if (!outFile) {
                WHBLogPrintf("无法创建文件: %s", fullPath.c_str());
                FileLogger::GetInstance().LogError("无法创建文件: %s", filenameInZip);
                unzCloseCurrentFile(zipFile);
                continue;
            }
            
            // 解压并写入文件
            int bytesRead;
            while ((bytesRead = unzReadCurrentFile(zipFile, buffer, sizeof(buffer))) > 0) {
                fwrite(buffer, 1, bytesRead, outFile);
            }
            
            fclose(outFile);
            unzCloseCurrentFile(zipFile);
            
            WHBLogPrintf("已解压: %s (%lu 字节)", filenameInZip, fileInfo.uncompressed_size);
            if (Config::GetInstance().IsVerboseLogging()) {
                FileLogger::GetInstance().Log("解压文件: %s (%lu 字节)", filenameInZip, fileInfo.uncompressed_size);
            }
        }
        
        // 移动到下一个文件
        if (i + 1 < globalInfo.number_entry) {
            if (unzGoToNextFile(zipFile) != UNZ_OK) {
                WHBLogPrintf("无法移动到下一个文件");
                break;
            }
        }
        
        // 更新进度
        float progress = (float)(i + 1) / globalInfo.number_entry;
        mProgress = progress;
        if (mProgressCallback) {
            mProgressCallback(progress, i + 1, globalInfo.number_entry);
        }
        
        // 每处理10个文件就处理一次SDL事件，让UI能够更新
        if ((i + 1) % 10 == 0) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                // 处理事件但不响应（只是为了让系统保持响应）
            }
            SDL_Delay(1);  // 短暂延迟让系统有机会更新显示
        }
    }
    
    unzClose(zipFile);
    
    // 解压完成后删除ZIP文件
    WHBLogPrintf("解压完成，删除ZIP文件: %s", zipPath.c_str());
    FileLogger::GetInstance().Log("删除临时ZIP文件: %s", zipPath.c_str());
    remove(zipPath.c_str());
    
    mState = DOWNLOAD_COMPLETE;
    mProgress = 1.0f;
    if (mStateCallback) mStateCallback(mState, "下载并解压完成");
    
    WHBLogPrintf("解压成功完成");
    FileLogger::GetInstance().LogSuccess("解压完成! 共解压 %lu 个条目", globalInfo.number_entry);
    FileLogger::GetInstance().Log("文件已提取到: %s", extractPath.c_str());
    
    return true;
}

void DownloadManager::CleanupTempFiles() {
    mState = DOWNLOAD_CLEANING;
    
    // 删除临时ZIP文件
    if (!mTempFilePath.empty()) {
        if (remove(mTempFilePath.c_str()) == 0) {
            WHBLogPrintf("已删除临时文件: %s", mTempFilePath.c_str());
            FileLogger::GetInstance().Log("已删除临时文件: %s", mTempFilePath.c_str());
        } else {
            WHBLogPrintf("无法删除临时文件: %s", mTempFilePath.c_str());
        }
    }
}

bool DownloadManager::Update() {
    if (!mIsDownloading || !mCurlMulti) {
        return false;
    }
    
    // 执行 multi handle (非阻塞)
    int still_running = 0;
    CURLMcode mc = curl_multi_perform(mCurlMulti, &still_running);
    
    if (mc != CURLM_OK) {
        WHBLogPrintf("curl_multi_perform 错误: %s", curl_multi_strerror(mc));
        mIsDownloading = false;
        return false;
    }
    
    // 检查是否有完成的传输
    int msgs_left = 0;
    CURLMsg *msg = nullptr;
    while ((msg = curl_multi_info_read(mCurlMulti, &msgs_left))) {
        if (msg->msg == CURLMSG_DONE) {
            // 下载完成
            CURL* easy_handle = msg->easy_handle;
            CURLcode res = msg->data.result;
            
            // 清理
            curl_multi_remove_handle(mCurlMulti, easy_handle);
            curl_easy_cleanup(easy_handle);
            curl_multi_cleanup(mCurlMulti);
            mCurl = nullptr;
            mCurlMulti = nullptr;
            mIsDownloading = false;
            
            // 关闭文件
            if (mDownloadFile) {
                fclose(mDownloadFile);
                mDownloadFile = nullptr;
            }
            
            // 检查结果
            if (res != CURLE_OK) {
                // 下载失败
                if (mCancelRequested) {
                    mErrorMessage = "下载已取消";
                    mState = DOWNLOAD_ERROR;
                    if (mStateCallback) mStateCallback(mState, mErrorMessage);
                    CleanupTempFiles();
                    return false;
                }
                
                // 尝试下一个镜像
                mCurrentMirrorIndex++;
                if ((size_t)mCurrentMirrorIndex < mMirrorUrls.size()) {
                    WHBLogPrintf("镜像 %d 失败: %s，尝试下一个镜像", 
                                 mCurrentMirrorIndex - 1, curl_easy_strerror(res));
                    mProgress = 0.0f;
                    mDownloadedSize = 0;
                    return DownloadFromUrl(mMirrorUrls[mCurrentMirrorIndex], mTempFilePath);
                } else {
                    mErrorMessage = std::string("所有镜像下载失败: ") + curl_easy_strerror(res);
                    WHBLogPrintf("%s", mErrorMessage.c_str());
                    mState = DOWNLOAD_ERROR;
                    if (mStateCallback) mStateCallback(mState, mErrorMessage);
                    CleanupTempFiles();
                    return false;
                }
            }
            
            // 下载成功
            WHBLogPrintf("下载完成，开始校验");
            FileLogger::GetInstance().Log("下载完成: %ld 字节", mDownloadedSize);
            
            // SHA-256 校验（如果提供了期望值）
            if (!mExpectedSHA256.empty()) {
                if (!VerifySHA256(mTempFilePath, mExpectedSHA256)) {
                    mErrorMessage = "SHA-256 校验失败! 期望: " + mExpectedSHA256 + ", 实际: " + mComputedSHA256;
                    WHBLogPrintf("%s", mErrorMessage.c_str());
                    FileLogger::GetInstance().LogError("%s", mErrorMessage.c_str());
                    mState = DOWNLOAD_ERROR;
                    if (mStateCallback) mStateCallback(mState, mErrorMessage);
                    CleanupTempFiles();
                    return false;
                }
                WHBLogPrintf("SHA-256 校验成功: %s", mComputedSHA256.c_str());
                FileLogger::GetInstance().LogSuccess("SHA-256 校验通过: %s", mComputedSHA256.c_str());
            } else {
                // 即使不校验，也计算并记录 SHA-256
                mComputedSHA256 = CalculateSHA256(mTempFilePath);
                WHBLogPrintf("文件 SHA-256: %s", mComputedSHA256.c_str());
                FileLogger::GetInstance().Log("文件 SHA-256: %s", mComputedSHA256.c_str());
            }
            
            // 开始解压
            WHBLogPrintf("开始解压");
            ExtractZip(mTempFilePath, mExtractPath);
            return false;
        }
    }
    
    return still_running > 0;
}

void DownloadManager::CancelDownload() {
    mCancelRequested = true;
    WHBLogPrintf("请求取消下载");
    
    // 清理 CURL handles
    if (mCurlMulti && mCurl) {
        curl_multi_remove_handle(mCurlMulti, mCurl);
        curl_easy_cleanup(mCurl);
        curl_multi_cleanup(mCurlMulti);
        mCurl = nullptr;
        mCurlMulti = nullptr;
    }
    
    if (mDownloadFile) {
        fclose(mDownloadFile);
        mDownloadFile = nullptr;
    }
    
    mIsDownloading = false;
}

// CURL 写入回调
size_t DownloadManager::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    FILE* file = static_cast<FILE*>(userp);
    return fwrite(contents, size, nmemb, file);
}

// CURL 进度回调
int DownloadManager::ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, 
                                      curl_off_t ultotal, curl_off_t ulnow) {
    DownloadManager* manager = static_cast<DownloadManager*>(clientp);
    
    if (manager->mCancelRequested) {
        return 1;  // 返回非0值取消下载
    }
    
    manager->mTotalSize = static_cast<long>(dltotal);
    manager->mDownloadedSize = static_cast<long>(dlnow);
    
    if (dltotal > 0) {
        manager->mProgress = static_cast<float>(static_cast<double>(dlnow) / static_cast<double>(dltotal));
    }
    
    // 计算速度（简化版本）
    static curl_off_t lastDlnow = 0;
    static time_t lastTime = time(nullptr);
    time_t currentTime = time(nullptr);
    
    if (currentTime > lastTime) {
        manager->mDownloadSpeed = static_cast<long>((dlnow - lastDlnow) / (currentTime - lastTime));
        lastDlnow = dlnow;
        lastTime = currentTime;
    }
    
    if (manager->mProgressCallback) {
        manager->mProgressCallback(manager->mProgress, manager->mDownloadedSize, manager->mTotalSize);
    }
    
    return 0;
}

// CURL Socket 优化回调 - 关键性能优化!
int DownloadManager::SocketOptCallback(void* clientp, curl_socket_t curlfd, curlsocktype purpose) {
    (void)clientp;  // 未使用
    (void)purpose;  // 未使用
    
    int opt = 1;
    int result;
    
    // 设置 TCP_NODELAY - 禁用Nagle算法，减少小包延迟
    result = setsockopt(curlfd, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(opt));
    if (result != 0) {
        WHBLogPrintf("Socket优化: TCP_NODELAY 设置失败");
    }
    
    // 设置接收缓冲区大小为 512KB
    opt = 524288;  // 512KB
    result = setsockopt(curlfd, SOL_SOCKET, SO_RCVBUF, (char*)&opt, sizeof(opt));
    if (result != 0) {
        WHBLogPrintf("Socket优化: SO_RCVBUF 设置失败");
    }
    
    // 设置发送缓冲区大小为 512KB
    opt = 524288;  // 512KB
    result = setsockopt(curlfd, SOL_SOCKET, SO_SNDBUF, (char*)&opt, sizeof(opt));
    if (result != 0) {
        WHBLogPrintf("Socket优化: SO_SNDBUF 设置失败");
    }
    
    FileLogger::GetInstance().Log("Socket优化已应用: TCP_NODELAY + 512KB缓冲区");
    
    return CURL_SOCKOPT_OK;
}

// 计算文件的 SHA-256 值
std::string DownloadManager::CalculateSHA256(const std::string& filePath) {
    FILE* file = fopen(filePath.c_str(), "rb");
    if (!file) {
        WHBLogPrintf("无法打开文件进行 SHA-256 计算: %s", filePath.c_str());
        return "";
    }
    
    mbedtls_sha256_context sha256Context;
    mbedtls_sha256_init(&sha256Context);
    mbedtls_sha256_starts_ret(&sha256Context, 0);  // 0 = SHA-256 (not SHA-224)
    
    unsigned char buffer[8192];
    size_t bytesRead;
    
    mState = DOWNLOAD_VERIFYING;
    if (mStateCallback) mStateCallback(mState, "正在计算 SHA-256...");
    
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        mbedtls_sha256_update_ret(&sha256Context, buffer, bytesRead);
        
        // 每处理一定数据就处理一次SDL事件，保持响应
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // 处理事件但不响应（只是为了让系统保持响应）
        }
    }
    
    unsigned char sha256Hash[32];  // SHA-256 输出是 32 字节
    mbedtls_sha256_finish_ret(&sha256Context, sha256Hash);
    mbedtls_sha256_free(&sha256Context);
    
    fclose(file);
    
    // 转换为十六进制字符串
    char sha256String[65];  // 32 字节 * 2 + null terminator
    for (int i = 0; i < 32; i++) {
        sprintf(&sha256String[i * 2], "%02x", sha256Hash[i]);
    }
    sha256String[64] = '\0';
    
    return std::string(sha256String);
}

// 验证文件的 SHA-256 值
bool DownloadManager::VerifySHA256(const std::string& filePath, const std::string& expectedSHA256) {
    mComputedSHA256 = CalculateSHA256(filePath);
    
    if (mComputedSHA256.empty()) {
        mErrorMessage = "无法计算 SHA-256";
        return false;
    }
    
    // 转换为小写进行比较（SHA-256 通常不区分大小写）
    std::string expected = expectedSHA256;
    std::string computed = mComputedSHA256;
    
    // 转小写
    for (char& c : expected) c = tolower(c);
    for (char& c : computed) c = tolower(c);
    
    return expected == computed;
}
