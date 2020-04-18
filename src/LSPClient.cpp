/*
 * Copyright (C) 2019-2020 Ashar Khan <ashar786khan@gmail.com>
 *
 * This file is part of cpeditor.
 *
 * cpeditor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * I will not be responsible if cpeditor behaves in unexpected way and
 * causes your ratings to go down and or lose any important contest.
 *
 * Believe Software is "Software" and it isn't immune to bugs.
 *
 */

#include <LSPClient.hpp>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>

using RequestID = std::string;

LSPClient::LSPClient(QString path, QStringList args)
{
    clientProcess = new QProcess();
    clientProcess->setProgram(path);
    clientProcess->setArguments(args);

    connect(clientProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), this,
            SLOT(onClientError(QProcess::ProcessError)));
    connect(clientProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(onClientReadReadyStdout()));
    connect(clientProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this,
            SLOT(onClientFinished(int, QProcess::ExitStatus)));

    clientProcess->setProcessChannelMode(QProcess::ForwardedErrorChannel);
    clientProcess->setReadChannel(QProcess::StandardOutput);

    clientProcess->start();
}

// slots

void LSPClient::onClientReadReadyStdout()
{
    // Fixme(coder3101): If all buffer does not comes in one go, then we are in trouble
    // We can fix it later by specifying that if incomplete buffer recieved in one time
    // then in the next time. We should append and emit the signal only onces.

    QByteArray buffer = clientProcess->readAllStandardOutput();

    int messageStart = buffer.indexOf("\r\n\r\n") + 4;
    int lenStart = buffer.indexOf("Content-Length: ") + 16;
    int lenEnd = buffer.indexOf("\r\n");
    bool ok = false;
    int contentLength = buffer.mid(lenStart, lenEnd - lenStart).toInt(&ok);

    if (!ok)
        return;

    QByteArray payload = buffer.mid(messageStart);
    if (payload.size() != contentLength)
    {
        // Warning: Incomplete message has arrived,
        // At this point we should keep it in buffer but we are discarding this message;
        return;
    }
    QJsonParseError error{};

    auto msg = QJsonDocument::fromJson(payload, &error);

    if (error.error != QJsonParseError::NoError || !msg.isObject())
    {
        // Some JSON Parse Error
        return;
    }
    auto obj = msg.object();

    if (obj.contains("id"))
    {
        if (obj.contains("method"))
        {
            emit onRequest(obj["method"].toString(), obj["param"].toObject(), obj["id"].toObject());
        }
        else if (obj.contains("result"))
        {
            emit onResponse(obj["id"].toObject(), obj["result"].toObject());
        }
        else if (obj.contains("error"))
        {
            emit onError(obj["id"].toObject(), obj["error"].toObject());
        }
    }
    else if (obj.contains("method"))
    {
        // notification
        if (obj.contains("params"))
        {
            emit onNotify(obj["method"].toString(), obj["params"].toObject());
        }
    }
}

void LSPClient::onClientError(QProcess::ProcessError error)
{
    emit onServerError(error);
}

void LSPClient::onClientFinished(int exitCode, QProcess::ExitStatus status)
{
    emit onServerFinished(exitCode, status);
}

