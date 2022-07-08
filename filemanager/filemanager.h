#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QtPlugin>

#include "iplugin/iplugin.h"
#include "filemanager_global.h"

class FILEMANAGERSHARED_EXPORT FileManager : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.syberos.syberh.SyberhPlugin" FILE "plugin.json")

public:
    FileManager();

    void invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params);

private :
   
    /**
     * @brief move          移动
     * @param callBackID
     * @param srcPath       源路径
     * @param destPath      目标路径
     */
    void move(const QString &callbackID, const QString &srcPath, const QString &destPath);
    /**
     * @brief copy          复制
     * @param callBackID
     * @param srcPath       源路径
     * @param destPath      目标路径
     */
    void copy(const QString &callbackID, const QString &srcPath, const QString &destPath);
    /**
     * @brief fileList      获取文件列表
     * @param callBackID
     * @param srcPath       源路径
     */
    void fileList(const QString &callbackID, const QString &srcPath);

    /**
     * @brief getInfo       获取文件信息
     * @param callBackID
     * @param srcPath       源路径
     */
    void getInfo(const QString &callbackID, const QString &srcPath);

    /**
     * @brief remove        删除
     * @param callBackID
     * @param srcPath       源路径
     * @param recursive     递归删除 0：否，1：是
     */
    void remove(const QString &callbackID, const QString &srcPath, int recursive);

};

#endif // FILEMANAGER_H
