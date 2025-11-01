#pragma once
#include <string>
#include <vector>
#include <functional>
#include <curl/curl.h>

class DownloadManager {
public:
    DownloadManager();
    ~DownloadManager();
    
    // 下载状态枚举
    enum DownloadState {
        DOWNLOAD_IDLE,           // 空闲
        DOWNLOAD_CONNECTING,     // 连接中
        DOWNLOAD_DOWNLOADING,    // 下载中
        DOWNLOAD_VERIFYING,      // 校验中
        DOWNLOAD_EXTRACTING,     // 解压中
        DOWNLOAD_CLEANING,       // 清理中
        DOWNLOAD_COMPLETE,       // 完成
        DOWNLOAD_ERROR           // 错误
    };
    
    // 开始下载（会自动尝试所有镜像）
    bool StartDownload(const std::vector<std::string>& mirrorUrls, 
                       const std::string& savePath);
    
    // 开始下载并设置期望的 SHA-256（用于校验）
    bool StartDownload(const std::vector<std::string>& mirrorUrls, 
                       const std::string& savePath,
                       const std::string& expectedSHA256);
    
    // 更新下载进度（每帧调用）
    bool Update();
    
    // 取消下载
    void CancelDownload();
    
    // 获取当前状态
    DownloadState GetState() const { return mState; }
    
    // 获取下载进度 (0.0 - 1.0)
    float GetProgress() const { return mProgress; }
    
    // 获取下载速度 (bytes/sec)
    long GetSpeed() const { return mDownloadSpeed; }
    
    // 获取已下载大小
    long GetDownloadedSize() const { return mDownloadedSize; }
    
    // 获取总大小
    long GetTotalSize() const { return mTotalSize; }
    
    // 获取当前使用的镜像索引
    int GetCurrentMirrorIndex() const { return mCurrentMirrorIndex; }
    
    // 获取错误信息
    std::string GetError() const { return mErrorMessage; }
    
    // 获取计算出的 SHA-256 值
    std::string GetComputedSHA256() const { return mComputedSHA256; }
    
    // 设置进度回调
    void SetProgressCallback(std::function<void(float, long, long)> callback) {
        mProgressCallback = callback;
    }
    
    // 设置状态回调
    void SetStateCallback(std::function<void(DownloadState, const std::string&)> callback) {
        mStateCallback = callback;
    }

private:
    // 内部下载函数（尝试单个URL）
    bool DownloadFromUrl(const std::string& url, const std::string& savePath);
    
    // 解压ZIP文件
    bool ExtractZip(const std::string& zipPath, const std::string& extractPath);
    
    // 清理临时文件
    void CleanupTempFiles();
    
    // SHA-256 计算
    std::string CalculateSHA256(const std::string& filePath);
    bool VerifySHA256(const std::string& filePath, const std::string& expectedSHA256);
    
    // CURL回调函数
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static int ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
    static int SocketOptCallback(void* clientp, curl_socket_t curlfd, curlsocktype purpose);
    
    // 成员变量
    DownloadState mState;
    float mProgress;
    long mDownloadSpeed;
    long mDownloadedSize;
    long mTotalSize;
    int mCurrentMirrorIndex;
    std::string mErrorMessage;
    std::string mTempFilePath;
    std::string mExtractPath;
    std::string mOriginalFileName;  // 原始文件名
    std::string mExpectedSHA256;    // 期望的 SHA-256 值
    std::string mComputedSHA256;    // 计算出的 SHA-256 值
    
    std::vector<std::string> mMirrorUrls;
    std::function<void(float, long, long)> mProgressCallback;
    std::function<void(DownloadState, const std::string&)> mStateCallback;
    
    FILE* mDownloadFile;
    bool mCancelRequested;
    
    // CURL multi handle (用于非阻塞下载)
    CURLM* mCurlMulti;
    CURL* mCurl;
    bool mIsDownloading;
};
