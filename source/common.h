#pragma once
#include "version.h"

#define MLC_STORAGE_PATH           "storage_mlc_lingobrew"
#define SLC_STORAGE_PATH           "storage_slc_lingobrew"
#define SLCCMPT_STORAGE_PATH       "storage_slccmpt_lingobrew"

#define HAX_SOURCE_PATH			   "fs:/vol/external01/title"
#define HAX_DESTINATION_PATH	   MLC_STORAGE_PATH ":/sys/title"

// 字体文件路径
#define HAX_PLUGINS_SOURCE_PATH			   "fs:/vol/external01/wiiu/fonts"
#define HAX_PLUGINS_DESTINATION_PATH       HAX_DESTINATION_PATH "/0005001B/10042400/content"

// 备份路径 (SD卡)
#define BACKUP_BASE_PATH                   "fs:/vol/external01/mlcbackup"
#define BACKUP_TITLE_PATH                  BACKUP_BASE_PATH "/title"
#define BACKUP_FONTS_PATH                  BACKUP_BASE_PATH "/fonts"
#define BACKUP_SLC_PATH                    "fs:/vol/external01/slcbackup/slc"
#define BACKUP_SLCCMPT_PATH                "fs:/vol/external01/slcbackup/slccmpt"

#define APP_VERSION                "v1.0"
#define APP_VERSION_FULL           APP_VERSION APP_VERSION_EXTRA
