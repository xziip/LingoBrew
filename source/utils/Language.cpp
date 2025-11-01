#include "Language.hpp"
#include "Config.hpp"
#include "../Gfx.hpp"

Language::Language() 
    : mCurrentLanguage(LanguageCode::CHINESE) {
}

Language::~Language() {
}

Language& Language::GetInstance() {
    static Language instance;
    return instance;
}

void Language::SetLanguage(LanguageCode lang) {
    if (lang < LanguageCode::LANG_MIN || lang > LanguageCode::LANG_MAX) {
        lang = LanguageCode::CHINESE;
    }
    mCurrentLanguage = lang;
    LoadLanguage(lang);
    
    // 重新加载字体以支持日文系统字体
    Gfx::ReloadFonts();
}

const char* Language::Get(const char* key) const {
    auto it = mTexts.find(key);
    if (it != mTexts.end()) {
        return it->second.c_str();
    }
    return key; // 如果找不到翻译,返回键名
}

const char* Language::GetLanguageName(LanguageCode lang) const {
    switch (lang) {
        case LanguageCode::CHINESE:  return "简体中文";
        case LanguageCode::ENGLISH:  return "English";
        case LanguageCode::JAPANESE: return "日本語";
        default: return "Unknown";
    }
}

void Language::LoadLanguage(LanguageCode lang) {
    mTexts.clear();
    
    switch (lang) {
        case LanguageCode::CHINESE:
            // 主菜单
            mTexts["menu.copy"] = "复制 title 文件夹到 MLC";
            mTexts["menu.backup"] = "备份系统文件";
            mTexts["menu.restore"] = "恢复系统文件";
            mTexts["menu.reboot"] = "重启系统";
            mTexts["menu.settings"] = "设置";
            mTexts["menu.debug"] = "Debug 设置";
            mTexts["menu.about"] = "关于 LingoBrew";
            
            // 设置页面
            mTexts["settings.title"] = "设置";
            mTexts["settings.language"] = "语言";
            mTexts["settings.language.desc"] = "切换程序显示语言";
            mTexts["settings.show_copy_detect"] = "显示文件检测提醒";
            mTexts["settings.show_copy_detect.desc"] = "复制时是否显示文件检测提醒";
            mTexts["settings.download"] = "下载语言文件";
            mTexts["settings.download.desc"] = "从网络下载最新的语言文件";
            mTexts["settings.clear_titles"] = "清除Title文件夹";
            mTexts["settings.clear_titles.desc"] = "删除SD卡上的title文件夹";
            mTexts["settings.back"] = "返回";
            mTexts["settings.back.desc"] = "返回主菜单";
            
            // Debug页面
            mTexts["debug.title"] = "Debug 设置";
            mTexts["debug.logging"] = "日志功能";
            mTexts["debug.verbose"] = "详细日志";
            mTexts["debug.defaultpage"] = "默认页面";
            mTexts["debug.clearlogs"] = "清除日志";
            mTexts["debug.hidemenu"] = "隐藏菜单";
            mTexts["debug.logging.on.desc"] = "记录备份/恢复操作到 SD:/log/lingobrew/";
            mTexts["debug.logging.off.desc"] = "不记录日志";
            mTexts["debug.verbose.on.desc"] = "记录每个文件的详细复制信息";
            mTexts["debug.verbose.off.desc"] = "仅记录摘要信息";
            mTexts["debug.defaultpage.desc"] = "程序启动时自动打开的页面";
            mTexts["debug.clearlogs.desc"] = "删除 SD:/log/lingobrew/ 中的所有日志文件";
            mTexts["debug.hidemenu.desc"] = "隐藏后按住 +/-/L 三键重新显示";
            mTexts["debug.back.desc"] = "返回主菜单";
            mTexts["debug.page"] = "第";
            
            // 页面名称
            mTexts["page.main"] = "主菜单";
            mTexts["page.copy"] = "复制Title";
            mTexts["page.backup"] = "备份";
            mTexts["page.restore"] = "恢复";
            mTexts["page.reboot"] = "重启";
            mTexts["page.settings"] = "设置";
            mTexts["page.debug"] = "Debug";
            mTexts["page.about"] = "关于";
            mTexts["page.download"] = "下载";
            
            // 对话框
            mTexts["dialog.confirm.delete"] = "确定要删除所有日志文件吗?";
            mTexts["dialog.confirm.delete.titles"] = "确定要删除SD卡title文件夹吗?";
            mTexts["dialog.warning.irreversible"] = "此操作不可恢复";
            
            // 按钮提示
            mTexts["button.navigate"] = "导航";
            mTexts["button.select"] = "选择";
            mTexts["button.confirm"] = "确认";
            mTexts["button.back"] = "返回";
            mTexts["button.exit"] = "退出";
            mTexts["button.cancel"] = "取消";
            mTexts["button.page"] = "翻页";
            mTexts["button.on"] = "开启";
            mTexts["button.off"] = "关闭";
            
            // 通用提示
            mTexts["common.exit"] = "\ue044 退出";
            mTexts["common.back"] = "\ue001 返回";
            mTexts["common.back_hint"] = "按 \ue001 返回";
            mTexts["common.enabled"] = "已启用";
            mTexts["common.disabled"] = "已禁用";
            mTexts["common.select"] = "\ue07d 选择";
            mTexts["common.confirm"] = "\ue000 确认";
            mTexts["common.continue"] = "\ue000 继续";
            mTexts["common.continue_hint"] = "按 \ue000 继续";
            mTexts["common.start"] = "\ue000 开始";
            mTexts["common.start_hint"] = "按 \ue000 开始备份";
            mTexts["common.start_restore"] = "\ue000 开始恢复";
            mTexts["common.return"] = "\ue000 返回";
            mTexts["common.return_menu"] = "按 \ue000 返回菜单";
            mTexts["common.please_wait"] = "请稍候...";
            mTexts["common.button_a"] = "\ue000";
            mTexts["common.button_b"] = "\ue001";
            mTexts["common.button_x"] = "\ue002";
            mTexts["common.button_y"] = "\ue003";
            mTexts["common.button_updown"] = "\ue07d";
            
            // 下载页面
            mTexts["download.title"] = "下载语言文件";
            mTexts["download.select_region"] = "选择系统区域";
            mTexts["download.region_eu"] = "EU (欧洲)";
            mTexts["download.region_us"] = "US (美国)";
            mTexts["download.region_jp"] = "JP (日本)";
            mTexts["download.select_mirror"] = "选择镜像源";
            mTexts["download.mirror_original"] = "原镜像 (GitHub)";
            mTexts["download.mirror_accelerated"] = "加速镜像 (中国用户推荐)";
            mTexts["download.recommend_original"] = "推荐：原镜像 (国际用户)";
            mTexts["download.recommend_accelerated"] = "推荐：加速镜像 (中国用户)";
            mTexts["download.select_language"] = "选择要下载的语言";
            mTexts["download.lang_chinese"] = "简体中文";
            mTexts["download.file_to_download"] = "即将下载";
            mTexts["download.start"] = "开始下载";
            mTexts["download.downloading"] = "正在下载...";
            mTexts["download.extracting"] = "正在解压...";
            mTexts["download.complete"] = "下载完成！";
            mTexts["download.error"] = "下载失败";
            mTexts["download.cancel"] = "取消下载";
            mTexts["download.confirm_cancel"] = "确定要取消下载吗？";
            mTexts["download.cancel_warning"] = "已下载的数据将会丢失";
            mTexts["download.speed"] = "速度";
            mTexts["download.files"] = "个文件";
            mTexts["download.mirror"] = "镜像";
            mTexts["download.ask_copy"] = "是否前往复制页面复制文件？";
            mTexts["download.goto_copy"] = "前往复制";
            
            // 主屏幕(初始化)
            mTexts["main.init.mocha.error"] = "初始化 mocha 失败！\n请确保已更新或安装 Tiramisu/Aroma。";
            mTexts["main.init.mocha.loading"] = "正在初始化 mocha...";
            mTexts["main.init.fs.error"] = "初始化文件系统失败！";
            mTexts["main.init.fs.loading"] = "正在初始化文件系统...";
            mTexts["main.menu.loading"] = "正在加载菜单...";
            
            // 关于页面
            mTexts["about.credits"] = "制作人员";
            mTexts["about.fonts"] = "字体";
            mTexts["about.source"] = "源代码";
            mTexts["about.developer"] = "开发者:";
            mTexts["about.based_on"] = "基于 Maschell 的 WiiUCrashLogDumper\n 和 GaryOderNichts 的 WiiUIdent\n 和 YveltalGriffin 的 Haxcopy";
            mTexts["about.main_font"] = "主字体:";
            mTexts["about.main_font.name"] = "Wii U 系统字体";
            mTexts["about.icon_font"] = "图标字体:";
            mTexts["about.mono_font"] = "等宽字体:";
            
            // 重启页面
            mTexts["reboot.confirm"] = "确认要重启系统吗？";
            mTexts["reboot.confirm.button"] = "确认重启";
            mTexts["reboot.action"] = "重启";
            
            // 备份页面
            mTexts["backup.title"] = "备份 MLC/SLC 文件";
            mTexts["backup.select_storage"] = "选择存储类型";
            mTexts["backup.mlc_storage"] = "MLC 存储";
            mTexts["backup.mlc_desc1"] = "主存储 (8GB/32GB)";
            mTexts["backup.mlc_desc2"] = "包含游戏、存档、用户数据";
            mTexts["backup.slc_storage"] = "SLC 存储";
            mTexts["backup.slc_desc1"] = "系统闪存 (~512MB)";
            mTexts["backup.slc_desc2"] = "包含核心系统文件";
            mTexts["backup.select_mlc"] = "选择要备份的 MLC 内容";
            mTexts["backup.select_slc"] = "选择要备份的 SLC 内容";
            mTexts["backup.select_mlc_folder"] = "选择要备份的 MLC 文件夹";
            mTexts["backup.full_slc"] = "完整 SLC 备份";
            mTexts["backup.full_slc_desc"] = "备份整个 SLC 系统闪存";
            mTexts["backup.full_slc_path"] = "备份到: SD:/slcbackup";
            mTexts["backup.title_folder"] = "Title 文件夹";
            mTexts["backup.title_path"] = "路径: mlc:/sys/title";
            mTexts["backup.title_dest"] = "备份到: SD:/mlcbackup/title";
            mTexts["backup.fonts"] = "字体文件";
            mTexts["backup.fonts_path"] = "路径: mlc:/sys/title/.../content";
            mTexts["backup.fonts_dest"] = "备份到: SD:/mlcbackup/fonts";
            mTexts["backup.full_mlc"] = "完整 MLC 备份";
            mTexts["backup.full_mlc_desc"] = "可选择备份 sys 或 usr 文件夹";
            mTexts["backup.full_mlc_dest"] = "备份到: SD:/mlcbackup/full";
            mTexts["backup.select_mode"] = "选择备份模式";
            mTexts["backup.mode_all"] = "全量备份";
            mTexts["backup.mode_all_desc1"] = "- 备份所有现有文件";
            mTexts["backup.mode_all_desc2"] = "- 最安全，但占用空间较大";
            mTexts["backup.mode_full"] = "全量备份";
            mTexts["backup.mode_full_desc1"] = "- 备份所有现有文件";
            mTexts["backup.mode_full_desc2"] = "- 最安全，但占用空间较大";
            mTexts["backup.mode_selective"] = "选择性备份";
            mTexts["backup.mode_selective_desc1"] = "- 只备份SD卡中存在的同名文件";
            mTexts["backup.mode_selective_desc2"] = "- 节省空间";
            mTexts["backup.start_hint"] = "按 \ue000 开始备份  按 \ue001 返回";
            mTexts["backup.select_full"] = "选择要备份的 MLC 目录";
            mTexts["backup.sys_folder"] = "sys 文件夹";
            mTexts["backup.sys_desc"] = "完整系统文件夹";
            mTexts["backup.sys_path"] = "路径: mlc:/sys";
            mTexts["backup.usr_folder"] = "usr 文件夹";
            mTexts["backup.usr_desc"] = "用户数据文件夹";
            mTexts["backup.usr_path"] = "路径: mlc:/usr";
            mTexts["backup.select_slc_type"] = "选择 SLC 类型";
            mTexts["backup.slccmpt"] = "SLCCMPT (兼容模式)";
            mTexts["backup.slccmpt_desc"] = "推荐使用 - 更安全的访问方式";
            mTexts["backup.slccmpt_path"] = "路径: storage_slccmpt";
            mTexts["backup.slc_direct"] = "SLC (直接访问)";
            mTexts["backup.slc_direct_desc"] = "直接访问 SLC - 需谨慎操作";
            mTexts["backup.slc_direct_path"] = "路径: storage_slc";
            mTexts["backup.continue_hint"] = "按 \ue000 继续  按 \ue001 返回";
            mTexts["backup.scanning"] = "正在扫描文件...";
            mTexts["backup.scanned_dirs"] = "已扫描: %d 个目录";
            mTexts["backup.pending_dirs"] = "待扫描: %d 个目录";
            mTexts["backup.found_items"] = "已找到: %d 个项目";
            mTexts["backup.items_note"] = "  (包括文件和文件夹)";
            mTexts["backup.items_include"] = "(包括文件和文件夹)";
            mTexts["backup.please_wait"] = "请稍候,扫描大容量目录可能需要较长时间...";
            mTexts["backup.wait_scanning"] = "请稍候,扫描大容量目录可能需要较长时间...";
            mTexts["backup.wait_copying"] = "请勿在此过程中关机或退出，请耐心等待...";
            mTexts["backup.progress"] = "总进度: %d%% (%d/%d)";
            mTexts["backup.skipped"] = "  跳过: %d 个文件 (权限不足或特殊文件)";
            mTexts["backup.skipped_files"] = "跳过: %d 个文件";
            mTexts["backup.current_file"] = "当前文件: ";
            mTexts["backup.file_progress"] = "文件进度: %d%% (%s)";
            mTexts["backup.do_not_power_off"] = "请勿在此过程中关机或退出，请耐心等待...";
            mTexts["backup.complete"] = "备份完成！";
            mTexts["backup.total_items"] = "总计: %d 个项目";
            mTexts["backup.scanned"] = "扫描: %d 个目录";
            mTexts["backup.skipped_items"] = "跳过: %d 个文件";
            mTexts["backup.skipped_count"] = "跳过: %d 个文件";
            mTexts["backup.fonts_backed_up"] = "字体文件已备份到:";
            mTexts["backup.fonts_dest"] = "SD:/mlcbackup/fonts";
            mTexts["backup.title_backed_up"] = "Title 文件夹已备份到:";
            mTexts["backup.title_dest"] = "SD:/mlcbackup/title";
            mTexts["backup.sys_backed_up"] = "系统文件夹已备份到:";
            mTexts["backup.sys_dest"] = "SD:/mlcbackup/sys";
            mTexts["backup.usr_backed_up"] = "用户文件夹已备份到:";
            mTexts["backup.usr_dest"] = "SD:/mlcbackup/usr";
            mTexts["backup.slccmpt_backed_up"] = "SLCCMPT 已备份到:";
            mTexts["backup.slccmpt_dest"] = "SD:/slcbackup/slccmpt";
            mTexts["backup.slc_backed_up"] = "SLC 已备份到:";
            mTexts["backup.slc_dest"] = "SD:/slcbackup/slc";
            mTexts["backup.press_a_return"] = "按 \ue000 返回菜单";
            mTexts["backup.error_create_dir"] = "创建备份目录失败";
            
            // 恢复页面
            mTexts["restore.title"] = "恢复 MLC/SLC 文件";
            mTexts["restore.warning"] = "警告：恢复将覆盖现有文件！";
            mTexts["restore.select_storage"] = "选择存储类型";
            mTexts["restore.mlc_desc"] = "从 SD:/mlcbackup 恢复";
            mTexts["restore.slc_desc"] = "从 SD:/slcbackup 恢复";
            mTexts["restore.select_slc_type"] = "选择 SLC 类型";
            mTexts["restore.slccmpt_restore"] = "SLCCMPT (兼容模式)";
            mTexts["restore.slccmpt_desc"] = "推荐使用 - 更安全的访问方式";
            mTexts["restore.slccmpt_path"] = "从 SD:/slcbackup/slccmpt 恢复";
            mTexts["restore.slc_restore"] = "SLC (直接访问)";
            mTexts["restore.slc_desc2"] = "直接访问 SLC - 需谨慎操作";
            mTexts["restore.slc_path"] = "从 SD:/slcbackup/slc 恢复";
            mTexts["restore.slc_title"] = "恢复 SLCCMPT 存储";
            mTexts["restore.slc_title2"] = "恢复 SLC 存储";
            mTexts["restore.select_content"] = "选择要恢复的内容";
            mTexts["restore.title_folder"] = "Title 文件夹";
            mTexts["restore.title_path"] = "从 SD:/mlcbackup/title 恢复";
            mTexts["restore.fonts_folder"] = "字体文件";
            mTexts["restore.fonts_path"] = "从 SD:/mlcbackup/fonts 恢复";
            mTexts["restore.scanning_backup"] = "正在扫描备份文件...";
            mTexts["restore.progress"] = "恢复进度: %d%% (%d/%d)";
            mTexts["restore.current_file"] = "恢复文件: %s";
            mTexts["restore.complete"] = "恢复完成！";
            mTexts["restore.fonts_restored"] = "字体文件已从 SD:/mlcbackup/fonts 恢复";
            mTexts["restore.need_reboot"] = "是否需要重启主机？";
            mTexts["restore.reboot_hint"] = "按 \ue000 重启  按 \ue001 返回菜单";
            mTexts["restore.error_no_backup"] = "错误：备份文件夹不存在";
            mTexts["restore.error_start"] = "错误：无法开始恢复";
            mTexts["restore.in_progress"] = "正在恢复中，请稍候...";
            
            // Restore 额外文本
            mTexts["restore.mlc_storage"] = "MLC 存储";
            mTexts["restore.slc_storage"] = "SLC 存储";
            mTexts["restore.mlc_from"] = "从 SD:/mlcbackup 恢复";
            mTexts["restore.slc_from"] = "从 SD:/slcbackup 恢复";
            mTexts["restore.slccmpt"] = "SLCCMPT (兼容模式)";
            mTexts["restore.slccmpt_from"] = "从 SD:/slcbackup/slccmpt 恢复";
            mTexts["restore.slc_direct"] = "SLC (直接访问)";
            mTexts["restore.slc_direct_desc"] = "直接访问 SLC - 需谨慎操作";
            mTexts["restore.slccmpt_title"] = "恢复 SLCCMPT 存储";
            mTexts["restore.slccmpt_full"] = "完整 SLCCMPT 备份";
            mTexts["restore.slc_full"] = "完整 SLC 备份";
            mTexts["restore.fonts"] = "字体文件";
            mTexts["restore.scanned"] = "已扫描 %d 个目录";
            mTexts["restore.found_files"] = "找到 %d 个文件";
            mTexts["restore.restoring_file"] = "正在恢复: ";
            mTexts["restore.wait_message"] = "请稍候...";
            mTexts["restore.title_restored"] = "标题文件夹已从 SD:/mlcbackup/title 恢复";
            mTexts["restore.ask_reboot"] = "是否需要重启主机？";
            
            // 复制页面
            mTexts["copy.title"] = "复制 /title 到 MLC";
            mTexts["copy.checking_files"] = "正在检测文件...";
            mTexts["copy.has_lang_files"] = "检测到 title 文件夹已有语言文件";
            mTexts["copy.ask_download_anyway"] = "是否仍需从网络上下载语言文件？";
            mTexts["copy.has_other_files"] = "检测到非语言文件";
            mTexts["copy.ask_copy_other"] = "是否复制到 MLC？";
            mTexts["copy.no_any_files"] = "没有检测到任何文件";
            mTexts["copy.ask_download"] = "是否从网络上下载语言文件？";
            mTexts["copy.press_a_confirm"] = "按 \ue000 确认";
            mTexts["copy.press_b_cancel"] = "按 \ue001 取消";
            mTexts["copy.press_x_skip"] = "按 \ue002 跳过";
            mTexts["copy.press_y_no_remind"] = "按 \ue003 不再提醒";
            mTexts["copy.no_files_found"] = "未找到语言文件";
            mTexts["copy.download_first"] = "请先使用下载功能下载语言文件";
            mTexts["copy.expected_path"] = "期望路径";
            mTexts["copy.detected_region"] = "检测到的区域";
            mTexts["copy.prepare"] = "准备复制文件到 MLC";
            mTexts["copy.ask_backup"] = "是否先备份现有的 MLC 文件？";
            mTexts["copy.backup_path"] = "备份路径: SD:/mlcbackup/title";
            mTexts["copy.press_a_backup"] = "按 \ue000 选择备份方式";
            mTexts["copy.scanning"] = "正在扫描文件...";
            mTexts["copy.scanned_dirs"] = "已扫描 %d 个目录";
            mTexts["copy.found_files"] = "已找到 %d 个文件";
            mTexts["copy.backup_progress"] = "备份进度: %d%% (%d/%d)";
            mTexts["copy.backup_file"] = "备份文件: %s";
            mTexts["copy.copy_progress"] = "复制进度: %d%% (%d/%d)";
            mTexts["copy.create_dir"] = "创建目录: %s";
            mTexts["copy.copy_file"] = "复制文件: %s";
            mTexts["copy.file_progress"] = "文件进度: %d%%";
            mTexts["copy.copying"] = "复制文件...";
            mTexts["copy.cleanup"] = "清理中...";
            mTexts["copy.font_detected"] = "检测到字体目录 (wiiu/fonts)";
            mTexts["copy.copy_fonts_q"] = "是否复制字体到 MLC？";
            mTexts["copy.press_a_copy"] = "按 \ue000 复制字体";
            mTexts["copy.press_b_skip"] = "按 \ue001 跳过";
            mTexts["copy.prepare_fonts"] = "准备复制字体文件";
            mTexts["copy.backup_fonts_q"] = "是否先备份 MLC 中将被覆盖的字体？";
            mTexts["copy.fonts_backup_path"] = "备份路径: SD:/mlcbackup/fonts";
            mTexts["copy.press_a_backup_fonts"] = "按 \ue000 备份";
            mTexts["copy.press_b_skip_backup"] = "按 \ue001 跳过备份";
            mTexts["copy.scanning_fonts"] = "正在扫描字体文件...";
            mTexts["copy.fonts_backup_progress"] = "字体备份进度: %d%% (%d/%d)";
            mTexts["copy.complete"] = "复制完成!";
            mTexts["copy.files_copied"] = "文件已复制到:";
            mTexts["copy.press_a_reboot"] = "按 \ue000 重启系统";
            mTexts["copy.press_b_return"] = "按 \ue001 返回菜单";
            mTexts["copy.error_create_dir"] = "创建目录失败: %s";
            mTexts["copy.error_open_src"] = "无法打开源文件: %s";
            mTexts["copy.error_create_dst"] = "无法创建目标文件: %s";
            mTexts["copy.error_write"] = "写入文件失败";
            mTexts["copy.error_create_target"] = "创建目标目录失败";
            mTexts["copy.error_create_backup"] = "创建备份目录失败";
            mTexts["copy.error_create_fonts_backup"] = "创建字体备份目录失败";
            mTexts["copy.error_create_fonts_target"] = "创建字体目标目录失败";
            mTexts["copy.unknown_error"] = "未知错误";
            
            // Copy 额外文本
            mTexts["copy.warning"] = "   警告：此操作会覆盖 MLC 中的现有文件！";
            mTexts["copy.select_backup_hint"] = "按 \ue000 选择备份方式";
            mTexts["copy.skip_backup_hint"] = "按 \ue002 跳过备份直接复制";
            mTexts["copy.select_backup_mode"] = "请选择备份模式：";
            mTexts["copy.backup_full"] = "完整备份";
            mTexts["copy.backup_full_desc1"] = "备份所有 MLC 文件（较慢但最安全）";
            mTexts["copy.backup_full_desc2"] = "可完全恢复到当前状态";
            mTexts["copy.backup_selective"] = "选择性备份";
            mTexts["copy.backup_selective_desc1"] = "仅备份将被覆盖的文件（快速）";
            mTexts["copy.backup_selective_desc2"] = "只能恢复被覆盖的文件";
            mTexts["copy.backing_up_file"] = "正在备份: ";
            mTexts["copy.wait_message"] = "请稍候...";
            mTexts["copy.progress"] = "复制进度: %d%% (%d/%d)";
            mTexts["copy.creating_dir"] = "正在创建目录: ";
            mTexts["copy.copying_file"] = "正在复制文件: ";
            mTexts["copy.title_complete"] = "标题文件夹复制完成!";
            mTexts["copy.fonts_detected"] = "检测到字体目录 (wiiu/fonts)";
            mTexts["copy.ask_copy_fonts"] = "是否复制字体到 MLC？";
            mTexts["copy.copy_fonts_hint"] = "按 \ue000 复制字体";
            mTexts["copy.skip_fonts_hint"] = "按 \ue001 跳过字体复制";
            mTexts["copy.prepare_fonts"] = "准备复制字体文件";
            mTexts["copy.ask_backup_fonts"] = "是否先备份 MLC 中将被覆盖的字体？";
            mTexts["copy.backup_fonts_desc"] = "（推荐备份以防需要恢复）";
            mTexts["copy.fonts_backup_path"] = "备份路径: SD:/mlcbackup/fonts";
            mTexts["copy.backup_fonts_hint"] = "按 \ue000 备份字体";
            mTexts["copy.skip_backup_fonts_hint"] = "按 \ue001 跳过字体备份";
            
            // Copy 提示
            mTexts["copy.backup_hint"] = "\ue000 备份";
            mTexts["copy.skip_hint"] = "\ue002 跳过";
            mTexts["copy.skip_hint_B"] = "\ue001 跳过";
            mTexts["copy.copy_hint"] = "\ue000 复制";
            mTexts["copy.full_hint"] = "\ue000 全部";
            mTexts["copy.selective_hint"] = "\ue002 选择性";
            mTexts["copy.reboot_hint"] = "\ue000 重启";
            mTexts["copy.menu_hint"] = "\ue001 菜单";
            
            break;
            
        case LanguageCode::ENGLISH:
            // Main menu
            mTexts["menu.copy"] = "Copy title folders to MLC";
            mTexts["menu.backup"] = "Backup System Files";
            mTexts["menu.restore"] = "Restore System Files";
            mTexts["menu.reboot"] = "Reboot System";
            mTexts["menu.settings"] = "Settings";
            mTexts["menu.debug"] = "Debug Settings";
            mTexts["menu.about"] = "About LingoBrew";
            
            // Settings page
            mTexts["settings.title"] = "Settings";
            mTexts["settings.language"] = "Language";
            mTexts["settings.language.desc"] = "Change program language";
            mTexts["settings.show_copy_detect"] = "Show File Detection";
            mTexts["settings.show_copy_detect.desc"] = "Show file detection prompt when copying";
            mTexts["settings.download"] = "Download Language Files";
            mTexts["settings.download.desc"] = "Download latest language files from internet";
            mTexts["settings.clear_titles"] = "Clear Title Folder";
            mTexts["settings.clear_titles.desc"] = "Delete title folder on SD card";
            mTexts["settings.back"] = "Back";
            mTexts["settings.back.desc"] = "Return to main menu";
            
            // Debug page
            mTexts["debug.title"] = "Debug Settings";
            mTexts["debug.logging"] = "Logging";
            mTexts["debug.verbose"] = "Verbose Logging";
            mTexts["debug.defaultpage"] = "Default Page";
            mTexts["debug.clearlogs"] = "Clear Logs";
            mTexts["debug.hidemenu"] = "Hide Menu";
            mTexts["debug.logging.on.desc"] = "Log backup/restore operations to SD:/log/lingobrew/";
            mTexts["debug.logging.off.desc"] = "Do not log";
            mTexts["debug.verbose.on.desc"] = "Log detailed file copy information";
            mTexts["debug.verbose.off.desc"] = "Log summary only";
            mTexts["debug.defaultpage.desc"] = "Page to open on startup";
            mTexts["debug.clearlogs.desc"] = "Delete all log files in SD:/log/lingobrew/";
            mTexts["debug.hidemenu.desc"] = "Press +/-/L to show again";
            mTexts["debug.back.desc"] = "Return to main menu";
            mTexts["debug.page"] = "Page";
            
            // Page names
            mTexts["page.main"] = "Main Menu";
            mTexts["page.copy"] = "Copy Title";
            mTexts["page.backup"] = "Backup";
            mTexts["page.restore"] = "Restore";
            mTexts["page.reboot"] = "Reboot";
            mTexts["page.settings"] = "Settings";
            mTexts["page.debug"] = "Debug";
            mTexts["page.about"] = "About";
            mTexts["page.download"] = "Download";
            
            // Dialogs
            mTexts["dialog.confirm.delete"] = "Delete all log files?";
            mTexts["dialog.confirm.delete.titles"] = "Delete title folder on SD card?";
            mTexts["dialog.warning.irreversible"] = "This cannot be undone";
            
            // Button hints
            mTexts["button.navigate"] = "Navigate";
            mTexts["button.select"] = "Select";
            mTexts["button.confirm"] = "Confirm";
            mTexts["button.back"] = "Back";
            mTexts["button.exit"] = "Exit";
            mTexts["button.cancel"] = "Cancel";
            mTexts["button.page"] = "Page";
            mTexts["button.on"] = "On";
            mTexts["button.off"] = "Off";
            
            // Common hints
            mTexts["common.exit"] = "\ue044 Exit";
            mTexts["common.back"] = "\ue001 Back";
            mTexts["common.back_hint"] = "Press \ue001 Back";
            mTexts["common.enabled"] = "Enabled";
            mTexts["common.disabled"] = "Disabled";
            mTexts["common.select"] = "\ue07d Select";
            mTexts["common.confirm"] = "\ue000 Confirm";
            mTexts["common.continue"] = "\ue000 Continue";
            mTexts["common.continue_hint"] = "Press \ue000 Continue";
            mTexts["common.start"] = "\ue000 Start";
            mTexts["common.start_hint"] = "Press \ue000 Start Backup";
            mTexts["common.start_restore"] = "\ue000 Start Restore";
            mTexts["common.return"] = "\ue000 Return";
            mTexts["common.return_menu"] = "Press \ue000 Return to Menu";
            mTexts["common.please_wait"] = "Please wait...";
            mTexts["common.button_a"] = "\ue000";
            mTexts["common.button_b"] = "\ue001";
            mTexts["common.button_x"] = "\ue002";
            mTexts["common.button_y"] = "\ue003";
            mTexts["common.button_updown"] = "\ue079\ue07a";
            
            // Download screen
            mTexts["download.title"] = "Download Language Files";
            mTexts["download.select_region"] = "Select System Region";
            mTexts["download.region_eu"] = "EU (Europe)";
            mTexts["download.region_us"] = "US (Americas)";
            mTexts["download.region_jp"] = "JP (Japan)";
            mTexts["download.select_mirror"] = "Select Mirror Source";
            mTexts["download.mirror_original"] = "Original Mirror (GitHub)";
            mTexts["download.mirror_accelerated"] = "Accelerated Mirror (China Users)";
            mTexts["download.recommend_original"] = "Recommended: Original (International)";
            mTexts["download.recommend_accelerated"] = "Recommended: Accelerated (China)";
            mTexts["download.select_language"] = "Select Language to Download";
            mTexts["download.lang_chinese"] = "Simplified Chinese";
            mTexts["download.file_to_download"] = "File to download";
            mTexts["download.start"] = "Start Download";
            mTexts["download.downloading"] = "Downloading...";
            mTexts["download.extracting"] = "Extracting...";
            mTexts["download.complete"] = "Download Complete!";
            mTexts["download.error"] = "Download Failed";
            mTexts["download.cancel"] = "Cancel";
            mTexts["download.confirm_cancel"] = "Are you sure you want to cancel?";
            mTexts["download.cancel_warning"] = "Downloaded data will be lost";
            mTexts["download.speed"] = "Speed";
            mTexts["download.files"] = "files";
            mTexts["download.mirror"] = "Mirror";
            mTexts["download.ask_copy"] = "Go to Copy screen to install files?";
            mTexts["download.goto_copy"] = "Go to Copy";
            
            // Main screen (initialization)
            mTexts["main.init.mocha.error"] = "Failed to initialize mocha!\nMake sure Tiramisu/Aroma is updated or installed.";
            mTexts["main.init.mocha.loading"] = "Initializing mocha...";
            mTexts["main.init.fs.error"] = "Failed to initialize filesystem!";
            mTexts["main.init.fs.loading"] = "Initializing filesystem...";
            mTexts["main.menu.loading"] = "Loading menu...";
            
            // About page
            mTexts["about.credits"] = "Credits";
            mTexts["about.fonts"] = "Fonts";
            mTexts["about.source"] = "Source Code";
            mTexts["about.developer"] = "Developer:";
            mTexts["about.based_on"] = "Based on Maschell's WiiUCrashLogDumper\n GaryOderNichts' WiiUIdent\n and YveltalGriffin's Haxcopy";
            mTexts["about.main_font"] = "Main Font:";
            mTexts["about.main_font.name"] = "Wii U System Font";
            mTexts["about.icon_font"] = "Icon Font:";
            mTexts["about.mono_font"] = "Monospace Font:";
            
            // Reboot page
            mTexts["reboot.confirm"] = "Confirm system reboot?";
            mTexts["reboot.confirm.button"] = "Confirm Reboot";
            mTexts["reboot.action"] = "Reboot";
            
            // Backup page
            mTexts["backup.title"] = "Backup MLC/SLC Files";
            mTexts["backup.select_storage"] = "Select Storage Type";
            mTexts["backup.mlc_storage"] = "MLC Storage";
            mTexts["backup.mlc_desc1"] = "Main storage (8GB/32GB)";
            mTexts["backup.mlc_desc2"] = "Contains games, saves, user data";
            mTexts["backup.slc_storage"] = "SLC Storage";
            mTexts["backup.slc_desc1"] = "System flash (~512MB)";
            mTexts["backup.slc_desc2"] = "Contains core system files";
            mTexts["backup.select_mlc"] = "Select MLC Content to Backup";
            mTexts["backup.select_slc"] = "Select SLC Content to Backup";
            mTexts["backup.select_mlc_folder"] = "Select MLC Folder to Backup";
            mTexts["backup.full_slc"] = "Full SLC Backup";
            mTexts["backup.full_slc_desc"] = "Backup entire SLC system flash";
            mTexts["backup.full_slc_path"] = "Backup to: SD:/slcbackup";
            mTexts["backup.title_folder"] = "Title Folder";
            mTexts["backup.title_path"] = "Path: mlc:/sys/title";
            mTexts["backup.title_dest"] = "Backup to: SD:/mlcbackup/title";
            mTexts["backup.fonts"] = "Font Files";
            mTexts["backup.fonts_path"] = "Path: mlc:/sys/title/.../content";
            mTexts["backup.fonts_dest"] = "Backup to: SD:/mlcbackup/fonts";
            mTexts["backup.full_mlc"] = "Full MLC Backup";
            mTexts["backup.full_mlc_desc"] = "Choose to backup sys or usr folder";
            mTexts["backup.full_mlc_dest"] = "Backup to: SD:/mlcbackup/full";
            mTexts["backup.select_mode"] = "Select Backup Mode";
            mTexts["backup.mode_all"] = "Full Backup";
            mTexts["backup.mode_all_desc1"] = "- Backup all existing files";
            mTexts["backup.mode_all_desc2"] = "- Safest, but takes more space";
            mTexts["backup.mode_full"] = "Full Backup";
            mTexts["backup.mode_full_desc1"] = "- Backup all existing files";
            mTexts["backup.mode_full_desc2"] = "- Safest, but takes more space";
            mTexts["backup.mode_selective"] = "Selective Backup";
            mTexts["backup.mode_selective_desc1"] = "- Only backup files that exist on SD";
            mTexts["backup.mode_selective_desc2"] = "- Saves space";
            mTexts["backup.start_hint"] = "Press \ue000 to Start  Press \ue001 to Return";
            mTexts["backup.select_full"] = "Select MLC Directory to Backup";
            mTexts["backup.sys_folder"] = "sys Folder";
            mTexts["backup.sys_desc"] = "Complete system folder";
            mTexts["backup.sys_path"] = "Path: mlc:/sys";
            mTexts["backup.usr_folder"] = "usr Folder";
            mTexts["backup.usr_desc"] = "User data folder";
            mTexts["backup.usr_path"] = "Path: mlc:/usr";
            mTexts["backup.select_slc_type"] = "Select SLC Type";
            mTexts["backup.slccmpt"] = "SLCCMPT (Compatibility Mode)";
            mTexts["backup.slccmpt_desc"] = "Recommended - Safer access method";
            mTexts["backup.slccmpt_path"] = "Path: storage_slccmpt";
            mTexts["backup.slc_direct"] = "SLC (Direct Access)";
            mTexts["backup.slc_direct_desc"] = "Direct SLC access - Use with caution";
            mTexts["backup.slc_direct_path"] = "Path: storage_slc";
            mTexts["backup.continue_hint"] = "Press \ue000 to Continue  Press \ue001 to Return";
            mTexts["backup.scanning"] = "Scanning files...";
            mTexts["backup.scanned_dirs"] = "Scanned: %d directories";
            mTexts["backup.pending_dirs"] = "Pending: %d directories";
            mTexts["backup.found_items"] = "Found: %d items";
            mTexts["backup.items_note"] = "  (including files and folders)";
            mTexts["backup.items_include"] = "(including files and folders)";
            mTexts["backup.please_wait"] = "Please wait, scanning large directories may take a while...";
            mTexts["backup.wait_scanning"] = "Please wait, scanning large directories may take a while...";
            mTexts["backup.wait_copying"] = "Do not power off or exit during this process, please wait...";
            mTexts["backup.progress"] = "Progress: %d%% (%d/%d)";
            mTexts["backup.skipped"] = "  Skipped: %d files (insufficient permissions or special files)";
            mTexts["backup.skipped_files"] = "Skipped: %d files";
            mTexts["backup.current_file"] = "Current file: ";
            mTexts["backup.file_progress"] = "File progress: %d%% (%s)";
            mTexts["backup.do_not_power_off"] = "Do not power off or exit during this process, please wait...";
            mTexts["backup.complete"] = "Backup Complete!";
            mTexts["backup.total_items"] = "Total: %d items";
            mTexts["backup.scanned"] = "Scanned: %d directories";
            mTexts["backup.skipped_items"] = "Skipped: %d files";
            mTexts["backup.skipped_count"] = "Skipped: %d files";
            mTexts["backup.fonts_backed_up"] = "Font files backed up to:";
            mTexts["backup.fonts_dest"] = "SD:/mlcbackup/fonts";
            mTexts["backup.title_backed_up"] = "Title folder backed up to:";
            mTexts["backup.title_dest"] = "SD:/mlcbackup/title";
            mTexts["backup.sys_backed_up"] = "System folder backed up to:";
            mTexts["backup.sys_dest"] = "SD:/mlcbackup/sys";
            mTexts["backup.usr_backed_up"] = "User folder backed up to:";
            mTexts["backup.usr_dest"] = "SD:/mlcbackup/usr";
            mTexts["backup.slccmpt_backed_up"] = "SLCCMPT backed up to:";
            mTexts["backup.slccmpt_dest"] = "SD:/slcbackup/slccmpt";
            mTexts["backup.slc_backed_up"] = "SLC backed up to:";
            mTexts["backup.slc_dest"] = "SD:/slcbackup/slc";
            mTexts["backup.press_a_return"] = "Press \ue000 to Return to Menu";
            mTexts["backup.error_create_dir"] = "Failed to create backup directory";
            
            // Restore page
            mTexts["restore.title"] = "Restore MLC/SLC Files";
            mTexts["restore.warning"] = "Warning: Restore will overwrite existing files!";
            mTexts["restore.select_storage"] = "Select Storage Type";
            mTexts["restore.mlc_desc"] = "Restore from SD:/mlcbackup";
            mTexts["restore.slc_desc"] = "Restore from SD:/slcbackup";
            mTexts["restore.select_slc_type"] = "Select SLC Type";
            mTexts["restore.slccmpt_restore"] = "SLCCMPT (Compatibility Mode)";
            mTexts["restore.slccmpt_desc"] = "Recommended - Safer access method";
            mTexts["restore.slccmpt_path"] = "Restore from SD:/slcbackup/slccmpt";
            mTexts["restore.slc_restore"] = "SLC (Direct Access)";
            mTexts["restore.slc_desc2"] = "Direct SLC access - Use with caution";
            mTexts["restore.slc_path"] = "Restore from SD:/slcbackup/slc";
            mTexts["restore.slc_title"] = "Restore SLCCMPT Storage";
            mTexts["restore.slc_title2"] = "Restore SLC Storage";
            mTexts["restore.select_content"] = "Select Content to Restore";
            mTexts["restore.title_folder"] = "Title Folder";
            mTexts["restore.title_path"] = "Restore from SD:/mlcbackup/title";
            mTexts["restore.fonts_folder"] = "Font Files";
            mTexts["restore.fonts_path"] = "Restore from SD:/mlcbackup/fonts";
            mTexts["restore.scanning_backup"] = "Scanning backup files...";
            mTexts["restore.progress"] = "Restore progress: %d%% (%d/%d)";
            mTexts["restore.current_file"] = "Restoring file: %s";
            mTexts["restore.complete"] = "Restore Complete!";
            mTexts["restore.fonts_restored"] = "Font files restored from SD:/mlcbackup/fonts";
            mTexts["restore.need_reboot"] = "Do you need to reboot the console?";
            mTexts["restore.reboot_hint"] = "Press \ue000 to Reboot  Press \ue001 to Return to Menu";
            mTexts["restore.error_no_backup"] = "Error: Backup folder does not exist";
            mTexts["restore.error_start"] = "Error: Failed to start restore";
            mTexts["restore.in_progress"] = "Restoring, please wait...";
            
            // Restore extra text
            mTexts["restore.mlc_storage"] = "MLC Storage";
            mTexts["restore.slc_storage"] = "SLC Storage";
            mTexts["restore.mlc_from"] = "Restore from SD:/mlcbackup";
            mTexts["restore.slc_from"] = "Restore from SD:/slcbackup";
            mTexts["restore.slccmpt"] = "SLCCMPT (Compatibility Mode)";
            mTexts["restore.slccmpt_from"] = "Restore from SD:/slcbackup/slccmpt";
            mTexts["restore.slc_direct"] = "SLC (Direct Access)";
            mTexts["restore.slc_direct_desc"] = "Direct SLC access - Use with caution";
            mTexts["restore.slccmpt_title"] = "Restore SLCCMPT Storage";
            mTexts["restore.slccmpt_full"] = "Full SLCCMPT Backup";
            mTexts["restore.slc_full"] = "Full SLC Backup";
            mTexts["restore.fonts"] = "Font Files";
            mTexts["restore.scanned"] = "Scanned %d directories";
            mTexts["restore.found_files"] = "Found %d files";
            mTexts["restore.restoring_file"] = "Restoring: ";
            mTexts["restore.wait_message"] = "Please wait...";
            mTexts["restore.title_restored"] = "Title folder restored from SD:/mlcbackup/title";
            mTexts["restore.ask_reboot"] = "Do you need to reboot the console?";
            
            // Copy page
            mTexts["copy.title"] = "Copy /title to MLC";
            mTexts["copy.checking_files"] = "Checking files...";
            mTexts["copy.has_lang_files"] = "Language files detected in title folder";
            mTexts["copy.ask_download_anyway"] = "Download from network anyway?";
            mTexts["copy.has_other_files"] = "Non-language files detected";
            mTexts["copy.ask_copy_other"] = "Copy to MLC?";
            mTexts["copy.no_any_files"] = "No files detected";
            mTexts["copy.ask_download"] = "Download language files from network?";
            mTexts["copy.press_a_confirm"] = "Press \ue000 to Confirm";
            mTexts["copy.press_b_cancel"] = "Press \ue001 to Cancel";
            mTexts["copy.press_x_skip"] = "Press \ue002 to Skip";
            mTexts["copy.press_y_no_remind"] = "Press \ue003 Don't Remind";
            mTexts["copy.no_files_found"] = "No language files found";
            mTexts["copy.download_first"] = "Please use the download function to download language files first";
            mTexts["copy.expected_path"] = "Expected path";
            mTexts["copy.detected_region"] = "Detected region";
            mTexts["copy.prepare"] = "Preparing to copy files to MLC";
            mTexts["copy.ask_backup"] = "Backup existing MLC files first?";
            mTexts["copy.backup_path"] = "Backup path: SD:/mlcbackup/title";
            mTexts["copy.press_a_backup"] = "Press \ue000 to Select Backup Mode";
            mTexts["copy.scanning"] = "Scanning files...";
            mTexts["copy.scanned_dirs"] = "Scanned %d directories";
            mTexts["copy.found_files"] = "Found %d files";
            mTexts["copy.backup_progress"] = "Backup progress: %d%% (%d/%d)";
            mTexts["copy.backup_file"] = "Backing up file: %s";
            mTexts["copy.copy_progress"] = "Copy progress: %d%% (%d/%d)";
            mTexts["copy.create_dir"] = "Creating directory: %s";
            mTexts["copy.copy_file"] = "Copying file: %s";
            mTexts["copy.file_progress"] = "File progress: %d%%";
            mTexts["copy.copying"] = "Copying files...";
            mTexts["copy.cleanup"] = "Cleaning up...";
            mTexts["copy.font_detected"] = "Font directory detected (wiiu/fonts)";
            mTexts["copy.copy_fonts_q"] = "Copy fonts to MLC?";
            mTexts["copy.press_a_copy"] = "Press \ue000 to Copy Fonts";
            mTexts["copy.press_b_skip"] = "Press \ue001 to Skip";
            mTexts["copy.prepare_fonts"] = "Preparing to copy font files";
            mTexts["copy.backup_fonts_q"] = "Backup MLC fonts that will be overwritten?";
            mTexts["copy.fonts_backup_path"] = "Backup path: SD:/mlcbackup/fonts";
            mTexts["copy.press_a_backup_fonts"] = "Press \ue000 to Backup";
            mTexts["copy.press_b_skip_backup"] = "Press \ue001 to Skip Backup";
            mTexts["copy.scanning_fonts"] = "Scanning font files...";
            mTexts["copy.fonts_backup_progress"] = "Font backup progress: %d%% (%d/%d)";
            mTexts["copy.complete"] = "Copy Complete!";
            mTexts["copy.files_copied"] = "Files copied to:";
            mTexts["copy.press_a_reboot"] = "Press \ue000 to Reboot System";
            mTexts["copy.press_b_return"] = "Press \ue001 to Return to Menu";
            mTexts["copy.error_create_dir"] = "Failed to create directory: %s";
            mTexts["copy.error_open_src"] = "Failed to open source file: %s";
            mTexts["copy.error_create_dst"] = "Failed to create target file: %s";
            mTexts["copy.error_write"] = "Failed to write file";
            mTexts["copy.error_create_target"] = "Failed to create target directory";
            mTexts["copy.error_create_backup"] = "Failed to create backup directory";
            mTexts["copy.error_create_fonts_backup"] = "Failed to create font backup directory";
            mTexts["copy.error_create_fonts_target"] = "Failed to create font target directory";
            mTexts["copy.unknown_error"] = "Unknown error";
            
            // Copy extra text
            mTexts["copy.warning"] = "   Warning: This will overwrite existing files on MLC!";
            mTexts["copy.select_backup_hint"] = "Press \ue000 to Select Backup Mode";
            mTexts["copy.skip_backup_hint"] = "Press \ue002 to Skip and Copy Directly";
            mTexts["copy.select_backup_mode"] = "Please select backup mode:";
            mTexts["copy.backup_full"] = "Full Backup";
            mTexts["copy.backup_full_desc1"] = "Backup all MLC files (slower but safest)";
            mTexts["copy.backup_full_desc2"] = "Can fully restore to current state";
            mTexts["copy.backup_selective"] = "Selective Backup";
            mTexts["copy.backup_selective_desc1"] = "Only backup files that will be overwritten (fast)";
            mTexts["copy.backup_selective_desc2"] = "Can only restore overwritten files";
            mTexts["copy.backing_up_file"] = "Backing up: ";
            mTexts["copy.wait_message"] = "Please wait...";
            mTexts["copy.progress"] = "Copy progress: %d%% (%d/%d)";
            mTexts["copy.creating_dir"] = "Creating directory: ";
            mTexts["copy.copying_file"] = "Copying file: ";
            mTexts["copy.title_complete"] = "Title folder copy complete!";
            mTexts["copy.fonts_detected"] = "Font directory detected (wiiu/fonts)";
            mTexts["copy.ask_copy_fonts"] = "Copy fonts to MLC?";
            mTexts["copy.copy_fonts_hint"] = "Press \ue000 to Copy Fonts";
            mTexts["copy.skip_fonts_hint"] = "Press \ue002 to Skip Fonts";
            mTexts["copy.prepare_fonts"] = "Preparing to copy font files";
            mTexts["copy.ask_backup_fonts"] = "Backup MLC fonts that will be overwritten?";
            mTexts["copy.backup_fonts_desc"] = "(Recommended to backup for recovery)";
            mTexts["copy.fonts_backup_path"] = "Backup path: SD:/mlcbackup/fonts";
            mTexts["copy.backup_fonts_hint"] = "Press \ue000 to Backup Fonts";
            mTexts["copy.skip_backup_fonts_hint"] = "Press \ue001 to Skip Font Backup";
            
            // Copy hints
            mTexts["copy.backup_hint"] = "\ue000 Backup";
            mTexts["copy.skip_hint"] = "\ue002 Skip";
            mTexts["copy.skip_hint_B"] = "\ue001 Skip";
            mTexts["copy.copy_hint"] = "\ue000 Copy";
            mTexts["copy.full_hint"] = "\ue000 Full";
            mTexts["copy.selective_hint"] = "\ue002 Selective";
            mTexts["copy.reboot_hint"] = "\ue000 Reboot";
            mTexts["copy.menu_hint"] = "\ue001 Menu";
            
            break;
            
        case LanguageCode::JAPANESE:
            // メインメニュー
            mTexts["menu.copy"] = "titleフォルダをMLCにコピー";
            mTexts["menu.backup"] = "システムファイルのバックアップ";
            mTexts["menu.restore"] = "システムファイルの復元";
            mTexts["menu.reboot"] = "システム再起動";
            mTexts["menu.settings"] = "設定";
            mTexts["menu.debug"] = "デバッグ設定";
            mTexts["menu.about"] = "LingoBrew について";
            
            // 設定ページ
            mTexts["settings.title"] = "設定";
            mTexts["settings.language"] = "言語";
            mTexts["settings.language.desc"] = "プログラム表示言語を変更";
            mTexts["settings.show_copy_detect"] = "ファイル検出通知を表示";
            mTexts["settings.show_copy_detect.desc"] = "コピー時にファイル検出プロンプトを表示";
            mTexts["settings.download"] = "言語ファイルのダウンロード";
            mTexts["settings.download.desc"] = "インターネットから最新の言語ファイルをダウンロード";
            mTexts["settings.clear_titles"] = "Titleフォルダを削除";
            mTexts["settings.clear_titles.desc"] = "SDカードのtitleフォルダを削除";
            mTexts["settings.back"] = "戻る";
            mTexts["settings.back.desc"] = "メインメニューに戻る";
            
            // デバッグページ
            mTexts["debug.title"] = "デバッグ設定";
            mTexts["debug.logging"] = "ログ機能";
            mTexts["debug.verbose"] = "詳細ログ";
            mTexts["debug.defaultpage"] = "デフォルトページ";
            mTexts["debug.clearlogs"] = "ログ削除";
            mTexts["debug.hidemenu"] = "メニュー非表示";
            mTexts["debug.logging.on.desc"] = "SD:/log/lingobrew/ にログを記録";
            mTexts["debug.logging.off.desc"] = "ログを記録しない";
            mTexts["debug.verbose.on.desc"] = "各ファイルの詳細情報を記録";
            mTexts["debug.verbose.off.desc"] = "概要のみ記録";
            mTexts["debug.defaultpage.desc"] = "起動時に開くページ";
            mTexts["debug.clearlogs.desc"] = "SD:/log/lingobrew/ の全ログを削除";
            mTexts["debug.hidemenu.desc"] = "+/-/L 同時押しで再表示";
            mTexts["debug.back.desc"] = "メインメニューに戻る";
            mTexts["debug.page"] = "ページ";
            
            // ページ名
            mTexts["page.main"] = "メインメニュー";
            mTexts["page.copy"] = "Titleコピー";
            mTexts["page.backup"] = "バックアップ";
            mTexts["page.restore"] = "復元";
            mTexts["page.reboot"] = "再起動";
            mTexts["page.settings"] = "設定";
            mTexts["page.debug"] = "デバッグ";
            mTexts["page.about"] = "について";
            mTexts["page.download"] = "ダウンロード";
            
            // ダイアログ
            mTexts["dialog.confirm.delete"] = "全てのログファイルを削除しますか？";
            mTexts["dialog.confirm.delete.titles"] = "SDカードのtitleフォルダを削除しますか？";
            mTexts["dialog.warning.irreversible"] = "この操作は元に戻せません";
            
            // ボタンヒント
            mTexts["button.navigate"] = "ナビゲート";
            mTexts["button.select"] = "選択";
            mTexts["button.confirm"] = "確認";
            mTexts["button.back"] = "戻る";
            mTexts["button.exit"] = "終了";
            mTexts["button.cancel"] = "キャンセル";
            mTexts["button.page"] = "ページ切替";
            mTexts["button.on"] = "オン";
            mTexts["button.off"] = "オフ";
            
            // 共通ヒント
            mTexts["common.exit"] = "\ue044 終了";
            mTexts["common.back"] = "\ue001 戻る";
            mTexts["common.back_hint"] = "\ue001 を押して戻る";
            mTexts["common.enabled"] = "有効";
            mTexts["common.disabled"] = "無効";
            mTexts["common.select"] = "\ue07d 選択";
            mTexts["common.confirm"] = "\ue000 確認";
            mTexts["common.continue"] = "\ue000 続行";
            mTexts["common.continue_hint"] = "\ue000 を押して続行";
            mTexts["common.start"] = "\ue000 開始";
            mTexts["common.start_hint"] = "\ue000 を押してバックアップ開始";
            mTexts["common.start_restore"] = "\ue000 復元開始";
            mTexts["common.return"] = "\ue000 戻る";
            mTexts["common.return_menu"] = "\ue000 を押してメニューに戻る";
            mTexts["common.please_wait"] = "お待ちください...";
            mTexts["common.button_a"] = "\ue000";
            mTexts["common.button_b"] = "\ue001";
            mTexts["common.button_x"] = "\ue002";
            mTexts["common.button_y"] = "\ue003";
            mTexts["common.button_updown"] = "\ue079\ue07a";
            
            // ダウンロード画面
            mTexts["download.title"] = "言語ファイルのダウンロード";
            mTexts["download.select_region"] = "システム地域を選択";
            mTexts["download.region_eu"] = "EU（ヨーロッパ）";
            mTexts["download.region_us"] = "US（アメリカ）";
            mTexts["download.region_jp"] = "JP（日本）";
            mTexts["download.select_mirror"] = "ミラーソースを選択";
            mTexts["download.mirror_original"] = "オリジナルミラー（GitHub）";
            mTexts["download.mirror_accelerated"] = "高速ミラー（中国ユーザー向け）";
            mTexts["download.recommend_original"] = "推奨：オリジナル（国際ユーザー）";
            mTexts["download.recommend_accelerated"] = "推奨：高速（中国ユーザー）";
            mTexts["download.select_language"] = "ダウンロードする言語を選択";
            mTexts["download.lang_chinese"] = "簡体字中国語";
            mTexts["download.file_to_download"] = "ダウンロードファイル";
            mTexts["download.start"] = "ダウンロード開始";
            mTexts["download.downloading"] = "ダウンロード中...";
            mTexts["download.extracting"] = "展開中...";
            mTexts["download.complete"] = "ダウンロード完了！";
            mTexts["download.error"] = "ダウンロード失敗";
            mTexts["download.cancel"] = "キャンセル";
            mTexts["download.confirm_cancel"] = "ダウンロードをキャンセルしますか？";
            mTexts["download.cancel_warning"] = "ダウンロードしたデータは失われます";
            mTexts["download.speed"] = "速度";
            mTexts["download.files"] = "個のファイル";
            mTexts["download.mirror"] = "ミラー";
            mTexts["download.ask_copy"] = "コピー画面に移動してファイルをインストールしますか？";
            mTexts["download.goto_copy"] = "コピーへ移動";
            
            // メイン画面(初期化)
            mTexts["main.init.mocha.error"] = "mocha の初期化に失敗しました！\nTiramisu/Aroma が更新または\nインストールされているか確認してください。";
            mTexts["main.init.mocha.loading"] = "mocha を初期化中...";
            mTexts["main.init.fs.error"] = "ファイルシステムの初期化に失敗しました！";
            mTexts["main.init.fs.loading"] = "ファイルシステムを初期化中...";
            mTexts["main.menu.loading"] = "メニューを読み込み中...";
            
            // Aboutページ
            mTexts["about.credits"] = "クレジット";
            mTexts["about.fonts"] = "フォント";
            mTexts["about.source"] = "ソースコード";
            mTexts["about.developer"] = "開発者:";
            mTexts["about.based_on"] = "Maschell の WiiUCrashLogDumper\n GaryOderNichts の WiiUIdent\n YveltalGriffin の Haxcopy をベース";
            mTexts["about.main_font"] = "メインフォント:";
            mTexts["about.main_font.name"] = "Wii U システムフォント";
            mTexts["about.icon_font"] = "アイコンフォント:";
            mTexts["about.mono_font"] = "等幅フォント:";
            
            // 再起動ページ
            mTexts["reboot.confirm"] = "システムを再起動しますか？";
            mTexts["reboot.confirm.button"] = "再起動を確認";
            mTexts["reboot.action"] = "再起動";
            
            // バックアップページ
            mTexts["backup.title"] = "MLC/SLC ファイルのバックアップ";
            mTexts["backup.select_storage"] = "ストレージタイプを選択";
            mTexts["backup.mlc_storage"] = "MLC ストレージ";
            mTexts["backup.mlc_desc1"] = "メインストレージ (8GB/32GB)";
            mTexts["backup.mlc_desc2"] = "ゲーム、セーブデータ、ユーザーデータを含む";
            mTexts["backup.slc_storage"] = "SLC ストレージ";
            mTexts["backup.slc_desc1"] = "システムフラッシュ (~512MB)";
            mTexts["backup.slc_desc2"] = "コアシステムファイルを含む";
            mTexts["backup.select_mlc"] = "バックアップする MLC コンテンツを選択";
            mTexts["backup.select_slc"] = "バックアップする SLC コンテンツを選択";
            mTexts["backup.select_mlc_folder"] = "バックアップする MLC フォルダを選択";
            mTexts["backup.full_slc"] = "完全 SLC バックアップ";
            mTexts["backup.full_slc_desc"] = "SLC システムフラッシュ全体をバックアップ";
            mTexts["backup.full_slc_path"] = "バックアップ先: SD:/slcbackup";
            mTexts["backup.title_folder"] = "Title フォルダ";
            mTexts["backup.title_path"] = "パス: mlc:/sys/title";
            mTexts["backup.title_dest"] = "バックアップ先: SD:/mlcbackup/title";
            mTexts["backup.fonts"] = "フォントファイル";
            mTexts["backup.fonts_path"] = "パス: mlc:/sys/title/.../content";
            mTexts["backup.fonts_dest"] = "バックアップ先: SD:/mlcbackup/fonts";
            mTexts["backup.full_mlc"] = "完全 MLC バックアップ";
            mTexts["backup.full_mlc_desc"] = "sys または usr フォルダを選択";
            mTexts["backup.full_mlc_dest"] = "バックアップ先: SD:/mlcbackup/full";
            mTexts["backup.select_mode"] = "バックアップモードを選択";
            mTexts["backup.mode_all"] = "完全バックアップ";
            mTexts["backup.mode_all_desc1"] = "- すべての既存ファイルをバックアップ";
            mTexts["backup.mode_all_desc2"] = "- 最も安全ですが、容量が大きくなります";
            mTexts["backup.mode_full"] = "完全バックアップ";
            mTexts["backup.mode_full_desc1"] = "- すべての既存ファイルをバックアップ";
            mTexts["backup.mode_full_desc2"] = "- 最も安全ですが、容量が大きくなります";
            mTexts["backup.mode_selective"] = "選択的バックアップ";
            mTexts["backup.mode_selective_desc1"] = "- SD カードに存在するファイルのみバックアップ";
            mTexts["backup.mode_selective_desc2"] = "- 容量を節約";
            mTexts["backup.start_hint"] = "\ue000 を押して開始  \ue001 を押して戻る";
            mTexts["backup.select_full"] = "バックアップする MLC ディレクトリを選択";
            mTexts["backup.sys_folder"] = "sys フォルダ";
            mTexts["backup.sys_desc"] = "完全なシステムフォルダ";
            mTexts["backup.sys_path"] = "パス: mlc:/sys";
            mTexts["backup.usr_folder"] = "usr フォルダ";
            mTexts["backup.usr_desc"] = "ユーザーデータフォルダ";
            mTexts["backup.usr_path"] = "パス: mlc:/usr";
            mTexts["backup.select_slc_type"] = "SLC タイプを選択";
            mTexts["backup.slccmpt"] = "SLCCMPT (互換モード)";
            mTexts["backup.slccmpt_desc"] = "推奨 - より安全なアクセス方法";
            mTexts["backup.slccmpt_path"] = "パス: storage_slccmpt";
            mTexts["backup.slc_direct"] = "SLC (直接アクセス)";
            mTexts["backup.slc_direct_desc"] = "直接 SLC アクセス - 注意して使用";
            mTexts["backup.slc_direct_path"] = "パス: storage_slc";
            mTexts["backup.continue_hint"] = "\ue000 を押して続行  \ue001 を押して戻る";
            mTexts["backup.scanning"] = "ファイルをスキャン中...";
            mTexts["backup.scanned_dirs"] = "スキャン済み: %d ディレクトリ";
            mTexts["backup.pending_dirs"] = "保留中: %d ディレクトリ";
            mTexts["backup.found_items"] = "検出: %d アイテム";
            mTexts["backup.items_note"] = "  (ファイルとフォルダを含む)";
            mTexts["backup.items_include"] = "(ファイルとフォルダを含む)";
            mTexts["backup.please_wait"] = "お待ちください。大容量ディレクトリのスキャンには時間がかかる場合があります...";
            mTexts["backup.wait_scanning"] = "お待ちください。大容量ディレクトリのスキャンには時間がかかる場合があります...";
            mTexts["backup.wait_copying"] = "この処理中は電源を切らないでください。お待ちください...";
            mTexts["backup.progress"] = "進行状況: %d%% (%d/%d)";
            mTexts["backup.skipped"] = "  スキップ: %d ファイル (権限不足または特殊ファイル)";
            mTexts["backup.skipped_files"] = "スキップ: %d ファイル";
            mTexts["backup.current_file"] = "現在のファイル: ";
            mTexts["backup.file_progress"] = "ファイル進行状況: %d%% (%s)";
            mTexts["backup.do_not_power_off"] = "この処理中は電源を切らないでください。お待ちください...";
            mTexts["backup.complete"] = "バックアップ完了！";
            mTexts["backup.total_items"] = "合計: %d アイテム";
            mTexts["backup.scanned"] = "スキャン: %d ディレクトリ";
            mTexts["backup.skipped_items"] = "スキップ: %d ファイル";
            mTexts["backup.skipped_count"] = "スキップ: %d ファイル";
            mTexts["backup.fonts_backed_up"] = "フォントファイルのバックアップ先:";
            mTexts["backup.fonts_dest"] = "SD:/mlcbackup/fonts";
            mTexts["backup.title_backed_up"] = "Title フォルダのバックアップ先:";
            mTexts["backup.title_dest"] = "SD:/mlcbackup/title";
            mTexts["backup.sys_backed_up"] = "システムフォルダのバックアップ先:";
            mTexts["backup.sys_dest"] = "SD:/mlcbackup/sys";
            mTexts["backup.usr_backed_up"] = "ユーザーフォルダのバックアップ先:";
            mTexts["backup.usr_dest"] = "SD:/mlcbackup/usr";
            mTexts["backup.slccmpt_backed_up"] = "SLCCMPT のバックアップ先:";
            mTexts["backup.slccmpt_dest"] = "SD:/slcbackup/slccmpt";
            mTexts["backup.slc_backed_up"] = "SLC のバックアップ先:";
            mTexts["backup.slc_dest"] = "SD:/slcbackup/slc";
            mTexts["backup.press_a_return"] = "\ue000 を押してメニューに戻る";
            mTexts["backup.error_create_dir"] = "バックアップディレクトリの作成に失敗";
            
            // 復元ページ
            mTexts["restore.title"] = "MLC/SLC ファイルの復元";
            mTexts["restore.warning"] = "警告：復元すると既存のファイルが上書きされます！";
            mTexts["restore.select_storage"] = "ストレージタイプを選択";
            mTexts["restore.mlc_desc"] = "SD:/mlcbackup から復元";
            mTexts["restore.slc_desc"] = "SD:/slcbackup から復元";
            mTexts["restore.select_slc_type"] = "SLC タイプを選択";
            mTexts["restore.slccmpt_restore"] = "SLCCMPT (互換モード)";
            mTexts["restore.slccmpt_desc"] = "推奨 - より安全なアクセス方法";
            mTexts["restore.slccmpt_path"] = "SD:/slcbackup/slccmpt から復元";
            mTexts["restore.slc_restore"] = "SLC (直接アクセス)";
            mTexts["restore.slc_desc2"] = "直接 SLC アクセス - 注意して使用";
            mTexts["restore.slc_path"] = "SD:/slcbackup/slc から復元";
            mTexts["restore.slc_title"] = "SLCCMPT ストレージの復元";
            mTexts["restore.slc_title2"] = "SLC ストレージの復元";
            mTexts["restore.select_content"] = "復元するコンテンツを選択";
            mTexts["restore.title_folder"] = "Title フォルダ";
            mTexts["restore.title_path"] = "SD:/mlcbackup/title から復元";
            mTexts["restore.fonts_folder"] = "フォントファイル";
            mTexts["restore.fonts_path"] = "SD:/mlcbackup/fonts から復元";
            mTexts["restore.scanning_backup"] = "バックアップファイルをスキャン中...";
            mTexts["restore.progress"] = "復元進行状況: %d%% (%d/%d)";
            mTexts["restore.current_file"] = "復元中のファイル: %s";
            mTexts["restore.complete"] = "復元完了！";
            mTexts["restore.fonts_restored"] = "フォントファイルを SD:/mlcbackup/fonts から復元しました";
            mTexts["restore.need_reboot"] = "コンソールを再起動しますか？";
            mTexts["restore.reboot_hint"] = "\ue000 を押して再起動  \ue001 を押してメニューに戻る";
            mTexts["restore.error_no_backup"] = "エラー: バックアップフォルダが存在しません";
            mTexts["restore.error_start"] = "エラー: 復元を開始できません";
            mTexts["restore.in_progress"] = "復元中です。お待ちください...";
            
            // Restore 追加テキスト
            mTexts["restore.mlc_storage"] = "MLC ストレージ";
            mTexts["restore.slc_storage"] = "SLC ストレージ";
            mTexts["restore.mlc_from"] = "SD:/mlcbackup から復元";
            mTexts["restore.slc_from"] = "SD:/slcbackup から復元";
            mTexts["restore.slccmpt"] = "SLCCMPT (互換モード)";
            mTexts["restore.slccmpt_from"] = "SD:/slcbackup/slccmpt から復元";
            mTexts["restore.slc_direct"] = "SLC (直接アクセス)";
            mTexts["restore.slc_direct_desc"] = "直接 SLC アクセス - 注意して使用";
            mTexts["restore.slccmpt_title"] = "SLCCMPT ストレージの復元";
            mTexts["restore.slccmpt_full"] = "完全 SLCCMPT バックアップ";
            mTexts["restore.slc_full"] = "完全 SLC バックアップ";
            mTexts["restore.fonts"] = "フォントファイル";
            mTexts["restore.scanned"] = "%d ディレクトリをスキャンしました";
            mTexts["restore.found_files"] = "%d ファイルを検出しました";
            mTexts["restore.restoring_file"] = "復元中: ";
            mTexts["restore.wait_message"] = "お待ちください...";
            mTexts["restore.title_restored"] = "タイトルフォルダを SD:/mlcbackup/title から復元しました";
            mTexts["restore.ask_reboot"] = "コンソールを再起動しますか？";
            
            // コピーページ
            mTexts["copy.title"] = "/title を MLC にコピー";
            mTexts["copy.checking_files"] = "ファイルを確認中...";
            mTexts["copy.has_lang_files"] = "title フォルダに言語ファイルが検出されました";
            mTexts["copy.ask_download_anyway"] = "それでもネットワークからダウンロードしますか？";
            mTexts["copy.has_other_files"] = "非言語ファイルが検出されました";
            mTexts["copy.ask_copy_other"] = "MLC にコピーしますか？";
            mTexts["copy.no_any_files"] = "ファイルが検出されませんでした";
            mTexts["copy.ask_download"] = "ネットワークから言語ファイルをダウンロードしますか？";
            mTexts["copy.press_a_confirm"] = "\ue000 を押して確認";
            mTexts["copy.press_b_cancel"] = "\ue001 を押してキャンセル";
            mTexts["copy.press_x_skip"] = "\ue002 を押してスキップ";
            mTexts["copy.press_y_no_remind"] = "\ue003 を押して再通知なし";
            mTexts["copy.no_files_found"] = "言語ファイルが見つかりません";
            mTexts["copy.download_first"] = "最初にダウンロード機能を使用して言語ファイルをダウンロードしてください";
            mTexts["copy.expected_path"] = "期待されるパス";
            mTexts["copy.detected_region"] = "検出された地域";
            mTexts["copy.prepare"] = "MLC へのファイルのコピーを準備中";
            mTexts["copy.ask_backup"] = "既存の MLC ファイルを先にバックアップしますか？";
            mTexts["copy.backup_path"] = "バックアップパス: SD:/mlcbackup/title";
            mTexts["copy.press_a_backup"] = "\ue000 を押してバックアップモードを選択";
            mTexts["copy.press_x_skip"] = "\ue002 を押してバックアップをスキップして直接コピー";
            mTexts["copy.scanning"] = "ファイルをスキャン中...";
            mTexts["copy.scanned_dirs"] = "%d ディレクトリをスキャンしました";
            mTexts["copy.found_files"] = "%d ファイルを検出しました";
            mTexts["copy.backup_progress"] = "バックアップ進行状況: %d%% (%d/%d)";
            mTexts["copy.backup_file"] = "バックアップ中のファイル: %s";
            mTexts["copy.copy_progress"] = "コピー進行状況: %d%% (%d/%d)";
            mTexts["copy.create_dir"] = "ディレクトリを作成中: %s";
            mTexts["copy.copy_file"] = "ファイルをコピー中: %s";
            mTexts["copy.file_progress"] = "ファイル進行状況: %d%%";
            mTexts["copy.copying"] = "ファイルをコピー中...";
            mTexts["copy.cleanup"] = "クリーンアップ中...";
            mTexts["copy.font_detected"] = "フォントディレクトリを検出 (wiiu/fonts)";
            mTexts["copy.copy_fonts_q"] = "フォントを MLC にコピーしますか？";
            mTexts["copy.press_a_copy"] = "\ue000 を押してフォントをコピー";
            mTexts["copy.press_b_skip"] = "\ue001 を押してスキップ";
            mTexts["copy.prepare_fonts"] = "フォントファイルのコピーを準備中";
            mTexts["copy.backup_fonts_q"] = "上書きされる MLC フォントをバックアップしますか？";
            mTexts["copy.fonts_backup_path"] = "バックアップパス: SD:/mlcbackup/fonts";
            mTexts["copy.press_a_backup_fonts"] = "\ue000 を押してバックアップ";
            mTexts["copy.press_b_skip_backup"] = "\ue001 を押してバックアップをスキップ";
            mTexts["copy.scanning_fonts"] = "フォントファイルをスキャン中...";
            mTexts["copy.fonts_backup_progress"] = "フォントバックアップ進行状況: %d%% (%d/%d)";
            mTexts["copy.complete"] = "コピー完了！";
            mTexts["copy.files_copied"] = "ファイルのコピー先:";
            mTexts["copy.press_a_reboot"] = "\ue000 を押してシステムを再起動";
            mTexts["copy.press_b_return"] = "\ue001 を押してメニューに戻る";
            mTexts["copy.error_create_dir"] = "ディレクトリの作成に失敗: %s";
            mTexts["copy.error_open_src"] = "ソースファイルを開けません: %s";
            mTexts["copy.error_create_dst"] = "ターゲットファイルの作成に失敗: %s";
            mTexts["copy.error_write"] = "ファイルの書き込みに失敗";
            mTexts["copy.error_create_target"] = "ターゲットディレクトリの作成に失敗";
            mTexts["copy.error_create_backup"] = "バックアップディレクトリの作成に失敗";
            mTexts["copy.error_create_fonts_backup"] = "フォントバックアップディレクトリの作成に失敗";
            mTexts["copy.error_create_fonts_target"] = "フォントターゲットディレクトリの作成に失敗";
            mTexts["copy.unknown_error"] = "不明なエラー";
            
            // Copy 追加テキスト
            mTexts["copy.warning"] = "   警告: この操作は MLC 内の既存ファイルを上書きします！";
            mTexts["copy.select_backup_hint"] = "\ue000 を押してバックアップモードを選択";
            mTexts["copy.skip_backup_hint"] = "\ue002 を押してバックアップをスキップして直接コピー";
            mTexts["copy.select_backup_mode"] = "バックアップモードを選択してください:";
            mTexts["copy.backup_full"] = "完全バックアップ";
            mTexts["copy.backup_full_desc1"] = "すべての MLC ファイルをバックアップ（遅いが最も安全）";
            mTexts["copy.backup_full_desc2"] = "現在の状態に完全に復元可能";
            mTexts["copy.backup_selective"] = "選択的バックアップ";
            mTexts["copy.backup_selective_desc1"] = "上書きされるファイルのみバックアップ（高速）";
            mTexts["copy.backup_selective_desc2"] = "上書きされたファイルのみ復元可能";
            mTexts["copy.backing_up_file"] = "バックアップ中: ";
            mTexts["copy.wait_message"] = "お待ちください...";
            mTexts["copy.progress"] = "コピー進行状況: %d%% (%d/%d)";
            mTexts["copy.creating_dir"] = "ディレクトリを作成中: ";
            mTexts["copy.copying_file"] = "ファイルをコピー中: ";
            mTexts["copy.title_complete"] = "タイトルフォルダのコピーが完了しました！";
            mTexts["copy.fonts_detected"] = "フォントディレクトリを検出 (wiiu/fonts)";
            mTexts["copy.ask_copy_fonts"] = "フォントを MLC にコピーしますか？";
            mTexts["copy.copy_fonts_hint"] = "\ue000 を押してフォントをコピー";
            mTexts["copy.skip_fonts_hint"] = "\ue002 を押してフォントをスキップ";
            mTexts["copy.prepare_fonts"] = "フォントファイルのコピーを準備中";
            mTexts["copy.ask_backup_fonts"] = "上書きされる MLC フォントをバックアップしますか？";
            mTexts["copy.backup_fonts_desc"] = "（復元のためバックアップを推奨）";
            mTexts["copy.fonts_backup_path"] = "バックアップパス: SD:/mlcbackup/fonts";
            mTexts["copy.backup_fonts_hint"] = "\ue000 を押してフォントをバックアップ";
            mTexts["copy.skip_backup_fonts_hint"] = "\ue001 を押してフォントバックアップをスキップ";
            
            // Copy ヒント
            mTexts["copy.backup_hint"] = "\ue000 バックアップ";
            mTexts["copy.skip_hint"] = "\ue002 スキップ";
            mTexts["copy.skip_hint_B"] = "\ue001 スキップ";
            mTexts["copy.copy_hint"] = "\ue000 コピー";
            mTexts["copy.full_hint"] = "\ue000 完全";
            mTexts["copy.selective_hint"] = "\ue002 選択的";
            mTexts["copy.reboot_hint"] = "\ue000 再起動";
            mTexts["copy.menu_hint"] = "\ue001 メニュー";
            
            break;
    }
}
