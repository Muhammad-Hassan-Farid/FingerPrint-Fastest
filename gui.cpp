#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QTabWidget>
#include <QProgressBar>
#include <QProcess>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <iostream>
#include <fstream>
#include <sstream>

// ============================================================================
// WORKER THREAD FOR RUNNING C++ EVALUATION APP
// ============================================================================

class EvaluationWorker : public QObject {
    Q_OBJECT

public:
    EvaluationWorker(const QString &appPath) : m_appPath(appPath) {}

public slots:
    void run() {
        QProcess process;
        process.setProcessChannelMode(QProcess::MergedChannels);
        
        // Connect to output signals
        connect(&process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, &process]() {
            emit finished(process.exitCode() == 0);
        });

        process.start(m_appPath);
        
        if (!process.waitForStarted()) {
            emit error("Failed to start evaluation app");
            emit finished(false);
            return;
        }

        // Read output in real-time
        while (process.state() == QProcess::Running) {
            if (process.canReadLine()) {
                QString line = QString::fromUtf8(process.readLine()).trimmed();
                if (!line.isEmpty()) {
                    emit outputReady(line);
                }
            }
            QThread::msleep(100);
        }

        // Read remaining output
        while (process.canReadLine()) {
            QString line = QString::fromUtf8(process.readLine()).trimmed();
            if (!line.isEmpty()) {
                emit outputReady(line);
            }
        }
    }

signals:
    void outputReady(const QString &text);
    void finished(bool success);
    void error(const QString &message);

private:
    QString m_appPath;
};

// ============================================================================
// MAIN GUI WINDOW
// ============================================================================

class BiometricEvaluationWindow : public QMainWindow {
    Q_OBJECT

public:
    BiometricEvaluationWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setupUI();
        setWindowTitle("Fingerprint Biometric Evaluation Framework - C++ GUI");
        setGeometry(100, 100, 1200, 800);
    }

private:
    void setupUI() {
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

        // ---- LEFT PANEL: Controls ----
        QWidget *leftPanel = new QWidget();
        QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);

        // Title
        QLabel *titleLabel = new QLabel("Evaluation Controls");
        QFont titleFont("Arial", 14);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        leftLayout->addWidget(titleLabel);

        leftLayout->addSpacing(10);

        // Run Button
        m_runButton = new QPushButton("▶ Run Evaluation");
        m_runButton->setMinimumHeight(40);
        QFont buttonFont("Arial", 11);
        m_runButton->setFont(buttonFont);
        m_runButton->setStyleSheet(
            "QPushButton {"
            "  background-color: #4CAF50;"
            "  color: white;"
            "  padding: 10px;"
            "  border-radius: 5px;"
            "  font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "  background-color: #45a049;"
            "}"
            "QPushButton:pressed {"
            "  background-color: #3d8b40;"
            "}"
            "QPushButton:disabled {"
            "  background-color: #cccccc;"
            "}"
        );
        connect(m_runButton, &QPushButton::clicked, this, &BiometricEvaluationWindow::runEvaluation);
        leftLayout->addWidget(m_runButton);

        // Export Button
        m_exportButton = new QPushButton("💾 Export Results");
        m_exportButton->setMinimumHeight(40);
        m_exportButton->setFont(buttonFont);
        m_exportButton->setStyleSheet(
            "QPushButton {"
            "  background-color: #2196F3;"
            "  color: white;"
            "  padding: 10px;"
            "  border-radius: 5px;"
            "  font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "  background-color: #0b7dda;"
            "}"
        );
        connect(m_exportButton, &QPushButton::clicked, this, &BiometricEvaluationWindow::exportResults);
        leftLayout->addWidget(m_exportButton);

        leftLayout->addSpacing(10);

        // Status Label
        QLabel *statusLabelTitle = new QLabel("Status:");
        QFont boldFont("Arial", 10);
        boldFont.setBold(true);
        statusLabelTitle->setFont(boldFont);
        leftLayout->addWidget(statusLabelTitle);

        m_statusLabel = new QLabel("Ready");
        m_statusLabel->setFont(QFont("Monospace", 10));
        m_statusLabel->setStyleSheet("color: green;");
        leftLayout->addWidget(m_statusLabel);

        leftLayout->addSpacing(10);

        // Progress Bar
        m_progressBar = new QProgressBar();
        m_progressBar->setVisible(false);
        leftLayout->addWidget(m_progressBar);

        // Output Display
        QLabel *outputLabelTitle = new QLabel("Output:");
        outputLabelTitle->setFont(boldFont);
        leftLayout->addWidget(outputLabelTitle);

        m_outputText = new QTextEdit();
        m_outputText->setReadOnly(true);
        m_outputText->setFont(QFont("Monospace", 9));
        m_outputText->setMaximumHeight(400);
        leftLayout->addWidget(m_outputText);

        leftLayout->addStretch();

        mainLayout->addWidget(leftPanel, 1);

        // ---- RIGHT PANEL: Results Tabs ----
        QTabWidget *tabWidget = new QTabWidget();

        // Tab 1: Metrics
        m_metricsText = new QTextEdit();
        m_metricsText->setReadOnly(true);
        m_metricsText->setFont(QFont("Monospace", 10));
        tabWidget->addTab(m_metricsText, "📋 Metrics");

        // Tab 2: About
        QTextEdit *aboutText = new QTextEdit();
        aboutText->setReadOnly(true);
        aboutText->setHtml(
            "<h2>Fingerprint Biometric Evaluation Framework</h2>"
            "<p><b>Version 1.0 - Pure C++ Implementation</b></p>"
            "<hr>"
            "<h3>Features:</h3>"
            "<ul>"
            "  <li>✅ Rank-1/5/10 Identification Accuracy</li>"
            "  <li>✅ Equal Error Rate (EER) Computation</li>"
            "  <li>✅ ROC Curve & AUC Analysis</li>"
            "  <li>✅ TAR @ FAR Operating Points</li>"
            "  <li>✅ Score Distribution Analysis</li>"
            "  <li>✅ CSV Export for External Analysis</li>"
            "  <li>✅ C++ GUI Interface (Qt5)</li>"
            "</ul>"
            "<h3>Quick Start:</h3>"
            "<ol>"
            "  <li>Click <b>'▶ Run Evaluation'</b> to start</li>"
            "  <li>View metrics in the <b>Metrics</b> tab</li>"
            "  <li>Export results with <b>'💾 Export Results'</b></li>"
            "</ol>"
            "<h3>Output Files:</h3>"
            "<ul>"
            "  <li>roc_curve.csv</li>"
            "  <li>score_distributions.csv</li>"
            "  <li>rank_distribution.csv</li>"
            "  <li>tar_at_far.csv</li>"
            "  <li>evaluation_metrics.txt</li>"
            "</ul>"
            "<p><b>No Python dependencies required!</b></p>"
            "<p>100% Pure C++ - Fast, efficient, cross-platform</p>"
        );
        tabWidget->addTab(aboutText, "ℹ️ About");

        mainLayout->addWidget(tabWidget, 1);
    }

