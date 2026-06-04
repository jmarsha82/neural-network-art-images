#pragma once

#include "artnet/ImageRecord.h"

#include <QMainWindow>

#include <filesystem>
#include <vector>

class QListWidget;
class QLabel;
class QPushButton;
class QTableWidget;

namespace artnet {

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private:
    void buildUi();
    void scanLibrary();
    void runInference();
    void updateSelection(int row);
    void showSimilar();
    void exportCsv();
    void exportJson();

    std::filesystem::path libraryRoot_;
    std::vector<ImageRecord> records_;

    QListWidget* imageList_ = nullptr;
    QLabel* preview_ = nullptr;
    QTableWidget* predictions_ = nullptr;
    QTableWidget* similar_ = nullptr;
    QLabel* status_ = nullptr;
    QPushButton* inferButton_ = nullptr;
};

} // namespace artnet
