#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "parser.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QLineEdit>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentIndex(0);
    ui->functional_widget->setVisible(false);
    m_parser = new Parser();
    qRegisterMetaType<ParseResults>("ParseResults");
    m_parser->moveToThread(QThread::currentThread());
    connect(m_parser, &Parser::parsingError,
        this, &MainWindow::onParsingError);
    connect(m_parser, &Parser::parsingSuccess,
        this, &MainWindow::onParsingSuccess);
    connect(m_parser, &Parser::saveSuccess,
        this, &MainWindow::onSaveSuccess);
    connect(m_parser, &Parser::saveError,
        this, &MainWindow::onSaveError);

    connect(ui->gu_choice, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onComboBoxTextChanged);
    connect(ui->pressureChoice, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onIdPressureChanged);
    connect(ui->forceChoice, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::onIdForceChanged);
    connect(ui->forceChoice2, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::onIdForceRaspChanged);
    connect(ui->removeChoice, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::onIdRemoveChanged);
    ui->btnOk->setEnabled(false);
    connect(ui->filenameInput, &QLineEdit::textChanged,
            this, &MainWindow::onFilenameChanged);

}

MainWindow::~MainWindow()
{

    if (m_timeoutTimer) {
        m_timeoutTimer->stop();
        delete m_timeoutTimer;
        m_timeoutTimer = nullptr;
    }

    if (m_calcProcess) {
        if (m_calcProcess->state() == QProcess::Running) {
            m_calcProcess->terminate();
            m_calcProcess->waitForFinished(3000);
        }
        delete m_calcProcess;
        m_calcProcess = nullptr;
    }
    delete m_parser;
    delete ui;
}

void MainWindow::on_btnChoice_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Выберите файл для загрузки",
        "",
        "FC files (*.fc);;All files (*.*)"
    );
    if (filePath != "")
    {
        ui->filenameInput->setText(filePath);
    }
    
}

void MainWindow::notFound(const QString& gu)
{
    ui->functional_widget->setVisible(false);
    QString str = "В препроцессоре не настроено " + gu;
    QMessageBox::warning(this, "Предупреждение", str);

}


void MainWindow::on_btnOk_clicked()
{
    ui->btnOk->setEnabled(false);
    m_parser->parse(ui->filenameInput->text().toStdString());
 }

void MainWindow::onParsingError()
{
    ui->functional_widget->setVisible(false);
    ui->gu_choice->setEnabled(false);
    QMessageBox::critical(this, "Ошибка парсинга", "Во время парсинга произошла ошибка, файл поврежден или пуст!");
}

void MainWindow::onParsingSuccess(ParseResults* pr)
{
    QMessageBox::information(this, "Успех!", "Данные прочитаны! Загрузка может занять какое-то время");
    presult = *pr;
    delete pr;
    pr = nullptr;
    this->loadId();
    QMessageBox::information(this, "Успех!", "Данные загружены");
    ui->gu_choice->setEnabled(true);
}

void MainWindow::on_btnSaveParams_clicked()
{
    QString gu = ui->gu_choice->currentText();

    if (gus[gu] == 3) {
        savePressureValues(ui->pressureChoice->currentText().toInt());
    }
    else if (gus[gu] == 5) {
        saveForceValues(ui->forceChoice->currentText().toInt());
    }
    else if (gus[gu] == 35) {
        saveForceRaspValues(ui->forceChoice2->currentText().toInt());
    }
    else {
        saveRemoveValues(ui->removeChoice->currentText().toInt());
    }
}

void MainWindow::on_btnCalc_clicked()
{
    ui->btnCalc->setEnabled(false);
    m_parser->saveAllChanges(current_file, presult);
}

void MainWindow::onSaveSuccess()
{
        QMessageBox::information(this, "Успех", "Изменения внесены в модель, далее необходимо выбрать файл для результатов рассчета");
        QString filePath = QFileDialog::getSaveFileName(
            this,
            "Выберите файл для сохранения результатов",
            "",
            "PVD files (*.pvd);;All files (*.*)"
        );
        if (!filePath.isEmpty())
        {
            this->res_file = filePath;
            this->runExternalProgram(filePath);
        }
        else ui->btnCalc->setEnabled(true);
    }

