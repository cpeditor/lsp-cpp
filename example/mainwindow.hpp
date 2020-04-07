#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <LSPClient.hpp>
#include <QPlainTextEdit>
#include <QTemporaryFile>
#include <QTimer>
class Mainwindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit Mainwindow(QWidget *parent = nullptr);
    ~Mainwindow() override;

public slots:
    void OnNotify(QString method, QJsonObject param);
    void OnResponse(QJsonObject id, QJsonObject response);
    void OnRequest(QString method, QJsonObject param, QJsonObject id);
    void OnError(QJsonObject id, QJsonObject error);
    void OnServerError(QProcess::ProcessError error);
    void OnServerFinished(int exitCode, QProcess::ExitStatus status);

    void textChanged();
    void requestDiagonistics();


private:
    QPlainTextEdit *code;
    QPlainTextEdit *info;
    LSPClient *lsp;

    QTemporaryFile file;
    QTimer timer;

    void setConnections();
    QString jsonObjectToString(QJsonObject&);
};

#endif // MAINWINDOW_HPP
