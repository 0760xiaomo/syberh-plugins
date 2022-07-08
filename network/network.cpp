#include "network.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include "framework/common/errorinfo.h"
#include "util/validator.h"
#include "util/util.h"


#define NETWORK_CHECK_PARAM(params, name, callbackID, msg) \
    if(!NativeSdk::Util::CheckParam(params, name)) { \
        signalManager()->failed(callbackID.toLong(), NativeSdk::ErrorInfo::InvalidParameter, msg); \
        return; \
    }


using namespace NativeSdk;

// ===== NetworkTasks
class NetworkTasks
{
public:
    NetworkTasks() = default;
    virtual ~NetworkTasks();

    NetworkPrivate* get(const QString &callbackID);
    void add(const QString &callbackID, NetworkPrivate *private_);
    void remove(const QString &callbackID);
    void clear();
private:
    // key: callbackID
    QMap<QString, NetworkPrivate*> m_tasks;
    Q_DISABLE_COPY(NetworkTasks)
};


// ===== Network

Network::Network() : m_networkTasks(new NetworkTasks)
{
}

Network::~Network()
{
    if(!m_networkTasks.isNull()){
        m_networkTasks->clear();
    }
}


bool checkParam(const QVariantMap &params, const QString &paramName)
{
    QVariant var = params.value(paramName);
    if(var.isNull() || !var.isValid()){
        return false;
    }
    return true;
}


void Network::invoke(const QString &callbackID, const QString &actionName, const QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "callbackID:" << callbackID << "actionName:" << actionName << "params:" << params;

    // 检查网络
    // if (!Validator::netWorkConnected()) {
    //     signalManager()->failed(callbackID.toLong(), ErrorInfo::NetworkError, ErrorInfo::message(ErrorInfo::NetworkError, "请检查网络状态"));
    //     return;
    // }

    NETWORK_CHECK_PARAM(params, "url", callbackID, "URL为空")
    NETWORK_CHECK_PARAM(params, "method", callbackID, "method为空")

    QString url = params.value("url").toString();
    QString method = params.value("method").toString();
    QString dataType = params.value("dataType").toString();
    QVariantMap headers = params.value("header").toMap();
    QVariant cookiesVar = params.value("cookie");
    QVariant dataVar = params.value("data");

    NetworkPrivate* private_ = NetworkPrivate::New(callbackID);
    m_networkTasks->add(callbackID, private_);    // add task

    private_->setUrl(url)->setMethod(method)->setHeaders(headers)->setDataType(dataType);

    if(cookiesVar.canConvert<QVariantMap>()){
        private_->setCookies(cookiesVar.toMap());
    }else if(cookiesVar.canConvert<QString>()){
        private_->setCookies(cookiesVar.toString());
    }

    if(dataVar.canConvert<QVariantMap>()){
        private_->setData(dataVar.toMap());
    }else if(dataVar.canConvert<QString>()){
        private_->setData(dataVar.toString());
    }

    connect(private_, &NetworkPrivate::error, this, &Network::error);
    connect(private_, &NetworkPrivate::finished, this, &Network::finished);

    private_->request();
    if(private_->hasError()){
        error(callbackID, private_->getError());
        return;
    }
}

void Network::finished(const QString &callbackID, const NetworkResponse &response)
{
    qDebug() << Q_FUNC_INFO;

    QJsonObject result;
    result.insert("statusCode", response.statusCode);
    result.insert("header", response.headerJson());
    result.insert("cookie", response.cookieJson());

    switch (response.dataType) {
    case NetworkPrivate::JSON:
            result.insert("data", response.bodyJson());
            break;
    case NetworkPrivate::TEXT:
            result.insert("data", response.body);
            break;
    default:
            result.insert("data", QJsonValue());
            break;
    }
    m_networkTasks->remove(callbackID);   // remove task
    signalManager()->success(callbackID.toLong(), result);
}


void Network::error(const QString &callbackID, const QString &err)
{
    m_networkTasks->remove(callbackID);   // remove task
    signalManager()->failed(callbackID.toLong(), ErrorInfo::SystemError, err);
}


// ====== NetworkTasks

NetworkTasks::~NetworkTasks()
{
    clear();
}

NetworkPrivate* NetworkTasks::get(const QString &callbackID)
{
    return m_tasks.value(callbackID);
}

void NetworkTasks::add(const QString &callbackID, NetworkPrivate *private_)
{
    if(!private_){
        return;
    }
    if(m_tasks.contains(callbackID)){
        remove(callbackID);
    }
    m_tasks.insert(callbackID, private_);
}

void NetworkTasks::remove(const QString &callbackID)
{
    NetworkPrivate* private_ = m_tasks.value(callbackID);
    if(private_){
        m_tasks.remove(callbackID);
        private_->deleteLater();
        private_ = nullptr;
    }
}

void NetworkTasks::clear()
{
    if(m_tasks.isEmpty()){
        return;
    }
    QMapIterator<QString,NetworkPrivate*> it(m_tasks);
    while(it.hasNext()){
        it.next();
        NetworkPrivate* private_ = it.value();
        if(private_){
            private_->deleteLater();
            private_ = nullptr;
        }
    }
    m_tasks.clear();
}