void MainWindow::onSaveError()
{
    QMessageBox::critical(this, "Ошибка", "Не удалось сохранить результаты в модель, повторите попытку.");
    ui->btnCalc->setEnabled(true);
}


void MainWindow::onFilenameChanged()
{
    if (ui->filenameInput->text() != "")
    {
        ui->btnOk->setEnabled(true);
        presult.pressures_map.clear();
        presult.forces_map.clear();
        presult.forcesRasp_map.clear();
        presult.restraints_map.clear();
        current_file = ui->filenameInput->text();
        ui->gu_choice->setCurrentIndex(0);
        ui->maxMises->setText("");
    }

}

void MainWindow::loadId()
{
    ui->pressureChoice->blockSignals(true);
    ui->forceChoice->blockSignals(true);
    ui->forceChoice2->blockSignals(true);
    ui->removeChoice->blockSignals(true);

    ui->pressureChoice->clear();
    ui->forceChoice->clear();
    ui->forceChoice2->clear();
    ui->removeChoice->clear();

    for (int id : presult.pressures_map.keys()) {
        ui->pressureChoice->addItem(QString::number(id));
    }

    for (int id : presult.forces_map.keys()) {
        ui->forceChoice->addItem(QString::number(id));
    }

    for (int id : presult.forcesRasp_map.keys()) {
        ui->forceChoice2->addItem(QString::number(id));
    }

    for (int id : presult.restraints_map.keys()) {
        ui->removeChoice->addItem(QString::number(id));
    }

    ui->pressureChoice->blockSignals(false);
    ui->forceChoice->blockSignals(false);
    ui->forceChoice2->blockSignals(false);
    ui->removeChoice->blockSignals(false);

}
template<typename T>
void MainWindow::loadValuesFromMap(int id,
    const QHash<int, T>& map,
    QLineEdit* nameEdit,
    std::initializer_list<QLineEdit*> valueEdits)
{
    if (map.contains(id)) {
        const T& cur = map[id];
        nameEdit->setText(QString::fromStdString(cur.name));
        int i = 0;
        for (QLineEdit* edit : valueEdits) {
            if (edit && i < cur.data.size()) {
                edit->setText(QString::number(cur.data[i]));
            }
            i++;
        }
    }

}
template<typename T>
void MainWindow::saveValuesToMap(int id, QHash<int, T>& map,
    std::initializer_list<QLineEdit*> valueEdits)
{
    if (!map.contains(id)) return;

    T& cur = map[id];
    bool hasError = false;

    int i = 0;
    for (QLineEdit* edit : valueEdits) {
        if (edit && i < cur.data.size()) {
            bool ok;
            float val = edit->text().toDouble(&ok);
            if (ok) {
                cur.data[i] = val;
            }
            else {
                hasError = true;
            }
        }
        i++;
    }

    if (hasError) {
        QMessageBox::critical(this, "Ошибка",
            "Некоторые поля содержат некорректные значения");
    }
    else
    {
        QMessageBox::information(this, "Успех", "Данные сохранены");
    }
}

void MainWindow::loadPressurePage()
{
    ui->stackedWidget->setCurrentIndex(0);
    QString default_id = ui->pressureChoice->currentText();
    this->loadPressureValues(default_id.toInt());

}
void MainWindow::loadForcePage()
{
    ui->stackedWidget->setCurrentIndex(1);
    QString default_id = ui->forceChoice->currentText();
    this->loadForceValues(default_id.toInt());
}

void MainWindow::loadForceRaspPage()
{
    ui->stackedWidget->setCurrentIndex(2);
    QString default_id = ui->forceChoice2->currentText();
    this->loadForceRaspValues(default_id.toInt());
}

void MainWindow::loadRemovesPage()
{
    ui->stackedWidget->setCurrentIndex(3);
    QString default_id = ui->removeChoice->currentText();
    this->loadRemoveValues(default_id.toInt());
}

void MainWindow::loadPressureValues(int id)
{
    loadValuesFromMap(id,
        presult.pressures_map,
        ui->pressureName,
        { ui->pressureValue });
}

void MainWindow::loadForceValues(int id)
{
    loadValuesFromMap(id,
        presult.forces_map,
        ui->forceName,
        { ui->forceX, ui->forceY, ui->forceZ,
         ui->momentX, ui->momentY, ui->momentZ });
}

