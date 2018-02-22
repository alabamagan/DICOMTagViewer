#include "MainWindow.h"


#include <QList>
#include <QDropEvent>
#include <QMimeData>
#include <QTreeView>
#include <QFileDialog>
#include <QStringListModel>
#include <QMessageBox>
#include <QModelIndex>
#include <QTreeWidgetItem>
#include <QStandardItemModel>
#include <QDebug>
#include <algorithm>

#include <itkGDCMImageIO.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <ui_MainWindow.h>
#include <itkMetaDataObject.h>
#include <QVector>

using namespace std;


MainWindow::MainWindow(QObject *parent)
{
    this->m_ui = new Ui_MainWindow();
    this->m_ui->setupUi(this);

    connect(this->m_ui->actionLoad_Directories, SIGNAL(triggered()), SLOT(slotActionLoadDirs()));
    connect(this->m_ui->actionLoad_File, SIGNAL(triggered()), SLOT(slotActionLoadFile()));
	connect(this->m_ui->actionLoad_DICOM_Tag_Dictionary, SIGNAL(triggered()), SLOT(slotActionLoadDICOMTag()));
    connect(this->m_ui->treeWidgetFiles,
            SIGNAL(itemSelectionChanged()),
            SLOT(slotTreeWidgetCurrentChanged()),
            Qt::UniqueConnection);

	// Accept drag and drop
	this->setAcceptDrops(true);
	//this->m_ui->treeWidgetFiles->setAcceptDrops(true);

    this->m_ui->treeWidgetFiles->setHeaderHidden(true);
    this->m_ui->treeWidgetFiles->setSelectionMode(QAbstractItemView::SingleSelection);
    this->m_ui->tableViewDICOMTags->setStyleSheet("alternate-background-color: lightgray; ");
    this->m_ui->tableViewDICOMTags->setAlternatingRowColors(true);
    this->m_ui->tableViewDICOMTags->setShowGrid(false);
}

MainWindow::~MainWindow() {}

void MainWindow::slotActionLoadDirs()
{
    QFileDialog* dialog = new QFileDialog();
    dialog->setFileMode(QFileDialog::DirectoryOnly);
    int res = dialog->exec();

    if (res)
    {
        QString dir = dialog->selectedFiles()[0];

        this->LoadFolder(dir);
    }
	delete dialog;
}

void MainWindow::slotActionLoadFile() {

}


void MainWindow::slotActionLoadDICOMTag()
{
	QFileDialog dialog;
	QString l = dialog.getOpenFileName(this, tr("Load DICOM Tag"), this->windowFilePath(), "*.txt");
	if (QFileInfo(l).exists())
	{

	}
}

void MainWindow::LoadFiles(QStringList) {

}

void MainWindow::updateTreeWidget()
{
    /* Clean the treewidget */
    this->m_ui->treeWidgetFiles->clear();


    for (int i = 0; i < this->m_folderNames.size(); ++i) {
        QTreeWidgetItem* item = new QTreeWidgetItem(this->m_ui->treeWidgetFiles);
        item->setText(0, (this->m_folderNames[i].split('/').last()));

		QStringList series = this->m_seriesNames[this->m_folderNames[i]];
		for (QStringList::ConstIterator iter = series.cbegin(); iter != series.cend(); iter++)
		{
			QTreeWidgetItem* node = new QTreeWidgetItem(item);
			node->setText(0, (*iter).split('/').last());

			QStringList files = this->m_seriesFiles[this->m_folderNames[i] + "/" + *iter];
			for (QStringList::ConstIterator inner_iter = files.cbegin(); inner_iter != files.cend(); inner_iter++)
			{
				QTreeWidgetItem* file_node = new QTreeWidgetItem(node);
				file_node->setText(0, (*inner_iter).split('/').last());
				this->m_itemToFile[file_node] = *inner_iter;
			}
		}
        this->m_ui->treeWidgetFiles->addTopLevelItem(item);
    }


}


void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasUrls())
		event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
	const QMimeData* mimeData = event->mimeData();

	// check for our needed mime type, here a file or a list of files
	if (mimeData->hasUrls())
	{
		QStringList pathList;
		QList<QUrl> urlList = mimeData->urls();

		// extract the local paths of the files
		for (int i = 0; i < urlList.size(); i++)
		{
			pathList.append(urlList.at(i).toLocalFile());
		}

		// call a function to open the files
		this->LoadFolder(pathList);
	}
}

