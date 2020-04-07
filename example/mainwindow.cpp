#include "mainwindow.hpp"
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QTemporaryFile>
#include <QTimer>
Mainwindow::Mainwindow(QWidget *parent) : QMainWindow(parent)
{
    auto out = new QVBoxLayout();

    code = new QPlainTextEdit();
    code->setReadOnly(false);

    info = new QPlainTextEdit();
    info->setReadOnly(true);

    out->addWidget(code);
    out->addWidget(info);

    timer.setSingleShot(true);
    timer.setInterval(2000);
    timer.start();

    auto widget = new QWidget();
    widget->setLayout(out);
    setCentralWidget(widget);

    lsp = new LSPClient("clangd.exe", {});
    setConnections();
    file.open();

    file.rename("C://Users/Ashar/AppData/Local/Temp/sol.cpp");
    info->appendPlainText(file.fileName());
    lsp->initialize();
    lsp->didOpen("file://" + file.fileName().toStdString(), "", "cpp");

}

Mainwindow::~Mainwindow()
{
    delete code;
    delete info;
    lsp->didClose("file://"+file.fileName().toStdString());
    lsp->shutdown();
    file.remove();
    delete lsp;
}

void Mainwindow::setConnections()
{
    connect(code, &QPlainTextEdit::textChanged, this, &Mainwindow::textChanged);

    connect(lsp, &LSPClient::onError, this, &Mainwindow::OnError);
    connect(lsp, &LSPClient::onNotify, this, &Mainwindow::OnNotify);
    connect(lsp, &LSPClient::onRequest, this, &Mainwindow::OnRequest);
    connect(lsp, &LSPClient::onResponse, this, &Mainwindow::OnResponse);
    connect(&timer, &QTimer::timeout, this, &Mainwindow::requestDiagonistics);
    connect(lsp, &LSPClient::onServerError, this, &Mainwindow::OnServerError);
    connect(lsp, &LSPClient::onServerFinished, this, &Mainwindow::OnServerFinished);
}

QString Mainwindow::jsonObjectToString(QJsonObject& obj)
{
    return QJsonDocument::fromVariant(obj.toVariantMap()).toJson();
}

//************ SLOTS **************

void Mainwindow::textChanged()
{
    timer.start(2000);
    timer.setSingleShot(true);

//    Trigger to do something,
//    file.resize(0);
//    file.write(code->toPlainText().toLocal8Bit());
//    std::vector<TextDocumentContentChangeEvent> ch;
//    TextDocumentContentChangeEvent ev;
//    ev.text = code->toPlainText().toStdString();
//    ch.push_back(ev);
//    lsp->didChange("file://" + file.fileName().toStdString(), ch, true);
}

void Mainwindow::requestDiagonistics()
{
    //file.resize(0);
    //file.write(code->toPlainText().toLocal8Bit());
    std::vector<TextDocumentContentChangeEvent> ch;
    TextDocumentContentChangeEvent ev;
    ev.text = code->toPlainText().toStdString();
    ch.push_back(ev);
    lsp->didChange("file://" + file.fileName().toStdString(), ch, true);
}

void Mainwindow::OnError(QJsonObject id, QJsonObject err)
{
    info->appendPlainText("Error ocurred[" + jsonObjectToString(id) + "]: " + jsonObjectToString(err));
}

void Mainwindow::OnNotify(QString method, QJsonObject param)
{
    info->appendPlainText("Notification[" + method + "]: " + jsonObjectToString(param));
    // diagonistic notifiacation arrives here!
}

void Mainwindow::OnRequest(QString method, QJsonObject param, QJsonObject id)
{
    info->appendPlainText("Request[" + method + "]: " + jsonObjectToString(param) + "id: " + jsonObjectToString(id));
}

void Mainwindow::OnResponse(QJsonObject id, QJsonObject response)
{
    info->appendPlainText("Response[" + jsonObjectToString(id) + "]: " + jsonObjectToString(response));
}

void Mainwindow::OnServerError(QProcess::ProcessError err)
{
    info->appendPlainText("LSP failed to fork process");
}

void Mainwindow::OnServerFinished(int exitCode, QProcess::ExitStatus status)
{
    info->appendPlainText("LSP finished");
}




