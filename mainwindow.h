#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void digitClicked(int button);
signals:
    void buttonClicked();
    void selectView(int index);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
