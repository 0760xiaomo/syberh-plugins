#include "filepreview.h"
#include "framework/common/errorinfo.h"

#include "filepreview_p.h"
#include "texteditor.h"

#include <QTimer>

#define DELAY_RETRY_MSEC 800

FilePreviewPrivate::FilePreviewPrivate(QObject *parent) : QObject(parent)
{
}

FilePreviewPrivate::~FilePreviewPrivate()
{
}

void FilePreviewPrivate::reset()
{
    m_retried = false;
    cleanError();
}


void FilePreviewPrivate::setError(ErrorInfo::ErrorCode errCode, const QString &errString)
{
    m_hasError = true;
    m_errorCode = errCode;
    m_errorString = errString;
}

void FilePreviewPrivate::cleanError()
{
    m_hasError = false;
    m_errorCode = (ErrorInfo::ErrorCode)-1;
    m_errorString = QString();
}

void FilePreviewPrivate::previewTxt(QString &callbackID, QVariantMap &params)
{
    qDebug() << Q_FUNC_INFO << "callbackID:" << callbackID << " params" << params;
    m_callbackID = callbackID;
    m_params = params;

    QString filePath = params.value("path").toString();
    if(filePath.startsWith("file://")) {
        filePath = filePath.mid(7);
    }
    qDebug() << Q_FUNC_INFO << " filePath:" << filePath;

    QmlManager qmlManager(this);
    QmlObject *textPreviewQml = qmlManager.open("qrc:/qml/STextPreview.qml");

    if(textPreviewQml->hasError()) {
        qDebug() << Q_FUNC_INFO << "error:" << textPreviewQml->errorMessage();

        textPreviewQml->deleteLater();
        textPreviewQml = nullptr;

        if(!m_retried) {
            m_retried = true;
            QTimer::singleShot(DELAY_RETRY_MSEC, this, &FilePreviewPrivate::slotPreviewTxt);
        }else{
            setError(ErrorInfo::PluginError, "返回数据格式错误");
        }
        return;
    }

    qmlManager.setProperty(textPreviewQml, "filePath", filePath);

    if(textPreviewQml != nullptr){
        textPreviewQml->deleteLater();
        textPreviewQml = nullptr;
    }
    cleanError();
}

void FilePreviewPrivate::slotPreviewTxt()
{
    previewTxt(m_callbackID, m_params);
}

// ---- FilePreview ----

FilePreview::FilePreview()
{
    d = new FilePreviewPrivate(this);

    qmlRegisterType<TextEditor>("textEditor", 1, 0, "TextEditor");
}

void FilePreview::invoke(QString callbackID, QString actionName, QVariantMap params)
{
    qDebug() << Q_FUNC_INFO << "callbackID:" << callbackID << "actionName:" << actionName << "params:" << params;
    d->reset();
    if (actionName == "text") {
        d->previewTxt(callbackID, params);
    }

    if (d->hasError()) {
        signalManager()->failed(callbackID.toLong(), d->error(), d->errorString());
    } else {
        signalManager()->success(callbackID.toLong(), QVariant());
    }
}