// Protocol methods
RequestID LSPClient::initialize(option<DocumentUri> rootUri)
{
    InitializeParams params;
    params.processId = static_cast<unsigned int>(QCoreApplication::applicationPid());
    params.rootUri = rootUri;
    return SendRequest("initialize", params);
}
RequestID LSPClient::shutdown()
{
    return SendRequest("shutdown", json());
}
RequestID LSPClient::sync()
{
    return SendRequest("sync", json());
}
void LSPClient::exit()
{
    SendNotification("exit", json());
}
void LSPClient::initialized()
{
    SendNotification("initialized", json());
}
RequestID LSPClient::registerCapability()
{
    return SendRequest("client/registerCapability", json());
}
void LSPClient::didOpen(DocumentUri uri, string_ref text, string_ref languageId = "cpp")
{
    DidOpenTextDocumentParams params;
    params.textDocument.uri = uri;
    params.textDocument.text = text;
    params.textDocument.languageId = languageId;
    SendNotification("textDocument/didOpen", params);
}
void LSPClient::didClose(DocumentUri uri)
{
    DidCloseTextDocumentParams params;
    params.textDocument.uri = uri;
    SendNotification("textDocument/didClose", params);
}
void LSPClient::didChange(DocumentUri uri, std::vector<TextDocumentContentChangeEvent> &changes,
                          option<bool> wantDiagnostics)
{
    DidChangeTextDocumentParams params;
    params.textDocument.uri = uri;
    params.contentChanges = std::move(changes);
    params.wantDiagnostics = wantDiagnostics;
    SendNotification("textDocument/didChange", params);
}
RequestID LSPClient::rangeFomatting(DocumentUri uri, Range range)
{
    DocumentRangeFormattingParams params;
    params.textDocument.uri = uri;
    params.range = range;
    return SendRequest("textDocument/rangeFormatting", params);
}
RequestID LSPClient::foldingRange(DocumentUri uri)
{
    FoldingRangeParams params;
    params.textDocument.uri = uri;
    return SendRequest("textDocument/foldingRange", params);
}
RequestID LSPClient::selectionRange(DocumentUri uri, std::vector<Position> &positions)
{
    SelectionRangeParams params;
    params.textDocument.uri = uri;
    params.positions = std::move(positions);
    return SendRequest("textDocument/selectionRange", params);
}
RequestID LSPClient::onTypeFormatting(DocumentUri uri, Position position, string_ref ch)
{
    DocumentOnTypeFormattingParams params;
    params.textDocument.uri = uri;
    params.position = position;
    params.ch = ch;
    return SendRequest("textDocument/onTypeFormatting", params);
}
RequestID LSPClient::formatting(DocumentUri uri)
{
    DocumentFormattingParams params;
    params.textDocument.uri = uri;
    return SendRequest("textDocument/formatting", params);
}
RequestID LSPClient::codeAction(DocumentUri uri, Range range, CodeActionContext context)
{
    CodeActionParams params;
    params.textDocument.uri = uri;
    params.range = range;
    params.context = std::move(context);
    return SendRequest("textDocument/codeAction", std::move(params));
}
RequestID LSPClient::completion(DocumentUri uri, Position position, option<CompletionContext> context)
{
    CompletionParams params;
    params.textDocument.uri = uri;
    params.position = position;
    params.context = context;
    return SendRequest("textDocument/completion", params);
}
RequestID LSPClient::signatureHelp(DocumentUri uri, Position position)
{
    TextDocumentPositionParams params;
    params.textDocument.uri = uri;
    params.position = position;
    return SendRequest("textDocument/signatureHelp", params);
}
RequestID LSPClient::gotoDefinition(DocumentUri uri, Position position)
{
    TextDocumentPositionParams params;
    params.textDocument.uri = uri;
    params.position = position;
    return SendRequest("textDocument/definition", params);
}
RequestID LSPClient::gotoDeclaration(DocumentUri uri, Position position)
{
    TextDocumentPositionParams params;
    params.textDocument.uri = uri;
    params.position = position;
    return SendRequest("textDocument/declaration", params);
}
RequestID LSPClient::references(DocumentUri uri, Position position)
{
    ReferenceParams params;
    params.textDocument.uri = uri;
    params.position = position;
    return SendRequest("textDocument/references", params);
}
RequestID LSPClient::switchSourceHeader(DocumentUri uri)
{
    TextDocumentIdentifier params;
    params.uri = uri;
    return SendRequest("textDocument/references", params);
}
RequestID LSPClient::rename(DocumentUri uri, Position position, string_ref newName)
{
    RenameParams params;
    params.textDocument.uri = uri;
    params.position = position;
    params.newName = newName;
    return SendRequest("textDocument/rename", std::move(params));
}
RequestID LSPClient::hover(DocumentUri uri, Position position)
{
    TextDocumentPositionParams params;
    params.textDocument.uri = uri;
    params.position = position;
    return SendRequest("textDocument/hover", params);
}
RequestID LSPClient::documentSymbol(DocumentUri uri)
{
    DocumentSymbolParams params;
    params.textDocument.uri = uri;
    return SendRequest("textDocument/documentSymbol", params);
}
RequestID LSPClient::documentColor(DocumentUri uri)
{
    DocumentSymbolParams params;
    params.textDocument.uri = uri;
    return SendRequest("textDocument/documentColor", params);
}
RequestID LSPClient::documentHighlight(DocumentUri uri, Position position)
{
    TextDocumentPositionParams params;
    params.textDocument.uri = uri;
    params.position = position;
    return SendRequest("textDocument/documentHighlight", params);
}
RequestID LSPClient::symbolInfo(DocumentUri uri, Position position)
{
    TextDocumentPositionParams params;
    params.textDocument.uri = uri;
    params.position = position;
    return SendRequest("textDocument/symbolInfo", params);
}
RequestID LSPClient::typeHierarchy(DocumentUri uri, Position position, TypeHierarchyDirection direction, int resolve)
{
    TypeHierarchyParams params;
    params.textDocument.uri = uri;
    params.position = position;
    params.direction = direction;
    params.resolve = resolve;
    return SendRequest("textDocument/typeHierarchy", params);
}
RequestID LSPClient::workspaceSymbol(string_ref query)
{
    WorkspaceSymbolParams params;
    params.query = query;
    return SendRequest("workspace/symbol", params);
}
RequestID LSPClient::executeCommand(string_ref cmd, option<TweakArgs> tweakArgs, option<WorkspaceEdit> workspaceEdit)
{
    ExecuteCommandParams params;
    params.tweakArgs = std::move(tweakArgs);
    params.workspaceEdit = std::move(workspaceEdit);
    params.command = cmd;
    return SendRequest("workspace/executeCommand", std::move(params));
}
RequestID LSPClient::didChangeWatchedFiles(std::vector<FileEvent> &changes)
{
    DidChangeWatchedFilesParams params;
    params.changes = std::move(changes);
    return SendRequest("workspace/didChangeWatchedFiles", std::move(params));
}
RequestID LSPClient::didChangeConfiguration(ConfigurationSettings &settings)
{
    DidChangeConfigurationParams params;
    params.settings = std::move(settings);
    return SendRequest("workspace/didChangeConfiguration", std::move(params));
}

