#include "artnet/MainWindow.h"

#include "artnet/ArtScanner.h"
#include "artnet/Clusterer.h"
#include "artnet/EmbeddingIndex.h"
#include "artnet/Export.h"
#include "artnet/OpenCvDnnModel.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolBar>

namespace artnet {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      libraryRoot_("C:/Users/jmars/OneDrive/Pictures/Art Images") {
    buildUi();
    scanLibrary();
}

void MainWindow::buildUi() {
    setWindowTitle("Neural Network Art Images");
    resize(1280, 820);

    auto* toolbar = addToolBar("Main");
    auto* scanAction = toolbar->addAction("Scan");
    auto* inferAction = toolbar->addAction("Run Network");
    auto* similarAction = toolbar->addAction("Similar");
    auto* csvAction = toolbar->addAction("Export CSV");
    auto* jsonAction = toolbar->addAction("Export JSON");

    auto* splitter = new QSplitter(this);
    imageList_ = new QListWidget(splitter);
    preview_ = new QLabel(splitter);
    predictions_ = new QTableWidget(splitter);
    similar_ = new QTableWidget(splitter);

    preview_->setAlignment(Qt::AlignCenter);
    preview_->setMinimumWidth(420);
    preview_->setStyleSheet("background:#111318;color:#f5f0e8;border:1px solid #30323a;");
    predictions_->setColumnCount(2);
    predictions_->setHorizontalHeaderLabels({"Prediction", "Confidence"});
    predictions_->horizontalHeader()->setStretchLastSection(true);
    similar_->setColumnCount(2);
    similar_->setHorizontalHeaderLabels({"Similar Image", "Score"});
    similar_->horizontalHeader()->setStretchLastSection(true);

    splitter->addWidget(imageList_);
    splitter->addWidget(preview_);
    splitter->addWidget(predictions_);
    splitter->addWidget(similar_);
    splitter->setStretchFactor(1, 2);
    setCentralWidget(splitter);

    status_ = new QLabel(this);
    statusBar()->addWidget(status_, 1);

    setStyleSheet(R"(
        QMainWindow { background: #181a20; color: #f4f0e8; }
        QToolBar { background: #20232b; border: 0; spacing: 8px; padding: 8px; }
        QToolButton, QPushButton {
            background: #2e6f95; color: white; border: 0; padding: 8px 12px; border-radius: 4px;
        }
        QToolButton:hover, QPushButton:hover { background: #3686b5; }
        QListWidget, QTableWidget {
            background: #f8f5ef; color: #1e2026; border: 0; alternate-background-color: #ece7dd;
        }
        QHeaderView::section { background: #d7cfc0; color: #1e2026; padding: 6px; border: 0; }
        QStatusBar { background: #20232b; color: #f4f0e8; }
    )");

    connect(scanAction, &QAction::triggered, this, &MainWindow::scanLibrary);
    connect(inferAction, &QAction::triggered, this, &MainWindow::runInference);
    connect(similarAction, &QAction::triggered, this, &MainWindow::showSimilar);
    connect(csvAction, &QAction::triggered, this, &MainWindow::exportCsv);
    connect(jsonAction, &QAction::triggered, this, &MainWindow::exportJson);
    connect(imageList_, &QListWidget::currentRowChanged, this, &MainWindow::updateSelection);
}

void MainWindow::scanLibrary() {
    records_ = ArtScanner(libraryRoot_).scan();
    imageList_->clear();
    for (const auto& record : records_) {
        imageList_->addItem(QString::fromStdString(record.path.filename().string()));
    }
    status_->setText(QString("Scanned %1 images from %2")
                         .arg(records_.size())
                         .arg(QString::fromStdString(libraryRoot_.string())));
}

void MainWindow::runInference() {
    const auto modelPath = QFileDialog::getOpenFileName(this, "Choose ONNX Model", "models", "ONNX Models (*.onnx)");
    if (modelPath.isEmpty()) {
        return;
    }

    const auto labelsPath = QFileDialog::getOpenFileName(this, "Choose Labels", "models", "Text Files (*.txt)");
    if (labelsPath.isEmpty()) {
        return;
    }

    try {
        OpenCvDnnModel model(modelPath.toStdString(), labelsPath.toStdString());
        for (auto& record : records_) {
            record.predictions = model.classify(record.path, 5);
            record.embedding = model.embedding(record.path);
        }
        Clusterer(8).assignClusters(records_);
        status_->setText("Inference complete");
        updateSelection(imageList_->currentRow());
    } catch (const std::exception& error) {
        QMessageBox::critical(this, "Model Error", error.what());
    }
}

void MainWindow::updateSelection(int row) {
    predictions_->setRowCount(0);
    if (row < 0 || static_cast<std::size_t>(row) >= records_.size()) {
        preview_->setText("Select an image");
        return;
    }

    const auto& record = records_[static_cast<std::size_t>(row)];
    QPixmap pixmap(QString::fromStdString(record.path.string()));
    preview_->setPixmap(pixmap.scaled(preview_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    predictions_->setRowCount(static_cast<int>(record.predictions.size()));
    for (int i = 0; i < static_cast<int>(record.predictions.size()); ++i) {
        predictions_->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(record.predictions[i].label)));
        predictions_->setItem(i, 1, new QTableWidgetItem(QString::number(record.predictions[i].confidence, 'f', 4)));
    }
}

void MainWindow::showSimilar() {
    const auto row = imageList_->currentRow();
    similar_->setRowCount(0);
    if (row < 0) {
        return;
    }

    EmbeddingIndex index(records_);
    const auto results = index.findSimilar(static_cast<std::size_t>(row), 25);
    similar_->setRowCount(static_cast<int>(results.size()));
    for (int i = 0; i < static_cast<int>(results.size()); ++i) {
        const auto& result = results[static_cast<std::size_t>(i)];
        similar_->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(records_[result.recordIndex].path.filename().string())));
        similar_->setItem(i, 1, new QTableWidgetItem(QString::number(result.score, 'f', 4)));
    }
}

void MainWindow::exportCsv() {
    const auto path = QFileDialog::getSaveFileName(this, "Export CSV", "art-network.csv", "CSV (*.csv)");
    if (!path.isEmpty()) {
        writeCsv(path.toStdString(), records_);
    }
}

void MainWindow::exportJson() {
    const auto path = QFileDialog::getSaveFileName(this, "Export JSON", "art-network.json", "JSON (*.json)");
    if (!path.isEmpty()) {
        writeJson(path.toStdString(), records_);
    }
}

} // namespace artnet
