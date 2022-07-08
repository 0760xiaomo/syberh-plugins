#include "storage.h"
#include <qdebug.h>
#include "util/log.h"
#include "framework/common/errorinfo.h"


using namespace NativeSdk;

Storage::Storage()
{
}

Storage::~Storage(){
}


void Storage::invoke(const QString &callbackId, const QString &actionName, const QVariantMap &params) {
    qDebug() << "callbackId:" << callbackId << "actionName:" << actionName << "params:" << params << endl;

    emit signalManager()->failed(callbackId.toLong(), ErrorInfo::InvalidCall, ErrorInfo::message(ErrorInfo::InvalidCall, "不支持的模块"));
}


void Storage::setItem(const QString &callbackId, const QString &key, const QVariant &value){
    Q_UNUSED(callbackId)
    Q_UNUSED(key)
    Q_UNUSED(value)
}


void Storage::getItem(const QString &callbackId, const QString &key){
    Q_UNUSED(callbackId)
    Q_UNUSED(key)
}

void Storage::removeItem(const QString &callbackId, const QString &key){
    Q_UNUSED(callbackId)
    Q_UNUSED(key)
}

void Storage::removeAll(const QString &callbackId){
    Q_UNUSED(callbackId)
}

void Storage::getAllKeys(const QString &callbackId){
    Q_UNUSED(callbackId)
}

bool Storage::checkKey(const QString &callbackId, const QString &key, bool sendFailSign){
    Q_UNUSED(callbackId)
    Q_UNUSED(key)
    Q_UNUSED(sendFailSign)
    return false;
}

