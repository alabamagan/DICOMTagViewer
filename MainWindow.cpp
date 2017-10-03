#include "MainWindow.h"


#include <QList>
#include <QDropEvent>
#include <QMimeData>
#include <QTreeView>
#include <QFileDialog>
#include <QStringListModel>
#include <QDialog>
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

using namespace std;


MainWindow::MainWindow(QObject *parent)
{
    this->m_ui = new Ui_MainWindow();
    this->m_ui->setupUi(this);

    connect(this->m_ui->actionLoad_Directories, SIGNAL(triggered()), SLOT(slotActionLoadDirs()));
    connect(this->m_ui->actionLoad_File, SIGNAL(triggered()), SLOT(slotActionLoadFile()));
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
}

void MainWindow::slotActionLoadFile() {

}

void MainWindow::LoadFiles(QStringList) {

}

void MainWindow::updateTreeWidget()
{
    /* Clean the treewidget */
    this->m_ui->treeWidgetFiles->clear();


    for (int i = 0; i < this->m_treebranchnames.size(); ++i) {
        QTreeWidgetItem* item = new QTreeWidgetItem(this->m_ui->treeWidgetFiles);
        item->setText(0, (this->m_treebranchnames[i].split('/').last()));
        for (int j = 0; j < this->m_loadedFiles[i].size(); ++j) {
            QTreeWidgetItem* node = new QTreeWidgetItem(item);
            node->setText(0, QString::fromStdString(this->m_loadedFiles[i][j]).split('/').last());
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
	s->SetRecursive(true);
	s->SetGlobalWarningDisplay(false);
    vector<string> fn = s->GetInputFileNames();
	if (fn.size() == 0)
	{
		return;
	}
    this->m_treebranchnames[this->m_treebranchnames.size()] = dir;

    /* Sort the file name */
    std::sort(fn.begin(), fn.end());
    this->m_loadedFiles[this->m_loadedFiles.size()] = fn;

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


    int i = this->m_ui->treeWidgetFiles->indexOfTopLevelItem(item->parent());
    if (i == -1)
        return;

    this->displayTags(QString::fromStdString(this->m_loadedFiles[i][item->parent()->indexOfChild(item)]));
}


void MainWindow::displayTags(QString filename)
{
    IoType::Pointer io = IoType::New();
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