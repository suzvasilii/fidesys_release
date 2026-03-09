#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QProcess>
#include <QTimer>
#include <unordered_map>
#include <QLineEdit>
#include <QThread>
#include <QObject>
#include <QVBoxLayout>
#include "parser.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    //слоты для ui
    void on_btnChoice_clicked();
    void on_btnOk_clicked();
    void on_btnSaveParams_clicked();
    void on_btnCalc_clicked();
    void onComboBoxTextChanged(int index);
    void onIdPressureChanged();
    void onIdForceChanged();
    void onIdForceRaspChanged();
    void onIdRemoveChanged();
    void onFilenameChanged();
    //Слоты для парсера
    void onParsingError();
    void onParsingSuccess(ParseResults* pr);
    void onSaveSuccess();
    void onSaveError();
    // Слоты для QProcess
    void readOutput();
    void readError();
    void onCalcFinished(int exitCode, QProcess::ExitStatus status);
    void onCalcError(QProcess::ProcessError error);

private:
    Ui::MainWindow* ui;
    Parser* m_parser;
    QProcess* m_calcProcess = nullptr;
    QTimer* m_timeoutTimer = nullptr;
    void notFound(const QString& gu);
    QString current_file ="";
    QString res_file="";
    ParseResults presult;

    // Вспомогательные функции
    void loadId();
    void runExternalProgram(QString& outPutFilePath);

    // Функции загрузки страниц
    void loadPressurePage();
    void loadForcePage();
    void loadForceRaspPage();
    void loadRemovesPage();
    void loadPressureValues(int id);
    void loadForceRaspValues(int id);
    void loadForceValues(int id);
    void loadRemoveValues(int id);
    void savePressureValues(int id);
    void saveForceValues(int id);
    void saveForceRaspValues(int id);
    void saveRemoveValues(int id);

    template<typename T>
    void loadValuesFromMap(int id,
        const QHash<int, T>& map,
        QLineEdit* nameEdit,
        std::initializer_list<QLineEdit*> valueEdits);

    // Также можно добавить для изменения значений (опционально)
    template<typename T>
    void saveValuesToMap(int id,
        QHash<int, T>& map,
        std::initializer_list<QLineEdit*> valueEdits);

    // Карта соответствия текста и типа нагрузки
    std::unordered_map<QString, int> gus = {
        {"давления", 3},
        {"точечные силы", 5},
        {"распределенные нагрузки", 35},
        {"перемещения", 0}
    };
};

#endif // MAINWINDOW_H
