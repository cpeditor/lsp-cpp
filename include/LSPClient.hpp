#ifndef LSPCLIENT_HPP
#define LSPCLIENT_HPP

#include "LSP.hpp"
#include "LSPUri.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QProcess>

class LSPClient : public QObject
{
    Q_OBJECT

    using RequestID = std::string;

  public:
    explicit LSPClient(QString processPath, QStringList args);

    LSPClient(LSPClient &&) = delete;
    LSPClient(LSPClient &) = delete;

    LSPClient &operator=(LSPClient &&) = delete;
    LSPClient &operator=(LSPClient &) = delete;

    ~LSPClient() final;

    // LSP methods Requests to send to server
    RequestID initialize(option<DocumentUri> rootUri = {});
    RequestID shutdown();
    RequestID sync();
    RequestID registerCapability();

    RequestID rangeFomatting(DocumentUri uri, Range range);
    RequestID foldingRange(DocumentUri uri);
    RequestID selectionRange(DocumentUri uri, std::vector<Position> &positions);
    RequestID onTypeFormatting(DocumentUri uri, Position position, string_ref ch);

    RequestID formatting(DocumentUri uri);
    RequestID codeAction(DocumentUri uri, Range range, CodeActionContext context);
    RequestID completion(DocumentUri uri, Position position, option<CompletionContext> context = {});
    RequestID signatureHelp(DocumentUri uri, Position position);
    RequestID gotoDefinition(DocumentUri uri, Position position);
    RequestID gotoDeclaration(DocumentUri uri, Position position);
    RequestID references(DocumentUri uri, Position position);
    RequestID switchSourceHeader(DocumentUri uri);
    RequestID rename(DocumentUri uri, Position position, string_ref newName);
    RequestID hover(DocumentUri uri, Position position);
    RequestID documentSymbol(DocumentUri uri);
    RequestID documentColor(DocumentUri uri);
    RequestID documentHighlight(DocumentUri uri, Position position);
    RequestID symbolInfo(DocumentUri uri, Position position);
    RequestID typeHierarchy(DocumentUri uri, Position position, TypeHierarchyDirection direction, int resolve);
    RequestID workspaceSymbol(string_ref query);
    RequestID executeCommand(string_ref cmd, option<TweakArgs> tweakArgs = {},
                             option<WorkspaceEdit> workspaceEdit = {});

    RequestID didChangeWatchedFiles(std::vector<FileEvent> &changes);
    RequestID didChangeConfiguration(ConfigurationSettings &settings);

    // LSP methods notifications to Send to server
    void exit();
    void initialized();
    void didOpen(DocumentUri uri, string_ref code, string_ref lang);
    void didClose(DocumentUri uri);
    void didChange(DocumentUri uri, std::vector<TextDocumentContentChangeEvent> &changes,
                   option<bool> wantDiagnostics = {});

    // General sender and requester for sever
    void sendNotification(string_ref method, QJsonDocument &jsonDoc);
    RequestID sendRequest(string_ref method, QJsonDocument &jsonDoc);

  signals:
    void onNotify(QString method, QJsonObject param);
    void onResponse(QJsonObject id, QJsonObject response);
    void onRequest(QString method, QJsonObject param, QJsonObject id);
    void onError(QJsonObject id, QJsonObject error);
    void onServerError(QProcess::ProcessError error);
    void onServerFinished(int exitCode, QProcess::ExitStatus status);
    void newStderr(const QString &content);

  private slots:
    void onClientReadyReadStdout();
    void onClientReadyReadStderr();
    void onClientError(QProcess::ProcessError error);
    void onClientFinished(int exitCode, QProcess::ExitStatus status);

  private:
    QProcess *clientProcess = nullptr;
    std::vector<std::string> writeToServerBuffer;
    bool hasInitialized = false;

    void writeToServer(std::string &in);

    QJsonDocument toJSONDoc(json &nlohman);
    json toNlohmann(QJsonDocument &doc);

    void notify(string_ref method, json value);
    void request(string_ref mthod, json param, RequestID id);

    void SendNotification(string_ref method, json jsonDoc);
    RequestID SendRequest(string_ref method, json jsonDoc);
};

#endif
