#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <QObject>
#include <QString>
#include <QMainWindow>
#include <QStringList>
#include <QMap>
#include <ui_MainWindow.h>
#include <vector>
#include <string>

#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>

class QTreeWidgetItem;

namespace UI {
    class Ui_MainWindow;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QObject* parent = nullptr);
    ~MainWindow();

    typedef itk::GDCMImageIO IoType;
    typedef itk::GDCMSeriesFileNames SeriesGeneratorType;

public slots:
    void slotActionLoadDirs();
    void slotActionLoadFile();
    void slotTreeWidgetCurrentChanged();

protected:
    void LoadFolder(QString);
    void LoadFolder(QStringList);
    void LoadFolder(std::string);
    void LoadFiles(QStringList);
    void updateTreeWidget();
    void displayTags(QString filename);

    QMap<int, QString> m_treebranchnames;
    QMap<int, std::vector<std::string>> m_loadedFiles;
    Ui_MainWindow* m_ui;
};

#endif