void MainWindow::loadForceRaspValues(int id)
{
    loadValuesFromMap(id,
        presult.forcesRasp_map,
        ui->forceName2,
        { ui->forceX_2, ui->forceY_2, ui->forceZ_2,
         ui->momentX_2, ui->momentY_2, ui->momentZ_2 });
}

void MainWindow::loadRemoveValues(int id)
{
    loadValuesFromMap(id,
        presult.restraints_map,
        ui->removeName,
        { ui->removeX, ui->removeY, ui->removeZ,
         ui->aroundX, ui->aroundY, ui->aroundZ });
}

void MainWindow::savePressureValues(int id)
{
    saveValuesToMap(id,
        presult.pressures_map,
        { ui->pressureValue });

}

void MainWindow::saveForceValues(int id)
{
    saveValuesToMap(id,
        presult.forces_map,
        { ui->forceX, ui->forceY, ui->forceZ,
         ui->momentX, ui->momentY, ui->momentZ });
}

void MainWindow::saveForceRaspValues(int id)
{
    saveValuesToMap(id,
        presult.forcesRasp_map,
        { ui->forceX_2, ui->forceY_2, ui->forceZ_2,
         ui->momentX_2, ui->momentY_2, ui->momentZ_2 });
}

void MainWindow::saveRemoveValues(int id)
{
    saveValuesToMap(id,
        presult.restraints_map,
        { ui->removeX, ui->removeY, ui->removeZ,
         ui->aroundX, ui->aroundY, ui->aroundZ });

}

void MainWindow::onComboBoxTextChanged(int index)
{
    if (index == 0)
    {
        ui->functional_widget->setVisible(false);
        return;
    }
    ui->functional_widget->setVisible(true);
    QString gu = ui->gu_choice->currentText();


    if (gus[gu] == 3)
    {
        if (presult.pressures_map.empty())
        {
            this->notFound(gu);
            return;
        }
        this->loadPressurePage();
        return;
    }
    else if (gus[gu] == 5)
    {
        if (presult.forces_map.empty())
        {
            this->notFound(gu);
            return;
        }
        this->loadForcePage();
        return;
    }
    else if (gus[gu] == 35)
    {
        if (presult.forcesRasp_map.empty())
        {
            this->notFound(gu);
            return;
        }
        this->loadForceRaspPage();
        return;
    }
    else {
        if (presult.restraints_map.empty())
        {
            this->notFound(gu);
            return;
        }
       this->loadRemovesPage();
    }

}
void MainWindow::onIdPressureChanged()
{
    QString id = ui->pressureChoice->currentText();
    this->loadPressureValues(id.toInt());
}
void MainWindow::onIdForceChanged()
{
    QString id = ui->forceChoice->currentText();
    this->loadForceValues(id.toInt());
}
void MainWindow::onIdForceRaspChanged()
{
    QString id = ui->forceChoice2->currentText();
    this->loadForceRaspValues(id.toInt());
}
void MainWindow::onIdRemoveChanged()
{
    QString id = ui->removeChoice->currentText();
    this->loadRemoveValues(id.toInt());
}