private slots:
    void runEvaluation() {
        if (m_isRunning) {
            QMessageBox::warning(this, "In Progress", "Evaluation already running!");
            return;
        }

        m_outputText->clear();
        m_metricsText->clear();
        m_runButton->setEnabled(false);
        m_statusLabel->setText("⏳ Running...");
        m_statusLabel->setStyleSheet("color: orange;");
        m_progressBar->setVisible(true);
        m_isRunning = true;

        // Start evaluation in separate thread
        QThread *thread = new QThread(this);
        EvaluationWorker *worker = new EvaluationWorker("./fingerprint_app");

        worker->moveToThread(thread);

        connect(thread, &QThread::started, worker, &EvaluationWorker::run);
        connect(worker, &EvaluationWorker::outputReady, this, [this](const QString &text) {
            m_outputText->append(text);
            m_outputText->verticalScrollBar()->setValue(
                m_outputText->verticalScrollBar()->maximum());
        });
        connect(worker, &EvaluationWorker::finished, this, &BiometricEvaluationWindow::onEvaluationFinished);
        connect(worker, &EvaluationWorker::finished, thread, &QThread::quit);
        connect(thread, &QThread::finished, worker, &QObject::deleteLater);
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);

        thread->start();
    }

    void onEvaluationFinished(bool success) {
        m_runButton->setEnabled(true);
        m_progressBar->setVisible(false);
        m_isRunning = false;

        if (success) {
            m_statusLabel->setText("✅ Completed");
            m_statusLabel->setStyleSheet("color: green;");

            // Load metrics
            QFile metricsFile("evaluation_metrics.txt");
            if (metricsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QString content = QString::fromUtf8(metricsFile.readAll());
                m_metricsText->setText(content);
                metricsFile.close();
            }

            QMessageBox::information(this, "Success", "Evaluation completed successfully!");
        } else {
            m_statusLabel->setText("❌ Failed");
            m_statusLabel->setStyleSheet("color: red;");
            QMessageBox::critical(this, "Error", "Evaluation failed. Check output for details.");
        }
    }

    void exportResults() {
        QString folder = QFileDialog::getExistingDirectory(this, "Select Export Directory");
        if (folder.isEmpty()) {
            return;
        }

        QStringList filesToCopy = {
            "roc_curve.csv",
            "score_distributions.csv",
            "rank_distribution.csv",
            "tar_at_far.csv",
            "evaluation_metrics.txt"
        };

        int copied = 0;
        for (const QString &file : filesToCopy) {
            QFile sourceFile(file);
            if (sourceFile.exists()) {
                QString destination = folder + "/" + file;
                if (sourceFile.copy(destination)) {
                    copied++;
                } else {
                    QMessageBox::warning(this, "Error", "Failed to copy: " + file);
                }
            }
        }

        QString message = QString::number(copied) + " files exported to:\n" + folder;
        QMessageBox::information(this, "Export Complete", message);
    }

private:
    QPushButton *m_runButton;
    QPushButton *m_exportButton;
    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;
    QTextEdit *m_outputText;
    QTextEdit *m_metricsText;
    bool m_isRunning = false;
};

// ============================================================================
// MAIN APPLICATION ENTRY POINT
// ============================================================================

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    BiometricEvaluationWindow window;
    window.show();

    return app.exec();
}

#include "gui.moc"