// general send and notify
void LSPClient::sendNotification(string_ref method, QJsonDocument &jsonDoc)
{
    json doc = toNlohmann(jsonDoc);
    notify(method, doc);
}

RequestID LSPClient::sendRequest(string_ref method, QJsonDocument &jsonDoc)
{
    std::string id = method.str();
    json doc = toNlohmann(jsonDoc);
    request(method, doc, id);
    return id;
}

// general send and notify
void LSPClient::SendNotification(string_ref method, json jsonDoc)
{
    notify(method, std::move(jsonDoc));
}

RequestID LSPClient::SendRequest(string_ref method, json jsonDoc)
{
    std::string id = method.str();
    request(method, std::move(jsonDoc), id);
    return id;
}

// private

void LSPClient::writeToServer(std::string &content)
{
    if (clientProcess == nullptr || clientProcess->state() != QProcess::Running)
        return;
    std::string withHeader = "Content-Length: " + std::to_string(content.length()) + "\r\n";
    clientProcess->write(withHeader.c_str());
    clientProcess->write("\r\n");
    clientProcess->write(content.c_str());
}

json LSPClient::toNlohmann(QJsonDocument &doc)
{
    return json{doc.toJson().toStdString()};
}

QJsonDocument LSPClient::toJSONDoc(json &nlohman)
{
    QString rawStr = QString::fromStdString(nlohman.dump());
    return QJsonDocument::fromJson(rawStr.toLocal8Bit());
}

void LSPClient::notify(string_ref method, json value)
{
    json payload = {{"jsonrpc", "2.0"}, {"method", method}, {"params", value}};
    std::string content = payload.dump();
    writeToServer(content);
}

void LSPClient::request(string_ref method, json param, RequestID id)
{
    json rpc = {{"jsonrpc", "2.0"}, {"id", id}, {"method", method}, {"params", param}};
    std::string content = rpc.dump();
    writeToServer(content);
}

LSPClient::~LSPClient()
{
    {
        if (clientProcess != nullptr)
        {
            clientProcess->kill();
            delete clientProcess;
        }
    }
}