void MainWindow::LoadFolder(QString dir)
{
    /* Load all dicoms in the selected folder */
    SeriesGeneratorType::Pointer s = SeriesGeneratorType::New();
    s->SetInputDirectory(dir.toStdString().c_str());
    s->SetLoadSequences(true);
	s->SetRecursive(false);
	s->SetNumberOfThreads(4);
	//s->SetUseSeriesDetails(true);
	s->SetGlobalWarningDisplay(false);
	gdcm::SerieHelper* helper = s->GetSeriesHelper();


	vector<string> fn;
	vector<string> series = s->GetSeriesUIDs();
	for (vector<string>::const_iterator iter = series.cbegin(); iter != series.cend(); iter++)
	{
		vector<string> l_fn = s->GetFileNames(*iter);
		fn.insert(fn.end(), l_fn.begin(), l_fn.end());
	}

	if (fn.size() == 0)
	{
		QMessageBox* msg = new QMessageBox(
			QMessageBox::Critical,
			QString("DICOM Tag Viewer - Error"),
			QString("Cannot find any DICOM images in the specified directory!"),
			QMessageBox::Ok
			);
		msg->exec();
		return;
	}

    /* Sort the file name */
    std::sort(fn.begin(), fn.end());
    this->m_loadedFiles[this->m_loadedFiles.size()] = fn;

	this->parseInputFileList(dir, fn);

    /* Display the filenames to tree widget */
    this->updateTreeWidget();
}

void MainWindow::LoadFolder(QStringList folderlist) {
	for (QStringList::const_iterator iter = folderlist.cbegin(); iter != folderlist.cend(); iter++)
	{
		QString folder = *iter;
		this->LoadFolder(*iter);
	}
}

void MainWindow::LoadFolder(std::string dir) {
    this->LoadFolder(QString::fromStdString(dir));
}

void MainWindow::slotTreeWidgetCurrentChanged()
{
    QTreeWidgetItem* item = this->m_ui->treeWidgetFiles->selectedItems()[0];
	if (!item)
		return;

	try {
		QString s = this->m_itemToFile[item];

		this->displayTags(s);
	}
	catch (...)
	{
		return;
	}
}


void MainWindow::displayTags(QString filename)
{
	GDCMImageIO::Pointer io = GDCMImageIO::New();
    io->SetFileName(filename.toStdString());
	io->LoadPrivateTagsOn();
    io->ReadImageInformation();
    itk::MetaDataDictionary dict = io->GetMetaDataDictionary();

    int row = 0;
    QStandardItemModel* model = new QStandardItemModel(dict.GetKeys().size(), 2, this->m_ui->tableViewDICOMTags);
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Tag")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("Content")));
    for (itk::MetaDataDictionary::Iterator iter = dict.Begin(); iter != dict.End() ; iter++)
    {
        model->setItem(row, 0, new QStandardItem(QString::fromStdString(iter->first)));

        typedef itk::MetaDataObject<std::string> MetaDataStringType;

        MetaDataStringType::Pointer entryvalue =
                dynamic_cast<MetaDataStringType*> (iter->second.GetPointer());
        if (entryvalue)
        {
            model->setItem(row, 1, new QStandardItem(QString::fromStdString(entryvalue->GetMetaDataObjectValue())));
        }
        else
        {
            model->setItem(row, 1, new QStandardItem(QString("Not Readable")));
        }

        row += 1;
    }

    this->m_ui->tableViewDICOMTags->setModel(model);
    this->m_ui->tableViewDICOMTags->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    this->m_ui->tableViewDICOMTags->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    this->m_ui->tableViewDICOMTags->verticalHeader()->hide();
}

void MainWindow::parseInputFileList(QString folder, std::vector<string> files)
{
	try {
		/* Return if this folder is already parsed */
		for (int i = 0; i < this->m_folderNames.count(); i++)
		{
			if (this->m_folderNames[i] == folder)
				return;
		}
		this->m_folderNames[this->m_folderNames.size()] = folder;

		// for all files
		for (vector<string>::const_iterator iter = files.cbegin(); iter != files.cend(); iter++)
		{
			/* Read dictionary */
			GDCMImageIO::Pointer io = GDCMImageIO::New();
			io->SetFileName(*iter);
			io->LoadPrivateTagsOn();
			io->ReadImageInformation();
			itk::MetaDataDictionary dict = io->GetMetaDataDictionary();

			std::string patientname;
			std::string description;
			/* Read patient name and series name */
			io->GetValueFromTag("0010|0010", patientname);
			io->GetValueFromTag("0008|103e", description);
			QString key = QString::fromStdString(patientname + "-" + description);
			QString folderMapSeries = folder + QString::fromStdString( "/" ) + key;
			/* if new patient/series, create a new key */
			if (this->m_seriesNames.keys().indexOf(folder) == -1)
			{
				this->m_seriesNames[folder].push_back(key);
			}
			else
			{
				if (this->m_seriesNames[folder].indexOf(key) == -1)
				{
					this->m_seriesNames[folder].push_back(key);
				}
			}
			this->m_seriesFiles[folderMapSeries].push_back(QString::fromStdString(*iter));
		}
		/*qDebug() << this->m_folderNames;
		qDebug() << "============";
		qDebug() << this->m_seriesNames;
		qDebug() << "============";
		qDebug() << this->m_seriesFiles;
		qDebug() << "============";
*/
	}
	catch (itk::ExceptionObject& e) {

	}
}
