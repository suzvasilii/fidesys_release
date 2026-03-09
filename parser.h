#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QDebug>
#include <fstream>
#include <QObject>
#include <locale>
#include <clocale>
#include <string>
#include "parse_module.h"
#include "json.hpp"

using json = nlohmann::json;

class Parser : public QObject
{
    Q_OBJECT
    public:
        Parser();
        ~Parser();

        int parse(const std::string& filename);

        bool saveAllChanges(const QString& input_file,
            ParseResults& presult);

        QString getLatestVTU(const QString& pvdFile);

        double parseVTU(const QString& filename);
private:
    ParseResults* parseResult = nullptr;

signals:
    void parsingError();
    void parsingSuccess(ParseResults* pr);
    void saveError();
    void saveSuccess();
};

#endif // PARSER_H