void MainWindow::runExternalProgram(QString& outPutFilePath)
{
    QString programPath = "C:\\Program Files\\Fidesys\\CAE-Fidesys-8.1\\bin\\FidesysCalc.exe";
    QString inputFile = current_file;
    QString outputFile = outPutFilePath;

    if (m_calcProcess) {
        if (m_calcProcess->state() == QProcess::Running) {
            qDebug() << "Завершаем предыдущий расчет...";
            m_calcProcess->terminate();
            if (!m_calcProcess->waitForFinished(5000)) {
                m_calcProcess->kill();
            }
        }
        m_calcProcess->deleteLater();
        m_calcProcess = nullptr;
    }

    m_calcProcess = new QProcess(this);
    m_calcProcess->setWorkingDirectory(QFileInfo(programPath).path());

    connect(m_calcProcess, &QProcess::readyReadStandardOutput,
        this, &MainWindow::readOutput);
    connect(m_calcProcess, &QProcess::readyReadStandardError,
        this, &MainWindow::readError);
    connect(m_calcProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, &MainWindow::onCalcFinished);
    connect(m_calcProcess, &QProcess::errorOccurred,
        this, &MainWindow::onCalcError);

    m_calcProcess->start(programPath, QStringList()
        << "--pkey" << "1105506612537117203"
        << "-i" << inputFile
        << "-o" << outputFile);

    // Проверяем, запустился ли
    if (!m_calcProcess->waitForStarted(5000)) {
        QMessageBox::critical(this, "Ошибка",
            "Не удалось запустить расчет\n" + m_calcProcess->errorString());
        m_calcProcess->deleteLater();
        m_calcProcess = nullptr;
        return;
    }

    // Таймер на случай зависания
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, [this]() {
        if (m_calcProcess && m_calcProcess->state() == QProcess::Running) {
            qDebug() << "⚠️ Расчет завис, убиваем процесс";
            m_calcProcess->kill();

            if (!m_calcProcess->waitForFinished(3000)) {
                qDebug() << "❌ Процесс не отвечает на kill";
            }
            m_calcProcess->deleteLater();
            m_calcProcess = nullptr;

            if (m_timeoutTimer) {
                m_timeoutTimer->deleteLater();
                m_timeoutTimer = nullptr;
            }

            QMessageBox::warning(this, "Ошибка",
                "Расчет превысил время ожидания (60 секунд)");
        }
        });
    m_timeoutTimer->start(60000);

    qDebug() << "🚀 Процесс запущен, PID:" << m_calcProcess->processId();

}


void MainWindow::readOutput()
{
    if (m_calcProcess == nullptr) return;
    QByteArray data = m_calcProcess->readAllStandardOutput();
    QString output = QString::fromLocal8Bit(data);

    QStringList lines = output.split("\r\n", Qt::SkipEmptyParts);

    foreach(QString line, lines) {
        line = line.trimmed();

        if (line.startsWith("PROGRESS_INFO:")) {
            QString percentStr = line.mid(QString("PROGRESS_INFO:").length());
            bool ok;
            int percent = percentStr.toInt(&ok);

            if (ok) {
                ui->progressBar->setValue(percent);
                qDebug() << "Прогресс:" << percent << "%";
            }
        }
    }
}

void MainWindow::readError()
{
    if (m_calcProcess == nullptr) return;
    QByteArray data = m_calcProcess->readAllStandardError();
    qDebug() << "⚠️" << data.trimmed();
}

void MainWindow::onCalcFinished(int exitCode, QProcess::ExitStatus status)
{
    if (m_timeoutTimer != nullptr) {
        m_timeoutTimer->stop();
        m_timeoutTimer->deleteLater();
        m_timeoutTimer = nullptr;
    }

    if (status == QProcess::NormalExit && exitCode == 0) {
        QMessageBox::information(this, "Успех", "Расчет завершен!");
        ui->progressBar->setValue(0);
    }
    else {
        QMessageBox::warning(this, "Ошибка",
            QString("Рассчет завершился с ошибкой, перепроверьте корректность модели"));
    }

    qDebug() << "🏁 Процесс завершен";
    m_calcProcess->deleteLater();
    m_calcProcess = nullptr;


    QString latest = m_parser->getLatestVTU(this->res_file);
    double maxMises = m_parser->parseVTU(latest);
    qDebug()<<"maxMises " << maxMises;
    ui->maxMises->setText(QString::number(maxMises));
    ui->btnCalc->setEnabled(true);
}

void MainWindow::onCalcError(QProcess::ProcessError error)
{
    if (m_timeoutTimer!=nullptr) {
        m_timeoutTimer->stop();
        m_timeoutTimer->deleteLater();
        m_timeoutTimer = nullptr;
    }
    QString errorMsg;
    switch (error) {
    case QProcess::FailedToStart:
        errorMsg = "Не удалось запустить (нет прав или не найден exe)";
        break;
    case QProcess::Crashed:
        errorMsg = "Процесс упал";
        break;
    case QProcess::Timedout:
        errorMsg = "Таймаут";
        break;
    default:
        errorMsg = "Неизвестная ошибка";
    }

    QMessageBox::critical(this, "Ошибка расчета", errorMsg);

    if (m_calcProcess) {
        m_calcProcess->deleteLater();
        m_calcProcess = nullptr;
    }
    ui->btnCalc->setEnabled(true);
}
