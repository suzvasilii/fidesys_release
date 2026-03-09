#include <iostream>
#include <QFile>
#include <QXmlStreamReader>
#include <vtkSmartPointer.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <QFileInfo>
#include <QDir>
#include <QFileInfoList>
#include "parser.h"

Parser::Parser() {
    setlocale(LC_ALL, "ru_RU.UTF-8");
}

Parser::~Parser() {
    delete parseResult;
    parseResult = nullptr;
}

int Parser::parse(const std::string& filepath)
{
    parseResult = new ParseResults();
    try{
        ifstream file(filepath);
        json j;
        file >> j;
        if (j.contains("restraints"))
        {
            for (const auto& load_json : j["restraints"])
            {
                FCRestraint restraint;
                restraint.id = load_json["id"];
                restraint.name = load_json["name"];
                restraint.cs = load_json.value("cs", 0);
                std::string apply_to_str = load_json["apply_to"];
                restraint.apply_to = decode_vector<int32>(apply_to_str);
                if (load_json.contains("data")) {
                    for (auto& encoded_d : load_json["data"])
                    {
                        if (encoded_d == "" || encoded_d.is_null())
                        {
                            restraint.data.push_back(0.0);
                        }
                        else
                        {
                            std::vector<float64> decoded = decode_vector<float64>(encoded_d);
                            restraint.data.insert(restraint.data.end(), decoded.begin(), decoded.end());
                        }

                    }
                }
                parseResult->restraints_map.insert(restraint.id, restraint);
            }
        }

        if (j.contains("loads")) {
            for (auto& load_json : j["loads"]) {
                FCLoad load;
                load.id = load_json["id"];
                load.name = load_json["name"];
                load.type = load_json["type"];
                load.cs = load_json.value("cs", 0);
                std::string apply_to_str = load_json["apply_to"];
                load.apply_to = decode_vector<int32>(apply_to_str);

                load.apply_dim = 0;

                if (load_json.contains("data") && load_json.contains("dependency_type")) {
                    for (auto& dep : load_json["dependency_type"]) {
                        if (dep.is_number()) {
                            load.dependency_type.push_back(dep);
                        }
                        else {
                            load.dependency_type.push_back(0);
                        }
                    }

                    for (auto& encoded_d : load_json["data"]) {
                        if (encoded_d == "" || encoded_d.is_null()) {
                            load.data.push_back(0.0);
                        }
                        else {
                            std::vector<float64> decoded = decode_vector<float64>(encoded_d);
                            load.data.insert(load.data.end(), decoded.begin(), decoded.end());
                        }
                    }
                }
                if (load.type == 3)
                {
                    parseResult->pressures_map.insert(load.id, load);
                }
                else if (load.type == 5)
                {
                    parseResult->forces_map.insert(load.id, load);
                }
                else
                {
                    parseResult->forcesRasp_map.insert(load.id, load);
                }
            }
        }
        emit parsingSuccess(parseResult);
        return 0;
    }
    catch (const _exception& e) {
        delete parseResult;
        parseResult = nullptr;
        emit parsingError();
        return 1;
    }
}


bool Parser::saveAllChanges(const QString& input_file,
    ParseResults& presult)
{
    try {
        std::ifstream in(input_file.toStdString());
        json j;
        in >> j;
        in.close();
        if (j.contains("loads")) {
            for (auto& load_json : j["loads"]) {
                int id = load_json["id"];
                int type = load_json["type"];
                if (type == 3 && presult.pressures_map.contains(id)) {
                    load_json["data"] = encode_vector_elements(presult.pressures_map[id].data);
                }
                else if (type == 5 && presult.forces_map.contains(id)) {
                    load_json["data"] = encode_vector_elements(presult.forces_map[id].data);
                }
                else if (type == 35 && presult.forcesRasp_map.contains(id)) {
                    load_json["data"] = encode_vector_elements(presult.forcesRasp_map[id].data);
                }
            }
        }
        if (j.contains("restraints")) {
            for (auto& rest_json : j["restraints"]) {
                int id = rest_json["id"];
                if (presult.restraints_map.contains(id)) {
                    rest_json["data"] = encode_vector_elements(presult.restraints_map[id].data);
                }
            }
        }
        std::ofstream out(input_file.toStdString());
        out << j.dump(4);
        out.close();
        emit saveSuccess();
        return true;

    }
    catch (...) {
        emit saveError();
        return false;
    }
}


double Parser::parseVTU(const QString& filename) {
    try {
        qDebug() << "Пытаюсь открыть файл:" << filename;
        QFileInfo checkFile(filename);
        if (!checkFile.exists()) {
            qDebug() << "Файл не существует!";
            return 0.0;
        }
        qDebug() << "Размер файла:" << checkFile.size() << "байт";
        vtkSmartPointer<vtkXMLUnstructuredGridReader> reader =
            vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();

        reader->SetFileName(filename.toStdString().c_str());
        reader->Update();

        vtkUnstructuredGrid* grid = reader->GetOutput();
        vtkPointData* pointData = grid->GetPointData();
        vtkDoubleArray* stressArray =
            vtkDoubleArray::SafeDownCast(pointData->GetArray("Stress"));

        if (!stressArray) {
            qDebug() << "Массив Stress не найден!";
            qDebug() << "Доступные массивы:";
            for (int i = 0; i < pointData->GetNumberOfArrays(); ++i) {
                qDebug() << "  " << pointData->GetArrayName(i);
            }
        }

        int misesIndex = 6;
        double maxMises = -1e300;

        for (vtkIdType i = 0; i < stressArray->GetNumberOfTuples(); ++i) {
            double* values = stressArray->GetTuple(i);
            double mises = values[misesIndex];

            if (i < 20) {
                qDebug() << "Точка" << i << ":" << mises;
            }

            if (mises > maxMises) {
                maxMises = mises;
            }
        }

        qDebug() << "\nМаксимальное значение Mises:" << maxMises;
        return maxMises;
    }
    catch (...)
    {
        qDebug() << "Ошибка в библиотеке vtk";
        return 0.0;
    }
}

QString Parser::getLatestVTU(const QString& pvdFile)
{
    QFileInfo pvdInfo(pvdFile);
    QString baseName = pvdInfo.completeBaseName();
    QString resultFolder = pvdInfo.absolutePath() + "/" + baseName;
    qDebug() << "Ищем VTU в папке:" << resultFolder;
    QDir dir(resultFolder);
    QStringList filters;
    filters << "*.vtu";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time);
    if (files.isEmpty()) return QString();
    return files.first().absoluteFilePath();
}




