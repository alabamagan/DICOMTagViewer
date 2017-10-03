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

    typedef itk::GDCMImageIO GDCMImageIO;
    typedef itk::GDCMSeriesFileNames SeriesGeneratorType;

public slots:
    void slotActionLoadDirs();
    void slotActionLoadFile();
    void slotTreeWidgetCurrentChanged();


protected:
	/// Interaction
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dropEvent(QDropEvent *event) override;


	/// IO
    void LoadFolder(QString);
    void LoadFolder(QStringList);
    void LoadFolder(std::string);
    void LoadFiles(QStringList);
    void updateTreeWidget();
    void displayTags(QString filename);
	void parseInputFileList(QString folder, std::vector<std::string> files);

    QMap<int, QString> m_folderNames;
	// folder ->series
	QMap<QString, QStringList> m_seriesNames;
	// folder_series -> files
	QMap<QString, QStringList> m_seriesFiles;

    QMap<int, std::vector<std::string>> m_loadedFiles;
	QMap<QTreeWidgetItem*, QString> m_itemToFile;
    Ui_MainWindow* m_ui;
};

#endif